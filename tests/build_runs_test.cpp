#include "../grd_build.h"
#include "../grd_testing.h"

GRD_TEST_CASE(build_run_simple) {
	GRD_BUILD_RUN("params.add_define((BUILD_RUN_SIMPLE, 5816431927631))")
	GRD_EXPECT_EQ(BUILD_RUN_SIMPLE, 5816431927631)
}

GRD_TEST_CASE(build_run_raw_literal) {
	GRD_BUILD_RUN(R"(params.add_define((BUILD_RUN_RAW_LITERAL, 5816431927631)))")
	GRD_EXPECT_EQ(BUILD_RUN_RAW_LITERAL, 5816431927631)
}
