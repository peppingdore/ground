#pragma once

#include "reflection.h"
#include <initializer_list>

template <typename... Args>
struct Tuple;

template <>
struct Tuple<> {
	constexpr static u32 size = 0;
};

template <typename A>
struct Tuple<A> {
	constexpr static u32 size = 1;

	A _0;
};

template <typename A, typename B>
struct Tuple<A, B> {
	constexpr static u32 size = 2;

	A _0;
	B _1;
};

template <typename A, typename B, typename C>
struct Tuple<A, B, C> {
	constexpr static u32 size = 3;

	A _0;
	B _1;
	C _2;
};

template <typename A, typename B, typename C, typename D>
struct Tuple<A, B, C, D> {
	constexpr static u32 size = 4;

    A _0;
    B _1;
    C _2;
    D _3;
};

template <typename A, typename B, typename C, typename D, typename E>
struct Tuple<A, B, C, D, E> {
	constexpr static u32 size = 5;

    A _0;
    B _1;
    C _2;
    D _3;
    E _4;
};

template <typename A, typename B, typename C, typename D, typename E, typename F>
struct Tuple<A, B, C, D, E, F> {
	constexpr static u32 size = 6;

    A _0;
    B _1;
    C _2;
    D _3;
    E _4;
    F _5;
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
struct Tuple<A, B, C, D, E, F, G> {
	constexpr static u32 size = 7;

    A _0;
    B _1;
    C _2;
    D _3;
    E _4;
    F _5;
    G _6;
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
struct Tuple<A, B, C, D, E, F, G, H> {
	constexpr static u32 size = 8;

    A _0;
    B _1;
    C _2;
    D _3;
    E _4;
    F _5;
    G _6;
    H _7;
};

template <int N>
struct TupleGetter;
#define TUPLE_GET(N) template <> struct TupleGetter<N> { auto* operator()(auto* tuple) { return &tuple->_##N; } };
TUPLE_GET(0);
TUPLE_GET(1);
TUPLE_GET(2);
TUPLE_GET(3);
TUPLE_GET(4);
TUPLE_GET(5);
TUPLE_GET(6);
TUPLE_GET(7);

template <int N>
auto* tuple_get_ptr(auto* tuple) {
	return TupleGetter<N>{}(tuple);
}

template <int N>
auto& tuple_get(auto tuple) {
	return *tuple_get_ptr<N>(&tuple);
}

auto make_tuple(auto... args) {
	return Tuple<decltype(args)...> { args... };
}

template <int Index>
int tuple_reflect_member(StructType* type, s64* capacity, auto* tuple) {
	static char name[3];
	strcpy(name, "_0");
	name[1] = '0' + Index;

	auto member = tuple_get_ptr<Index>(tuple);
	auto offset = pointer_diff(member, tuple);
	auto member_type = reflect.type_of<std::remove_pointer_t<decltype(member)>>();
	auto m = StructMember {
		.name = name, 
		.type = member_type,
		.offset = s32(offset),
	};
	reflect_struct_type_add_member(type, capacity, m);
	return 0;
}

void tuple_reflect_dummy(std::initializer_list<int> x) {

}

template <typename... Args>
StructType* reflect_type(Tuple<Args...>* tuple, StructType* type) {
	s64 members_capacity = 0;
	auto impl = [&]
		<s64... Indices>
		(std::integer_sequence<s64, Indices...>) {
		type->name = heap_join("Tuple<", "%s", ">",
			reflect.type_of<
				std::remove_pointer_t<
					decltype(tuple_get_ptr<Indices>(tuple))
				>
			>()->name...);
		// Vanilla function call arguments have undefined evaluation order,
		//   so in order to reflect members in order they are declared in,
		//   we use initializer_list, which has defined(left to right) evaluation order.
		tuple_reflect_dummy({ tuple_reflect_member<Indices>(type, &members_capacity, tuple)... });
	};
	impl(std::make_integer_sequence<s64, Tuple<Args...>::size>{});

	type->subkind = "tuple";
	return type;
}
