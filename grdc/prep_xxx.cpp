#pragma once

#include "c_like_parser.h"

int main() {
	GrdUnicodeString prog = UR"XXXX(
	// comment
	int main() {
		// xxxx
		return 0;
	}
	)XXXX"_b;

	auto prep = grdc_make_preprocessor(c_allocator);
	auto e = grdc_prep_file(prep, prog, U"xxx.cpp"_b);
	if (e) {
		print_parser_error(e);
		return -1;
	}
	grd_println(prep->pr);
}
