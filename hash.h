#pragma once

#include "third_party/spooky_hash.h"
#include "type_utils.h"
#include <math.h>

using Hash64 = u64;

struct Hash128 {
	u64 lower;
	u64 upper;

	bool operator==(Hash128 rhs) {
		return lower == rhs.lower && upper == rhs.upper;
	}

	bool operator>(Hash128 rhs) {
		if (upper == rhs.upper) {
			return lower > rhs.lower;
		}
		return upper > rhs.upper;
	}

	bool operator<(Hash128 rhs) {
		if (upper == rhs.upper) {
			return lower < rhs.lower;
		}
		return upper < rhs.upper;
	}
};

template<typename T>
concept Hash_Type = std::is_same_v<T, Hash64> || std::is_same_v<T, Hash128>;

template <typename T>
concept Hash_Primitive = !std::is_class_v<T> && !std::is_union_v<T> && !does_type_have_padding<T>();

struct Hasher;

template <typename T>
concept Custom_Hashable = requires(T a) {
	type_hash(std::declval<Hasher*>(), a);
};

template <typename T>
concept Pod_Hashable = Hash_Primitive<T> && !Custom_Hashable<T>;

struct Hasher {
	spookyhash_context ctx;

	void hash(void* data, u64 size) {
		spookyhash_update(&ctx, data, size);
	}

	Hash128 get_hash128() {
		Hash128 result;
		spookyhash_final(&ctx, &result.lower, &result.upper);
		return result;
	}

	Hash64 get_hash64() {
		return get_hash128().lower;
	}

	template <Custom_Hashable T>
	void hash(T thing) {
		type_hash(this, thing);
	}

	template <Pod_Hashable T>
	void hash(T thing) {
		hash(&thing, sizeof(T));
	}
};

constexpr Hash128 spookyhash_seed = {
	.lower = 0x23ad'aad3'dad3'7089,
	.upper = 0x7200'a02f'79b2'70c5,
};

Hasher make_hasher() {
	Hasher result;
	spookyhash_context_init(&result.ctx, spookyhash_seed.lower, spookyhash_seed.upper);
	return result;
}

Hash128 hash128(void* data, u64 size) {
	Hash128 result = spookyhash_seed;
	spookyhash_128(data, size, &result.lower, &result.upper);
	return result;
}

Hash64 hash64(void* data, u64 size) {
	return hash128(data, size).lower;
}

Hash128 hash128(Custom_Hashable auto x) {
	Hasher hasher = make_hasher();
	type_hash(&hasher, x);
	return hasher.get_hash128();
}

Hash128 hash128(Pod_Hashable auto x) {
	return hash128(&x, sizeof(x));
}

Hash64 hash64(auto thing) {
	return hash128(thing).lower;
}

template <typename T>
void hash_fp_naive(Hasher* hasher, T num) {
	static_assert(std::is_same_v<T, f32> || std::is_same_v<T, f64>);

	auto hash_bytes = [=](T num) {
		hasher->hash(&num, sizeof(num));
	};

	switch (fpclassify(num)) {
		case FP_INFINITE:  hash_bytes(INFINITY);
		case FP_NAN:       hash_bytes(NAN);
		case FP_ZERO:      hash_bytes(0);
		case FP_SUBNORMAL: // Is this proper??
		case FP_NORMAL:
		default:
			hash_bytes(num);
	}
}
