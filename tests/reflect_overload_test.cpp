#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif
#include "../grd_testing.h"
#include "../grd_reflect.h"
#include "../grd_format.h"

struct A {
	int a = 10;
};

GRD_TEST_CASE(reflect_overload_test) {
	auto tp = grd_reflect_type_of<A>(); // make sure this call picks up GRD_REFLECT(A) below.
	GRD_EXPECT(strcmp(tp->name, "A") == 0, tp->name);
}

GRD_REFLECT(A) {
	GRD_MEMBER(a);
}
