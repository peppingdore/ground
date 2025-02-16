#include "../grd_testing.h"
#include "../grd_format.h"

GRD_TEST_CASE(base_constants) {
	auto sign = grd_sign(GRD_NAN);
	GRD_EXPECT_EQ(sign, 1);

	sign = grd_sign(GRD_INFINITY);
	GRD_EXPECT_EQ(sign, 1);
}
