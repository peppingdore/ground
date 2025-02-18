#pragma once

#include "grd_base.h"
#include "grd_code_location.h"
#include "grd_build.h"
#include "grd_heap_sprintf.h"
#include "grd_allocator.h"
#include <stdio.h>
#include <stdlib.h>

struct GrdTestScopeNode;

struct GrdTestFailedExpect {
	GrdTestFailedExpect* next = NULL;
	const char*          cond_str = NULL;
	const char*          message = NULL;
	GrdTestScopeNode*    scope = NULL;
};

struct GrdTestCase {
	GrdTestCase*         next = NULL;
	void               (*proc)();
	const char*          name = NULL;
	s64                  expect_succeeded = 0;
	s64                  expect_failed = 0;
	s64                  idx = 0;
	GrdTestFailedExpect* first_failed_expect = NULL;
};

struct GrdTestScopeNode {
	GrdTestScopeNode* next = NULL;
	GrdCodeLoc        loc;
};

struct GrdTestStringNode {
	GrdTestStringNode* next = NULL;
	char*              str = NULL;
};

struct GrdTester {
	GrdAllocator       allocator;
	GrdTestCase*       first_case;
	GrdTestScopeNode*  scope;
	GrdTestCase*       current_test = NULL;
	GrdTestStringNode* first_ignore_test = NULL;
	GrdTestStringNode* first_whitelist_test = NULL;
	bool               write_test_results = false;
	bool               skip_cases = false;
	bool               whitelist_mode = false;
};
GRD_DEDUP GrdTester tester;

GRD_DEDUP void grd_tester_write_result_line(const char* str) {
	if (!tester.write_test_results) {
		return;
	}
	fwrite(str, 1, strlen(str), stderr);
	fwrite("\n", 1, 1, stderr);
	fflush(stderr);
}

GRD_DEDUP void grd_tester_write_result_int(s64 num) {
	if (!tester.write_test_results) {
		return;
	}
	fprintf(stderr, "%lli\n", num);
}

