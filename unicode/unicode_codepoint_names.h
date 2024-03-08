#pragma once

#include "unicode_data.h"
#include "generated_name_table.h"

const char* get_unicode_codepoint_name(UnicodeCodepoint codepoint) {
	return UNICODE_CODEPOINT_NAME_TABLE[codepoint.name_index];
}

const char* get_unicode_codepoint_old_name(UnicodeCodepoint codepoint) {
	if (codepoint.field_flags & UNICODE_CODEPOINT_FIELD_OLD_NAME) {
		return UNICODE_CODEPOINT_OLD_NAME_TABLE[codepoint.old_name_index];
	}
	return NULL;
}
