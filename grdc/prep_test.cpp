#include "preprocess.h"
#include "../grd_testing.h"

Tuple<Error*, GrdcPrep*> simple_preprocess(GrdUnicodeString str, GrdSpan<Tuple<GrdUnicodeString, GrdUnicodeString>> files) {
	auto prep = grd_make_prep();
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
	return { preprocess_file(prep, root), prep };
}

// GRD_TEST(include_add_missing_line_breaks) {
// 	GrdArray<Tuple<GrdUnicodeString, GrdUnicodeString>> files;
// 	grd_add(&files, { U"file.txt"_b, U"FILE_CONTENT"_b });
// 	auto [e, prep] = simple_preprocess(U"#include \"file.txt\""_b, files);
// 	if (e) {
// 		grdc_print_prep_error(e);
// 	}
// 	grd_println("prep_str: %", prep_str(prep));
// 	for (auto tok: prep->tokens) {
// 		grd_println("%, '%', %", tok->kind, grdc_tok_str(tok), tok->src_kind);
// 		if (tok->src_kind == PREP_TOKEN_SOURCE_INCLUDED_FILE) {
// 			grd_println("  %", tok->included_file->file->fullpath);
// 		}
// 	}
// 	// One newline for file.txt and other for <root_file>.
// 	EXPECT(prep_str(prep) == U"FILE_CONTENT\n\n"_b);
// }

// GRD_TEST(include_computed_include) {
//     GrdArray<Tuple<GrdUnicodeString, GrdUnicodeString>> files;
//     // Add test files with content
//     grd_add(&files, { U"computed_include.txt"_b, U"COMPUTED_CONTENT"_b });
//     // Preprocess code that includes a file via macro expansion
//     auto [e, prep] = simple_preprocess(
//         U"#define FILE_NAME \"computed_include.txt\"\n#include FILE_NAME"_b,
//         files
//     );
//     if (e) {
//         grdc_print_prep_error(e);
//     }
//     // Output the preprocessed string for verification
//     grd_println("prep_str: %", prep_str(prep));
//     // Expect the content of the included file followed by two newlines
//     EXPECT(prep_str(prep) == U"COMPUTED_CONTENT\n\n"_b);
// }


GRD_TEST(error_gen) {
	// auto [e, prep] = simple_preprocess(U"#define MACRO(a, b) a##b\n MACRO(2, \"a\")"_b, {});
	auto [e, prep] = simple_preprocess(U"#define MACRO(a, b) a##b\n MACRO(2, \"a\")"_b, {});
	if (e) {
		grdc_print_prep_error(e);
	}
	for (auto tok: prep->tokens) {
		grd_println("%, '%', %", tok->kind, grdc_tok_str(tok), tok->src_kind);
	}
	grd_println("prep_str: %", prep_str(prep));
}
