#pragma once

#include "grd_testing.h"
#include "grd_array.h"
#include "grd_sort.h"
#include "grd_format.h"

GRD_TEST(sort) {
	GrdArray<s64> arr;
	grd_add(&arr, {
		4,
		32,
		8,
		123,
		39120,
		23,
		9,
		21321,
		65,
	});

	grd_sort(arr);
	GRD_EXPECT(grd_is_sorted(arr));
}
