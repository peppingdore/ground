#pragma once

#include <coroutine>

template <typename T>
struct Generator {
	using Item_Type = T;

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

	auto begin() {
		return std::move(*this);
	}

	int end() {
		return 0;
	}

	bool operator!=(int rhs /*ignored*/) {
		return !handle.done();
	}

	void operator++() {
		handle.resume();
	}

	T operator*() {
		return handle.promise().stored_value;
	}
};

template <typename T>
struct Is_Generator_Type: std::false_type {};
template <typename T>
struct Is_Generator_Type<Generator<T>>: std::true_type {};

template <typename T>
concept Has_Generator_Iterator = requires (T t) {
	requires Is_Generator_Type<decltype(t.iterate())>::value;
};

auto begin(Has_Generator_Iterator auto t) {
	return t.iterate();
}

int end(Has_Generator_Iterator auto t) {
	return 0;
}
