#pragma once

#include "allocator.h"
#include "reflection.h"
#include "hash.h"
#include <initializer_list>

template <typename T>
struct ArrayView {
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

	bool operator==(ArrayView rhs) {
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
inline constexpr ArrayView<T> make_array_view(const T (&literal)[N]) {
	return {
		.data = (T*) literal,
		.count = N
	};
}

template <typename T>
inline constexpr ArrayView<T> make_array_view(std::initializer_list<T> list) {
	return {
		.data  = (T*) list.begin(),
		.count = (s64) list.size()
	};
}

template <typename T>
inline constexpr ArrayView<T> make_array_view(T* data, s64 count) {
	return {
		.data = data,
		.count = count
	};
}

template <typename T>
inline void reverse(ArrayView<T> view) {
	reverse(view.data, view.count);
}

template <typename T>
s64 len(ArrayView<T> view) {
	return view.count;
}

template <typename T>
void type_hash(Hasher* hasher, ArrayView<T> array) {
	hasher->hash(array.count);
	for (auto item: array) {
		hasher->hash(item);
	}
}

template <typename T>
ArrayType* reflect_type(ArrayView<T>* x, ArrayType* type) {
	type->inner = reflect.type_of<T>();
	type->name = heap_sprintf("ArrayView<%s>", type->inner->name);
	type->subkind = "ArrayView";
	type->get_count = [](void* arr) {
		auto casted = (ArrayView<int>*) arr;
		return casted->count;
	};
	type->get_capacity = [](void* arr) {
		auto casted = (ArrayView<int>*) arr;
		return casted->count;
	};
	type->get_item = [](void* arr, s64 index) {
		auto casted = (ArrayView<int>*) arr;
		void* item = ptr_add(casted->data, sizeof(T) * index);
		return item;
	};
	return type;
}
