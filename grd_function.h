#pragma once

#include "grd_base.h"
#include "grd_tuple.h"
#include "grd_allocator.h"
#include "grd_code_location.h"

#include <utility>

template <typename... T>
struct GrdDecomposedFunctionType;

template <typename __ReturnType, typename... Args>
struct GrdDecomposedFunctionType<__ReturnType(Args...)> {
	using ReturnType = __ReturnType;
	using ArgsTupleType = GrdTuple<Args...>;
};

template <typename Class, typename __ReturnType, typename... Args>
struct GrdDecomposedMemberFunctionType {
	using RawFunctionType = GrdDecomposedFunctionType<__ReturnType(Args...)>;
	// using ReturnType = __ReturnType;
};

template <typename ReturnType, typename... Args>
GrdDecomposedFunctionType<ReturnType(Args...)> grd_decompose_function_type( ReturnType (*) (Args...)) {
	return {};
}

template <typename Class, typename ReturnType, typename... Args>
GrdDecomposedMemberFunctionType<Class, ReturnType, Args...> grd_decompose_member_function_type(ReturnType(Class::*)(Args...)) {
	return {};
}

template <typename Class, typename ReturnType, typename... Args>
GrdDecomposedMemberFunctionType<Class, ReturnType, Args...> grd_decompose_member_function_type(ReturnType(Class::*)(Args...) const) {
	return {};
}

auto grd_decompose_function_type(auto lambda) -> decltype(auto) {
	using Decomposed = decltype(grd_decompose_member_function_type(&lambda.operator())());
	return typename Decomposed::RawFunctionType();
}

// Question of syntax taste.
// Allows us to use Proc<int(int)>* syntax instead of int (*ptr)(int).
template <typename T>
using GrdProc = T;

enum class GrdFunctionKind: u8 {
	None = 0,             // Function is empty
	Small_Lambda = 1,     // Lambda that can fit inside a small optimization container.
	Big_Lambda = 2,       // Lambda that has to be allocated separately.
	Function_Pointer = 3, // Simple C function pointer. 
};

template <typename... T>
struct GrdFunction {
	static_assert(sizeof...(T) == 0, "Use function type syntax, not <R, Args...>, but <R(Args...)");
};

constexpr u64 SMALL_LAMBDA_STORAGE_SIZE = 24; // 24 + lambda flattened_proc ptr (8) = 32 bytes.

// We tradeoff readibility for struct size.
template <typename ReturnType, typename... ArgsTypes>
struct GrdFunction<ReturnType(ArgsTypes...)> {
	union {
		struct {
			ReturnType (*ptr)(ArgsTypes...);
		} function_ptr;

		struct {
			union {
				struct {
					u8 storage[SMALL_LAMBDA_STORAGE_SIZE];
				} small_lambda;

				struct  {
					void* mem;
					s64   size;
				} big_lambda;
			};

			ReturnType (*flattened_proc)(GrdFunction*, ArgsTypes...);
		} lambda;
	};

	GrdFunctionKind kind                     : 4 = GrdFunctionKind::None;
	bool         is_using_custom_allocator: 4 = false;

	bool is_empty() {
		return kind == GrdFunctionKind::None;
	}

	ReturnType operator()(ArgsTypes... args) {
		switch (kind) {
			case GrdFunctionKind::Function_Pointer: {
				return function_ptr.ptr(args...);
			}
			break;

			case GrdFunctionKind::Big_Lambda:
			case GrdFunctionKind::Small_Lambda: {
				return lambda.flattened_proc(this, args...);
			}
			break;

			default:
				assert(false);
				return ReturnType();
		}
	}

	void replace_with(GrdFunction other, GrdCodeLoc loc = grd_caller_loc()) {
		free();
		*this = other;
	}

	void operator<<=(auto other) {
		replace_with(grd_make_function(other));
	}

	GrdFunction copy(GrdCodeLoc loc = grd_caller_loc()) const {
		auto copied = *this;
		if (kind == GrdFunctionKind::Big_Lambda) {
			void* new_mem = GrdMalloc(lambda.big_lambda.size, loc);
			memcpy(new_mem, lambda.big_lambda.mem, lambda.big_lambda.size);
			copied.lambda.big_lambda.mem = new_mem;
		}
		return copied;
	}

	void free(GrdCodeLoc loc = grd_caller_loc()) {
		assert(!is_using_custom_allocator);
		if (kind == GrdFunctionKind::Big_Lambda){
			GrdFree(lambda.big_lambda.mem, loc);
		}
		kind = GrdFunctionKind::None;
	}

