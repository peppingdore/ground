import os
from pathlib import Path

def collect():
	blacklist = ['.git', '.vscode', 'third_party', 'b_lib', 'testing.h', 'misc']
	files = []
	root = Path(__file__).parent
	def gen(dir, level=1):
		for it in os.scandir(Path(__file__).parent / dir):
			if it.name in blacklist: continue
			if it.is_dir():
				gen(dir / it.name, level=level+1)
				continue
			if it.name.endswith(".h"):
				skiplist = (['posix_', 'darwin_'] if os.name == 'nt' else ['win_']) + ["_test"]
				if any (x in it.name for x in skiplist): continue
				files.append(dir / it.name)
	gen(root)
	# @TODO: remove.
	files = filter(lambda x: all(not skip in str(x) for skip in ["ssa.h", "grdc_parser.h", "ast_printer.h", "window.h", "grd_regexp.h"]) , files)
	files = list(map(lambda x: x.relative_to(root), files))
	return root.resolve(), files
