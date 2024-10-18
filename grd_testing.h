#pragma once

#include "grd_base.h"
#include "grd_code_location.h"
#include "grd_build.h"
#include "grd_heap_sprintf.h"
#include <stdio.h>
#include <stdlib.h>

struct GrdTest {
	GrdTest*            next = NULL;
	void              (*proc)();
	const char*         name = NULL;
	s64                 expect_succeeded = 0;
	s64                 expect_failed = 0;
	s64                 idx = 0;
};

struct GrdTestScopeNode {
	GrdTestScopeNode* next = NULL;
	GrdCodeLoc   loc;
};

struct GrdTester {
	GrdTest*          first_test;
	GrdTestScopeNode* scope;
	GrdTest*          current_test = NULL;
};
inline GrdTester tester;

void grd_run_test(GrdTest* test) {
	tester.current_test = test;
	tester.scope = NULL;
	test->proc();
	tester.current_test = NULL;
}

void grd_register_test(void(*proc)(), const char* name, s64 idx) {
	GrdTest* test = new GrdTest();
	test->proc = proc;
	test->name = name;
	test->idx = idx;
	if (!tester.first_test) {
		tester.first_test = test;
	} else {
		auto** dst = &tester.first_test;
		while (*dst) {
			if (idx <= (*dst)->idx) {
				break;
			}
			dst = &(*dst)->next;
		}
		test->next = *dst;
		*dst = test;
	}
}

bool GRD_WRITE_TEST_RESULTS = false;

void grd_tester_write_result_line(const char* str) {
	if (!GRD_WRITE_TEST_RESULTS) {
		return;
	}
	fwrite(str, 1, strlen(str), stderr);
	fwrite("\n", 1, 1, stderr);
	fflush(stderr);
}

void grd_tester_write_result_int(s64 num) {
	if (!GRD_WRITE_TEST_RESULTS) {
		return;
	}
	fprintf(stderr, "%lli\n", num);
}

void grd_tester_write_result_str(const char* str) {
	s64 line_count = 1;
	const char* ptr = str;
	while (*ptr != '\0') {
		if (*ptr == '\n') {
			line_count += 1;
		}
		ptr += 1;
	}
	grd_tester_write_result_int(line_count);
	grd_tester_write_result_line(str);
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--write_test_results_to_stderr") == 0) {
			GRD_WRITE_TEST_RESULTS = true;
		}
	}

	grd_tester_write_result_line("TESTER_OUTPUT_START");
	grd_tester_write_result_int(1); // Version.

	auto ptr = tester.first_test;
	while (ptr) {
		grd_tester_write_result_line("TESTER_TEST_START");
		grd_tester_write_result_str(ptr->name);
		grd_run_test(ptr);
		grd_tester_write_result_line("TESTER_TEST_FINISHED");
		printf("%s - %s\n", ptr->name, ptr->expect_failed > 0 ? "failed" : "success");
		ptr = ptr->next;
	}
	grd_tester_write_result_line("TESTER_EXECUTABLE_DONE");
	return 0;
}

void grd_tester_write_result_loc(GrdCodeLoc loc) {
	grd_tester_write_result_str(loc.file);
	grd_tester_write_result_int(loc.line);
}

void grd_test_expect(bool cond, const char* cond_str, const char* message, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_write_result_line("TESTER_EXPECT");
	if (cond) {
		tester.current_test->expect_succeeded += 1;
		grd_tester_write_result_int(1);
	} else {
		tester.current_test->expect_failed += 1;
		if (strcmp(cond_str, "") != 0) {
			printf("  %s - failed\n", cond_str);
		}
		printf("  %s\n", message);
		grd_tester_write_result_int(0);
	}

	grd_tester_write_result_str(cond_str);
	grd_tester_write_result_str(message);
	int scope_count = 0;
	GrdTestScopeNode* ptr = tester.scope;
	while (ptr) {
		scope_count += 1;
		ptr = ptr->next;
	}
	grd_tester_write_result_int(scope_count);
	grd_tester_write_result_loc(loc);
	ptr = tester.scope;
	while (ptr) {
		grd_tester_write_result_loc(ptr->loc);
		if (!cond) {
			printf("   %s: %d\n", ptr->loc.file, ptr->loc.line);
		}
		ptr = ptr->next;
	}
	if (!cond) {
		printf("   %s: %d\n", loc.file, loc.line);
	}
}

// void grd_test_expect(bool cond, const char* cond_str, Unicode_String message, GrdCodeLoc loc = caller_loc()) {
// 	grd_test_expect(cond, cond_str, encode_utf8(message), loc);
// }

void grd_test_expect(bool cond, const char* cond_str, GrdCodeLoc loc = grd_caller_loc()) {
	char* str = grd_heap_sprintf("Expected '%s'", cond_str);
	grd_test_expect(cond, cond_str, str, loc);
	free(str);
}

void grd_tester_scope_push(GrdCodeLoc loc) {
	auto scope_node = new GrdTestScopeNode();
	scope_node->loc = loc;
	if (!tester.scope) {
		tester.scope = scope_node;
	} else {
		scope_node->next = tester.scope;
		tester.scope = scope_node;
	}
}

void grd_tester_scope_pop() {
	if (tester.scope) {
		tester.scope = tester.scope->next;
	}
}

#define GRD_TEST(name)\
GRD_BUILD_RUN("if 'test' in globals(): test.get_case(\"" #name "\")");\
void grd_test_##name ();\
int grd_register_test_##name = []() { grd_register_test(&grd_test_##name, #name, __COUNTER__); return 0; }();\
inline void grd_test_##name()

#define GRD_FAIL(message) grd_test_expect(false, "", message)
#define GRD_EXPECT(cond, ...) grd_test_expect(cond, #cond, ## __VA_ARGS__);
// Requires "format.h"
// #define EXPECT_OP(a, op, b, ...) EXPECT(a op b, sprint("Expected % " #op " %", a, b), ## __VA_ARGS__);
// #define EXPECT_EQ(a, b, ...) EXPECT_OP(a, ==, b, ## __VA_ARGS__)
