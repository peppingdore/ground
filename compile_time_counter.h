#pragma once

#include "base.h"

template <auto tag = []{}>
struct CTCounter {
	// Stolen from: https://mc-deltat.github.io/articles/stateful-metaprogramming-cpp20
	template <u64 N>
	struct CounterReader { 
		friend auto counted_flag(CounterReader<N>);
	};

	template <u64 N>
	struct CounterSetter {
		friend auto counted_flag(CounterReader<N>) {}
		static constexpr unsigned n = N;
	};

	template <auto Tag, u64 Next = 0>
	consteval static u64 counter_increment() {
		constexpr bool counted_past_value = requires(CounterReader<Next> r) {
			counted_flag(r);
		};

		if constexpr (counted_past_value) {
			return counter_increment<Tag, Next + 1>();
		} else {
			CounterSetter<Next> s;
			return s.n + 1;
		}
	}

	template <auto Tag, u64 Next = 0>
	consteval static u64 counter_read() {
		constexpr bool counted_past_value = requires(CounterReader<Next> r) {
			counted_flag(r);
		};

		if constexpr (counted_past_value) {
			return counter_read<Tag, Next + 1>();
		} else {
			return Next;
		}
	}


	template <auto Tag = []{}, u64 Val = counter_increment<Tag>()>
	constexpr static u64 Increment = Val;

	template <auto Tag = []{}, u64 Val = counter_read<Tag>()>
	constexpr static u64 Read = Val;
};
