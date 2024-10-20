#pragma once

#include "grd_base.h"
#include "grd_data_ops.h"

struct GrdMul128Result {
	u64 high;
	u64 low;
};

GrdMul128Result grd_mul128(u64 op1, u64 op2) {
    u64 u1 = (op1 & 0xffffffff);
    u64 v1 = (op2 & 0xffffffff);
    u64 t  = (u1 * v1);
    u64 w3 = (t & 0xffffffff);
    u64 k  = (t >> 32);

    op1 >>= 32;
    t = (op1 * v1) + k;
    k = (t & 0xffffffff);
    u64 w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    return {
    	.high = (op1 * op2) + w1 + k,
    	.low  = (t << 32)   + w3,
    };
}

u64 grd_wy_rand_64(u64* state) {
	*state += 0x60bee2bee120fc15;
	auto x = grd_mul128(*state,         0xa3b195354a39b70d);
	     x = grd_mul128(x.high ^ x.low, 0x1b03738712fad5c9);
	return x.high ^ x.low;
}

inline u64 GRD_DEFAULT_RANDOM_SEED = 0x49c6e7b299f58cca;

struct RandomState {
	u64 wy_state = GRD_DEFAULT_RANDOM_SEED;
};

inline thread_local RandomState grd_thread_local_random_state;

RandomState grd_make_random_state(u64 seed = GRD_DEFAULT_RANDOM_SEED) {
	return { .wy_state = seed };
}

u64 grd_rand_u64(RandomState* state = &grd_thread_local_random_state) {
	return grd_wy_rand_64(&state->wy_state);
}

s64 grd_rand_s64(RandomState* state = &grd_thread_local_random_state) {
	return grd_bitcast<s64>(grd_rand_u64(state));
}

u64 grd_rand_range_u64_state(RandomState* state, u64 range_length) {
	if (range_length == 0) {
		return 0;
	}
	u64 scaling = u64_max / range_length;
	u64 limit = range_length * scaling;
	u64 result;
	do {
		result = grd_rand_u64(state);
	} while (result >= limit);
	return result / scaling;
}

u64 grd_rand_range_u64(u64 range_length) {
	return grd_rand_range_u64_state(&grd_thread_local_random_state, range_length);
}

u64 grd_rand_range_u64_state(RandomState* state, u64 start, u64 end_exclusive) {
	if (end_exclusive < start) {
		end_exclusive = start;
	}
	return grd_rand_range_u64_state(state, end_exclusive - start) + start;
}

u64 grd_rand_range_u64(u64 start, u64 end_exclusive) {
	return grd_rand_range_u64_state(&grd_thread_local_random_state, start, end_exclusive);
}

u64 grd_rand_map_signed_to_unsigned(s64 num) {
	if (num >= 0) {
		return u64(num) + u64(s64_max) + 1;
	} else {
		return u64(s64_max) - u64(-num) + 1;
	}
}

s64 grd_rand_map_unsigned_to_signed(u64 num) {
	if (num >= s64_max) {
		return s64(num - u64(s64_max)) - 1;
	} else {
		return -(s64_max - s64(num)) - 1;
	}
}

s64 grd_rand_range_s64_state(RandomState* state, s64 start, s64 end_exclusive) {
	u64 result = grd_rand_range_u64_state(state,
		grd_rand_map_signed_to_unsigned(start),
		grd_rand_map_signed_to_unsigned(end_exclusive));
	return result; // @TODO: change.
}

s64 grd_rand_range_s64(s64 start, s64 end_exclusive) {
	return grd_rand_range_s64_state(&grd_thread_local_random_state, start, end_exclusive);
}
