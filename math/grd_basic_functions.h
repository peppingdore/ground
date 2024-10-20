#pragma once

#include "../grd_base.h"
#include <type_traits>

template <typename T>
constexpr T grd_sign(T a) {
	return a < 0 ? -1 : 1;
}

#undef max
#undef min

template <typename T>
constexpr T grd_max(T a, std::type_identity_t<T> b) {
	return a > b ? a : b;
}

template <typename T>
constexpr T grd_min(T a, std::type_identity_t<T> b) {
	return a < b ? a : b;
}

template <typename T>
constexpr T grd_clamp(T min, std::type_identity_t<T> max, std::type_identity_t<T> value) {
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

ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_clamp);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_min);
ALIAS_MATH_TEMPLATE_FUNCTION_WITH_DEFAULT_TYPES(grd_max);


bool grd_is_aligned(u64 number, u64 alignment) {
	return (number % alignment) == 0;
}

bool grd_is_aligned(auto* ptr, u64 alignment) {
	return (u64(ptr) % alignment) == 0;
}

template <typename T>
T grd_align(T number, u64 alignment) {
	if (grd_is_aligned(number, alignment)) {
		return number;
	}
	return number + (alignment - (number % alignment));
}
