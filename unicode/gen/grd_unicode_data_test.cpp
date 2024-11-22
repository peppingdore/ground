#pragma once

#include "grd_unicode_data.h"
#include "grd_unicode_data_names.h"
#include "../../grd_optional.h"
#include "../../grd_file.h"
#include "../../grd_log.h"
#include "../../grd_reflect_utils.h"
#include "../../grd_testing.h"

#include <stdio.h>

GrdString grd_codepoint_hex_padded(int codepoint) {
	auto xxxx = grd_sprint("%H", codepoint);
	auto result = xxxx[2, {}];
	if (grd_len(result) < 4) {
		for (auto i: grd_range(4 - grd_len(result))) {
			result = grd_sprint("0%", result);
		}
	}
	return result;
}

GrdArray<GrdString> synth_codepoint_unicode_data(u32 codepoint) {
	GrdArray<GrdString> synth;
	grd_add(&synth, grd_codepoint_hex_padded(codepoint));

	GrdUnicodeCodepoint cp;
	if (!grd_lookup_unicode_codepoint((char32_t) codepoint, &cp)) {
		GrdLog("Failed to lookup codepoint: %H", codepoint);
		return synth;
	}

	const char* name = GRD_UNICODE_CODEPOINT_NAMES[cp.name_index];
	grd_add(&synth, grd_make_string(name));
	grd_add(&synth, grd_sprint(cp.category));
	grd_add(&synth, grd_sprint(cp.combining_class));
	grd_add(&synth, grd_sprint(cp.bidi_category));

	GrdAllocatedString sb;

	for (auto i: grd_range(cp.decomposition.fields_count)) {
		auto field = cp.decomposition.fields[i];
		auto type = (GrdEnumType*) grd_reflect_type_of<GrdUnicodeDecompositionTag>();
		auto matching_value = grd_find_matching_enum_value(type, grd_make_primitive_value(field));
		if (matching_value.has_value) {
			grd_append(&sb, "<");
			grd_append(&sb, matching_value.value.name);
			grd_append(&sb, ">");
		} else {
			grd_append(&sb, grd_codepoint_hex_padded(field));
		}
		if (i < cp.decomposition.fields_count - 1) {
			grd_append(&sb, " ");
		}
	}
	grd_add(&synth, sb);

	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_DECIMAL_DIGIT_VALUE) {
		grd_add(&synth, grd_sprint(cp.decimal_digit_value));
	} else {
		grd_add(&synth, ""_b);
	}

	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_DIGIT_VALUE) {
		grd_add(&synth, grd_make_string(GRD_UNICODE_DIGIT_VALUES[cp.digit_value_index]));
	} else {
		grd_add(&synth, ""_b);
	}

	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_NUMERIC_VALUE) {
		grd_add(&synth, grd_make_string(GRD_UNICODE_NUMERIC_VALUES[cp.numeric_value_index]));
	} else {
		grd_add(&synth, ""_b);
	}
	grd_add(&synth, cp.is_mirrored ? "Y"_b : "N"_b);
	grd_add(&synth, ""_b);
	grd_add(&synth, ""_b);
	
	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_UPPERCASE_MAPPING) {
		grd_add(&synth, grd_codepoint_hex_padded(cp.uppercase_mapping));
	} else {
		grd_add(&synth, ""_b);
	}

	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_LOWERCASE_MAPPING) {
		grd_add(&synth, grd_codepoint_hex_padded(cp.lowercase_mapping));
	} else {
		grd_add(&synth, ""_b);
	}

	if (cp.field_flags & GRD_UNICODE_CODEPOINT_FIELD_TITLECASE_MAPPING) {
		grd_add(&synth, grd_codepoint_hex_padded(cp.titlecase_mapping));
	} else {
		grd_add(&synth, ""_b);
	}
	return synth;
}

GRD_TEST_CASE(unicode_data) {
	GRD_EXPECT(grd_ends_with(U"<CJK Ideograph Extension A, First>"_b, "First>"_b));

	auto [text, e] = grd_read_text_at_path(U"../UnicodeData.txt"_b);
	if (e) {
		e->free();
		GRD_FAIL("Failed to read UnicodeData.txt");
		return;
	}

	s64 i = 0;
	auto iter = grd_iterate_lines(text, false);
	for (auto line: iter) {
		if (line == ""_b) {
			continue;
		}
		auto original = grd_to_array(grd_split(line, grd_lambda($._0==';')));
		if (grd_len(original) != 15) {
			GRD_FAIL(grd_sprint("Expected length to be 15. Len: %", grd_len(original)).data);
			return;
		}
		original[10] = ""_b; // Unicode 1.0 name is not relevant.

		u32 codepoint;
		if (!grd_parse_integer(original[0], &codepoint, { .base = 16 })) {
			GRD_FAIL(grd_sprint("Failed to parse codepoint: %", original[0]).data);
			return;
		}

		if (grd_ends_with(original[1], "First>"_b)) {
			GrdString next_line;
			if (!iter.next(&next_line)) {
				GRD_FAIL("Expected a line");
				return;
			}
			original[1] = grd_remove_suffix(grd_remove_prefix(original[1], "<"_b), ", First>"_b);
			auto fields = grd_to_array(grd_split(next_line, grd_lambda($._0==';')));
			if (grd_len(fields) != 15) {
				GRD_FAIL("len(fields) != 15");
				return;
			}
			u32 last_codepoint;
			if (!grd_parse_integer(fields[0], &last_codepoint, { .base = 16 })) {
				GRD_FAIL("Can't parse last codepoint");
				return;
			}

			for (auto i: grd_range_from_to(codepoint, last_codepoint + 1)) {
				auto synth = synth_codepoint_unicode_data(i);
				original[0] = grd_codepoint_hex_padded(i);
				if (synth != original) {
					GRD_FAIL(grd_sprint("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth).data);
					return;
				}
			}
			continue;
		}

		auto synth = synth_codepoint_unicode_data(codepoint);
		if (synth != original) {
			GrdLog("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth);
			GRD_FAIL(grd_sprint("Codepoint: %H diverges. \nOriginal-- %\nSynth   -- %", codepoint, original, synth).data);
			return;
		}
	}
	GrdLog("Success!");
}
