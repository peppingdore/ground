#pragma once

#include <type_traits>

struct EmptyStruct {};

template <typename T>
constexpr bool does_type_have_padding() {
	return !std::has_unique_object_representations_v<T>;
}
