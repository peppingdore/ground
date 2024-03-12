#pragma once

#include "unicode_data.h"
#include "unicode_data_names.h"
#include "../../optional.h"
#include "../../file.h"
#include "../../log.h"

#include <stdio.h>

// #include "../../testing_light.h"

// TEST(unicode_base) {

// }





void print_codepoint(UnicodeCodepoint cp) {
	printf("Codepoint: %x\n", cp.codepoint);
	printf("Field flags: %x\n", cp.field_flags);
	printf("Name index: %x\n", cp.name_index);
	printf("Name: %s\n", UNICODE_CODEPOINT_NAMES[cp.name_index]);
	printf("General category: %d\n", (int) cp.category);
	printf("Combinding class: %d\n", cp.combining_class);
	printf("Bidi category: %d\n", (int) cp.bidi_category);
	printf("Decomposition fields count: %d\n", cp.decomposition.fields_count);
	for (int i = 0; i < cp.decomposition.fields_count; i++) {
		printf("  Decomposition field: %x\n", cp.decomposition.fields[i]);
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DECIMAL_DIGIT_VALUE) {
		printf("Decimal value: %d\n", cp.decimal_digit_value);
	}
	printf("Digit value index: %d\n", cp.digit_value_index);
	printf("Numeric value index: %d\n", cp.numeric_value_index);
	printf("Uppercase: %x\n", cp.uppercase_mapping);
	printf("Lowercase: %x\n", cp.lowercase_mapping);
	printf("Titlecase: %x\n", cp.titlecase_mapping);
}

void check_codepoint(char32_t c) {
	UnicodeCodepoint cp;
	bool found = lookup_unicode_codepoint(c, &cp);
	if (found) {
		print_codepoint(cp);
	} else {
		printf("cp not found\n");
	}
}

int main() {
	check_codepoint(0);
	check_codepoint('\n');
	check_codepoint(0x0020);
	check_codepoint(0x0826);
	check_codepoint(0xE01CA);

	auto [f, e] = open_file(U"UnicodeData.txt"_b);
	if (e) {
		Log("Failed to open UnicodeData.txt, %*", e);
		e->free();
		return -1;
	}

	return 0;
}
