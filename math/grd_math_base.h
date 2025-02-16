#pragma once

#include "../grd_base.h"
#include "../grd_data_ops.h"
#include <type_traits>
#include <math.h>

template <typename T>
GRD_DEDUP constexpr T grd_sign(T a) {
	return a < 0 ? -1 : 1;
}

template <typename T>
GRD_DEDUP constexpr T grd_max(T a, std::type_identity_t<T> b) {
	return a > b ? a : b;
}

template <typename T>
GRD_DEDUP constexpr T grd_min(T a, std::type_identity_t<T> b) {
	return a < b ? a : b;
}

template <typename T>
GRD_DEDUP constexpr T grd_clamp(T min, std::type_identity_t<T> max, std::type_identity_t<T> value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


#define ALIAS_MATH_TEMPLATE_FUNCTION(name, type) GRD_DEDUP constexpr auto& name##_##type = name<type>;
#define ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(name)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, s8)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, u8)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, s16)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, u16)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, s32)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, u32)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, s64)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, u64)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, f32)\
ALIAS_MATH_TEMPLATE_FUNCTION(name, f64)

ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_clamp);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_min);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_max);


GRD_DEDUP bool grd_is_aligned(u64 number, u64 alignment) {
	return (number % alignment) == 0;
}

GRD_DEDUP bool grd_is_aligned(void* ptr, u64 alignment) {
	return (u64(ptr) % alignment) == 0;
}

GRD_DEDUP u64 grd_align(u64 number, u64 alignment) {
	if (grd_is_aligned(number, alignment)) {
		return number;
	}
	return number + (alignment - (number % alignment));
}

enum GrdFpClass {
	GRD_FP_NAN = 0,
	GRD_FP_INFINITE = 1,
	GRD_FP_ZERO = 2,
	GRD_FP_SUBNORMAL = 3,
	GRD_FP_NORMAL = 4,
};

GRD_DEDUP GrdFpClass grd_fp_classify(f64 x) {
	union{ f64 d; u64 u;}u = {x};
	u32 exp = (u32) ( (u.u & 0x7fffffffffffffffULL) >> 52 );
	if (exp == 0) {
		if (u.u & 0x000fffffffffffffULL)
			return GRD_FP_SUBNORMAL;
		return GRD_FP_ZERO;
	}
	if (0x7ff == exp ) {	
		if (u.u & 0x000fffffffffffffULL) {
			return GRD_FP_NAN;
		}
		return GRD_FP_INFINITE;
	}
	return GRD_FP_NORMAL;
}

#if GRD_COMPILER_MSVC
	#define GRD_INFINITY (1e300 * 1e300)
	#define GRD_NAN ((-(float)(GRD_INFINITY * 0.0f)))
#else
	#define GRD_INFINITY (1.0f / 0.0f)
	#define GRD_NAN (0.0f / 0.0f)
#endif

GRD_DEDUP bool grd_signbit(f64 x) {
	return grd_bitcast<u64>(x) & (1ULL << 63);
}

GRD_DEDUP bool grd_is_nan(f64 x) {
	return grd_fp_classify(x) == GRD_FP_NAN;
}

GRD_DEF grd_lerp(f64 a, f64 b, f64 t) -> f64 {
	return a + (b - a) * t;
}
