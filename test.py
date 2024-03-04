#!/usr/bin/env python3

import os
import sys
import threading
from pathlib import Path
import argparse
import subprocess
from io import StringIO
from multiprocessing.pool import ThreadPool
root_dir = Path(__file__).parent
sys.path.append(root_dir)
import build as builder
import glob

ARGS = None

class Test:
	def __init__(self, path, kind):
		self.path = path
		self.kind = kind
		self.cases = {}
		self.run = None
		self.run_error = None
		self.exec_path = None

def collect_dir_tests(folder):
	x = []
	verbose(f'Collecting tests from {folder}')
	for it in os.scandir(folder):
		if it.name.lower().endswith(("_test.h", "_test.cpp", "_test.hpp", "_test.c")):
			x.append(Test(it.path, "cpp"))
		if it.name.lower().endswith("_test.py"):
			x.append(Test(it.path, "py"))
		if it.is_dir():
			x.extend(collect_dir_tests(folder / it))
	return x

def scan_tests(folder):
	tests = []
	tests.extend(collect_dir_tests(Path(folder)))
	return tests

class ParseException(Exception):
	pass

class Case:
	def __init__(self, name):
		self.name = name
		self.status = "didn't run"
		self.expects = []

def parse_results(test, output):
	lines = output.decode('utf-8').splitlines()
	def read_line():
		nonlocal lines
		if len(lines) == 0:
			raise ParseException("Expected a line.");
		res = lines[0]
		lines = lines[1:]
		return res
	def read_int():
		return int(read_line())
	def read_str():
		line_count = read_int()
		return '\n'.join(read_line() for i in range(line_count))
	def read_loc():
		return (read_str(), read_int())
	def expect(x, expected):
		if x != expected:
			raise ParseException(f"Test output parser expected ({expected}), but got ({x})")

	def parse_test_expect():
		ok = True if read_int() else False
		cond_str = read_str()
		message = read_str()
		scope_count = read_int()
		scope = []
		for i in range(scope_count):
			loc = read_loc()
			scope.append(loc)
		scope.append(read_loc())
		return (ok, cond_str, message, scope)

	expect(read_line(), "TESTER_OUTPUT_START")
	expect(read_int(), 1)
	current_case = None
	while True:
		verb = read_line()
		if verb == "TESTER_EXECUTABLE_DONE":
			break
		elif verb == "TESTER_TEST_START":
			name = read_str()
			if name in test.cases:
				current_case = test.cases[name]
			else:
				current_case = Case(name)
				test.cases[name] = current_case
			current_case.status = "crashed"
		elif verb == "TESTER_TEST_FINISHED":
			if current_case:
				current_case.status = 'failed' if any(not it[0] for it in current_case.expects) else 'ok'
		elif verb == "TESTER_EXPECT":
			if current_case:
				current_case.expects.append(parse_test_expect())
		else:
			raise ParseException(f"Unknown verb {verb}")

def run_cpp_test(test, executable_path):
	process = run(f'{executable_path} --write_test_results_to_stderr', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	try:
		parse_results(test, process.stderr)
	except ParseException as e:
		test.run_error = e

def build_test(test, build_scope):
	if test.kind == 'cpp':
		stdout = StringIO()
		scope = {
			"tester": __import__(__name__),
			"test": test,
		}
		res = builder.build(test.path, stdout=stdout, scope=scope)
		if isinstance(res, builder.Runnable_Executable):
			test.exec_path = res.path
			test.run = lambda: run_cpp_test(test, res.path)
			# Doing read separately because variable is shared across threads.
			val = build_scope.built
			build_scope.built += 1
			thread_print(f'{test.path} - ok [{build_scope.built}/{len(build_scope.tests)}]')
		else:
			for k, v in test.cases.items():
				v.status = 'compilation failed'
			thread_print(f'{test.path} - failed')

def run(cmd, *, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, shell=True):
	verbose(f'running {cmd}')
	return subprocess.run(cmd, stdout=stdout, stderr=stderr, stdin=stdin, shell=shell)

RED = '\x1b[31m'
GREEN = '\x1b[32m'
RESET_COLOR = '\x1b[39m'

def verbose(*args):
	if ARGS.verbose:
		print(*args)

print_lock = threading.Lock()
def thread_print(*args):
	with print_lock:
		print(*args)

class Build_Scope:
	def __init__(self, tests):
		self.tests = tests
		self.built = 0

def run_tests(path):
	tests = scan_tests(path)
	# tests = filter(lambda test: any(it in test.path for it in ['stable']), tests)
	# tests = list(tests)
	thread_print(f'Collected {len(tests)} files.')
	thread_print(f'Building...')
	build_scope = Build_Scope(tests)
	sequential = False
	sequential_run = False
	if sequential:
		for test in tests:
			build_test(test, build_scope)
	else:
		pool = ThreadPool()
		pool.map(lambda test: build_test(test, build_scope), tests)

	thread_print(f"Running...")
	runnable_tests = list(filter(lambda test: test.run, tests))
	if sequential or sequential_run:
		for test in runnable_tests:
			test.run()
	else:
		pool = ThreadPool()
		pool.map(lambda it: it.run(), runnable_tests)

	total_cases = sum(len(test.cases) for test in tests)
	ok_cases = 0
	for test in tests:
		path = str(test.exec_path if test.exec_path else test.path)
		for case_name, case in test.cases.items():
			suffix = ''
			if case.status == 'ok':
				ok_cases += 1
				suffix = f' [{ok_cases}/{total_cases}]'
			thread_print(f"{path} {case.name} - {case.status}{suffix}")
			for ok, cond, msg, scope in case.expects:
				if ok:
					continue
				thread_print(f'')
				thread_print(f'  {msg} at')
				for file, line in scope:
					thread_print(f'     {file}: {line}')
				thread_print(f'')
		# if test.run_error:
			# if hasattr(test.run_error, '__traceback__'):
			# 	traceback.print_tb(test.run_error.__traceback__)
			# thread_print(f" Error: {test.run_error}")
	thread_print(f'{ok_cases}/{total_cases} cases ok')

def main():
	global ARGS
	parser = argparse.ArgumentParser()
	parser.add_argument('--path', default=os.getcwd())
	parser.add_argument('--verbose', action='store_true')
	ARGS = parser.parse_args()
	if ARGS.verbose:
		builder.VERBOSE = True
	os.chdir(root_dir)
	run_tests(ARGS.path)

if __name__ == '__main__':
	main()
