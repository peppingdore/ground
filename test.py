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

ARGS = None

class Tester:
	def __init__(self, path, blacklist=[]):
		self.tests = []
		self.path = path
		self.msg_queue = Queue()
		self.parallel = True
		self.blacklist = blacklist
		self.verbose(f'blacklist = {self.blacklist}')

	def run_exec(self, cmd, *, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, shell=True):
		self.verbose(f'Running: {cmd}')
		return subprocess.run(cmd, stdout=stdout, stderr=stderr, stdin=stdin, shell=shell, cwd=Path(cmd).parent)

	def add(self, test):
		self.tests.append(test)
	
	def print(self, str):
		self.msg_queue.put(str)

	def verbose(self, msg):
		if ARGS.verbose:
			self.msg_queue.put(msg)

	def test_build_failed(self, test, output=""):
		if ARGS.verbose:
			self.print(f"Failed to build {test.path}\n{output}")
		else:
			self.print(f"Failed to build {test.path}")

	def test_run_failed(self, test, error=None):
		if error and ARGS.verbose: self.print(f"Failed to run test {test.path} {error}")
		else:                      self.print(f"Failed to run test {test.path}")

	def collect_tests(self):
		self.scan(self.path)
		self.print(f"Collected {len(self.tests)} tests.")
		if ARGS.ci_debug:
			self.tests = list(filter(lambda it: it.path.endswith("base.h_test.cpp"), self.tests))
		# chr(10) is newline. this way because f-string can't contain \n literal.
		self.verbose(f"Tests: \n{chr(10).join(map(lambda it: it.path, self.tests))}")

	def should_ignore(self, entry):
		if entry.is_dir():
			if entry.name in map(lambda x: x.removeprefix('/'), filter(lambda x: x.startswith('/'), self.blacklist)):
				return True
		if entry.name in filter(lambda x: not x.startswith('/'), self.blacklist):
			return True
		return False

	def run_hook(self, path):
		import importlib.machinery
		module = importlib.machinery.SourceFileLoader(path, path).load_module()
		module.run_hook(self)

	def scan_and_run_hooks(self, dir):
		self.verbose(f"Scanning and running hooks in {dir}")
		for it in os.scandir(dir):
			if self.should_ignore(it): continue
			if it.is_dir():
				self.scan_and_run_hooks(it.path)
				continue
			if it.name.lower().endswith("_test_hook.py"):
				self.run_hook(it.path)

	def scan(self, dir):
		self.verbose(f'Scanning for tests in {dir}')
		for it in os.scandir(dir):
			if self.should_ignore(it): continue
			if it.is_dir():
				self.scan(it.path)
				continue
			if it.name.lower().endswith(("_test.h", "_test.cpp", "_test.hpp", "_test.c")):
				self.add(CppTest(it.path, self))

	def cases(self):
		cases = []
		for it in self.tests: cases.extend(it.cases.values())
		return cases

	def run(self):
		self.scan_and_run_hooks(self.path)
		self.collect_tests()
		exit_code = -1
		def runner_thread():
			nonlocal exit_code
			pool = ThreadPool()
			
			def run_test(test):
				self.print(f"Running: {test.path}")
				test.run()
			try:
				pool.map(lambda test: test.build(), self.tests)
				pool.map(lambda test: run_test(test), filter(lambda it: it.build_ok, self.tests))
				self.print(" -- Summary -- ")
				for test in self.tests:
					if not test.build_ok:
						self.print(f"Test {test.path}: failed to build")
					else:
						if test.is_ok():
							self.print(f"Test {test.path}: success")
						else:
							self.print(f"Test {test.path}: failed")
							for case in test.cases.values():
								if not case.is_ok():
									self.print(f"  Case {case.name}: failed")
									for expect in case.expects:
										if not expect.ok:
											self.print(f"    {expect.message}")
											if expect.condition:
												self.print(f"    Condition: {expect.condition}")
											for it in expect.scope:
												self.print(f"      at {it[0]}:{it[1]}")
				ok_tests = [x for x in self.tests if x.is_ok()]
				ok_cases = [x for x in self.cases() if x.is_ok()]
				self.print(f"{len(ok_tests)}/{len(self.tests)} tests ok - {len(ok_tests)/len(self.tests)*100:.2f}%")
				self.print(f"{len(ok_cases)}/{len(self.cases())} cases ok - {len(ok_cases)/len(self.cases())*100:.2f}%")
				if len(ok_tests) == len(self.tests):
					exit_code = 0
				self.print("Success!" if exit_code == 0 else "Failed.")
			finally:
				self.msg_queue.put(None)
		threading.Thread(target=runner_thread, daemon=True).start()
		while it := self.msg_queue.get():
			print(it)
		return exit_code
		
class TestCase:
	def __init__(self, test, name):
		self.test = test
		self.name = name
		self.status = 'did not run'
		self.expects = []
	
	def is_ok(self):
		return self.status == 'finished' and all(map(lambda it: it.ok, self.expects))

class Test:
	def __init__(self, path, tester):
		self.path = path
		self.tester = tester
		self.cases = {}
		self.build_ok = False

	def is_ok(self):
		return self.build_ok and all(map(lambda it: it.is_ok(), self.cases.values()))

	def get_case(self, name):
		if name in self.cases: return self.cases[name]
		case = TestCase(self, name)
		self.cases[name] = case

	def build(self): self.build_ok = True
	def run(self): self.tester.print(f"Running: {self.path}")

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

	def parse_results(self, output):
		lines = output.decode('utf-8').splitlines()
		self.tester.verbose(f"Lines {self.path}:\n {chr(10).join(lines)}")
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
				current_case.status = 'started'
			elif verb == "TESTER_TEST_FINISHED":
				if current_case: current_case.status = 'finished'
			elif verb == "TESTER_EXPECT":
				if current_case: current_case.expects.append(parse_test_expect())
			else:
				raise ParseResultsException(f"Unknown verb {verb}")

	def run(self):
		process = self.tester.run_exec(f'{self.exec_path} --write_test_results_to_stderr', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		try:
			self.parse_results(process.stderr)
		except ParseResultsException as e:
			self.tester.test_run_failed(self, e)

	def build(self):
		self.tester.print(f'Building: {self.path}')
		stdout = StringIO()
		scope = { "test": self }
		res = builder.build(self.path, stdout=stdout, scope=scope)	
		if isinstance(res, builder.Runnable_Executable):
			self.exec_path = res.path
			self.build_ok = True
		else:
			self.tester.test_build_failed(self, output=stdout.getvalue())

def main():
	global ARGS
	parser = argparse.ArgumentParser()
	parser.add_argument('--path', default=os.getcwd())
	parser.add_argument('--verbose', action='store_true')
	parser.add_argument('--blacklist', nargs='*', default=[])
	parser.add_argument('--ci_debug', action='store_true') 
	ARGS = parser.parse_args()
	builder.VERBOSE = ARGS.verbose
	tester = Tester(Path(__file__).parent, blacklist=ARGS.blacklist)
	return tester.run()

if __name__ == "__main__":
	exit(main())
