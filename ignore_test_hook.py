import os
from pathlib import Path
import re

def grd_test_hook(tester):
	@tester.add_file_filter
	def file_filter(path: Path):
		if os.name == 'nt':
			if str(path).endswith("posix_sync.h_test.cpp"):
				return True
		else:
			if str(path).endswith("win_sync.h_test.cpp"):
				return True
			
	@tester.add_path_filter
	def path_filter(path: Path):
		for it in path.parts:
			if it in ["b_lib", ".git", ".vscode", ".vs", "__pycache__"]:
				return True
