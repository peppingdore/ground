#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#include "grdc_preprocess.h"
#include "../grd_testing.h"

GrdAllocatedUnicodeString test_escape_string(GrdUnicodeString str) {
	GrdAllocatedUnicodeString res;
	grd_add(&res, '"');
	for (auto c: str) {
		if (c <= 0x1f || c >= 0x7f) {
			grd_format(&res, U"\\x%", u64(c));
		} else {
			grd_add(&res, c);
		}
	}
	grd_add(&res, '"');
	return res;
}

void expect_str(GrdTuple<GrdError*, GrdcPrep*> x, GrdUnicodeString expected, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(loc);
	auto [e, p] = x;
	GRD_EXPECT(!e, e);
	GRD_EXPECT_EQ(test_escape_string(grdc_prep_str(p)), test_escape_string(expected));
	grd_tester_scope_pop();
}

void expect_error(GrdTuple<GrdError*, GrdcPrep*> x, GrdUnicodeString expected, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(loc);
	auto [e, p] = x;
	GRD_EXPECT(e);
	if (e) {
		GRD_EXPECT(grd_contains(e->text, expected), e);
	}
	grd_tester_scope_pop();
}

GrdTuple<GrdError*, GrdcPrep*> simple_prep(GrdUnicodeString str, GrdSpan<GrdTuple<GrdUnicodeString, GrdUnicodeString>> files) {
	auto prep = grdc_make_prep();
	prep->aux_data = &files;
	prep->load_file_hook = [](GrdcPrep* p, GrdUnicodeString fullpath) -> GrdcPrepFileSource* {
		for (auto [name, content]: *(decltype(files)*) p->aux_data) {
			if (name == fullpath) {
				auto file = grdc_make_mem_prep_file(p, content, fullpath);
				return file;
			}
		}
		return NULL;
	};
	auto root = grdc_make_mem_prep_file(prep, str, U"<root_file>"_b);
	return { grdc_preprocess_file(prep, root), prep };
}

#if 1
GRD_TEST_CASE(if_cond) {
	expect_str(simple_prep(U"#if 1\n42\n#endif\n"_b, {}), U"\n42\n\n"_b);
	expect_str(simple_prep(U"#if 0\n42\n#endif\n"_b, {}), U"\n"_b);
	expect_str(simple_prep(U"#if 0\n42\n#else\n99\n#endif\n"_b, {}), U"\n99\n\n"_b);
	expect_str(simple_prep(U"#if 1\n42\n#else\n99\n#endif\n"_b, {}), U"\n42\n\n"_b);
	expect_str(simple_prep(U"#if 0\n42\n#else\n99\n#endif\n"_b, {}), U"\n99\n\n"_b);
	expect_str(simple_prep(U"#   if 0\n42\n    #   else\n99\n   #   endif\n"_b, {}), U"\n99\n   \n"_b);
	expect_error(simple_prep(U"xd  #   if 0\n42\n    #   else\n99\n   #   endif\n"_b, {}), U"Only spaces are allowed on the same line before preprocessor directive"_b);

	// Nested ifs
	expect_str(simple_prep(U"#if 1\n#if 1\n42\n#endif\n#endif\n"_b, {}), U"\n\n42\n\n\n"_b);
	expect_str(simple_prep(U"#if 0\n#if 1\n42\n#endif\n#endif\n"_b, {}), U"\n"_b);
	expect_str(simple_prep(U"#if 0\n#if 0\n42\n#endif\n#endif\n"_b, {}), U"\n"_b);
	// elif
	expect_str(simple_prep(U"#if 0\n42\n#elif 1\n99\n#endif\n"_b, {}), U"\n99\n\n"_b);
}

GRD_TEST_CASE(if_cond_complex) {
	// Simple if-elif-else scenarios
	expect_str(simple_prep(U"#if 0\n42\n#elif 0\n84\n#elif 1\n99\n#else\n123\n#endif\n"_b, {}), U"\n99\n\n"_b);
	expect_str(simple_prep(U"#if 0\n42\n#elif 1\n84\n#elif 1\n99\n#else\n123\n#endif\n"_b, {}), U"\n84\n\n"_b);
	expect_str(simple_prep(U"#if 0\n42\n#elif 0\n84\n#elif 0\n99\n#else\n123\n#endif\n"_b, {}), U"\n123\n\n"_b);

	// Nested if-elif-else conditions
	expect_str(simple_prep(U"#if 1\n42\n#if 0\n24\n#elif 1\n33\n#endif\n#endif\n"_b, {}), U"\n42\n\n33\n\n\n"_b);
	expect_str(simple_prep(U"#if 0\n#if 1\n42\n#endif\n#else\n#if 1\n84\n#else\n123\n#endif\n#endif\n"_b, {}), U"\n\n84\n\n\n"_b);

	// Error handling
	expect_error(simple_prep(U"#if 1\n42\n#if 1\n#error Nested error\n#endif\n#endif\n"_b, {}), U"Nested error"_b);
	expect_str(simple_prep(U"#if 0\n#if 1\n#error Unreachable error\n#endif\n#endif\n"_b, {}), U"\n"_b);

	// Invalid syntax
	expect_error(simple_prep(U"#if 1\n42\n#invalid_directive\n99\n#endif\n"_b, {}), U"Unknown preprocessor directive 'invalid_directive'"_b);
	// 1 newline comes from #if inner block, other one is added before eof, because it's missing.
	expect_str(simple_prep(U"#if 1\n42\n#elif\n99\n#endif"_b, {}), U"\n42\n\n"_b); 
	expect_error(simple_prep(U"#if 0\n42\n#elif\n99\n#endif\n"_b, {}), U"Expected preprocessor expression"_b);

	// Complex whitespace handling
	expect_str(simple_prep(U"#if 1\n   42\n#if 1\n    24\n#endif\n  \n#endif\n"_b, {}), U"\n   42\n\n    24\n\n  \n\n"_b);
	expect_error(simple_prep(U"#if 1\n   42\n #  if 0\n 84\n#endif\n"_b, {}), U"#if is not closed"_b);

	// Test multiple elif conditions with whitespace
	expect_str(simple_prep(U"#if 0\n42\n#elif 0\n   84\n#elif 1\n 99\n#endif\n"_b, {}), U"\n 99\n\n"_b);

	// Testing with more complex expressions (if your preprocessor supports it)
	expect_str(simple_prep(U"#if (1 + 2 == 3)\n42\n#else\n99\n#endif\n"_b, {}), U"\n42\n\n"_b);
	expect_str(simple_prep(U"#if (1 + 2 == 4)\n42\n#else\n99\n#endif\n"_b, {}), U"\n99\n\n"_b);
	expect_error(simple_prep(U"#if (1 +)\n42\n#endif\n"_b, {}), U"Expected preprocessor expression"_b);
}
#endif
