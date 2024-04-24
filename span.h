#pragma once

#include "base.h"
#include "range.h"
#include "hash.h"
#include "reflect.h"
#include "optional.h"

template <typename T>
struct Span {
	T*  data  = NULL;
	s64 count = 0;

	T* begin() { return data; }
	T* end() { return data + count; }

	T& operator[](s64 index) {
		if (index < 0) {
			index += count;
		}
		assert(index >= 0);
		assert(index < count);
		return data[index];
	}

	Span operator[](Optional<s64> start, Optional<s64> end) {
		s64 start_val = start.has_value ? start.value : 0;
		s64 end_val   = end.has_value   ? end.value   : count;
		if (start_val < 0) {
			start_val += count + 1;
		}
		if (end_val < 0) {
			end_val += count + 1;
		}
		assert(start_val >= 0);
		assert(end_val >= 0);
		assert(start_val <= count);
		assert(end_val <= count);
		assert(end_val >= start_val);
		Span slice = { data + start_val, end_val - start_val };
		return slice;
	}

	template <typename U>
	bool operator==(Span<U> rhs) {
		if (count != rhs.count) return false;
		if ((void*) data == (void*) rhs.data) return true;
		for (auto i: range(count)) {
			if (data[i] != rhs.data[i]) {
				return false;
			}
		}
		return true;
	}

	template <typename U, s64 N>
	bool operator==(const U (&rhs)[N]) {
		Span<U> span = { (U*) rhs, N - 1 }; 
		return operator==(span);  
	}
};

template <typename T, s64 N>
Span<T> make_span(const T (&arr)[N]) {
	return { (T*) arr, N };
}

template <typename T>
s64 len(Span<T> span) {
	return span.count;
}

template <typename T>
T pop(Span<T>* span) {
	assert(span->count > 0);
	span->count -= 1;
	return span->data[span->count];
}

template <typename T>
s64 index_of(Span<T> span, T item) {
	for (auto i: range(len(span))) {
		if (span[i] == item) {
			return i;
		}
	}
	return -1;
}

template <typename T, typename U>
s64 index_of(Span<T> span, Span<U> item) {
	if (len(item) > len(span)) {
		return -1;
	}
	for (auto i: range(len(span) - len(item) + 1)) {
		if (span[i, i + len(item)] == item) {
			return i;
		}
	}
	return -1;
}

template <typename T>
s64 index_of_fast(Span<T> span, T* ptr) {
	if (ptr >= span.data && ptr < span.data + span.count) {
		return ptr - span.data;
	}
	return -1;
}

template <typename T>
bool contains(Span<T> span, auto item) {
	return index_of(span, item) != -1;
}

template <typename T>
void type_hash(Hasher* hasher, Span<T> array) {
	hasher->hash(array.count);
	for (auto item: array) {
		hasher->hash(item);
	}
}

template <typename T>
SpanType* reflect_type(Span<T>* x, SpanType* type) {
	type->inner = reflect_type_of<T>();
	type->name = heap_sprintf("Span<%s>", type->inner->name);
	type->subkind = "span";
	type->get_count = [](void* arr) {
		auto casted = (Span<int>*) arr;
		return casted->count;
	};
	type->get_item = [](void* arr, s64 index) {
		auto casted = (Span<int>*) arr;
		void* item = ptr_add(casted->data, sizeof(T) * index);
		return item;
	};
	return type;
}

template <typename T>
void reverse(Span<T> span) {
	reverse(span.data, span.count);
}
