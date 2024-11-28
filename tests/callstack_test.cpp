#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#pragma once

#include "../grd_callstack.h"
#include "../grd_log.h"
#include "../grd_testing.h"

GRD_TEST_CASE(callstack) {
	auto cs = grd_get_callstack();
	grd_print_callstack_verbose(cs);
	GRD_EXPECT(false);
}
