#pragma once

#include "c_like_parser.h"

int main() {
	UnicodeString prog = UR"XXXX(
	// comment
	int main() {
		// xxxx
		return 0;
	}
	)XXXX"_b;

	auto prep = make_preprocessor(c_allocator);
	auto e = prep_file(prep, prog, U"xxx.cpp"_b);
	if (e) {
		print_parser_error(e);
		return -1;
	}
	println(prep->pr);
}
