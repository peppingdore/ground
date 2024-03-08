#pragma once

#include "unicode_data.h"

#define UNICODE_FIGURE_SPACE 0x2007
#define UNICODE_NBSP 0xa0
#define UNICODE_NARROW_NBSP 0x202f

bool is_unicode_codepoint_whitespace(UnicodeCodepoint cp) {
	if (cp.codepoint == UNICODE_FIGURE_SPACE ||
		cp.codepoint == UNICODE_NBSP ||
		cp.codepoint == UNICODE_NARROW_NBSP)
	{
		return false;
	}

	if (cp.category == UnicodeGeneralCategory::Zs ||
		cp.category == UnicodeGeneralCategory::Zl ||
		cp.category == UnicodeGeneralCategory::Zp)
	{
		return true;
	}
}

bool unicode_iswhitespace(char32_t c) {
	// @TODO:
}
