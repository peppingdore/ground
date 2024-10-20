#pragma once

#include <type_traits>

struct GrdEmptyStruct {};

template <typename T>
constexpr bool grd_does_type_have_padding() {
	return !std::has_unique_object_representations_v<T>;
}
