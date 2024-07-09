#pragma once

#include "testing.h"
#include "array.h"
#include "sort.h"
#include "format.h"

TEST(sort) {
	Array<s64> arr;
	add(&arr, {
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

	sort(arr);
	EXPECT(is_sorted(arr));
}