GRD_DEDUP void grd_tester_write_result_str(const char* str) {
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

GRD_DEDUP void grd_tester_write_result_loc(GrdCodeLoc loc) {
	grd_tester_write_result_str(loc.file);
	grd_tester_write_result_int(loc.line);
}

GRD_DEDUP void grd_run_test(GrdTestCase* test) {
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

GRD_DEDUP void grd_register_test_case(void(*proc)(), const char* name, s64 idx) {
	GrdTestCase* test = new GrdTestCase();
	test->proc = proc;
	test->name = name;
	test->idx = idx;
	if (!tester.first_case) {
		tester.first_case = test;
	} else {
		auto** dst = &tester.first_case;
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

GRD_DEDUP GrdTestScopeNode* grd_tester_copy_scope(GrdTestScopeNode* scope) {
	GrdTestScopeNode* root = NULL;
	GrdTestScopeNode* dst = NULL;
	while (scope) {
		auto node = grd_make<GrdTestScopeNode>(tester.allocator);
		*node = *scope;
		if (dst) {
			dst->next = node;
		}
		if (!root) {
			root = dst;
		}
		dst = node;
		scope = scope->next;
	}
	return root;
}

GRD_DEDUP void grd_tester_print_summary() {
	// print failed tests. use ascii colors
	printf("\n\n");
	auto test = tester.first_case;
	s64 successful_tests = 0;
	s64 failed_tests = 0;
	while (test) {
		if (test->expect_failed > 0) {
			failed_tests += 1;
			printf("Test: %s: \x1b[31mfailed\x1b[0m\n", test->name);
			auto expect = test->first_failed_expect;
			while (expect) {
				printf("  %s\n", expect->cond_str);
				printf("  %s\n", expect->message);
				auto scope = expect->scope;
				while (scope) {
					printf("  %s:%d\n", scope->loc.file, scope->loc.line);
					scope = scope->next;
				}
				expect = expect->next;
			}
		}
		test = test->next;
	}
	test = tester.first_case;
	while (test) {
		if (test->expect_failed == 0) {
			successful_tests += 1;
			printf("Test: %s: \x1b[32mpassed\x1b[0m\n", test->name);
		}
		test = test->next;
	}
	f64 percentage = (f64) successful_tests / (f64) (grd_max_s64(1, successful_tests + failed_tests));
	printf("%lld/%lld passed tests - %.2f %s\n", successful_tests, failed_tests + successful_tests, percentage * 100.0, "%"); 
}

void grd_remove_test(const char* name) {
	auto dst = &tester.first_case;
	while (*dst) {
		if (strcmp((*dst)->name, name) == 0) {
			*dst = (*dst)->next;
			continue;
		}
		dst = &(*dst)->next;
	}
}

GrdTestCase* grd_find_test(const char* name) {
	auto src = tester.first_case;
	while (src) {
		if (strcmp(src->name, name) == 0) {
			return src;
		}
		src = src->next;
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	grd_store_cmdline_args(argc, argv);
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--write_test_results_to_stderr") == 0) {
			tester.write_test_results = true;
		}
		if (strcmp(argv[i], "--skip_cases") == 0) {
			tester.skip_cases = true;
		}
		if (strcmp(argv[i], "--whitelist") == 0) {
			tester.whitelist_mode = true;
			for (int j = i + 1; j < argc; j++) {
				if (strstr(argv[j], "--") == argv[j]) {
					break;
				}
				auto node = new GrdTestStringNode();
				node->str = (char*) malloc(strlen(argv[j]) + 1);
				strcpy(node->str, argv[j]);
				node->next = NULL;
				auto dst = &tester.first_whitelist_test;
				while (*dst) {
					dst = &(*dst)->next;
				}
				*dst = node;
				i = j;
			}
		}
	}
	tester.allocator = c_allocator;

	if (tester.skip_cases) {
		int count = 0;
		scanf("%d\n", &count);
		printf("Ignored test count: %d\n", count);
		char buf[1024 * 4];
		for (int i = 0; i < count; i++) {
			scanf("%s\n", buf);
			auto node = new GrdTestStringNode();
			node->str = (char*) malloc(strlen(buf) + 1);
			strcpy(node->str, buf);
			node->next = NULL;
			auto dst = &tester.first_ignore_test;
			while (*dst) {
				dst = &(*dst)->next;
			}
			*dst = node;
			printf("Ignored test %s\n", buf);
		}
	}
	fflush(stdout);

	auto node = tester.first_ignore_test;
	while (node) {
		grd_remove_test(node->str);
		node = node->next;
	}
	if (tester.whitelist_mode) {
		GrdTestCase* start = NULL;
		GrdTestCase** dst = &start;
		node = tester.first_whitelist_test;
		while (node) {
			auto test = grd_find_test(node->str);
			if (test) {
				*dst = test;
				test->next = NULL;
				dst = &test->next;
			}
			node = node->next;
		}
		tester.first_case = start;
	}

	grd_tester_write_result_line("TESTER_OUTPUT_START");
	grd_tester_write_result_int(1); // Version.

	auto ptr = tester.first_case;
	while (ptr) {
		grd_run_test(ptr);
		ptr = ptr->next;
	}
	grd_tester_write_result_line("TESTER_EXECUTABLE_DONE");
	grd_tester_print_summary();
	return 0;
}

GRD_DEDUP void grd_tester_scope_push(GrdCodeLoc loc) {
	auto scope_node = new GrdTestScopeNode();
	scope_node->loc = loc;
	scope_node->next = tester.scope;
	tester.scope = scope_node;
}

GRD_DEDUP void grd_tester_scope_pop() {
	if (tester.scope) {
		tester.scope = tester.scope->next;
	}
}

GRD_DEDUP void grd_test_expect(bool cond, const char* cond_str, const char* message, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(loc);
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
		auto dst = &tester.current_test->first_failed_expect;
		while (*dst) {
			dst = &(*dst)->next;
		}
		*dst = new GrdTestFailedExpect();
		(*dst)->scope = grd_tester_copy_scope(tester.scope);
		(*dst)->cond_str = cond_str;
		(*dst)->message = message;
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
	grd_tester_scope_pop();
}

GRD_DEDUP void grd_test_expect(bool cond, const char* cond_str, GrdCodeLoc loc = grd_caller_loc()) {
	char* str = grd_heap_sprintf("Expected '%s'", cond_str);
	grd_test_expect(cond, cond_str, str, loc);
	free(str);
}

#define GRD_TEST_CASE(name)\
GRD_BUILD_RUN("if 'ctx' in globals() and getattr(ctx, 'test', None): ctx.test.get_case(\"" #name "\")");\
GRD_DEDUP void grd_test_##name ();\
GRD_DEDUP int grd_register_test_##name = []() { grd_register_test_case(&grd_test_##name, #name, __COUNTER__); return 0; }();\
GRD_DEDUP void grd_test_##name()

#define GRD_EXPECT_BASIC(cond, ...) grd_test_expect(cond, #cond, ## __VA_ARGS__);
// #include "grd_format.h" for if you want to use macros below.
#define GRD_EXPECT(cond, ...) grd_test_expect(cond, #cond, grd_copy_c_str(grd_sprint(__VA_ARGS__)));
#define GRD_EXPECT_OP(a, op, b, ...) GRD_EXPECT(a op b, grd_sprint("Expected % " #op " %\0", a, b).data);
#define GRD_EXPECT_EQ(a, b, ...) GRD_EXPECT_OP(a, ==, b)