	void free(GrdAllocator allocator, GrdCodeLoc loc = grd_caller_loc()) {
		assert(is_using_custom_allocator);
		if (kind == GrdFunctionKind::Big_Lambda) {
			GrdFree(allocator, lambda.big_lambda.mem, loc);
		}
		kind = GrdFunctionKind::None;
		is_using_custom_allocator = false;
	}
};


template <typename ReturnType, typename... ArgsTypes>
auto grd_make_function(ReturnType (*function_ptr)(ArgsTypes...)) {
	GrdFunction<ReturnType(ArgsTypes...)> result;
	result.kind = GrdFunctionKind::Function_Pointer;
	result.function_ptr.ptr = function_ptr;
	return result;
}

// Assert that we do not use heap allocator.
auto grd_make_no_heap_function(auto lambda, GrdCodeLoc loc = grd_caller_loc()) {
	static_assert(sizeof(lambda) <= SMALL_LAMBDA_STORAGE_SIZE);
	return grd_make_function(lambda, loc);
}

auto grd_make_function(auto lambda, GrdCodeLoc loc = grd_caller_loc()) {
	using Lambda_Type = decltype(lambda);

	// static_assert(std::is_same_v<std::invoke_result_t<Lambda_Type, ArgsTypes...>, ReturnType>, "Lambda return type is incorrect");
	// using Decomposed = Decompose_Member_FunctionType<decltype(&Lambda_Type::operator())>;

	auto do_stuff = [&]
		<typename ReturnType, typename... ArgsTypes>
		( const GrdDecomposedMemberFunctionType<Lambda_Type, ReturnType, ArgsTypes...>) {
		
		using FlattenedType = GrdFunction<ReturnType(ArgsTypes...)>;

		FlattenedType result;

		if constexpr (SMALL_LAMBDA_STORAGE_SIZE < sizeof(lambda)) {
			
			void* mem = GrdMalloc(sizeof(lambda), loc);
			memcpy(mem, &lambda, sizeof(lambda));

			auto flattened_proc = +[](FlattenedType* flattened, ArgsTypes... args) -> ReturnType {
				auto lambda = (Lambda_Type*) flattened->lambda.big_lambda.mem;
				return lambda->operator()(args...);
			};

			result.lambda.flattened_proc = flattened_proc;
			result.lambda.big_lambda.mem = mem;
			result.lambda.big_lambda.size = sizeof(lambda);
			result.kind = GrdFunctionKind::Big_Lambda;
		
		} else {
			memcpy(&result.lambda.small_lambda.storage, &lambda, sizeof(lambda));
			auto flattened_proc = +[](FlattenedType* flattened, ArgsTypes... args) -> ReturnType {
				auto lambda = (Lambda_Type*) &flattened->lambda.small_lambda;

				return lambda->operator()(args...);
			};

			result.lambda.flattened_proc = flattened_proc;
			result.kind = GrdFunctionKind::Small_Lambda;
		}

		return result;
	};
	
	using Decomposed = decltype(grd_decompose_member_function_type(&Lambda_Type::operator()));

	auto result = do_stuff(Decomposed());
	assert(result.kind != GrdFunctionKind::None);
	return result;
}

//  If you do not like wrapping lambdas in the function call:
//
//      Function<int(int, int)> f = grd_make_function([&](int x, int y) {
//         return x + y + captured_variable;	
//	    });
//
//  You can rewrite code above as shown below:
// 
//      Function<int(int, int)> f = function_cast [&](int x, int y) {
//         return x + y + captured_variable;	
//	    };
//
//

struct __grd_cast2function {
	auto operator<<(auto f) {
		auto function = grd_make_function(f);
		return function;
	}
};

struct __grd_cast2noheapfunction {
	auto operator<<(auto f) {
		auto function = grd_make_no_heap_function(f);
		return function;
	}
};

#define grd_function_cast  __grd_cast2function() << 
#define grd_no_heap_function_cast  __grd_cast2noheapfunction() << 

#define grd_lambda_1(expr) [&] () { return expr; }
#define grd_lambda_2(a, expr) [&] (auto a) { return expr; }
#define grd_lambda_3(a, b, expr) [&] (auto a, auto b) { return expr; }
#define grd_lambda_4(a, b, c, expr) [&] (auto a, auto b, auto c) { return expr; }
#define grd_lambda_5(a, b, c, d, expr) [&] (auto a, auto b, auto c, auto d) { return expr; }
#define grd_lambda_6(a, b, c, d, e, expr) [&] (auto a, auto b, auto c, auto d, auto e) { return expr; }
#define grd_lambda_7(a, b, c, d, e, f, expr) [&] (auto a, auto b, auto c, auto d, auto e, auto f) { return expr; }
#define grd_lambda_8(a, b, c, d, e, f, g, expr) [&] (auto a, auto b, auto c, auto d, auto e, auto f, auto g) { return expr; }
#define grd_lambda_9(a, b, c, d, e, f, g, h, expr) [&] (auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h) { return expr; }
#define grd_lambda_10(a, b, c, d, e, f, g, h, i, expr) [&] (auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h, auto i) { return expr; }

