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

GrdError* expect_error(GrdTuple<GrdError*, GrdcPrep*> x, GrdUnicodeString expected, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(loc);
	auto [e, p] = x;
	GRD_EXPECT(e);
	if (e) {
		GRD_EXPECT(grd_contains(e->text, expected), e);
	}
	grd_tester_scope_pop();
	return e;
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
	
}

bool          did_setup = false;
GrdArray<s64> expected_lines;
GrdArray<s64> lines_got;

GrdError* count_error_in(GrdTuple<GrdError*, GrdcPrep*> x, GrdUnicodeString expected, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(grd_current_loc());
	auto e = expect_error(x, expected, loc);
	if (e) {
		if (auto x = grd_reflect_cast<GrdcPrepDetailedError>(e)) {
			
		}
		grd_add(&lines_got, e->loc.line);
	}
	grd_tester_scope_pop();
	return e;
}

GRD_TEST_CASE(trigger_every_error_setup) {
	auto fpath = grd_copy_unicode_string(grd_make_string(__FILE__));
	GrdLogTrace("File: %"_b, fpath);
	auto path = grd_path_join({ grd_path_parent(fpath), U"grdc_preprocess.h"_b });
	GrdLogTrace("Path: %", path);
	auto [txt, e] = grd_read_text_at_path(path);
	GRD_EXPECT(!e, e);
	if (e) {
		return;
	}
	s64 line = 1;
	for (auto it: grd_iterate_lines(txt)) {
		if (grd_contains(it, U"grdc_make_prep_file_error"_b)) {
			grd_add(&expected_lines, line);
		}
		line += 1;
	}
	if (grd_len(expected_lines) > 0) {
		grd_remove(&expected_lines, 0);
	}
	GrdLogTrace("Found %", expected_lines);
	did_setup = true;
}

GRD_TEST_CASE(trigger_errors) {
	count_error_in(simple_prep(U"#define MACRO(a)\n MACRO"_b, {}), U"Expected '(' at the start of macro arguments"_b);
}

GRD_TEST_CASE(trigger_every_error_results) {
	if (!did_setup) {
		return;
	}
	grd_sort(expected_lines);
	grd_sort(lines_got);
	GRD_EXPECT_EQ(grd_len(expected_lines), grd_len(lines_got), "Expected %, got %", expected_lines, lines_got);
}
