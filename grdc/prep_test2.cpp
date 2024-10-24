// #define A(...) __VA_ARGS__
// #define C(a, b, c) a##b##c
// #define PARENL (
// #define PARENR )
// A(C PARENL a, b, c PARENR)

#include "preprocess.h"
#include "../grd_testing.h"
#include "../grd_format.h"

GRD_TEST(prep_splice) {
	Prep* p = NULL;
// 	GrdUnicodeString str = UR"RAW(line 1\
// line 2\
// line 3\
// line 4\
// line 5\
// line 6\
// line 7\
// line 8\
// line 9\
// line 10\)RAW"_b;

// 	auto p = grd_make_prep(c_allocator, str, U"test.txt"_b);
// 	splice_lines(p, 0, grd_len(p->src));
// 	print_prep_state(p);
// 	grd_println(p->src);

	GrdUnicodeString str2 = UR"RAW(
#define MACRO1 aboba
#define MACRO(x, y) x##y x y
#define MACRO2(x) #x
#define a DEFINED1
	MACRO( a , b )
	MACRO1
	MACRO2(a != b)
	#include "test.txt"
	)RAW"_b;
	p = grd_make_prep(c_allocator);
	p->load_file_hook = [](Prep* p, GrdUnicodeString path) -> PrepFile* {
		auto file = grd_make<PrepFile>(c_allocator);
		file->og_src = U"file content"_b;
		file->fullpath = path;
		return file;
	};
	auto file = grd_make_mem_prep_file(p, str2, U"/files/text.txt"_b);
	auto e = preprocess_file(p, file);
	grd_println(prep_str(p));
	if (e) {
		print_prep_error(e);
	}
}
