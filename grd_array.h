#pragma once

#include "grd_span.h"
#include "grd_allocator.h"

template <typename T>
struct GrdArray: GrdSpan<T> {
	using GrdSpan<T>::data;
	using GrdSpan<T>::count;

	s64          capacity  = 0;
	GrdAllocator    allocator = c_allocator;
	GrdCodeLoc loc       = grd_caller_loc();

	GrdArray copy(GrdAllocator cp_allocator = c_allocator, GrdCodeLoc loc = grd_caller_loc()) {
		GrdArray result = { .allocator = cp_allocator };
		grd_add(&result, *this, loc);
		return result;
	}

	void free(GrdCodeLoc loc = grd_caller_loc()) {
		if (data) {
			GrdFree(allocator, data, loc);
			data  = NULL;
			count = 0;
		}
	}
};

template <typename T>
T* grd_reserve(GrdArray<T>* arr, s64 length, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	if (index < 0) {
		index += grd_len(*arr) + 1;
	}

	assert(length >= 0);
	assert(index >= 0);
	assert(index <= grd_len(*arr));

	if (length == 0) {
		// Be canonical about returning address, even though
		//   we can't actually write to it.
		return arr->data + index;
	}

	s64 target_capacity = grd_len(*arr) + length;

	if (!arr->data) {
		if (target_capacity > arr->capacity) {
			arr->capacity = target_capacity;
		}
		if (arr->capacity <= 0) {
			arr->capacity = 8;
		}
		arr->data = GrdAlloc<T>(arr->allocator, arr->capacity, loc);
	}

	if (target_capacity > arr->capacity) {
		s64 old_capacity = arr->capacity;
		assert(old_capacity > 0);
		arr->capacity = grd_max(old_capacity * 2, target_capacity);
		arr->data     = (T*) GrdRealloc(arr->allocator, arr->data, old_capacity * sizeof(T), arr->capacity * sizeof(T), loc);
	}

	memmove(arr->data + index + length, arr->data + index, (grd_len(*arr) - index) * sizeof(T));
	arr->count += length;
	return arr->data + index;
}

template <typename T>
T* grd_add(GrdArray<T>* arr, std::type_identity_t<T> item, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	T* ptr = grd_reserve(arr, 1, index, loc);
	*ptr = item;
	return ptr;
}

template <typename T>
T* grd_add(GrdArray<T>* arr, T* src, s64 length, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	T* ptr = grd_reserve(arr, length, index, loc);
	for (auto i: grd_range(length)) {
		ptr[i] = src[i];
	}
	return ptr;
}

template <typename T>
T* grd_add(GrdArray<T>* arr, GrdSpan<T> src, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_add(arr, src.data, src.count, index, loc);
}

// add initializer list
template <typename T>
T* grd_add(GrdArray<T>* arr, std::initializer_list<std::type_identity_t<T>> list, s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_add(arr, (T*) list.begin(), list.size(), index, loc);
}

template <typename T, s64 N>
T* grd_add(GrdArray<T>* arr, const T (&src)[N], s64 index = -1, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_add(arr, (T*) src, N, index, loc);	
}

template <typename T>
void grd_clear(GrdArray<T>* arr, GrdCodeLoc loc = grd_caller_loc()) {
	arr->count = 0;
}

template <typename T>
GrdArray<T> grd_clear_array(GrdAllocator allocator, GrdSpan<T> src, GrdCodeLoc loc = grd_caller_loc()) {
	GrdArray<T> result = { .allocator = allocator };
	grd_add(&result, src, -1, loc);
	return result;
}

template <typename T>
GrdArray<T> grd_clear_array(GrdSpan<T> src, GrdCodeLoc loc = grd_caller_loc()) {
	return grd_clear_array(c_allocator, src, loc);
}

template <typename T>
GrdArray<T> grd_to_array(GrdAllocator allocator, GrdGenerator<T>&& generator) {
	GrdArray<T> arr = { .allocator = allocator };
	for (auto it: generator) {
		grd_add(&arr, it);
	}
	return arr;
}

template <typename T>
GrdArray<T> grd_to_array(GrdGenerator<T>&& generator) {
	return grd_to_array(c_allocator, std::move(generator));
}


struct GrdArrayType: GrdSpanType {
	s64 (*get_capacity)(void* arr);
};

template <typename T>
GrdArrayType* grd_reflect_type(GrdArray<T>* x, GrdArrayType* type) {
	type->inner = grd_reflect_type_of<T>();
	type->name = grd_heap_sprintf("GrdArray<%s>", type->inner->name);
	type->subkind = "array";
	type->get_count = [](void* arr) {
		auto casted = (GrdArray<int>*) arr;
		return casted->count;
	};
	type->get_capacity = [](void* arr) {
		auto casted = (GrdArray<int>*) arr;
		return casted->capacity;
	};
	type->get_item = [](void* arr, s64 index) {
		auto casted = (GrdArray<int>*) arr;
		void* item = grd_ptr_add(casted->data, sizeof(T) * index);
		return item;
	};
	return type;
}
