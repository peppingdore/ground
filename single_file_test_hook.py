#!/usr/bin/env python3
from pathlib import Path, PurePosixPath
import os
import shutil

CPP_CONTENT = '''
#pragma once
#include "{testing_header}"
#include "{file}"

GRD_TEST_CASE(single_file_compilation) {{

}}

'''

path = Path(__file__).parent / "single_file_compilation_tests"
path.mkdir(parents=True, exist_ok=True)
all_files = []

blacklist = ['.git', '.vscode', 'third_party', 'b_lib', 'testing.h', 'misc']

def gen(dir, level=1):
	for it in os.scandir(Path(__file__).parent / dir):
		if it.name in blacklist:
			continue
		if it.is_dir():
			gen(dir / it.name, level=level+1)
			continue
		if it.name.endswith(".h"):
			skiplist = (['posix_', 'darwin_'] if os.name == 'nt' else ['win_']) + ["_test"]
			if any (x in it.name for x in skiplist):
				continue
			file_path = (path / dir / f'{it.name}_test.cpp').resolve()
			file_path.parent.mkdir(parents=True, exist_ok=True)
			all_files.append(str(PurePosixPath("../") / dir / it.name))
			file_path.write_text(CPP_CONTENT.format(
				testing_header=str(PurePosixPath('../' * level) / "grd_testing.h"),
				file=str(PurePosixPath('../' * level) / dir / it.name),
			))

# Useful for compilation time analysis
def build_all_files():
	filtered = filter(lambda x: all(not skip in x for skip in ["ssa.h", "grdc_parser.h", "ast_printer.h", "window.h"]) , all_files)
	HEADER = """#pragma once
#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
"""
	FOOTER = """
	int main() {
		return 0;
	}
"""
	(path / "all_file_compilation_test.cpp").write_text(HEADER + '\n'.join(map(lambda x: '#include "' + x + '"', filtered)) + FOOTER)

def run_hook(tester):
	shutil.rmtree(path, ignore_errors=True)
	gen(Path('.'))
	build_all_files()
