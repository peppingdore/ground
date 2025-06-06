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
	auto e = grdc_preprocess_file(prep, root);
	auto toks = grdc_get_tokens(prep);
	for (auto it: toks) {
		// GrdLogTrace("Token: %, %, %, %, %", grdc_tok_str(it), it->file_start, it->file_end, it->kind, it->src_kind);
		if (it->set->src_kind == GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE) {
			// GrdLogTrace("  Included file: % (%:%)", it->included_file->file->fullpath, it->included_file_og_tok->file_start, it->included_file_og_tok->file_end);
		}
	}
	return { e, prep };
}

GRD_TEST_CASE(simple_expansion) {
	expect_str(simple_prep(U"#define MACRO 42\nMACRO"_b, {}), U"\n42\n"_b);
}

GRD_TEST_CASE(function_like_macro) {
	expect_str(simple_prep(U"#define ADD(a, b) (a + b)\nADD(1, 2)"_b, {}), U"\n(1 + 2)\n"_b);
}

GRD_TEST_CASE(stringizing) {
	expect_str(simple_prep(U"#define STR(a) #a\nSTR(hello)"_b, {}), U"\n\"hello\"\n"_b);
}

GRD_TEST_CASE(token_pasting) {
	expect_str(simple_prep(U"#define PASTE(a,b) a##b\nPASTE(hel,lo)"_b, {}), U"\nhello\n"_b);
}

GRD_TEST_CASE(multi_token_paste) {
	expect_str(simple_prep(U"#define PASTE(a, b) a##b\nPASTE(1 2, 3 4)"_b, {}), U"\n1 23 4\n"_b);
}

GRD_TEST_CASE(va_args_paste) {
	expect_str(simple_prep(U"#define PASTE(a, b) a##b\n#define VA_ARGS(...) __VA_ARGS__\nPASTE(1 2, VA_ARGS(3, 4))"_b, {}), U"\n\n1 2VA_ARGS(3, 4)\n"_b);
}

GRD_TEST_CASE(variadic_macro) {
	expect_str(simple_prep(U"#define VARIADIC(...) __VA_ARGS__\nVARIADIC(1, 2, 3)"_b, {}), U"\n1, 2, 3\n"_b);
}

