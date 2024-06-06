#pragma once

#include "reflect.h"
#include <initializer_list>

template <typename... Args>
struct Tuple;

template <>
struct Tuple<> {
	constexpr static u32 size = 0;

	void hash(Hasher* hasher) {
		// nop
	}
	bool operator==(Tuple rhs) {
		return true;
	}
};

template <typename A>
struct Tuple<A> {
	constexpr static u32 size = 1;

	#define TUPLE_LIST(func) \
		func(A, _0)
	
	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B>
struct Tuple<A, B> {
	constexpr static u32 size = 2;

	#define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C>
struct Tuple<A, B, C> {
	constexpr static u32 size = 3;

	#define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C, typename D>
struct Tuple<A, B, C, D> {
	constexpr static u32 size = 4;

	#define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C, typename D, typename E>
struct Tuple<A, B, C, D, E> {
	constexpr static u32 size = 5;

   #define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C, typename D, typename E, typename F>
struct Tuple<A, B, C, D, E, F> {
	constexpr static u32 size = 6;
	
	#define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
struct Tuple<A, B, C, D, E, F, G> {
	constexpr static u32 size = 7;

	#define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5) \
		func(G, _6)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
struct Tuple<A, B, C, D, E, F, G, H> {
	constexpr static u32 size = 8;

   #define TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5) \
		func(G, _6) \
		func(H, _7)

	#define TUPLE_MEMBER(T, N) T N;
	TUPLE_LIST(TUPLE_MEMBER)
	void hash(Hasher* hasher) {
		#define TUPLE_HASH(T, N) hasher->hash(N);
		TUPLE_LIST(TUPLE_HASH)
	}
	bool operator==(Tuple rhs) {
		#define TUPLE_EQ(T, N) if (N != rhs.N) return false;
		TUPLE_LIST(TUPLE_EQ)
		return true;
	}
	#undef TUPLE_LIST
	#undef TUPLE_MEMBER
	#undef TUPLE_HASH
	#undef TUPLE_EQ
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
int tuple_reflect_member(StructType* type, auto* tuple) {
	static char name[3];
	strcpy(name, "_0");
	name[1] = '0' + Index;

	auto member = tuple_get_ptr<Index>(tuple);
	auto offset = pointer_diff(member, tuple);
	auto member_type = reflect_type_of<std::remove_pointer_t<decltype(member)>>();
	auto m = StructMember {
		.name = name, 
		.type = member_type,
		.offset = s32(offset),
	};
	reflect_struct_type_add_member(type, m);
	return 0;
}

void tuple_reflect_dummy(std::initializer_list<int> x) {

}

template <typename... Args>
StructType* reflect_type(Tuple<Args...>* tuple, StructType* type) {
	auto impl = [&]
		<s64... Indices>
		(std::integer_sequence<s64, Indices...>) {
		type->name = heap_join("Tuple<", "%s", ">",
			reflect_type_of<
				std::remove_pointer_t<
					decltype(tuple_get_ptr<Indices>(tuple))
				>
			>()->name...);
		// Vanilla function call arguments have undefined evaluation order,
		//   so in order to reflect members in order they are declared in,
		//   we use initializer_list, which has defined(left to right) evaluation order.
		tuple_reflect_dummy({ tuple_reflect_member<Indices>(type, tuple)... });
	};
	impl(std::make_integer_sequence<s64, Tuple<Args...>::size>{});

	type->subkind = "tuple";
	return type;
}
