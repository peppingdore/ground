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

	static char insert_letter = 'N';

	auto patcher = grd_make_one_dim_patcher(&regions,
		[](void* x) {
			auto arr = (decltype(regions)*) x;
			return grd_len(*arr);
		},
		[](void* x, s64 idx) {
			auto reg = (*(decltype(regions)*) x)[idx];
			return GrdOneDimRegion {
				.start = reg.start,
				.end = reg.start + reg.length
			};
		},
		[](void* x, s64 idx, s64 start, s64 end) {
			auto arr = (decltype(regions)*) x;
			(*arr)[idx].start = start;
			(*arr)[idx].length = end - start;
		},
		[](void* x, s64 idx, s64 start, s64 end, s64 copy_idx) {
			auto arr = (decltype(regions)*) x;
			if (copy_idx == -1) {
				grd_add(arr, TestRegion{start, end - start, insert_letter}, idx);
			} else {
				grd_add(arr, TestRegion{start, end - start, (*arr)[copy_idx].c }, idx);
			}
		},
		[](void* x, s64 idx) {
			auto arr = (decltype(regions)*) x;
			grd_remove(arr, idx);
		}
	);

	verify_one_dim_array(regions, "aaaaaaaaaaBBBBBBBBBBccccccccccDDDDDDDDDD"_b);
	grd_one_dim_patch(&patcher, 3, 33);
	verify_one_dim_array(regions, "aaaNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'L';
	grd_one_dim_patch(&patcher, 6, 16);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNNNNNNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'M';
	grd_one_dim_patch(&patcher, 20, 24);
	verify_one_dim_array(regions, "aaaNNNLLLLLLLLLLNNNNMMMMNNNNNNNNNDDDDDDD"_b);
	insert_letter = 'P';
	grd_one_dim_patch(&patcher, 1, 39);
	verify_one_dim_array(regions, "aPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPD"_b);
	insert_letter = 'O';
	grd_one_dim_patch(&patcher, 0, 40);
	verify_one_dim_array(regions, "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"_b);
}
