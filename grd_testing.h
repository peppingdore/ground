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

struct GrdIgnoreTest {
	GrdIgnoreTest* next = NULL;
	char*          name = NULL;
};

struct GrdTester {
	GrdTest*          first_test;
	GrdTestScopeNode* scope;
	GrdTest*          current_test = NULL;
	GrdIgnoreTest*    first_ignore_text = NULL;
	bool              write_test_results = false;
	bool              skip_cases = false;
};
inline GrdTester tester;

void grd_tester_write_result_line(const char* str) {
	if (!tester.write_test_results) {
		return;
	}
	fwrite(str, 1, strlen(str), stderr);
	fwrite("\n", 1, 1, stderr);
	fflush(stderr);
}

void grd_tester_write_result_int(s64 num) {
	if (!tester.write_test_results) {
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

void grd_tester_write_result_loc(GrdCodeLoc loc) {
	grd_tester_write_result_str(loc.file);
	grd_tester_write_result_int(loc.line);
}

void grd_run_test(GrdTest* test) {
	auto node = tester.first_ignore_text;
	printf("grd_run_test(%s)\n", test->name);
	while (node) {
		printf("test skip: %s\n", node->name);
		if (strcmp(node->name, test->name) == 0) {
			return;
		}
		node = node->next;
	}
	fflush(stdout);
	grd_tester_write_result_line("TESTER_TEST_START");
	grd_tester_write_result_str(test->name);
	tester.current_test = test;
	tester.scope = NULL;
	test->proc();
	tester.current_test = NULL;
	grd_tester_write_result_line("TESTER_TEST_FINISHED");
	printf("%s - %s\n", test->name, test->expect_failed > 0 ? "failed" : "success");
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

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--write_test_results_to_stderr") == 0) {
			tester.write_test_results = true;
		}
		if (strcmp(argv[i], "--skip_cases") == 0) {
			tester.skip_cases = true;
		}
		printf("arg %s\n", argv[i]);
		fflush(stdout);
	}

	if (tester.skip_cases) {
		int count = 0;
		scanf("%d\n", &count);
		printf("Ignored test count: %d\n", count);
		char buf[1024 * 4];
		for (int i = 0; i < count; i++) {
			scanf("%s\n", buf);
			auto node = new GrdIgnoreTest();
			node->name = (char*) malloc(strlen(buf) + 1);
			strcpy(node->name, buf);
			node->next = NULL;
			auto dst = &tester.first_ignore_text;
			while (*dst) {
				dst = &(*dst)->next;
			}
			*dst = node;
			printf("Ignored test %s\n", buf);
		}
	}
	fflush(stdout);

	grd_tester_write_result_line("TESTER_OUTPUT_START");
	grd_tester_write_result_int(1); // Version.

	auto ptr = tester.first_test;
	while (ptr) {
		grd_run_test(ptr);
		ptr = ptr->next;
	}
	grd_tester_write_result_line("TESTER_EXECUTABLE_DONE");
	return 0;
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

#define GRD_EXPECT_BASIC(cond, ...) grd_test_expect(cond, #cond, ## __VA_ARGS__);
#define GRD_EXPECT(cond, ...) grd_test_expect(cond, #cond, grd_copy_c_str(grd_sprint(__VA_ARGS__)));
#define GRD_EXPECT_OP(a, op, b, ...) GRD_EXPECT(a op b, grd_sprint("Expected % " #op " %\0", a, b).data, ## __VA_ARGS__);
#define GRD_EXPECT_EQ(a, b, ...) GRD_EXPECT_OP(a, ==, b, ## __VA_ARGS__)