GRD_TEST_CASE(pasting) {
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1, 2, 3)"_b, {}), U"\n12, 3\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1)"_b, {}), U"\n1\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE()"_b, {}), U"\n\n"_b);
	expect_str(simple_prep(U"#define PASTE(a, ...) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"\n1 23 4\n"_b);
	expect_error(simple_prep(U"#define PASTE(a, ..., ...) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"Duplicate macro argument"_b);
	expect_error(simple_prep(U"#define PASTE(a) a##__VA_ARGS__\nPASTE(1 2, 3 4)"_b, {}), U"Expected 1 argument(s) at most for a macro"_b);
}

GRD_TEST_CASE(undefined_macro) {
	expect_str(simple_prep(U"UNDEFINED_MACRO"_b, {}), U"UNDEFINED_MACRO\n"_b);
}

GRD_TEST_CASE(invalid_token_concat) {
	expect_error(simple_prep(U"#define MACRO(a, b) a##b\nMACRO(1, \"2\")"_b, {}), U"doesn't form a valid token"_b);
}

GRD_TEST_CASE(file_include) {
	GrdArray<GrdTuple<GrdUnicodeString, GrdUnicodeString>> files;
	grd_add(&files, { U"header.h"_b, U"#define HEADER_DEF 100\n"_b });
	auto [e, prep] = simple_prep(U"#include \"header.h\"\nHEADER_DEF"_b, files);
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\n\n100\n"_b));
		output.free();
	}
	files.free();
}

GRD_TEST_CASE(recursive_include) {
	// Test recursive inclusion (should handle gracefully)
	GrdArray<GrdTuple<GrdUnicodeString, GrdUnicodeString>> files;
	grd_add(&files, { U"file1.h"_b, U"#include \"file2.h\"\n#define DEF1 1\n"_b });
	grd_add(&files, { U"file2.h"_b, U"#include \"file1.h\"\n#define DEF2 2\n"_b });
	auto [e, prep] = simple_prep(U"#include \"file1.h\"\nDEF1\nDEF2"_b, files);
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		// The preprocessor should prevent infinite recursion and include each file once
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\n\n\n\n\n1\n2\n"_b));
		output.free();
	}
	files.free();
}

GRD_TEST_CASE(comment_removal) {
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

GRD_TEST_CASE(line_splicing_removal) {
	// Test line splicing removal
	auto [e, prep] = simple_prep(U"#define TEST \\\n42\nTEST"_b, {});
	GRD_EXPECT(!e, e);
	if (!e) {
		GrdAllocatedUnicodeString output = grdc_prep_str(prep);
		GRD_EXPECT_EQ(test_escape_string(output), test_escape_string(U"\n42\n"_b));
		output.free();
	}
}

GRD_TEST_CASE(error_directive) {
	// Test handling of #error directive (if implemented)
	// Assuming your preprocessor handles #error
	auto [e, prep] = simple_prep(U"#  error This is an error message\nint k = 43;"_b, {});
	GRD_EXPECT(e, e);
	if (e) {
		GrdLog(e);
		GRD_EXPECT(grd_contains(e->text, "This is an error message"), grd_copy_c_str(e->text));
	}
}

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

GRD_TEST_CASE(errors) {
	expect_error(simple_prep(U"#define"_b, {}), U"Expected an identifier after #define"_b);
	expect_str(simple_prep(U"#define AAAAA"_b, {}), U"\n"_b);
	expect_str(simple_prep(U"#define AAAAA '\nAAAAA a AAAAA"_b, {}), U"\n' a '\n"_b);
	expect_error(simple_prep(U"#if (1"_b, {}), U"Expected ')' in preprocessor expression"_b);
	expect_error(simple_prep(U"#if (1  "_b, {}), U"Expected ')' in preprocessor expression"_b);
}

GRD_TEST_CASE(function_like_macro_2) {
    expect_str(simple_prep(U"#define ADD(a, b) (a + b)\nADD(1, 2)"_b, {}), U"\n(1 + 2)\n"_b);
}

GRD_TEST_CASE(stringizing_2) {
    expect_str(simple_prep(U"#define STR(a) #a\nSTR(hello)"_b, {}), U"\n\"hello\"\n"_b);
}

GRD_TEST_CASE(detect_parentheses) {
	auto base = UR"CODE(
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)
)CODE"_b;

    expect_str(simple_prep(grd_concat(base, U"#define TEST IS_PAREN(())\nTEST"_b), {}), U"\n\n\n\n\n\n\n1\n"_b);
    expect_str(simple_prep(grd_concat(base, U"#define TEST IS_PAREN(xxx)\nTEST"_b), {}), U"\n\n\n\n\n\n\n0\n"_b);
}

