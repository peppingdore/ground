#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#pragma once

#include "../grd_stack_trace.h"
#include "../grd_log.h"
#include "../grd_testing.h"
#include "../grd_string.h"
#include "../grd_format.h"

GrdStackTrace deep_func() {
	return grd_get_stack_trace();
}

GrdStackTrace func() {
	return deep_func();
}

GRD_TEST_CASE(stack_trace) {
	auto st = func();
	char buf[4 * 1024];
	buf[0] = '\0';
	grd_print_stack_trace(st, buf, sizeof(buf));
	auto str = grd_make_string(buf);
	grd_println(str);
	GRD_EXPECT(grd_contains(str, "func"), str);
	GRD_EXPECT(grd_contains(str, "deep_func"), str);
	GRD_EXPECT(grd_contains(str, "main"), str);
}
