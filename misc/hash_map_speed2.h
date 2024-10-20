#pragma once

#include "sub_allocator.h"
#include "../grd_random.h"
#include "../grd_format.h"
#include "../grd_file.h"
#include "../grd_panic.h"

f64 uniform_f64() {
	u64 uint64_value = (u64) rand_range_u64(u64_max);
	u64 mantissa = uint64_value >> 11; // Get top 53 bits
	f64 float_value = (f64) mantissa / 9007199254740992.0; // Divide by 2^53
	return float_value;
}

f64 normal_f64() {
	f64 u1 = uniform_f64();
	f64 u2 = uniform_f64();

    f64 z1 = sqrt(-2. * log(u1)) * cos(2. * M_PI * u2);
    f64 z2 = sqrt(-2. * log(u1)) * sin(2. * M_PI * u2);

    return z1;
}

f64 exp_curve(f64 x) {
	f64 n = 0.001;
	f64 m = 1.4;
	f64 t = -15.6;
	f64 p = 0.906;
	return n/(m+(t*(x-p))) / 5.;
	// return 1.2 / (0.8 + exp(-10.0 * (x - 0.99)));
}

u64 pick_alloc_size() {
	while (true) {
		f64 rand = uniform_f64();
		if (rand < 0) {
			rand = -rand;
		}
		f64 factor = exp_curve(rand);
		f64 sz = factor * 256. * 1024. * 1024.;
		if (sz <= 0) {
			continue;
		}
		return sz;
	}
}

int main() {
	Allocator sub = make_sub_allocator(c_allocator);
	GrdSubAllocator* xxx = (GrdSubAllocator*) sub.data;

	GrdHashMap<void*, u64> blocks;
	u64 COUNT = 100000;
	u64 total_size = 0;
	GrdAllocatedString str;
	u64 max_sz = 0;
	u64 min_sz = u64_max;
	for (auto i: range(COUNT)) {
		min_sz = min_u64(min_sz, sz);
		max_sz = max_u64(sz, max_sz);
		total_size += u64(sz);
		// printf("%f, %f, %lu\n", factor, sz, total_size);
		char buf[128];
		sprintf(buf, "%f", sz);
		format(&str, "%, %, %*, %\n", factor, sz, buf, total_size);

		void* allocted = Malloc(sub, sz);
		put(&blocks, allocted, sz);
	}
	auto e = write_string_to_file(str, U"/mnt/e/sz.csv"_b);
	if (e) {
		panic(e);
	}
	grd_println("max size: %(sz)", max_sz);
	grd_println("min size: %(sz)", min_sz);
	grd_println("total size: %, %(sz)", total_size, total_size);
	grd_println("total nanos: %, allocs count: %", xxx->total_nanos, xxx->allocs_count);
	grd_println("avg alloc time: % ns", f64(xxx->total_nanos) / xxx->allocs_count);
	return 0;
}
