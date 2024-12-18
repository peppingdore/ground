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

def cmake(ctx, root, *, opts=None, build_type=None, callback=None):
	opts = opts or []
	def build_hook(ctx):
		nonlocal root
		nonlocal build_type
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
			print(f"{Path(__file__).name}: building {root}\n{Path(__file__).name}: build type: {build_type}")
			os.makedirs(build, exist_ok=True)
			subprocess.check_call(["cmake", *opts, "-DCMAKE_CXX_STANDARD=23", f"-DCMAKE_BUILD_TYPE={build_type}", ".", f"-B{build.resolve()}"], cwd=root)
			subprocess.check_call(["cmake",  "--build", str(build.resolve()), f'--config {build_type}'], cwd=root)
			did_build = True
		if callback:
			callback(ctx, build_path=build.resolve(), build_type=build_type, did_build=did_build)
	ctx.add_pre_compile_hook(build_hook)
