#!/usr/bin/env python3
from pathlib import Path, PurePosixPath
import os
import shutil
import collect_grd_code_files

CPP_CONTENT = '''
#pragma once
#include "{testing_header}"
#include "{file}"

GRD_TEST_CASE(single_file_compilation) {{

}}

'''

JUMBO_HEADER = """#pragma once
#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
"""
JUMBO_FOOTER = """
	int main() {
		return 0;
	}
"""

def grd_test_hook(tester):
	print("Generating individual file build tests...")
	root, files = collect_grd_code_files.collect()
	single_files_root = Path(__file__).parent.resolve() / "tests" / "build_tests"
	shutil.rmtree(single_files_root, ignore_errors=True)
	single_files_root.mkdir(parents=True, exist_ok=True)
	for f in files:
		s_path = (single_files_root / f.parent / f'{f.name}_build_test.cpp').resolve()
		s_path.parent.mkdir(parents=True, exist_ok=True)
		s_path.write_text(CPP_CONTENT.format(
			testing_header=str(PurePosixPath('../'*(len(f.parts) + 1)) / "grd_testing.h"),
			file=str(PurePosixPath('../'*(len(f.parts) + 1)) / f),
		))
	(single_files_root / "JUMBO_BUILD_test.cpp").write_text(JUMBO_HEADER + '\n'.join(map(lambda x: '#include "' + str(Path("../../") / x) + '"', files)) + JUMBO_FOOTER)
	print(f"{len(files)} individual file build tests generated in {single_files_root}")
	