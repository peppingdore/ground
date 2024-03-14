#pragma once

#include "unicode_data.h"
#include "unicode_data_names.h"
#include "../../optional.h"
#include "../../file.h"
#include "../../log.h"
#include "../../string_conversion.h"
#include "../../reflect_utils.h"

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

String codepoint_hex_padded(int codepoint) {
	auto result = sprint("%H", codepoint);
	result = slice(result, 2);
	if (len(result) < 4) {
		for (auto i: range(4 - len(result))) {
			result = sprint("0%", result);
		}
	}
	return result;
}

int main() {
	// check_codepoint(0);
	// check_codepoint('\n');
	// check_codepoint(0x0020);
	// check_codepoint(0x0826);
	// check_codepoint(0xE01CA);

	auto [text, e] = read_text_at_path(U"UnicodeData.txt"_b);
	if (e) {
		Log("Failed to read UnicodeData.txt, %*", e);
		e->free();
		return -1;
	}

	s64 i = 0;
	auto iter = iterate_lines(text, false);
	for (auto line: iter) {
		if (line == ""_b) {
			continue;
		}
		auto original = to_array(split(line, lambda($0==';')));
		if (len(original) != 15) {
			Log("Expected length to be 15");
			Log("--- %", line);
			Log("Len: %", len(line));
			continue;
		}
		*original[10] = ""_b; // Unicode 1.0 name is not relevant.

		if (ends_with(*original[1], "First>"_b)) {

		}

		// if (i++ > 10) {
			// break;
		// }

		Array<String> synth;

		u32 codepoint;
		if (!parse_integer(*original[0], &codepoint, { .base = 16 })) {
			Log("Failed to parse codepoint: %", *original[0]);
			continue;
		}

		synth.add(codepoint_hex_padded(codepoint));

		UnicodeCodepoint cp;
		if (!lookup_unicode_codepoint((char32_t) codepoint, &cp)) {
			Log("Failed to lookup codepoint: %H", codepoint);
			continue;
		}

		const char* name = UNICODE_CODEPOINT_NAMES[cp.name_index];
		synth.add(make_string(name));
		synth.add(sprint(cp.category));
		synth.add(sprint(cp.combining_class));
		synth.add(sprint(cp.bidi_category));

		auto sb = build_string();
		defer { sb.free(); };

		for (auto i: range(cp.decomposition.fields_count)) {
			auto field = cp.decomposition.fields[i];
			auto type = (EnumType*) reflect.type_of<UnicodeDecompositionTag>();
			auto matching_value = find_matching_enum_value(type, make_primitive_value(field));
			if (matching_value.has_value) {
				sb.append("<");
				sb.append(matching_value.value.name);
				sb.append(">");
			} else {
				sb.append(codepoint_hex_padded(field));
			}
			if (i < cp.decomposition.fields_count - 1) {
				sb.append(" ");
			}
		}
		synth.add(sb.get_string());

		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DECIMAL_DIGIT_VALUE) {
			synth.add(sprint(cp.decimal_digit_value));
		} else {
			synth.add(""_b);
		}

		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DIGIT_VALUE) {
			synth.add(make_string(UNICODE_DIGIT_VALUES[cp.digit_value_index]));
		} else {
			synth.add(""_b);
		}

		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_NUMERIC_VALUE) {
			synth.add(make_string(UNICODE_NUMERIC_VALUES[cp.numeric_value_index]));
		} else {
			synth.add(""_b);
		}
		synth.add(cp.is_mirrored ? "Y"_b : "N"_b);
		synth.add(""_b);
		synth.add(""_b);
		
		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_UPPERCASE_MAPPING) {
			synth.add(codepoint_hex_padded(cp.uppercase_mapping));			
		} else {
			synth.add(""_b);
		}

		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_LOWERCASE_MAPPING) {
			synth.add(codepoint_hex_padded(cp.lowercase_mapping));			
		} else {
			synth.add(""_b);
		}

		if (cp.field_flags & UNICODE_CODEPOINT_FIELD_TITLECASE_MAPPING) {
			synth.add(codepoint_hex_padded(cp.titlecase_mapping));			
		} else {
			synth.add(""_b);
		}

		if (synth != original) {
			Log("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth);
		}
	}

	return 0;
}
