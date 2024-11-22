#include "grd_testing.h"
#include "grd_one_dim_intersect.h"
#include "grd_array.h"
#include "grd_format.h"
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

void verify_one_dim_array(GrdArray<TestRegion> regions, GrdString str, GrdCodeLoc loc = grd_caller_loc()) {
	grd_tester_scope_push(loc);
	grd_defer { grd_tester_scope_pop(); };

	s64 last_end = 0;
	for (auto it: regions) {
		GRD_EXPECT(it.length > 0);
		GRD_EXPECT(last_end == it.start);
		last_end = it.start + it.length;
	}

	GrdArray<char> sb;
	for (auto it: regions) {
		for (auto i: grd_range(it.length)) {
			grd_add(&sb, it.c);
		}
	}
	GRD_EXPECT(sb == str);
	grd_println(sb);
}

GrdArray<TestRegion> build_test_regions() {
	GrdArray<TestRegion> regions;
	grd_add(&regions, TestRegion{0,  10, 'a'});
	grd_add(&regions, TestRegion{10, 10, 'B'});
	grd_add(&regions, TestRegion{20, 10, 'c'});
	grd_add(&regions, TestRegion{30, 10, 'D'});
	return regions;
}

GRD_TEST_CASE(one_dim) {
	auto regions = build_test_regions();
	
	auto get_region = [&](s64 i) {
		auto reg = regions[i];
		return GrdOneDimRegion { reg.start, reg.length };
	};

	auto resize = [&](s64 i, s64 start, s64 end) {
		regions[i].start = start;
		regions[i].length = end - start;
	};

	char insert_letter = 'N';
	auto insert = [&](s64 i, s64 start, s64 end, s64 copy_idx) {
		if (copy_idx == -1) {
			grd_add(&regions, TestRegion{start, end - start, insert_letter}, i);
		} else {
			grd_add(&regions, TestRegion{start, end - start, regions[copy_idx].c }, i);
		}
	};

	auto remove = [&](s64 i) {
		grd_remove(&regions, i);
	};

	verify_one_dim_array(regions, "aaaaaaaaaaBBBBBBBBBBccccccccccDDDDDDDDDD"_b);
	grd_one_dim_patch(grd_len(regions), get_region, resize, insert, remove, 3, 30);
	verify_one_dim_array(regions, "aaaNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'L';
	grd_one_dim_patch(grd_len(regions), get_region, resize, insert, remove, 6, 10);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'M';
	grd_one_dim_patch(grd_len(regions), get_region, resize, insert, remove, 20, 4);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNMMMMNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'P';
	grd_one_dim_patch(grd_len(regions), get_region, resize, insert, remove, 1, 38);
	verify_one_dim_array(regions, "aPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPD"_b);
	insert_letter = 'O';
	grd_one_dim_patch(grd_len(regions), get_region, resize, insert, remove, 0, 40);
	verify_one_dim_array(regions, "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"_b);
}
