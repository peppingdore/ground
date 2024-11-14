#if 0
	`dirname "$0"`/../build.py "$0" -r -- ${@:1}; exit
#endif

#include "../grd_file.h"
#include "../grd_array.h"
#include "../grd_panic.h"

void trial_division3(GrdArray<u64>* factorization, u64 n) {
	for (int d : {2, 3, 5}) {
		while (n % d == 0) {
			grd_add(factorization, d);
			n /= d;
		}
	}
   	u64 increments[] = {4, 2, 4, 2, 4, 6, 2, 6};
	int i = 0;
	for (long long d = 7; d * d <= n; d += increments[i++]) {
		while (n % d == 0) {
			grd_add(factorization, d);
			n /= d;
		}
		if (i == 8)
			i = 0;
	}
	if (n > 1) {
		grd_add(factorization, n);
	}
	if (grd_len(*factorization) == 0) {
		grd_add(factorization, n);
	}
}

int main(int argc, char** argv) {
	GrdArray<u64> factor;
	GrdAllocatedString line;
	auto [f, e] = grd_open_file(U"factor_cache.csv"_b, GRD_FILE_WRITE | GRD_FILE_CREATE_NEW);
	if (e) {
		grd_panic(e);
	}
	for (auto i: grd_range(1, 10000000)) {
		trial_division3(&factor, i);
		GrdFormatter fmt = grd_make_formatter(&line, c_allocator);
		grd_format(&fmt, "%", i);
		while (grd_len(factor) > 0) {
			auto num = factor[0];
			s64 count = 0;
			for (s64 idx = 0; idx < grd_len(factor); idx++) {
				if (factor[idx] == num) {
					grd_remove(&factor, idx);
					idx -= 1;
					count += 1;
				}
			}
			grd_format(&fmt, " %-%", num, count);
		}
		grd_format(&fmt, "\n"_b);
		grd_write_file(&f, line.data, grd_len(line) * sizeof(line[0]));
		grd_clear(&factor);
		grd_clear(&line);
		if (i % (4096 * 8) == 0) {
			grd_println("Factoring %", i);
		}
	}
}
