#include "preprocess2.h"
#include "../testing.h"
#include "../format.h"

TEST(prep_splice) {
	Prep* p = NULL;
// 	UnicodeString str = UR"RAW(line 1\
// line 2\
// line 3\
// line 4\
// line 5\
// line 6\
// line 7\
// line 8\
// line 9\
// line 10\)RAW"_b;

// 	auto p = make_prep(c_allocator, str, U"test.txt"_b);
// 	splice_lines(p, 0, len(p->src));
// 	print_prep_state(p);
// 	println(p->src);

	UnicodeString str2 = UR"RAW(
#define MACRO1 aboba
#define MACRO(x, y) x##y
#define MACRO2(x) #x
    MACRO(a, b)
	MACRO1
	MACRO2(a != b)
	#include "test.txt"
	)RAW"_b;
	p = make_prep(c_allocator);
	p->load_file_hook = [](Prep* p, UnicodeString path) -> PrepFile* {
		auto file = make<PrepFile>(c_allocator);
		file->src = U"file content"_b;
		file->fullpath = path;
		return file;
	};
	auto file = make_mem_prep_file(p, str2, U"/files/text.txt"_b);
	auto e = preprocess(p, file);
	println(prep_str(p));
	if (e) {
		print_prep_error(e);
	}
}
