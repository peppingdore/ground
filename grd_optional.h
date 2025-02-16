#pragma once

#include "grd_reflect.h"

template <typename T>
struct GrdOptional {
	T    value;
	bool has_value = false;

	GrdOptional() {};
	GrdOptional(T value): value(value), has_value(true) {};

	T* get() {
		return has_value ? &value : NULL;
	}

	T _or(T substitute_value) {
		return has_value ? value : substitute_value; 
	}

	void operator=(T new_value) {
		has_value = true;
		value = new_value;
	}

	bool operator==(GrdOptional<T> rhs) {
		if (has_value != rhs.has_value) {
			return false;
		}
		if (!has_value) {
			return true;
		}
		return value == rhs.value;
	}

	bool operator==(T rhs) {
		if (!has_value) {
			return false;
		}
		return value == rhs;
	}
};

template <typename T>
GRD_DEDUP auto grd_make_optional() -> GrdOptional<T> {
	return {};
}

template <typename T>
GRD_DEDUP auto grd_make_optional(T value) -> GrdOptional<T> {
	return GrdOptional<T>(value);
}

struct GrdOptionalType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("opt");

	GrdType* inner = NULL;
};

template <typename T>
GRD_DEDUP GrdOptionalType* grd_reflect_create_type(GrdOptional<T>* x) {
	return grd_reflect_register_type<GrdOptional<T>, GrdOptionalType>("");
}

template <typename T>
GRD_DEDUP void grd_reflect_type(GrdOptional<T>* x, GrdOptionalType* type) {
	type->inner = grd_reflect_type_of<T>();
	type->name = grd_heap_sprintf("GrdOptional<%s>", type->inner->name);
}
