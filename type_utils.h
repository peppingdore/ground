#pragma once

#include <type_traits>

struct Empty_Struct {};

template <typename T>
constexpr bool does_type_have_padding() {
	return !std::has_unique_object_representations_v<T>;
}
