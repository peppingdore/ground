#include "re2_wrapper.h"

#include "../re2/re2/re2.h"

void re2_wrapper_make(GrdString str, GrdError** error_dst, GrdRe2** re_dst) {
	auto view = std::string_view(str.data, grd_len(str));
	auto re2 = new re2::RE2(view, RE2::Quiet);
	if (!re2->ok()) {
		*(GrdError**) error_dst = grd_make_error<GrdRe2Error>(re2->error().c_str());
		return;
	}
	*re_dst = (GrdRe2*) re2;
}

void re2_free(GrdRe2* re) {
	delete (re2::RE2*) re;
}

bool re2_wrapper_match(GrdRe2* re, GrdString str, GrdReMatch* out_match) {
	auto x = (re2::RE2*) re;
	auto view = std::string_view(str.data, grd_len(str));
	s64 dst_len = x->NumberOfCapturingGroups() + 1;
	auto dst = new absl::string_view[dst_len];
	grd_defer { delete[] dst; };
	bool did_match = x->Match(view, 0, view.length(), RE2::UNANCHORED, dst, dst_len);
	if (!did_match) {
		return false;
	}
	GrdReMatch match;
	for (s64 i = 0; i < dst_len; ++i) {
		GrdReSubmatch submatch;
		submatch.start = dst[i].data() - str.data;
		submatch.len = dst[i].length();
		grd_add(&match.submatches, submatch);
	}
	*out_match = match;
	return true;
}
