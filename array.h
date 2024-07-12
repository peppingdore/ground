#pragma once

#include "span.h"
#include "allocator.h"

template <typename T>
struct Array: Span<T> {
	using Span<T>::data;
	using Span<T>::count;

	s64          capacity  = 0;
	Allocator    allocator = c_allocator;
	CodeLocation loc       = caller_loc();

	Array copy(Allocator cp_allocator = c_allocator, CodeLocation loc = caller_loc()) {
		Array result = { .allocator = cp_allocator };
		add(&result, *this, loc);
		return result;
	}

	void free(CodeLocation loc = caller_loc()) {
		if (data) {
			Free(allocator, data, loc);
			data  = NULL;
			count = 0;
		}
	}
};

template <typename T>
T* reserve(Array<T>* arr, s64 length, s64 index = -1, CodeLocation loc = caller_loc()) {
	if (index < 0) {
		index += len(*arr) + 1;
	}

	assert(length >= 0);
	assert(index >= 0);
	assert(index <= len(*arr));

	if (length == 0) {
		// Be canonical about returning address, even though
		//   we can't actually write to it.
		return arr->data + index;
	}

	s64 target_capacity = len(*arr) + length;

	if (!arr->data) {
		if (target_capacity > arr->capacity) {
			arr->capacity = target_capacity;
		}
		if (arr->capacity <= 0) {
			arr->capacity = 8;
		}
		arr->data = Alloc<T>(arr->allocator, arr->capacity, loc);
	}

	if (target_capacity > arr->capacity) {
		s64 old_capacity = arr->capacity;
		assert(old_capacity > 0);
		arr->capacity = max_s64(old_capacity * 2, target_capacity);
		arr->data     = (T*) Realloc(arr->allocator, arr->data, old_capacity * sizeof(T), arr->capacity * sizeof(T), loc);
	}

	memmove(arr->data + index + length, arr->data + index, (len(*arr) - index) * sizeof(T));
	arr->count += length;
	return arr->data + index;
}

template <typename T>
T* add(Array<T>* arr, std::type_identity_t<T> item, s64 index = -1, CodeLocation loc = caller_loc()) {
	T* ptr = reserve(arr, 1, index, loc);
	*ptr = item;
	return ptr;
}

template <typename T>
T* add(Array<T>* arr, T* src, s64 length, s64 index = -1, CodeLocation loc = caller_loc()) {
	T* ptr = reserve(arr, length, index, loc);
	for (auto i: range(length)) {
		ptr[i] = src[i];
	}
	return ptr;
}

template <typename T>
T* add(Array<T>* arr, Span<T> src, s64 index = -1, CodeLocation loc = caller_loc()) {
	return add(arr, src.data, src.count, index, loc);
}

// add initializer list
template <typename T>
T* add(Array<T>* arr, std::initializer_list<std::type_identity_t<T>> list, s64 index = -1, CodeLocation loc = caller_loc()) {
	return add(arr, (T*) list.begin(), list.size(), index, loc);
}

template <typename T, s64 N>
T* add(Array<T>* arr, const T (&src)[N], s64 index = -1, CodeLocation loc = caller_loc()) {
	return add(arr, (T*) src, N, index, loc);	
}

template <typename T>
void clear(Array<T>* arr, CodeLocation loc = caller_loc()) {
	arr->count = 0;
}

template <typename T>
Array<T> copy(Allocator allocator, Span<T> src, CodeLocation loc = caller_loc()) {
	Array<T> result = { .allocator = allocator };
	add(&result, src, -1, loc);
	return result;
}

template <typename T>
Array<T> copy(Span<T> src, CodeLocation loc = caller_loc()) {
	return copy(c_allocator, src, loc);
}

template <typename T>
Array<T> to_array(Allocator allocator, Generator<T>&& generator) {
	Array<T> arr = { .allocator = allocator };
	for (auto it: generator) {
		add(&arr, it);
	}
	return arr;
}

template <typename T>
Array<T> to_array(Generator<T>&& generator) {
	return to_array(c_allocator, std::move(generator));
}


struct ArrayType: SpanType {
	s64 (*get_capacity)(void* arr);
};

template <typename T>
ArrayType* reflect_type(Array<T>* x, ArrayType* type) {
	type->inner = reflect_type_of<T>();
	type->name = heap_sprintf("Array<%s>", type->inner->name);
	type->subkind = "array";
	type->get_count = [](void* arr) {
		auto casted = (Array<int>*) arr;
		return casted->count;
	};
	type->get_capacity = [](void* arr) {
		auto casted = (Array<int>*) arr;
		return casted->capacity;
	};
	type->get_item = [](void* arr, s64 index) {
		auto casted = (Array<int>*) arr;
		void* item = ptr_add(casted->data, sizeof(T) * index);
		return item;
	};
	return type;
}
