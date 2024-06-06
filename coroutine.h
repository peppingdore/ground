#pragma once

#include <coroutine>
#include <utility>

template <typename T>
struct Generator {
	using ItemType = T;

	struct promise_type {
		T stored_value;

		Generator get_return_object() {
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

	Generator(std::coroutine_handle<promise_type> h): handle(h) {};
	Generator& operator=(const Generator&) = delete;
	Generator (const Generator&) = delete;
	Generator (Generator&& rhs): handle(rhs.handle) {
		rhs.handle = {};
	};

	~Generator() {
		if (handle) {
			handle.destroy();
		}
	}

	Generator& operator=(Generator&& other) {
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
		Generator* gen = NULL;

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
struct IsGeneratorType: std::false_type {};
template <typename T>
struct IsGeneratorType<Generator<T>>: std::true_type {};

template <typename T>
concept HasGeneratorIterator = requires (T t) {
	requires IsGeneratorType<decltype(t.iterate())>::value;
};

auto begin(HasGeneratorIterator auto t) {
	return t.iterate();
}

int end(HasGeneratorIterator auto t) {
	return 0;
}
