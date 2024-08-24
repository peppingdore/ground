#include "preprocess.h"
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
	)RAW"_b;
	p = make_prep(c_allocator, str2, U"test.txt"_b);
	preprocess(p, 0, len(p->src));
	print_prep_state(p);
	println(p->src);
}
