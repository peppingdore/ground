#pragma once

#include "../../grd_string.h"
#include "../../grd_tuple.h"
#include "../../grd_error.h"

struct GrdRe2 {};
struct GrdRe2Error: GrdError {};
GRD_REFLECT(GrdRe2Error) { GRD_BASE_TYPE(GrdError) }

struct GrdReSubmatch {
	s64 start;
	s64 len;
};

struct GrdReMatch {
	GrdArray<GrdReSubmatch> submatches;
};

GRD_EXPORT_C void re2_wrapper_make(GrdString str, GrdError** error_dst, GrdRe2** re_dst);
GRD_EXPORT_C bool re2_wrapper_match(GrdRe2* re, GrdString str, GrdReMatch* out_match);
