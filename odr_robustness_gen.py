import collect_grd_code_files
import build

def gen(ctx):
	root, files = collect_grd_code_files.collect()
	ok_files = []
	for f in files:
		try:
			res = build.build_file(root / f)
			ok_files.append(f)
		except Exception as e:
			print(f'Failed to build {f}: {e}')
	txt = map(lambda x: f'#include "../{x}"\n', ok_files)
	