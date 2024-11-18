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
	GRD_EXPECT_EQ(grdc_prep_str(p), expected);
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

GRD_TEST(simple_expansion) {
	auto [e, prep] = simple_prep(U"#define MACRO 42\nMACRO"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"42\n"_b));
		output.free();
	}
}

GRD_TEST(function_like_macro) {
	auto [e, prep] = simple_prep(U"#define ADD(a, b) (a + b)\nADD(1, 2)"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"(1 + 2)\n"_b));
		output.free();
	}
}

GRD_TEST(stringizing) {
	auto [e, prep] = simple_prep(U"#define STR(a) #a\nSTR(hello)"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\"hello\"\n"_b));
		output.free();
	}
}

GRD_TEST(token_pasting) {
	auto [e, prep] = simple_prep(U"#define PASTE(a,b) a##b\nPASTE(hel,lo)"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"hello\n"_b));
		output.free();
	}
}

GRD_TEST(multi_token_paste) {
	auto [e, prep] = simple_prep(U"#define PASTE(a, b) a##b\nPASTE(1 2, 3 4)"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		auto output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"1 23 4\n"_b));
	}
}

GRD_TEST(va_args_paste) {
	auto [e, prep] = simple_prep(U"#define PASTE(a, b) a##b\n#define VA_ARGS(...) __VA_ARGS__\nPASTE(1 2, VA_ARGS(3, 4))"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		auto output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"1 2VA_ARGS(3, 4)\n"_b));
	}
}

GRD_TEST(variadic_macro) {
	auto [e, prep] = simple_prep(U"#define VARIADIC(...) __VA_ARGS__\nVARIADIC(1, 2, 3)"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"1, 2, 3\n"_b));
		output.free();
	}
}

GRD_TEST(pasting) {
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1, 2, 3)"_b, {}), U"12, 3\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1)"_b, {}), U"1\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE()"_b, {}), U"\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"1 23 4\n"_b);
	expect_error(simple_prep(U"#define PASTE(a, ..., ...) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"Duplicate macro argument"_b);
	expect_error(simple_prep(U"#define PASTE(a) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"Expected 1 argument(s) at most for a macro"_b);
}

GRD_TEST(undefined_macro) {
	auto [e, prep] = simple_prep(U"UNDEFINED_MACRO"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		// Since the macro is undefined, it should remain as is
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"UNDEFINED_MACRO\n"_b));
		output.free();
	}
}

GRD_TEST(invalid_token_concat) {
	auto [e, prep] = simple_prep(U"#define MACRO(a, b) a##b\nMACRO(1, \"2\")"_b, {});
	GRD_EXPECT(e);
	if (e) {
		GRD_EXPECT(grd_contains(e->text, "doesn't form a valid token"));
		grdc_print_prep_error(e);
	}
}

GRD_TEST(file_include) {
	GrdArray<GrdTuple<GrdUnicodeString, GrdUnicodeString>> files;
	grd_add(&files, { U"header.h"_b, U"#define HEADER_DEF 100\n"_b });
	auto [e, prep] = simple_prep(U"#include \"header.h\"\nHEADER_DEF"_b, files);
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\n100\n"_b));
		output.free();
	}
	files.free();
}

GRD_TEST(recursive_include) {
	// Test recursive inclusion (should handle gracefully)
	GrdArray<GrdTuple<GrdUnicodeString, GrdUnicodeString>> files;
	grd_add(&files, { U"file1.h"_b, U"#include \"file2.h\"\n#define DEF1 1\n"_b });
	grd_add(&files, { U"file2.h"_b, U"#include \"file1.h\"\n#define DEF2 2\n"_b });
	auto [e, prep] = simple_prep(U"#include \"file1.h\"\nDEF1\nDEF2"_b, files);
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		// The preprocessor should prevent infinite recursion and include each file once
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\n\n\n1\n2\n"_b));
		output.free();
	}
	files.free();
}

GRD_TEST(comment_removal) {
	// Test comment removal
	auto [e, prep] = simple_prep(U"int a; // This is a comment\nint b; /* Multi-line\nComment */\nint c;"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		// Comments should be removed
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"int a; \nint b; \nint c;\n"_b));
		output.free();
	}
}

GRD_TEST(line_splicing_removal) {
	// Test line splicing removal
	auto [e, prep] = simple_prep(U"#define TEST \\\n42\nTEST"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"42\n"_b));
		output.free();
	}
}

GRD_TEST(error_directive) {
	// Test handling of #error directive (if implemented)
	// Assuming your preprocessor handles #error
	auto [e, prep] = simple_prep(U"#error This is an error message\nint k = 43;"_b, {});
	GRD_EXPECT(e, e);
	if (e) {
		GrdLog(e);
		GRD_EXPECT(grd_contains(e->text, "This is an error message"), grd_copy_c_str(e->text));
	}
}
