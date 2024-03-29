#pragma once

#include "allocator.h"
#include "array_view.h"
#include "reflect.h"
#include "coroutine.h"

#include <string.h>
#include <initializer_list>

constexpr s64 DEFAULT_DYNAMIC_ARRAY_CAPACITY = 8;

template <typename T>
struct Array: public ArrayView<T> {
	using ArrayView<T>::data;
	using ArrayView<T>::count;
	using ArrayView<T>::index_of;
	using ArrayView<T>::index_of_fast;

	s64          capacity  = 0;
	Allocator    allocator = c_allocator;
	CodeLocation loc       = caller_loc();


	Array& operator=(Array const& other) = default;

	s64 get_allocated_capacity() {
		return data ? capacity : 0;
	}

	void make_sure_initted() {
		if (data) {
			return;
		}
		if (capacity <= 0) {
			capacity = DEFAULT_DYNAMIC_ARRAY_CAPACITY;
		}
		data = allocator.alloc<T>(capacity, loc);
	}

	T* reserve_at_index(s64 index, s64 length, CodeLocation loc = caller_loc()) {
		assert(index >= 0 && index <= count);
		
		ensure_capacity(count + length, loc);
		memmove(data + index + length, data + index, (count - index) * sizeof(T));
		T* ptr = data + index;
		count += length;
		return ptr;
	}

	T* reserve(CodeLocation loc = caller_loc()) {
		return reserve_at_index(count, 1, loc);
	}

	T* add(T item, CodeLocation loc = caller_loc()) {
		auto item_location = reserve(loc);
		memcpy(item_location, &item, sizeof(T));
		return item_location;
	}

	T* add_at_index(s64 index, T item, CodeLocation loc = caller_loc()) {
		auto item_location = reserve_at_index(index, 1, loc);
		memcpy(item_location, &item, sizeof(T));
		return item_location;
	}

	bool add_or_replace_at_index(s64 index, T item, CodeLocation loc = caller_loc()) {
		if (index < count) {
			data[index] = item;
			return false;
		} else {
			add_at_index(index, item, loc);
			return true;
		}
	}

	void add_at_index(s64 index, T* src, s64 length, CodeLocation loc = caller_loc()) {

		assert(index <= count);
		T* dst = reserve_at_index(index, length, loc);
		memcpy(dst, src, length * sizeof(T));
	}

	void add(T* item, s64 length, CodeLocation loc = caller_loc()) {
		add_at_index(count, item, length, loc);
	}

	void add(ArrayView<T> other, CodeLocation loc = caller_loc()) {
		add(other.data, other.count, loc);
	}

	void add(std::initializer_list<T> other, CodeLocation loc = caller_loc()) {
		// (T*) cast is to remove const from other.begin(), so the compiler doesn't bitch.
		add((T*) other.begin(), other.size(), loc);
	}

	void add_at_index(s64 index, ArrayView<T> other, CodeLocation loc = caller_loc()) {
		add_at_index(index, other.data, other.count, loc);
	}

	void add_sorted(T item, auto key_proc) {
		auto index = bisect_rightmost(this, item, key_proc);
		add_at_index(index, item);
	}

	void add_sorted(T item) {
		add_sorted(item, [](auto i) { return i; });
	}

	void remove_fast(T* ptr) {
		remove_at_index(index_of_fast(ptr));
	}

	void remove_at_index(s64 index, s64 remove_count) {
		assert(remove_count <= (count - index));
		count -= remove_count;
		memmove(data + index, data + index + remove_count, (count - index) * sizeof(T));
	}

	void remove_at_index(s64 index) {
		remove_at_index(index, 1);
	}

	bool remove(T item) {
		auto index = index_of(item);
		if (index != -1) {
			remove_at_index(index);
			return true;
		} else {
			return false;
		}
	}

	bool remove_last() {
		if (count > 0) {
			count -= 1;
			return true;
		} else {
			return false;
		}
	}

	void clear(CodeLocation loc = caller_loc()) {
		count = 0;
	}

	void free(CodeLocation loc = caller_loc()) {
		if (data) {
			allocator.free(data, loc);
			data  = NULL;
			count = 0;
		}
	}

	T pop_last(CodeLocation loc = caller_loc()) {
		assert(count > 0);
		count -= 1;
		T result = data[count];
		return result;
	}

	void ensure_capacity(s64 target_capacity, CodeLocation loc = caller_loc()) {
		if (data == NULL && target_capacity > capacity) {
			capacity = target_capacity;
		}

		make_sure_initted();

		if (target_capacity > capacity) {
			s64 old_capacity = capacity;
			assert(old_capacity > 0);
			capacity = max(old_capacity * 2, target_capacity);
			data     = allocator.realloc<T>(data, old_capacity, capacity, loc);
		}
	}
	
	void shrink_to_capacity(s64 new_capacity, CodeLocation loc = caller_loc()) {
		if (!data) {
			return;
		}

		assert(new_capacity > 0);
		if (capacity <= new_capacity) {
			return;
		}
		assert(new_capacity >= count);
		data = allocator.realloc<T>(data, capacity, new_capacity, loc);
		capacity = new_capacity;
	}

	Array<T> copy(Allocator allocator = c_allocator, CodeLocation loc = caller_loc()) {
		Array<T> copied;
		copied.allocator = allocator;
		copied.allocator.capacity = capacity;
		copied.add(this);
		copied.loc = loc;
		return copied;
	}
};

template <typename T>
inline Array<T> make_array(Allocator allocator = c_allocator, s64 capacity = DEFAULT_DYNAMIC_ARRAY_CAPACITY, CodeLocation loc = caller_loc()) {
	Array<T> result;
	result.allocator = allocator;
	result.capacity = capacity;
	return result;
}

template <typename T>
inline Array<T> make_array(Allocator allocator, ArrayView<T> view, CodeLocation loc = caller_loc()) {
	auto arr = make_array<T>(allocator, view.count, loc);
	arr.add(view);
	return arr;
}

template <typename T>
inline Array<T> make_array(ArrayView<T> view, CodeLocation loc = caller_loc()) {
	return make_array(c_allocator, view, loc);
}

template <typename T>
inline Array<T> make_array(Allocator allocator, std::initializer_list<T> list, CodeLocation loc = caller_loc()) {
	return make_array(allocator, make_array_view(list), loc);
}

template <typename T>
inline Array<T> make_array(std::initializer_list<T> list, CodeLocation loc = caller_loc()) {
	return make_array(c_allocator, list, loc);
}

template <typename T>
inline void make_array(Array<T>* array, Allocator allocator = c_allocator, s64 capacity = DEFAULT_DYNAMIC_ARRAY_CAPACITY, CodeLocation loc = caller_loc()) {
	*array = make_array<T>(allocator, capacity, loc); 
}

template <typename T>
ArrayType* reflect_type(Array<T>* x, ArrayType* type) {
	type->inner = reflect_type_of<T>();
	type->name = heap_sprintf("Array<%s>", type->inner->name);
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

template <typename T>
Array<T> to_array(Allocator allocator, Generator<T>&& generator) {
	Array<T> arr = { .allocator = allocator };
	for (auto it: generator) {
		arr.add(it);
	}
	return arr;
}

template <typename T>
Array<T> to_array(Generator<T>&& generator) {
	return to_array(c_allocator, std::move(generator));
}
