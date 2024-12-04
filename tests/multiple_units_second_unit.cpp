#pragma once

#include "../grd_build.h"

GRD_BUILD_RUN(R"CODE(
ctx.params.add_define(('MULTIPLE_UNITS_SECOND_UNIT', 228))
)CODE")
#ifndef MULTIPLE_UNITS_SECOND_UNIT
	#define MULTIPLE_UNITS_SECOND_UNIT 0
#endif

extern "C" int multiple_units_second_unit() {
	return MULTIPLE_UNITS_SECOND_UNIT;
}
