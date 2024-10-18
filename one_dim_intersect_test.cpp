#include "grd_testing.h"
#include "one_dim_intersect.h"
#include "array.h"
#include "format.h"
#include "grd_defer.h"

struct TestRegion {
	s64  start;
	s64  length;
	char c;

	GRD_REFLECT(TestRegion) {
		GRD_MEMBER(start);
		GRD_MEMBER(length);
		GRD_MEMBER(c);
	}
};

void verify_one_dim_array(Array<TestRegion> regions, String str, CodeLocation loc = caller_loc()) {
	tester_scope_push(loc);
	grd_defer { tester_scope_pop(); };

	s64 last_end = 0;
	for (auto it: regions) {
		EXPECT(it.length > 0);
		EXPECT(last_end == it.start);
		last_end = it.start + it.length;
	}

	Array<char> sb;
	for (auto it: regions) {
		for (auto i: grd_range(it.length)) {
			add(&sb, it.c);
		}
	}
	EXPECT(sb == str);
	println(sb);
}

Array<TestRegion> build_test_regions() {
	Array<TestRegion> regions;
	add(&regions, TestRegion{0,  10, 'a'});
	add(&regions, TestRegion{10, 10, 'B'});
	add(&regions, TestRegion{20, 10, 'c'});
	add(&regions, TestRegion{30, 10, 'D'});
	return regions;
}

GRD_TEST(one_dim) {
	auto regions = build_test_regions();
	
	auto get_region = [&](s64 i) {
		auto reg = regions[i];
		return OneDimRegion { reg.start, reg.length };
	};

	auto resize = [&](s64 i, s64 start, s64 end) {
		regions[i].start = start;
		regions[i].length = end - start;
	};

	char insert_letter = 'N';
	auto insert = [&](s64 i, s64 start, s64 end, s64 copy_idx) {
		if (copy_idx == -1) {
			add(&regions, TestRegion{start, end - start, insert_letter}, i);
		} else {
			add(&regions, TestRegion{start, end - start, regions[copy_idx].c }, i);
		}
	};

	auto remove = [&](s64 i) {
		remove_at_index(&regions, i);
	};

	verify_one_dim_array(regions, "aaaaaaaaaaBBBBBBBBBBccccccccccDDDDDDDDDD"_b);
	one_dim_patch(len(regions), get_region, resize, insert, remove, 3, 30);
	verify_one_dim_array(regions, "aaaNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'L';
	one_dim_patch(len(regions), get_region, resize, insert, remove, 6, 10);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'M';
	one_dim_patch(len(regions), get_region, resize, insert, remove, 20, 4);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNMMMMNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'P';
	one_dim_patch(len(regions), get_region, resize, insert, remove, 1, 38);
	verify_one_dim_array(regions, "aPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPD"_b);
	insert_letter = 'O';
	one_dim_patch(len(regions), get_region, resize, insert, remove, 0, 40);
	verify_one_dim_array(regions, "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"_b);
}
