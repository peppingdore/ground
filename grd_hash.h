#pragma once

#include "third_party/spooky_hash/spooky_hash.h"
#include "grd_type_utils.h"
#include "math/grd_basic_functions.h"
#include <string.h>

using GrdHash64 = u64;

struct GrdHash128 {
	u64 lower;
	u64 upper;

	bool operator==(GrdHash128 rhs) {
		return lower == rhs.lower && upper == rhs.upper;
	}

	bool operator>(GrdHash128 rhs) {
		if (upper == rhs.upper) {
			return lower > rhs.lower;
		}
		return upper > rhs.upper;
	}

	bool operator<(GrdHash128 rhs) {
		if (upper == rhs.upper) {
			return lower < rhs.lower;
		}
		return upper < rhs.upper;
	}
};

template <typename T>
concept GrdHashPrimitive = !std::is_class_v<T> && !std::is_union_v<T> && !grd_does_type_have_padding<T>();

struct GrdHasher;

template <typename T>
concept GrdMemberHashable = requires(T a) {
	a.hash(std::declval<GrdHasher*>());
};

template <typename T>
concept GrdGlobalHashable = requires(T a) {
	grd_type_hash(std::declval<GrdHasher*>(), a);
};

template <typename T>
concept GrdPodHashable =
	GrdHashPrimitive<T> && !GrdMemberHashable<T> && !GrdGlobalHashable<T>;

struct GrdHasher {
	spookyhash_context ctx;
};

GRD_DEDUP void grd_update(GrdHasher* h, void* data, u64 size) {
	spookyhash_update(&h->ctx, data, size);
}

GRD_DEDUP GrdHash128 grd_hash128(GrdHasher* h) {
	GrdHash128 result;
	spookyhash_final(&h->ctx, &result.lower, &result.upper);
	return result;
}

GRD_DEDUP GrdHash64 grd_hash64(GrdHasher* h) {
	return grd_hash128(h).lower;
}

template <GrdGlobalHashable T>
GRD_DEDUP void grd_update(GrdHasher* h, T x) {
	grd_type_hash(h, x);
}

template <GrdMemberHashable T>
GRD_DEDUP void grd_update(GrdHasher* h, T x) {
	x.hash(h);
}

template <GrdPodHashable T>
GRD_DEDUP void grd_update(GrdHasher* h, T x) {
	grd_update(h, &x, sizeof(T));
}

GRD_DEDUP constexpr GrdHash128 grd_spookyhash_seed = {
	.lower = 0x23ad'aad3'dad3'7089,
	.upper = 0x7200'a02f'79b2'70c5,
};

GRD_DEDUP GrdHasher grd_make_hasher() {
	GrdHasher result;
	spookyhash_context_init(&result.ctx, grd_spookyhash_seed.lower, grd_spookyhash_seed.upper);
	return result;
}

GRD_DEDUP GrdHash128 grd_hash128(void* data, u64 size) {
	GrdHash128 result = grd_spookyhash_seed;
	spookyhash_128(data, size, &result.lower, &result.upper);
	return result;
}

GRD_DEDUP GrdHash64 grd_hash64(void* data, u64 size) {
	return grd_hash128(data, size).lower;
}

GRD_DEDUP GrdHash128 grd_hash128(GrdGlobalHashable auto x) {
	GrdHasher h = grd_make_hasher();
	grd_type_hash(&h, x);
	return grd_hash128(&h);
}

GRD_DEDUP GrdHash128 grd_hash128(GrdMemberHashable auto x) {
	GrdHasher h = grd_make_hasher();
	x.hash(&h);
	return grd_hash128(&h);
}

GRD_DEDUP GrdHash128 grd_hash128(GrdPodHashable auto x) {
	return grd_hash128(&x, sizeof(x));
}

GRD_DEDUP GrdHash64 grd_hash64(auto thing) {
	return grd_hash128(thing).lower;
}

template <typename T>
GRD_DEDUP void grd_hash_fp_naive(GrdHasher* h, T num) {
	static_assert(std::is_same_v<T, f32> || std::is_same_v<T, f64>);

	auto hash_bytes = [=](T num) {
		grd_update(&h, &num, sizeof(num));
	};

	switch (grd_fp_classify(num)) {
		case GRD_FP_INFINITE:  hash_bytes(GRD_INFINITY);
		case GRD_FP_NAN:       hash_bytes(GRD_NAN);
		case GRD_FP_ZERO:      hash_bytes(0);
		case GRD_FP_SUBNORMAL: // Is this proper??
		case GRD_FP_NORMAL:
		default:
			hash_bytes(num);
	}
}

void grd_type_hash(GrdHasher* h, const char* str) {
	auto len = strlen(str);
	grd_update(h, (void*) str, (u64) len);
}
