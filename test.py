#!/usr/bin/env python3
import os
import sys
from io import StringIO
from pathlib import Path
sys.path.append(Path(__file__).parent)
import build as builder
import subprocess
from queue import Queue
from multiprocessing.pool import ThreadPool
import argparse
import threading
import time
import re

ARGS = None

GREEN = '\033[92m'
RED = '\033[91m'
RESETC = '\033[0m'

class Tester:
	def __init__(self, path):
		self.tests = []
		self.path = path
		self.msg_queue = Queue()
		self.parallel = True
		self.file_filters = []
		self.path_filters = []

	def run_exec(self, cmd, args, *, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kwargs):
		self.verbose(f"run_exec({cmd}, {args})")
		return subprocess.run([Path(cmd).absolute(), *args], stdout=stdout, stderr=stderr, cwd=Path(cmd).parent, **kwargs)

	def add(self, test):
		if ARGS.whitelist and not re.match(ARGS.whitelist, test.path):
			return
		self.tests.append(test)
	
	def print(self, str):
		self.msg_queue.put(str)

	def verbose(self, msg):
		if ARGS.verbose:
			self.msg_queue.put(msg)

	def test_build_failed(self, test, output=""):
		self.print(f"{RED}Failed to build{RESETC} {test.path}\n{output}")

	def test_run_failed(self, test, error=None):
		if error and ARGS.verbose: self.print(f"{RED}Failed to run test{RESETC} {test.path} {error}")
		else:                      self.print(f"{RED}Failed to run test{RESETC} {test.path}")

	def add_file_filter(self, filter):
		self.file_filters.append(filter)
		return filter
	
	def add_path_filter(self, filter):
		self.path_filters.append(filter)
		return filter

	def collect_tests(self):
		self.scan(self.path)
		self.print(f"Collected {len(self.tests)} tests.")
		# chr(10) is newline. this way because f-string can't contain \n literal.
		self.verbose(f"Tests: \n{chr(10).join(map(lambda it: it.path, self.tests))}")

	def load_hook(self, path):
		import importlib.machinery
		import importlib.util
		loader = importlib.machinery.SourceFileLoader(Path(path).stem, path)
		spec = importlib.util.spec_from_loader(loader.name, loader)
		module = importlib.util.module_from_spec(spec)
		loader.exec_module(module)
		module.run_hook(self)

	def scan_and_load_hooks(self, dir):
		self.verbose(f"Scanning and running hooks in {dir}")
		for it in os.scandir(dir):
			if it.is_dir():
				self.scan_and_load_hooks(it.path)
				continue
			if it.name.lower().endswith("_test_hook.py"):
				self.load_hook(it.path)
		

	def scan(self, dir):
		if any(map(lambda x: x(dir), self.path_filters)): return
		self.verbose(f'Scanning for tests in {dir}')
		for it in os.scandir(dir):
			if it.is_dir():
				self.scan(Path(it.path))
				continue
			if it.name.lower().endswith(("_test.h", "_test.cpp", "_test.hpp", "_test.c")):
				if any(map(lambda x: x(it), self.file_filters)): continue
				self.add(CppTest(it.path, self))

	def cases(self):
		cases = []
		for it in self.tests: cases.extend(it.cases.values())
		return cases
	
	def div_or_zero(self, x, y):
		if y == 0: return 0
		return x / y

	def run(self):
		print(f"Running tests in {self.path.resolve()}")
		self.scan_and_load_hooks(self.path)
		self.collect_tests()
		exit_code = -1
		pool = ThreadPool()
		def printer_thread():
			while it := self.msg_queue.get():
				if it == None: break
				print(it)
		p = threading.Thread(target=printer_thread)
		p.start()
		def run_test(test):
			self.print(f"Running: {test.path}")
			test.run()
		try:
			res = pool.map_async(lambda test: test.build(), self.tests)
			while True:
				if res.ready(): break
				time.sleep(0.1)
			# res.get() is needed to catch exceptions
			res.get()
			res = pool.map_async(lambda test: run_test(test), filter(lambda it: it.status != 'test_failed_to_build', self.tests))
			while True:
				if res.ready(): break
				time.sleep(0.1)
			res.get()
			self.print(" -- Summary -- ")
			sorted_tests = sorted(self.tests, key=lambda it: (it.is_ok(), it.path), reverse=True)
			for test in sorted_tests:
				if test.status == 'test_failed_to_build':
					self.print(f"Test {test.path}: {RED}failed to build{RESETC}")
				else:
					if test.is_ok():
						self.print(f"Test {test.path}: {GREEN}success{RESETC} [{len(test.successful_cases())}/{len(test.cases)}]")
					else:
						self.print(f"Test {test.path}: {RED}failed{RESETC} [{len(test.successful_cases())}/{len(test.cases)}]")
						for case in test.cases.values():
							if not case.is_ok():
								self.print(f"  Case {case.name}: failed [{case.status}]")
								for expect in case.expects:
									if not expect.ok:
										self.print(f"    {expect.message}")
										if expect.condition:
											self.print(f"    Condition: {expect.condition}")
										for it in expect.scope:
											self.print(f"      at {it[0]}:{it[1]}")

			ok_tests = [x for x in sorted_tests if x.is_ok()]
			ok_cases = [x for x in self.cases() if x.is_ok()]
			# replace / with div_or_zero
			nb_tests = list(filter(lambda it: it.needs_build, sorted_tests))
			nb_tests_ok = len(list(filter(lambda it: it.status != 'test_failed_to_build', nb_tests)))
			self.print(f"{nb_tests_ok}/{len(nb_tests)} tests built ok - {self.div_or_zero(nb_tests_ok, len(nb_tests))*100:.2f}%")
			self.print(f"{len(ok_tests)}/{len(sorted_tests)} tests ok - {self.div_or_zero(len(ok_tests), len(sorted_tests))*100:.2f}%")
			self.print(f"{len(ok_cases)}/{len(self.cases())} cases ok - {self.div_or_zero(len(ok_cases), len(self.cases()))*100:.2f}%")
			if len(ok_tests) == len(sorted_tests):
				exit_code = 0
			self.print("Success!" if exit_code == 0 else "Failed.")
		finally:
			self.msg_queue.put(None)
		p.join()
		return exit_code
		
