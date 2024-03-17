#!/usr/bin/env python3
from pathlib import Path, PurePosixPath
import os
import sys

CPP_CONTENT = '''
#pragma once
#include "{testing_header}"
#include "{file}"

TEST(single_file_compilation) {{

}}

'''

path = Path(__file__).parent / "single_file_compilation_tests"
path.mkdir(parents=True, exist_ok=True)
to_build = []

blacklist = ['.git', '.vscode', 'third_party', 'b_lib', 'testing.h']
if sys.platform != 'win32':
	blacklist.append('win_sync.h')

def gen(dir, level=1):
	for it in os.scandir(Path(__file__).parent / dir):
		if it.name in blacklist:
			continue
		if it.is_dir():
			gen(dir / it.name, level=level+1)
			continue
		if it.name.endswith(".h"):
			file_path = (path / dir / f'{it.name}_test.cpp').resolve()
			file_path.parent.mkdir(parents=True, exist_ok=True)
			to_build.append(file_path)
			file_path.write_text(CPP_CONTENT.format(
				testing_header=str(PurePosixPath('../' * level) / "testing.h"),
				file=str(PurePosixPath('../' * level) / dir / it.name))
			)

def run_hook(tester):
	gen(Path('.'))
