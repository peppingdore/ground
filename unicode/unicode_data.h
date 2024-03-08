#pragma once

#include "generated_codepoint_table.h"
#include "generated_data_types.h"

#include "../range.h"
#include "../optional.h"

struct UnicodeDecompositionMapping {
	int        fields_count;
	const int* fields;
};

enum UnicodeCodepointFieldFlag {
	UNICODE_CODEPOINT_FIELD_NAME = 1 << 1,
	UNICODE_CODEPOINT_FIELD_CATEGORY = 1 << 2,
	UNICODE_CODEPOINT_FIELD_COMBINING_CLASS = 1 << 3,
	UNICODE_CODEPOINT_FIELD_BIDI_CATEGORY = 1 << 4,
	UNICODE_CODEPOINT_FIELD_DECOMPOSITION_MAPPING = 1 << 5,
	UNICODE_CODEPOINT_FIELD_DECIMAL_DIGIT_VALUE = 1 << 6,
	UNICODE_CODEPOINT_FIELD_DIGIT_VALUE = 1 << 7,
	UNICODE_CODEPOINT_FIELD_NUMERIC_VALUE = 1 << 8,
	UNICODE_CODEPOINT_FIELD_IS_MIRRORED = 1 << 9,
	UNICODE_CODEPOINT_FIELD_OLD_NAME = 1 << 10,
	UNICODE_CODEPOINT_FIELD_UPPERCASE_MAPPING = 1 << 12,
	UNICODE_CODEPOINT_FIELD_LOWERCASE_MAPPING = 1 << 13,
	UNICODE_CODEPOINT_FIELD_TITLECASE_MAPPING = 1 << 14,
};

struct UnicodeCodepoint {
	char32_t                    codepoint = 0;
	int                         field_flags = 0;
	int                         name_index = 0;
	UnicodeGeneralCategory      category = {};
	int                         combining_class = 0;
	UnicodeBidiCategory         bidi_category = {};
	UnicodeDecompositionMapping decomposition = {};
	int                         decimal_digit_value = 0;
	int                         digit_value_index = 0;
	int                         numeric_value_index = 0;
	int                         old_name_index = 0;
	int                         uppercase_mapping = 0;
	int                         lowercase_mapping = 0;
	int                         titlecase_mapping = 0;
};

Optional<UnicodeCodepoint> decode_unicode_codepoint(char32_t codepoint, int table_index) {
	auto offset = UNICODE_CODEPOINTS_OFFSETS_INTO_PACKED[table_index];
	if (offset == -1) {
		return {};
	}

	UnicodeCodepoint cp;
	cp.codepoint = codepoint;
	cp.name_index = table_index;
	cp.field_flags = offset == -2 ? 0 : UNICODE_PACKED_CODEPOINTS[offset];
	cp.category      = (UnicodeGeneralCategory) ((UNICODE_CATEGORY_MASK      & cp.field_flags) >> UNICODE_CATEGORY_SHIFT);
	cp.bidi_category = (UnicodeBidiCategory)    ((UNICODE_BIDI_CATEGORY_MASK & cp.field_flags) >> UNICODE_BIDI_CATEGORY_SHIFT);

	if (offset == -2) {
		return cp;
	}
	auto packed = &UNICODE_PACKED_CODEPOINTS[offset];
	int cursor = 1;
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_COMBINING_CLASS) {
		cp.combining_class = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DECOMPOSITION_MAPPING) {
		int count = packed[cursor++];
		cp.decomposition.fields_count = count;
		cp.decomposition.fields = &packed[cursor];
		cursor += count;
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DECIMAL_DIGIT_VALUE) {
		cp.decimal_digit_value = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_DIGIT_VALUE) {
		cp.digit_value_index = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_NUMERIC_VALUE) {
		cp.numeric_value_index = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_OLD_NAME) {
		cp.old_name_index = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_UPPERCASE_MAPPING) {
		cp.uppercase_mapping = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_LOWERCASE_MAPPING) {
		cp.lowercase_mapping = packed[cursor++];
	}
	if (cp.field_flags & UNICODE_CODEPOINT_FIELD_TITLECASE_MAPPING) {
		cp.titlecase_mapping = packed[cursor++];
	}
	return cp;
}

Optional<UnicodeCodepoint> lookup_unicode_codepoint(char32_t codepoint) {
	for (auto range: UNICODE_NONUNIFORM_CODEPOINT_RANGES) {
		if (codepoint >= range.first_codepoint && codepoint < range.first_codepoint + range.codepoints_count) {
			int table_index = range.codepoint_table_offset + (codepoint - range.first_codepoint);
			return decode_unicode_codepoint(codepoint, table_index);
		}
	}
	for (auto range: UNICODE_UNIFORM_CODEPOINT_RANGES) {
		if (codepoint >= range.first_codepoint && codepoint < range.first_codepoint + range.codepoints_count) {
			return decode_unicode_codepoint(codepoint, range.codepoint_table_offset);
		}
	}
	return {};
}
