import os

def run_hook(tester):
	tester.blacklist.extend([
		"/b_lib",
		"/.git",
		"/.vscode",
		"/.vs",
		"/__pycache__",
	])
	if os.name == 'nt':
		tester.blacklist.append('posix_sync.h_test.cpp')
	else:
		tester.blacklist.append('win_sync.h_test.cpp')
