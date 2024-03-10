#pragma once

#include "base.h"
#include "code_location.h"
#include "build.h"
#include "heap_sprintf.h"
#include <stdio.h>
#include <stdlib.h>

struct Test {
	Test*               next = NULL;
	void              (*proc)();
	const char*         name = NULL;
	s64                 expect_succeeded = 0;
	s64                 expect_failed = 0;
};

struct TestScopeNode {
	TestScopeNode* next = NULL;
	CodeLocation   loc;
};

struct Tester {
	Test*          first_test;
	TestScopeNode* scope;
	Test*          current_test = NULL;
};
inline Tester tester;

void run_test(Test* test) {
	tester.current_test = test;
	tester.scope = NULL;
	test->proc();
	tester.current_test = NULL;
}

void register_test(void(*proc)(), const char* name) {
	Test* test = new Test();
	test->proc = proc;
	test->name = name;
	if (!tester.first_test) {
		tester.first_test = test;
	} else {
		test->next = tester.first_test;
		tester.first_test = test;
	}
}

bool WRITE_TEST_RESULTS = false;

void tester_write_result_line(const char* str) {
	if (!WRITE_TEST_RESULTS) {
		return;
	}
	fwrite(str, 1, strlen(str), stderr);
	fwrite("\n", 1, 1, stderr);
	fflush(stderr);
}

void tester_write_result_int(s64 num) {
	if (!WRITE_TEST_RESULTS) {
		return;
	}
	fprintf(stderr, "%lli\n", num);
}

void tester_write_result_str(const char* str) {
	s64 line_count = 1;
	const char* ptr = str;
	while (*ptr != '\0') {
		if (*ptr == '\n') {
			line_count += 1;
		}
		ptr += 1;
	}
	tester_write_result_int(line_count);
	tester_write_result_line(str);
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--write_test_results_to_stderr") == 0) {
			WRITE_TEST_RESULTS = true;
		}
	}

	tester_write_result_line("TESTER_OUTPUT_START");
	tester_write_result_int(1); // Version.

	auto ptr = tester.first_test;
	while (ptr) {
		tester_write_result_line("TESTER_TEST_START");
		tester_write_result_str(ptr->name);
		run_test(ptr);
		tester_write_result_line("TESTER_TEST_FINISHED");
		printf("%s - %s\n", ptr->name, ptr->expect_failed > 0 ? "failed" : "success");
		ptr = ptr->next;
	}
	tester_write_result_line("TESTER_EXECUTABLE_DONE");
	return 0;
}

BUILD_RUN("def add_test(name):\n\tif 'test' in globals(): test.cases[name] = tester.Case(name)")

void tester_write_result_loc(CodeLocation loc) {
	tester_write_result_str(loc.file);
	tester_write_result_int(loc.line);
}

void test_expect(bool cond, const char* cond_str, const char* message, CodeLocation loc = caller_loc()) {
	tester_write_result_line("TESTER_EXPECT");
	if (cond) {
		tester.current_test->expect_succeeded += 1;
		printf("  %s - ok\n", cond_str);
		tester_write_result_int(1);
	} else {
		tester.current_test->expect_failed += 1;
		printf("  %s - failed\n", cond_str);
		tester_write_result_int(0);
	}

	tester_write_result_str(cond_str);
	tester_write_result_str(message);
	int scope_count = 0;
	TestScopeNode* ptr = tester.scope;
	while (ptr) {
		scope_count += 1;
		ptr = ptr->next;
	}
	tester_write_result_int(scope_count);
	ptr = tester.scope;
	while (ptr) {
		tester_write_result_loc(ptr->loc);
		printf("   %s: %d\n", ptr->loc.file, ptr->loc.line);
		ptr = ptr->next;
	}
	tester_write_result_loc(loc);
	printf("   %s: %d\n", loc.file, loc.line);
}

// void test_expect(bool cond, const char* cond_str, Unicode_String message, CodeLocation loc = caller_loc()) {
// 	test_expect(cond, cond_str, encode_utf8(message), loc);
// }

void test_expect(bool cond, const char* cond_str, CodeLocation loc = caller_loc()) {
	char* str = heap_sprintf("Expected %s", cond_str);
	test_expect(cond, cond_str, str, loc);
	free(str);
}

void tester_scope_push(CodeLocation loc) {
	auto scope_node = new TestScopeNode();
	scope_node->loc = loc;
	if (!tester.scope) {
		tester.scope = scope_node;
	} else {
		scope_node->next = tester.scope;
		tester.scope = scope_node;
	}
}

void tester_scope_pop() {
	if (tester.scope) {
		tester.scope = tester.scope->next;
	}
}

#define TEST(name)\
BUILD_RUN("add_test(\"" #name "\")");\
void test_##name ();\
int register_test_##name = []() { register_test(&test_##name, #name); return 0; }();\
inline void test_##name()

#define EXPECT(cond, ...) test_expect(cond, #cond, __VA_ARGS__);
#define EXPECT_OP(a, op, b, ...) EXPECT(a op b, sprint("Expected % " #op " %", a, b), __VA_ARGS__);
#define EXPECT_EQ(a, b, ...) EXPECT_OP(a, ==, b, __VA_ARGS__)
