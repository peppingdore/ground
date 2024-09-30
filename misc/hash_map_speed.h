#pragma once

#include "../hash_map.h"
#include "../stopwatch.h"
#include "../range.h"
#include "../random.h"
#include "../array.h"
#include "../format.h"

int main() {
	HashMap<s64, s64> map;
	map.capacity = 100;

	Array<s64> keys;
	Array<s64> values;

	s64 COUNT = 10000000;
	for (auto i: range(COUNT)) {
		add(&keys, rand_s64());
		add(&values, rand_s64());
	}

	Stopwatch w = make_stopwatch();
	for (auto i: range(COUNT)) {
		if (i % 65536 == 0) {
			print("insert %, %", keys[i], values[i]);
		}
		put(&map, keys[i], values[i]);
	}
	
	s64 time = nanos_elapsed_s64(&w);
	reset(&w);

	print("Avg insert time: % ns", f64(time) / f64(COUNT));
	return 0;
}