#define grd_lambda(...) GRD_CONCAT(grd_lambda_, GRD_NARGS(__VA_ARGS__))(__VA_ARGS__)

// #define grd_lambda(expr) [&] (auto... lambda_args) { auto $ = grd_make_tuple(lambda_args...); return expr; }
// #define $0 tuple_get<0>(lambda_args_tuple)
// #define $1 tuple_get<1>(lambda_args_tuple)
// #define $2 tuple_get<2>(lambda_args_tuple)
// #define $3 tuple_get<3>(lambda_args_tuple)
// #define $4 tuple_get<4>(lambda_args_tuple)
// #define $5 tuple_get<5>(lambda_args_tuple)
// #define $6 tuple_get<6>(lambda_args_tuple)

// #define GRD_GET_VA_ARGS_ELEM_0(_0, ...) _0
// #define GRD_GET_VA_ARGS_ELEM_1(_0, _1, ...) _1
// #define GRD_GET_VA_ARGS_ELEM_2(_0, _1, _2, ...) _2
// #define GRD_GET_VA_ARGS_ELEM_3(_0, _1, _2, _3, ...) _3
// #define GRD_GET_VA_ARGS_ELEM_4(_0, _1, _2, _3, _4, ...) _4
// #define GRD_GET_VA_ARGS_ELEM_5(_0, _1, _2, _3, _4, _5, ...) _5
// #define GRD_GET_VA_ARGS_ELEM_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
// #define GRD_GET_VA_ARGS_ELEM_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
// #define GRD_GET_VA_ARGS_ELEM_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
// #define GRD_GET_VA_ARGS_ELEM_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
// #define GRD_GET_VA_ARGS_ELEM_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
// #define GRD_GET_ALL_BUT_LAST_ARG_0(_0, ...) 
// #define GRD_GET_ALL_BUT_LAST_ARG_1(_0, _1, ...) _0
// #define GRD_GET_ALL_BUT_LAST_ARG_2(_0, _1, _2, ...) _0, _1
// #define GRD_GET_ALL_BUT_LAST_ARG_3(_0, _1, _2, _3, ...) _0, _1, _2
// #define GRD_GET_ALL_BUT_LAST_ARG_4(_0, _1, _2, _3, _4, ...) _0, _1, _2, _3
// #define GRD_GET_ALL_BUT_LAST_ARG_5(_0, _1, _2, _3, _4, _5, ...) _0, _1, _2, _3, _4
// #define GRD_GET_ALL_BUT_LAST_ARG_6(_0, _1, _2, _3, _4, _5, _6, ...) _0, _1, _2, _3, _4, _5
// #define GRD_GET_ALL_BUT_LAST_ARG_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _0, _1, _2, _3, _4, _5, _6
// #define GRD_GET_ALL_BUT_LAST_ARG_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _0, _1, _2, _3, _4, _5, _6, _7
// #define GRD_GET_ALL_BUT_LAST_ARG_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _0, _1, _2, _3, _4, _5, _6, _7, _8
// #define GRD_GET_ALL_BUT_LAST_ARG_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _0, _1, _2, _3, _4, _5, _6, _7, _8, _9

// #define GRD_GET_VA_ARGS_ELEM(N, ...) GRD_CONCAT(GRD_GET_VA_ARGS_ELEM_, N)(__VA_ARGS__)
// #define GRD_GET_LAST_ARG(...) GRD_GET_VA_ARGS_ELEM(GRD_NARGS(__VA_ARGS__), _, __VA_ARGS__ ,,,,,,,,,,,)
// #define grd_lambda(...) [&] (auto... lambda_args) { auto $ = grd_make_tuple(lambda_args...); return GRD_GET_LAST_ARG(__VA_ARGS__); }


template <typename Candidate, typename FunctionType, typename D = GrdDecomposedFunctionType<FunctionType>>
concept GrdCallable = requires(Candidate f, FunctionType signature_type) {
	{ grd_call_with_tuple(f, std::declval<typename D::ArgsTupleType>()) } -> std::same_as<typename D::ReturnType>;
};

auto grd_call_with_tuple(auto f, auto tuple) {
	auto impl = []
		<s64... Indices>
		(auto f, auto tuple, std::integer_sequence<s64, Indices...>) {
		return f(tuple_get<Indices>(tuple)...);
	};
	return impl(f, tuple, std::make_integer_sequence<s64, decltype(tuple)::size>{});
}
