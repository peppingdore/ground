#pragma once

#include "base.h"

template <typename T>
inline T sign(T a) {
	if (a < 0) {
		return -1;
	} else {
		return 1;
	}
}

#undef max
#undef min

template <typename T>
constexpr T max(T a, T b) {
	if (a > b) {
		return a;
	}
	return b;
}

template <typename T>
constexpr T min(T a, T b) {
	if (a < b) {
		return a;
	}
	return b;
}

template <typename T>
constexpr T clamp(T min, T max, T value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


#define ALIAS_MATH_TEMPLATE_FUNCTION(name, type) constexpr auto& name##_##type = name<type>;
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

ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(clamp);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(min);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(max);
