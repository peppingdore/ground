#pragma once

#include "grd_reflect.h"
#include <initializer_list>

#define GRD_TUPLE_MEMBER(T, N) T N = {};
#define GRD_TUPLE_HASH(T, N) update(hasher, N);
#define GRD_TUPLE_EQ(T, N) if (N != rhs.N) return false;

template <typename... Args>
struct GrdTuple;

template <>
struct GrdTuple<> {
	constexpr static u32 size = 0;

	void hash(GrdHasher* hasher) {
		// nop
	}
	bool operator==(GrdTuple rhs) {
		return true;
	}
};

template <typename A>
struct GrdTuple<A> {
	constexpr static u32 size = 1;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0)
	
	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B>
struct GrdTuple<A, B> {
	constexpr static u32 size = 2;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C>
struct GrdTuple<A, B, C> {
	constexpr static u32 size = 3;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C, typename D>
struct GrdTuple<A, B, C, D> {
	constexpr static u32 size = 4;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C, typename D, typename E>
struct GrdTuple<A, B, C, D, E> {
	constexpr static u32 size = 5;

   #define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C, typename D, typename E, typename F>
struct GrdTuple<A, B, C, D, E, F> {
	constexpr static u32 size = 6;
	
	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
struct GrdTuple<A, B, C, D, E, F, G> {
	constexpr static u32 size = 7;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5) \
		func(G, _6)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
struct GrdTuple<A, B, C, D, E, F, G, H> {
	constexpr static u32 size = 8;

	#define GRD_TUPLE_LIST(func) \
		func(A, _0) \
		func(B, _1) \
		func(C, _2) \
		func(D, _3) \
		func(E, _4) \
		func(F, _5) \
		func(G, _6) \
		func(H, _7)

	GRD_TUPLE_LIST(GRD_TUPLE_MEMBER)
	void hash(GrdHasher* hasher) {
		GRD_TUPLE_LIST(GRD_TUPLE_HASH)
	}
	bool operator==(GrdTuple rhs) {
		GRD_TUPLE_LIST(GRD_TUPLE_EQ)
		return true;
	}
	#undef GRD_TUPLE_LIST
};

#undef GRD_TUPLE_MEMBER
#undef GRD_TUPLE_HASH
#undef GRD_TUPLE_EQ

template <int N>
struct GrdTupleGetter;
#define GRD_TUPLE_GET(N) template <> struct GrdTupleGetter<N> { auto* operator()(auto* tuple) { return &tuple->_##N; } };
GRD_TUPLE_GET(0);
GRD_TUPLE_GET(1);
GRD_TUPLE_GET(2);
GRD_TUPLE_GET(3);
GRD_TUPLE_GET(4);
GRD_TUPLE_GET(5);
GRD_TUPLE_GET(6);
GRD_TUPLE_GET(7);

template <int N>
auto* grd_tuple_get_ptr(auto* tuple) {
	return GrdTupleGetter<N>{}(tuple);
}

template <int N>
auto& tuple_get(auto tuple) {
	return *grd_tuple_get_ptr<N>(&tuple);
}

auto grd_make_tuple(auto... args) {
	return GrdTuple<decltype(args)...> { args... };
}

template <int Index>
int tuple_reflect_member(GrdStructType* type, auto* tuple) {
	static char name[3];
	strcpy(name, "_0");
	name[1] = '0' + Index;

	auto member = grd_tuple_get_ptr<Index>(tuple);
	auto offset = grd_ptr_diff(member, tuple);
	auto member_type = grd_reflect_type_of<std::remove_pointer_t<decltype(member)>>();
	auto m = GrdStructMember {
		.name = name, 
		.type = member_type,
		.offset = s32(offset),
	};
	grd_reflect_struct_type_add_member(type, m);
	return 0;
}

void grd_tuple_reflect_dummy(std::initializer_list<int> x) {

}

template <typename... Args>
GrdStructType* grd_reflect_type(GrdTuple<Args...>* tuple, GrdStructType* type) {
	auto impl = [&]
		<s64... Indices>
		(std::integer_sequence<s64, Indices...>) {
		type->name = grd_heap_join("GrdTuple<", "%s", ">",
			grd_reflect_type_of<
				std::remove_pointer_t<
					decltype(grd_tuple_get_ptr<Indices>(tuple))
				>
			>()->name...);
		// Vanilla function call arguments have undefined evaluation order,
		//   so in order to reflect members in order they are declared in,
		//   we use initializer_list, which has defined(left to right) evaluation order.
		grd_tuple_reflect_dummy({ tuple_reflect_member<Indices>(type, tuple)... });
	};
	impl(std::make_integer_sequence<s64, GrdTuple<Args...>::size>{});

	type->subkind = "tuple";
	return type;
}
