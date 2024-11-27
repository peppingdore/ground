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

void tests_to_add() {
	expect_error(simple_prep(U"#define"_b, {}), U"Expected an identifier after #define"_b);
	expect_str(simple_prep(U"#define AAAAA"_b, {}), U"\n"_b);
	expect_str(simple_prep(U"#define AAAAA '\nAAAAA a AAAAA"_b, {}), U"\n' a '\n"_b);
	expect_error(simple_prep(U"#if (1"_b, {}), U"Expected ')' in preprocessor expression"_b);
	expect_error(simple_prep(U"#if (1  "_b, {}), U"Expected ')' in preprocessor expression"_b);
}

GRD_TEST_CASE(xxxx) {
	
}
