#pragma once

#include "grd_base.h"
#include "grd_range.h"
#include "grd_hash.h"
#include "grd_reflect.h"
#include "grd_optional.h"

template <typename T>
struct GrdSpan {
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

	GrdSpan operator[](GrdOptional<s64> start, GrdOptional<s64> end) {
		s64 start_val = start.has_value ? start.value : 0;
		s64 end_val   = end.has_value   ? end.value   : count;
		if (start_val < 0) {
			start_val += count;
		}
		if (end_val < 0) {
			end_val += count;
		}
		assert(start_val >= 0);
		assert(end_val >= 0);
		assert(start_val <= count);
		assert(end_val <= count);
		assert(end_val >= start_val);
		GrdSpan slice = { data + start_val, end_val - start_val };
		return slice;
	}

	struct GrdSpanRange {
		GrdOptional<s64> start;
		GrdOptional<s64> end;
	};

	GrdSpan operator[](GrdSpanRange x) {
		return operator[](x.start, x.end);
	}

	template <typename U>
	bool operator==(GrdSpan<U> rhs) {
		if (count != rhs.count) return false;
		if ((void*) data == (void*) rhs.data) return true;
		for (auto i: grd_range(count)) {
			if (data[i] != rhs.data[i]) {
				return false;
			}
		}
		return true;
	}

	template <typename U, s64 N>
	bool operator==(const U (&rhs)[N]) {
		GrdSpan<U> span = { (U*) rhs, N - 1 }; 
		return operator==(span);  
	}
};

template <typename T, s64 N>
GrdSpan<T> grd_make_span(const T (&arr)[N]) {
	return { (T*) arr, N };
}

template <typename T>
GrdSpan<T> grd_make_span(std::initializer_list<T> list) {
	return { (T*) list.begin(), (s64) list.size() };
}

template <typename T>
s64 grd_len(GrdSpan<T> span) {
	return span.count;
}

template <typename T>
T grd_pop(GrdSpan<T>* span) {
	assert(span->count > 0);
	span->count -= 1;
	return span->data[span->count];
}

template <typename T>
s64 grd_index_of(GrdSpan<T> span, T item) {
	for (auto i: grd_range(grd_len(span))) {
		if (span[i] == item) {
			return i;
		}
	}
	return -1;
}

template <typename T, typename U>
s64 grd_index_of(GrdSpan<T> span, GrdSpan<U> item) {
	if (grd_len(item) > grd_len(span)) {
		return -1;
	}
	for (auto i: grd_range(grd_len(span) - grd_len(item) + 1)) {
		if (span[i, i + grd_len(item)] == item) {
			return i;
		}
	}
	return -1;
}

template <typename T>
s64 grd_index_of_fast(GrdSpan<T> span, T* ptr) {
	if (ptr >= span.data && ptr < span.data + span.count) {
		return ptr - span.data;
	}
	return -1;
}

template <typename T>
void grd_remove(GrdSpan<T>* arr, s64 index, s64 remove_count = 1) {
	assert(remove_count <= (grd_len(*arr) - index));
	arr->count -= remove_count;
	memmove(arr->data + index, arr->data + index + remove_count, (grd_len(*arr) - index) * sizeof(T));
}

template <typename T>
bool grd_find_and_remove(GrdSpan<T>* arr, T item) {
	s64 index = grd_index_of(*arr, item);
	if (index == -1) {
		return false;
	}
	grd_remove(arr, index);
	return true;
}

template <typename T>
bool grd_find_and_remove(GrdSpan<T>* arr, GrdSpan<T> item) {
	s64 index = grd_index_of(arr, item);
	if (index == -1) {
		return false;
	}
	grd_remove(arr, index, grd_len(item));
	return true;
}

template <typename T>
bool grd_contains(GrdSpan<T> span, auto item) {
	return grd_index_of(span, item) != -1;
}

template <typename T>
s64 grd_len(std::initializer_list<T> list) {
	return list.size();
}

template <typename T>
s64 grd_index_of(std::initializer_list<T> list, auto item) {
	for (auto i: grd_range(grd_len(list))) {
		if (list.begin()[i] == item) {
			return i;
		}
	}
	return -1;
}

template <typename T>
bool grd_contains(std::initializer_list<T> list, auto item) {
	return grd_index_of(list, item) != -1;
}

template <typename T>
s64 grd_count_occurances(GrdSpan<T> src, GrdSpan<T> entry) {
	s64 cnt = 0;
	for (s64 i = 0; i < grd_len(src) - grd_len(entry); i++) {
		if (grd_starts_with(src[{i, {}}], entry)) {
			cnt += 1;
			i += grd_len(entry) - 1;
		}
	}
	return cnt;
}

template <typename T>
void grd_type_hash(GrdHasher* hasher, GrdSpan<T> array) {
	grd_update(hasher, array.count);
	for (auto item: array) {
		grd_update(hasher, item);
	}
}

template <typename T>
GrdSpanType* grd_reflect_create_type(GrdSpan<T>* x) {
	return grd_reflect_add_type_named<GrdSpan<T>, GrdSpanType>("");
}

template <typename T>
void grd_reflect_type(GrdSpan<T>* x, GrdSpanType* type) {
	type->inner = grd_reflect_type_of<T>();
	type->name = grd_heap_sprintf("GrdSpan<%s>", type->inner->name);
	type->subkind = "span";
	type->get_count = [](void* arr) {
		auto casted = (GrdSpan<int>*) arr;
		return casted->count;
	};
	type->get_item = [](void* arr, s64 index) {
		auto casted = (GrdSpan<int>*) arr;
		void* item = grd_ptr_add(casted->data, sizeof(T) * index);
		return item;
	};
}

template <typename T>
void grd_reverse(GrdSpan<T> span) {
	grd_reverse(span.data, span.count);
}