class TestCase:
	def __init__(self, test, name):
		self.test = test
		self.name = name
		self.status = 'case_did_not_run'
		self.expects = []
	
	def is_ok(self):
		return self.status == 'case_finished' and all(map(lambda it: it.ok, self.expects))

class Test:
	def __init__(self, path, tester):
		self.path = path
		self.tester = tester
		self.cases = {}
		self.status = 'test_not_built'
		self.needs_build = False

	def successful_cases(self):
		return [x for x in self.cases.values() if x.is_ok()]

	def failed_cases(self):
		return [x for x in self.cases.values() if not x.is_ok()]

	def is_ok(self):
		return self.status == 'test_finished' and all(map(lambda it: it.is_ok(), self.cases.values()))

	def get_case(self, name):	
		if name in self.cases: return self.cases[name]
		case = TestCase(self, name)
		self.cases[name] = case
		return case

	def build(self): self.status = 'test_built'
	def run(self): self.status = 'test_finished'

class TestCaseExpect:
	def __init__(self, ok, condition, message, scope):
		self.ok = ok
		self.condition = condition
		self.message = message
		self.scope = scope

class ParseResultsException(Exception): pass

class CppTest(Test):
	def __init__(self, path, tester):
		super().__init__(path, tester)
		self.exec_path = None
		self.needs_build = True

	def parse_results(self, output):
		lines = output.decode('utf-8', errors='ignore').splitlines()
		self.tester.verbose(f"Cpp test output {self.path}:\n {chr(10).join(lines)}")
		def read_line():
			nonlocal lines
			if len(lines) == 0:
				raise ParseResultsException("Expected a line")
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
				raise ParseResultsException(f"Test output parser expected ({expected}), but got ({x})")

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
			return TestCaseExpect(ok, cond_str, message, scope)

		expect(read_line(), "TESTER_OUTPUT_START")
		expect(read_int(), 1)
		current_case = None
		while True:
			verb = read_line()
			if verb == "TESTER_EXECUTABLE_DONE":
				break
			elif verb == "TESTER_TEST_START":
				name = read_str()
				current_case = self.get_case(name)
				current_case.status = 'case_started'
			elif verb == "TESTER_TEST_FINISHED":
				if current_case: current_case.status = 'case_finished'
			elif verb == "TESTER_EXPECT":
				if current_case: current_case.expects.append(parse_test_expect())
			else:
				raise ParseResultsException(f"Unknown verb {verb}")

	def run(self):
		prev_cases_to_skip_len = -1
		self.run_count = 0
		while True:
			cases_to_skip = []
			for name, case in self.cases.items():
				if case.status != 'case_did_not_run':
					if not name in cases_to_skip:
						cases_to_skip.append(name)
			if len(cases_to_skip) <= prev_cases_to_skip_len:
				self.status = 'test_finished'
				break
			prev_cases_to_skip_len = len(cases_to_skip)
			input = '\n'.join([str(len(cases_to_skip)), *cases_to_skip]) + '\n'
			self.tester.verbose(f"Running test {self.path} with skipped cases {cases_to_skip}")
			process = self.tester.run_exec(self.exec_path
			, ['--write_test_results_to_stderr', '--skip_cases'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, input=input.encode('utf-8'))
			self.run_count += 1
			self.tester.verbose(f"Test {self.path} stdout: \n{process.stdout.decode('utf-8', errors='ignore')}")
			try:
				self.parse_results(process.stderr)
			except ParseResultsException as e:
				# self.tester.test_run_failed(self, e)
				# return
				pass
			for name, case in self.cases.items():
				if case.status == 'case_started':
					case.status = 'case_crashed'

	def build(self):
		self.tester.print(f'Building: {self.path}')
		stdout = StringIO()
		ctx=builder.BuildCtx(stdout, self.path)
		ctx.test = self
		res = builder.build(self.path, stdout=stdout, ctx=ctx)	
		if isinstance(res, builder.RunnableExecutable):
			self.exec_path = res.path
			self.status = 'test_built'
		else:
			self.status = 'test_failed_to_build'
			self.tester.test_build_failed(self, output=stdout.getvalue())

def main():
	global ARGS
	parser = argparse.ArgumentParser()
	parser.add_argument('--path', default=os.getcwd())
	parser.add_argument('--verbose', action='store_true')
	parser.add_argument('--whitelist', help='Regex pattern', default=None)
	ARGS = parser.parse_args()
	builder.VERBOSE = ARGS.verbose
	os.chdir(ARGS.path)
	tester = Tester(Path('.'))
	tester.add_path_filter(lambda x: 'third_party' in Path(x).parts)
	return tester.run()

if __name__ == "__main__":
	exit(main())
