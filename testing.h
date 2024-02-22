#pragma once

#include "array.h"
#include "string.h"
#include "print.h"
#include "log.h"

struct Test {
	Proc<void()>*       proc;
	String              name;
	s64                 expect_succeeded = 0;
	s64                 expect_failed = 0;
};

struct Tester {
	Array<Test>          tests;
	Array<Code_Location> scope;
	Test*                current_test = NULL;
};
inline Tester tester;

void run_test(Test* test) {
	tester.current_test = test;
	tester.scope.clear();
	test->proc();
	tester.current_test = NULL;
}

void register_test(Proc<void()>* proc, const char* name) {
	Test test = {
		.proc = proc,
		.name = make_string(name),
	};
	tester.tests.add(test);
}

bool WRITE_TEST_RESULTS = false;

void tester_write_result_line(String str) {
	if (!WRITE_TEST_RESULTS) {
		return;
	}
	fwrite(str.data, 1, str.length, stderr);
	fwrite("\n", 1, 1, stderr);
	fflush(stderr);
}

void tester_write_result_int(s64 num) {
	Small_String str = to_string(num);
	tester_write_result_line(as_str(&str));
}

void tester_write_result_str(String str) {
	s64 line_count = 1;
	for (auto c: str) {
		if (c == '\n') {
			line_count += 1;
		}
	}
	tester_write_result_int(line_count);
	tester_write_result_line(str);
}

int main(int argc, char* argv[]) {
	for (auto i: range(argc)) {
		if (strcmp(argv[i], "--write_test_results_to_stderr") == 0) {
			WRITE_TEST_RESULTS = true;
		}
	}

	tester_write_result_line("TESTER_OUTPUT_START"_b);
	tester_write_result_int(1); // Version.
	for (auto& it: tester.tests) {
		tester_write_result_line("TESTER_TEST_START"_b);
		tester_write_result_str(it.name);
		run_test(&it);
		tester_write_result_line("TESTER_TEST_FINISHED"_b);
		Log("% - %", it.name, it.expect_failed > 0 ? "failed" : "success");
	}
	tester_write_result_line("TESTER_EXECUTABLE_DONE"_b);

	return 0;
}

BUILD_RUN("def add_test(name):\n\tif 'test' in globals(): test.cases[name] = tester.Case(name)")

void tester_write_result_loc(Code_Location loc) {
	tester_write_result_str(make_string(loc.file));
	tester_write_result_int(loc.line);
}

void test_expect(bool cond, const char* cond_str, String message, Code_Location loc = caller_loc()) {
	tester_write_result_line("TESTER_EXPECT"_b);
	if (cond) {
		tester.current_test->expect_succeeded += 1;
		Log("  % - ok", cond_str);
		tester_write_result_int(1);
	} else {
		tester.current_test->expect_failed += 1;
		Log("  % - failed", cond_str);
		tester_write_result_int(0);
	}

	tester_write_result_str(make_string(cond_str));
	tester_write_result_str(message);
	tester_write_result_int(tester.scope.count);
	for (auto scope: tester.scope) {
		tester_write_result_loc(scope);
		Log("   %: %", scope.file, scope.line);
	}
	tester_write_result_loc(loc);
	Log("   %: %", loc.file, loc.line);
}

void test_expect(bool cond, const char* cond_str, Unicode_String message, Code_Location loc = caller_loc()) {
	test_expect(cond, cond_str, encode_utf8(message), loc);
}

void test_expect(bool cond, const char* cond_str, Code_Location loc = caller_loc()) {
	test_expect(cond, cond_str, sprint("Expected %", make_string(cond_str)), loc);
}

void tester_scope_push(Code_Location loc) {
	tester.scope.add(loc);
}

void tester_scope_pop() {
	if (tester.scope.count > 0) {
		tester.scope.pop_last();
	}
}

#define TEST(name)\
BUILD_RUN("add_test(\"" #name "\")");\
void test_##name ();\
EXECUTE_ONCE([](){ register_test(&test_##name, #name); })\
inline void test_##name()

#define EXPECT(cond, ...) test_expect(cond, #cond, __VA_ARGS__);
#define EXPECT_OP(a, op, b, ...) EXPECT(a op b, sprint("Expected % " #op " %", a, b), __VA_ARGS__);
#define EXPECT_EQ(a, b, ...) EXPECT_OP(a, ==, b, __VA_ARGS__)
