#pragma once

#include "reflection.h"
#include "string_def.h"

template <typename T>
struct Optional {
	T    value;
	bool has_value = false;

	Optional() {};
	Optional(T value): value(value), has_value(true) {};

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

	bool operator==(Optional<T> rhs) {
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
auto make_optional() -> Optional<T> {
	return {};
}

template <typename T>
auto make_optional(T value) -> Optional<T> {
	return Optional<T>(value);
}

struct Optional_Type: Type {
	constexpr static auto KIND = make_type_kind("opt");

	Type* inner = NULL;
};

template <typename T>
Optional_Type* reflect_type(Optional<T>* opt, Optional_Type* type) {
	type->inner = reflect.type_of<T>();
	return type;
}
