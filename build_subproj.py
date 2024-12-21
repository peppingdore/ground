from pathlib import Path
import subprocess
import os
import sys

# def read_text(path):
# 	try: return path.read_text(errors='ignore')
# 	except: return ""

# def get_file_content_mtime(path: Path, ignore_root=False):
# 	mtime = 0 if ignore_root else path.stat().st_mtime
# 	if path.is_dir():
# 		for root, dirs, files in os.walk(path):
# 			for file in files:
# 				mtime = max(mtime, (Path(root) / file).stat().st_mtime)
# 			for dir in dirs:
# 				mtime = max(mtime, get_file_content_mtime(Path(root) / dir))
# 	return mtime

def get_binary_path_segments(build_type):
	if sys.platform == "win32":
		return build_type
	return ""

def pick_cmake_build_type(ctx):
	if ctx.params.compile_params.optimization_level == 0:
		return 'Debug'
	return 'RelWithDebInfo'
	# return 'Release'

def need_to_build(name):
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument("--subprojects", default="all")
	args, _ = parser.parse_known_args()
	return args.subprojects != "none" and (args.subprojects == "all" or (name in args.subprojects.split(',')))

def cmake(ctx, root, *, opts=None, build_type=None):
	opts = opts or []
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument("--subprojects", default="all")
	args, _ = parser.parse_known_args()
	root = Path(root).resolve()
	did_build = False
	name = root.name
	if not name:
		raise Exception("root path must be a normal directory")
	build = Path(root).parent / "grd_cmake_build" / name
	build_type = build_type or pick_cmake_build_type(ctx)
	if args.subprojects != "none" and (args.subprojects == "all" or (name in args.subprojects.split(','))):
		print(f"{Path(__file__).name}: building {root}\n{Path(__file__).name}: build type: {build_type}", file=ctx.stdout)
		os.makedirs(build, exist_ok=True)
		stdout = ctx.stdout if ctx.stdout.isatty() else subprocess.PIPE			
		cmake_0 = subprocess.run(["cmake", *opts, "-DCMAKE_CXX_STANDARD=23", f"-DCMAKE_BUILD_TYPE={build_type}", ".", f"-B{build.resolve()}"], cwd=root, stdout=stdout, stderr=subprocess.STDOUT)
		cmake_0.check_returncode()
		cmake_1 = subprocess.run(["cmake",  "--build", str(build.resolve()), f'--config {build_type}'], cwd=root, stdout=stdout, stderr=subprocess.STDOUT)
		cmake_1.check_returncode()
		if stdout == subprocess.PIPE:
			ctx.stdout.write(cmake_0.stdout.decode('utf-8', errors='ignore'))
			ctx.stdout.write(cmake_1.stdout.decode('utf-8', errors='ignore'))
		did_build = True
	from collections import namedtuple
	tp = namedtuple("BuildResult", ["build_path", "build_type", "did_build"])
	return tp(build_path=build.resolve(), build_type=build_type, did_build=did_build)
