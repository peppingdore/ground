#pragma once

#include "reflect.h"
#include "optional.h"
#include "string.h"

Optional<EnumValue> find_enum_value(EnumType* type, String name) {
	for (auto it: type->values) {
		if (grd_make_string(it.name) == name) {
			return it;
		}
	}
	return {};
}

Optional<EnumValue> find_matching_enum_value(EnumType* type, GrdPrimitiveValue value) {
	for (auto it: type->values) {
		if (it.value == value) {
			return it;
		}
	}
	return {};
}
