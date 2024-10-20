#pragma once

#include "../grd_hash_map.h"
#include "../grd_stopwatch.h"
#include "../grd_range.h"
#include "../grd_random.h"
#include "../grd_array.h"
#include "../grd_format.h"

int main() {
	GrdHashMap<s64, s64> map;
	map.capacity = 100;

	GrdArray<s64> keys;
	GrdArray<s64> values;

	s64 COUNT = 1000000;
	for (auto i: grd_range(COUNT)) {
		grd_add(&keys, rand_s64());
		grd_add(&values, rand_s64());
	}

	GrdStopwatch w = grd_make_stopwatch();
	for (auto i: grd_range(COUNT)) {
		if (i % 65536 == 0) {
			grd_println("insert %, %", keys[i], values[i]);
		}
		grd_put(&map, keys[i], values[i]);
	}
	
	s64 time = grd_nanos_elapsed_s64(&w);

	grd_println("Avg insert time: % ns", f64(time) / f64(COUNT));

	grd_reset(&w);
	for (auto i: grd_range(COUNT)) {
		grd_get(&map, keys[i]);
	}
	time = grd_nanos_elapsed_s64(&w);
	grd_println("Avg lookup time: % ns", f64(time) / f64(COUNT));

	return 0;
}
