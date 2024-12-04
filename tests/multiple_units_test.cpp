#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#include "../grd_testing.h"
#include "../grd_format.h"

GRD_BUILD_RUN(R"CODE(
from pathlib import Path
ctx.params.add_unit(Path(__FILE__).parent / 'multiple_units_second_unit.cpp')
)CODE")

extern "C" int multiple_units_second_unit();

GRD_TEST_CASE(multiple_units) {
	GRD_EXPECT_EQ(multiple_units_second_unit(), 228)
}
