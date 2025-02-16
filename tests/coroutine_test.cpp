#include "../grd_testing.h"
#include "../grd_coroutine.h"
#include "../grd_array.h"
#include "../grd_format.h"

GRD_TEST_CASE(coroutine_test) {
	auto proc = []() -> GrdGenerator<int> {
		co_yield 1;
		co_yield 2;
		co_yield 3;
	};

	GrdArray<s32> arr;

	for (auto i : proc()) {
		grd_add(&arr, i);
	}
	auto truth = { 1, 2, 3 };
	GRD_EXPECT_EQ(arr, truth); 
}

GRD_TEST_CASE(coroutine_test_map) {
	auto res = grd_to_array(grd_map(std::initializer_list<int> { 1, 2, 3, 4, 5 }, grd_lambda(x, x + 1)));
	auto truth = { 2, 3, 4, 5, 6 };
	GRD_EXPECT_EQ(res, truth); 
}
