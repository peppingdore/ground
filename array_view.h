#pragma once

#include "allocator.h"
#include "reflection.h"

template <typename T>
struct Array_View {

	T*  data  = NULL;
	s64 count = 0;


	void advance(s64 length) {
		assert(length <= count);
		data += length;
		count -= length;
	}

	decltype(auto) advanced(s64 length) {
		auto copy = *this;
		copy.advance(length);
		return copy;
	}

	void slice(s64 start, s64 length) {
		advance(start);
		assert(count >= length);
		count = length;
	}

	decltype(auto) sliced(s64 start, s64 length) {
		auto copy = *this;
		copy.slice(start, length);
		return copy;
	}

	void slice(s64 slice_start) {
		slice(slice_start, count - slice_start);
	}
	decltype(auto) sliced(s64 start) {
		auto copy = *this;
		copy.slice(start);
		return copy;
	}

	T* operator[](s64 index) const {
		assert(index >= 0 && index < count);
		return data + index;
	}


	T* first() const {
		assert(count > 0);
		return data;
	} 


	T* last() const {
		assert(count > 0);
		return data + (count - 1);
	}

	T* last_or_null() {
		if (count > 0) {
			return last();
		}
		return NULL;
	}

	T last_or(T value) const {
		if (count > 0) {
			return *last();
		}
		return value;
	}
	
	s64 index_of(const T& item, s64 starting_index = 0) {
		for (s64 i = starting_index; i < count; i++) {
			if (data[i] == item) {
				return i;
			}
		}
		return -1;
	}

	bool is_item_unique(const T& item) {
		s64 first_index = index_of(item);
		assert(first_index != -1);
		return index_of(item, first_index + 1) == -1;
	}

	s64 index_of_fast_no_bounds_check(const T* ptr) {
		return ptr - data;
	}

	s64 index_of_fast(const T* ptr) {
		s64 result = index_of_fast_no_bounds_check(ptr);
		assert(result >= 0 && result < count);
		return result;
	}

	bool is_last(T* ptr) {
		if (index_of_fast(ptr) == (count - 1)) {
			return true;
		}
		
		return false;
	}

	bool contains(const T& item) {
		for (s64 i = 0; i < count; i++) {
			if (data[i] == item) {
				return true;
			}
		}
		return false;
	}

	T* get_or_null(s64 index) {
		if (index < 0 || index >= count) {
			return NULL;
		}

		return &data[index];
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + count;
	}

	bool operator==(Array_View rhs) {
		if (count != rhs.count) {
			return false;
		}
		if (data == rhs.data) {
			return true;
		}

		for (auto i: range(count)) {
			if (data[i] != rhs.data[i]) {
				return false;
			}
		}
		return true;
	}
};

template <typename T, s64 N>
inline constexpr Array_View<T> make_array_view(const T (&literal)[N]) {
	return {
		.data = (T*) literal,
		.count = N
	};
}

template <typename T>
inline constexpr Array_View<T> make_array_view(std::initializer_list<T> list) {
	return {
		.data  = (T*) list.begin(),
		.count = (s64) list.size()
	};
}

template <typename T>
inline constexpr Array_View<T> make_array_view(T* data, s64 count) {
	return {
		.data = data,
		.count = count
	};
}

template <typename T>
inline void reverse(Array_View<T> view) {
	reverse(view.data, view.count);
}

struct Array_View_Type: Type {
	constexpr static const char* KIND = "array_view";

	Type* inner = NULL;
	s64   (*get_count)   (Array_Type*, void* view) = NULL;
	void* (*get_item)    (Array_Type*, void* view, s64 index) = NULL;
};

template <typename T>
Array_View_Type* reflect_type(Array_View<T>* x, Array_View_Type* type) {
	type->inner = reflect.type_of<T>();
	type->name = heap_sprintf("Array_View<%s>", type->inner->name);
	type->get_count = [](Array_View_Type* type, void* view) {
		auto casted = (Array_View<int>*) view;
		return casted->count;
	};
	type->get_item = [](Array_View_Type* type, void* view, s64 index) {
		auto casted = (Array_View<int>*) view;
		void* item = ptr_add(casted->data, type->inner->size * index);
		return item;
	};
	return type;
}
