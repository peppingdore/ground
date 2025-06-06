#pragma once

#include <coroutine>
#include <utility>

template <typename T>
struct GrdGenerator {
	using ItemType = T;

	struct promise_type {
		T stored_value;

		GrdGenerator get_return_object() {
			return { std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		auto yield_value(T value) {
			stored_value = value;
			return std::suspend_always{};
		}

		auto initial_suspend() {
			return std::suspend_never{};
		}

		auto final_suspend() noexcept {
			return std::suspend_always{};
		}

		void unhandled_exception() {

		}

		void return_void() {

		}
	};


	std::coroutine_handle<promise_type> handle;

	GrdGenerator(std::coroutine_handle<promise_type> h): handle(h) {};
	GrdGenerator& operator=(const GrdGenerator&) = delete;
	GrdGenerator (const GrdGenerator&) = delete;
	GrdGenerator (GrdGenerator&& rhs): handle(rhs.handle) {
		rhs.handle = {};
	};

	~GrdGenerator() {
		if (handle) {
			handle.destroy();
		}
	}

	GrdGenerator& operator=(GrdGenerator&& other) {
		if (this != &other) {
			if (handle) {
				handle.destroy();
			}
			handle = other.handle;
			other.handle = {};
		}
		return *this;
	}

	bool next(T* out_value) {
		if (handle.done()) {
			return false;
		}
		handle.resume();
		*out_value = handle.promise().stored_value;
		return true;
	}

	struct Range {
		GrdGenerator* gen = NULL;

		bool operator!=(int rhs /*ignored*/) {
			return !gen->handle.done();
		}

		void operator++() {
			gen->handle.resume();
		}

		T operator*() {
			return gen->handle.promise().stored_value;
		}
	};

	void operator++() {
		handle.resume();
	}

	T operator*() {
		return handle.promise().stored_value;
	}

	bool operator!=(int rhs /*ignored*/) {
		return !handle.done();
	}

	auto begin() {
		return Range{ .gen = this };
	}

	int end() {
		return 0;
	}
};

template <typename T>
struct IsGrdGeneratorType: std::false_type {};
template <typename T>
struct IsGrdGeneratorType<GrdGenerator<T>>: std::true_type {};

template <typename T>
concept GrdHasMemberGeneratorIterator = requires (T t) {
	requires IsGrdGeneratorType<decltype(t.iterate())>::value;
};

template <typename T>
concept GrdHasGlobalGeneratorIterator = requires (T t) {
	requires IsGrdGeneratorType<decltype(grd_iterate(t))>::value;
};

GRD_DEDUP auto begin(GrdHasMemberGeneratorIterator auto t) {
	return t.iterate();
}

GRD_DEDUP int end(GrdHasMemberGeneratorIterator auto t) {
	return 0;
}

GRD_DEDUP auto begin(GrdHasGlobalGeneratorIterator auto t) {
	return grd_iterate(t);
}

GRD_DEDUP int end(GrdHasGlobalGeneratorIterator auto t) {
	return 0;
}

template <typename T, typename V> // Can't use |auto|, MSVC bitches.  
GRD_DEF grd_map(T iter, V func) -> GrdGenerator<decltype(func(*begin(iter)))> {
	for (auto it: iter) {
		co_yield func(it);
	}
}
