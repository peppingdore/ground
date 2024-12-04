from pathlib import Path
import subprocess
import os
import json
import time

def verbose(ctx, *args):
	if ctx.verbose:
		print(*args)

def read_text(path):
	try: return path.read_text(errors='ignore')
	except: return ""

def get_file_content_mtime(path: Path):
	mtime = path.stat().st_mtime
	if path.is_dir():
		for root, dirs, files in os.walk(path):
			for file in files:
				mtime = max(mtime, (Path(root) / file).stat().st_mtime)
	return mtime

def build_cmake(ctx, root, *, track_paths=[]):
	print(f"{Path(__file__).name}: building {root}")
	root = Path(root)
	name = root.name
	if not name:
		raise Exception("root path must be a normal directory")
	build = Path(root).parent / "grd_cmake_build" / name
	os.makedirs(build, exist_ok=True)
	# needed = json.dumps({ "target": ctx.params.compile_params.target, "mtime": root.stat().st_mtime })
	mtime_check_start = time.perf_counter()
	mtime = get_file_content_mtime(root)
	mtime = max([mtime, *map(get_file_content_mtime, track_paths)])
	needed = json.dumps({ "target": ctx.params.compile_params.target, "mtime": mtime})
	verbose(ctx, f"{root}: mtime_check_time: {time.perf_counter() - mtime_check_start:.2f}s")
	verbose(ctx, f"{root}: needed: {needed}")
	verbose(ctx, f"{root}: grd_build.json: {read_text(build / 'grd_build.json')}")
	if read_text(build / "grd_build.json") != needed:
		subprocess.check_call(["cmake", ".", f"-B{build.resolve()}"], cwd=root)
		subprocess.check_call(["cmake", "--build", str(build.resolve())], cwd=root)
		Path(build / "grd_build.json").write_text(needed)
