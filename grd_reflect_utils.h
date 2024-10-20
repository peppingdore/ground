#pragma once

#include "grd_reflect.h"
#include "grd_optional.h"
#include "grd_string.h"

GrdOptional<GrdEnumValue> grd_find_enum_value(GrdEnumType* type, GrdString name) {
	for (auto it: type->values) {
		if (grd_make_string(it.name) == name) {
			return it;
		}
	}
	return {};
}

GrdOptional<GrdEnumValue> grd_find_matching_enum_value(GrdEnumType* type, GrdPrimitiveValue value) {
	for (auto it: type->values) {
		if (it.value == value) {
			return it;
		}
	}
	return {};
}
