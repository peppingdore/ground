#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#include "../grd_build.h"

#define JUMBO_EXCLUDE_MAIN 1
#include "build_tests/JUMBO_BUILD_test.cpp"

GRD_BUILD_RUN(R"CMD(
ctx.params.add_unit(__FILE__.parent / "no_multiple_definition_error_second_unit.cpp")
)CMD")

int main() {
	return 0;
}
