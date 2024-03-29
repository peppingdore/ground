#pragma once

#include "unicode_data.h"
#include "unicode_data_names.h"
#include "../../optional.h"
#include "../../file.h"
#include "../../log.h"
#include "../../string_conversion.h"
#include "../../reflect_utils.h"
#include "../../testing.h"

#include <stdio.h>

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

Array<String> synth_codepoint_unicode_data(u32 codepoint) {
	Array<String> synth;
	synth.add(codepoint_hex_padded(codepoint));

	UnicodeCodepoint cp;
	if (!lookup_unicode_codepoint((char32_t) codepoint, &cp)) {
		Log("Failed to lookup codepoint: %H", codepoint);
		return synth;
	}

	const char* name = UNICODE_CODEPOINT_NAMES[cp.name_index];
	synth.add(make_string(name));
	synth.add(sprint(cp.category));
	synth.add(sprint(cp.combining_class));
	synth.add(sprint(cp.bidi_category));

	auto sb = build_string();

	for (auto i: range(cp.decomposition.fields_count)) {
		auto field = cp.decomposition.fields[i];
		auto type = (EnumType*) reflect_type_of<UnicodeDecompositionTag>();
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
	return synth;
}

TEST(unicode_data) {
	auto [text, e] = read_text_at_path(U"../UnicodeData.txt"_b);
	if (e) {
		e->free();
		FAIL("Failed to read UnicodeData.txt");
		return;
	}

	s64 i = 0;
	auto iter = iterate_lines(text, false);
	for (auto line: iter) {
		if (line == ""_b) {
			continue;
		}
		auto original = to_array(split(line, lambda($0==';')));
		if (len(original) != 15) {
			FAIL(sprint("Expected length to be 15. Len: %", len(original)).data);
			return;
		}
		*original[10] = ""_b; // Unicode 1.0 name is not relevant.

		u32 codepoint;
		if (!parse_integer(*original[0], &codepoint, { .base = 16 })) {
			FAIL(sprint("Failed to parse codepoint: %", *original[0]).data);
			return;
		}

		if (ends_with(*original[1], "First>"_b)) {
			String next_line;
			if (!iter.next(&next_line)) {
				FAIL("Expected a line");
				return;
			}
			*original[1] = remove_suffix(remove_prefix(*original[1], "<"_b), ", First>"_b);
			auto fields = to_array(split(next_line, lambda($0==';')));
			if (len(fields) != 15) {
				FAIL("len(fields) != 15");
				return;
			}
			u32 last_codepoint;
			if (!parse_integer(*fields[0], &last_codepoint, { .base = 16 })) {
				FAIL("Can't parse last codepoint");
				return;
			}

			for (auto i: range_from_to(codepoint, last_codepoint + 1)) {
				auto synth = synth_codepoint_unicode_data(codepoint);
				if (synth != original) {
					FAIL(sprint("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth).data);
					return;
				}
			}
		}

		auto synth = synth_codepoint_unicode_data(codepoint);
		if (synth != original) {
			Log("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth);
			FAIL(sprint("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth).data);
			return;
		}
	}
	Log("Success!");
}