// https://mailund.dk/posts/macro-metaprogramming/
// @TODO.
GRD_TEST_CASE(mailmund_dk_posts_meta_programming) {
	expect_str(simple_prep(UR"CODE(
#define s(x) #x
#define bar(x) 2 * x
#define foo(f, x) s(f(x))
foo(bar, 42)
)CODE"_b, {}), U"\n\n\n\n\"bar(42)\"\n"_b);

	expect_str(simple_prep(UR"CODE(
#define baz() bar
baz()
)CODE"_b, {}), U"\n\nbar\n"_b);

	expect_str(simple_prep(UR"CODE(
#define s(x) #x
#define bar(x) 2 * x
#define baz() bar
#define foo(f, x) #f s(f) #x s(x)
foo(baz(), 42)
)CODE"_b, {}), U"\n\n\n\n\n\"baz()\" \"bar\" \"42\" \"42\"\n"_b);
}

GRD_TEST_CASE(retarded_if_macro) {
	expect_str(simple_prep(UR"CODE(
#define if_else(b) if_##b
#define if_0(...) else_0
#define if_1(...) __VA_ARGS__ else_1
#define else_0(...) __VA_ARGS__
#define else_1(...)
if_else(0)(abort())(printf("yeah!\n"));
if_else(1)(printf("also yeah!\n"))(abort());
)CODE"_b, {}), U"\n\n\n\n\n\nprintf(\"yeah!\\n\");\nprintf(\"also yeah!\\n\") ;\n"_b);
}

GRD_TEST_CASE(retarded_if_macro_2) {
	expect_str(simple_prep(UR"CODE(
#define if_else(b) if_##b
#define if_0(...) else_0
#define if_1(...) __VA_ARGS__ else_1
#define else_0(...) __VA_ARGS__
#define else_1(...)
#define second(a, b, ...) b
#define test(p) second(p, 0)
#define is_true() -, 1
test(x) // 0
test(is_true()) // 1
)CODE"_b, {}), U"\n\n\n\n\n\n\n\n\n0 \n1 \n"_b);
}

GRD_TEST_CASE(retarded_if_macro_3) {
	expect_str(simple_prep(UR"CODE(
#define if_else(b) if_##b
#define if_0(...) else_0
#define if_1(...) __VA_ARGS__ else_1
#define else_0(...) __VA_ARGS__
#define else_1(...)
#define second(a, b, ...) b
#define test(p) second(p, 0)
#define is_true() -, 1
#define cat(a,b) a##b
#define not(b) test(cat(not_,b))
#define not_0 is_true()
#define truthiness(b) not(not(b))
#define foo(x) foo_ ## x
truthiness(foo) // => 1
truthiness(0)   // => 0
truthiness(1)   // => 1
)CODE"_b, {}), U"\n\n\n\n\n\n\n\n\n\n\n\n\n\n1 \n0   \n1   \n"_b);
}


GRD_TEST_CASE(preprocessor_map) {
		expect_str(simple_prep(UR"CODE(
#define if_else(b) if_##b
#define if_0(...) else_0
#define if_1(...) __VA_ARGS__ else_1
#define else_0(...) __VA_ARGS__
#define else_1(...)
#define second(a, b, ...) b
#define test(p) second(p, 0)
#define is_true() -, 1
#define cat(a,b) a##b
#define not(b) test(cat(not_,b))
#define not_0 is_true()
#define truthiness(b) not(not(b))

#define if_else(b) if_else_(truthiness(b))
#define if_else_(b) cat(if_,b)

#define expand_1(...) __VA_ARGS__
#define expand_2(...) expand_1(expand_1(__VA_ARGS__))
#define expand_4(...) expand_2(expand_2(__VA_ARGS__))
#define expand_8(...) expand_4(expand_4(__VA_ARGS__))
#define expand_16(...) expand_8(expand_8(__VA_ARGS__))
#define eval expand_16

#define head(a, ...) a
#define tail(a, ...) __VA_ARGS__
#define first(a, ...) a
#define is_nil(...) test(first(is_true __VA_ARGS__)())
#define empty()
#define delay2(f) f empty empty()()
#define map(f, ...) \
    eval(map_(f, __VA_ARGS__))
#define map__() map_
#define map_(f, ...) \
    if_else(is_nil(__VA_ARGS__))\
    ()\
    (f(head(__VA_ARGS__)) \
     delay2(map__)()(f, tail(__VA_ARGS__)))

#define foo(x) f(x),

int x[] = { map(foo, x, y, z) };

)CODE"_b, {}), U"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nint x[] = { f(x),      f(y),      f(z),        };\n\n"_b);
}
#if 0
#define if_else(b) if_##b
#define if_0(...) else_0
#define if_1(...) __VA_ARGS__ else_1
#define else_0(...) __VA_ARGS__
#define else_1(...)
#define second(a, b, ...) b
#define test(p) second(p, 0)
#define is_true() -, 1
#define cat(a,b) a##b
#define not(b) test(cat(not_,b))
#define not_0 is_true()
#define truthiness(b) not(not(b))

#define if_else(b) if_else_(truthiness(b))
#define if_else_(b) cat(if_,b)

#define expand_1(...) __VA_ARGS__
#define expand_2(...) expand_1(expand_1(__VA_ARGS__))
#define expand_4(...) expand_2(expand_2(__VA_ARGS__))
#define expand_8(...) expand_4(expand_4(__VA_ARGS__))
#define expand_16(...) expand_8(expand_8(__VA_ARGS__))
#define eval expand_16

#define head(a, ...) a
#define tail(a, ...) __VA_ARGS__
#define first(a, ...) a
#define is_nil(...) test(first(is_true __VA_ARGS__)())
#define empty()
#define delay2(f) f empty empty()()
#define map(f, ...) \
    eval(map_(f, __VA_ARGS__))
#define map__() map_
#define map_(f, ...) \
    if_else(is_nil(__VA_ARGS__))\
    ()\
    (f(head(__VA_ARGS__)) \
     delay2(map__)()(f, tail(__VA_ARGS__)))


#define foo(x) f(x),

int x[] = { map(foo, x, y, z) };
#endif

GRD_TEST_CASE(gcc_replacement_example) {
	 expect_str(simple_prep(U" #define foo(x) bar x\nfoo(foo) (2)"_b, {}), U" \nbar foo (2)\n"_b);
}
