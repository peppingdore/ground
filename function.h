#pragma once

#include "grd_base.h"
#include "tuple.h"
#include "allocator.h"
#include "grd_code_location.h"

#include <utility>

template <typename... T>
struct DecomposedFunctionType;

template <typename __ReturnType, typename... Args>
struct DecomposedFunctionType<__ReturnType(Args...)> {
	using ReturnType = __ReturnType;
	using ArgsTupleType = Tuple<Args...>;
};

template <typename Class, typename __ReturnType, typename... Args>
struct DecomposedMemberFunctionType {
	using RawFunctionType = DecomposedFunctionType<__ReturnType(Args...)>;
	// using ReturnType = __ReturnType;
};

template <typename ReturnType, typename... Args>
DecomposedFunctionType<ReturnType(Args...)> decompose_function_type( ReturnType (*) (Args...)) {
	return {};
}

template <typename Class, typename ReturnType, typename... Args>
DecomposedMemberFunctionType<Class, ReturnType, Args...> decompose_member_function_type(ReturnType(Class::*)(Args...)) {
	return {};
}

template <typename Class, typename ReturnType, typename... Args>
DecomposedMemberFunctionType<Class, ReturnType, Args...> decompose_member_function_type(ReturnType(Class::*)(Args...) const) {
	return {};
}

auto decompose_function_type(auto lambda) -> decltype(auto) {
	using Decomposed = decltype(decompose_member_function_type(&lambda.operator())());
	return typename Decomposed::RawFunctionType();
}

// Question of syntax taste.
// Allows us to use Proc<int(int)>* syntax instead of int (*ptr)(int).
template <typename T>
using Proc = T;

enum class FunctionKind: u8 {
	None = 0,             // Function is empty
	Small_Lambda = 1,     // Lambda that can fit inside a small optimization container.
	Big_Lambda = 2,       // Lambda that has to be allocated separately.
	Function_Pointer = 3, // Simple C function pointer. 
};

template <typename... T>
struct Function {
	static_assert(sizeof...(T) == 0, "Use function type syntax, not <R, Args...>, but <R(Args...)");
};

constexpr u64 SMALL_LAMBDA_STORAGE_SIZE = 24; // 24 + lambda flattened_proc ptr (8) = 32 bytes.

// We tradeoff readibility for struct size.
template <typename ReturnType, typename... ArgsTypes>
struct Function<ReturnType(ArgsTypes...)> {
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

			ReturnType (*flattened_proc)(Function*, ArgsTypes...);
		} lambda;
	};

	FunctionKind kind                     : 4 = FunctionKind::None;
	bool         is_using_custom_allocator: 4 = false;

	bool is_empty() {
		return kind == FunctionKind::None;
	}

	ReturnType operator()(ArgsTypes... args) {
		switch (kind) {
			case FunctionKind::Function_Pointer: {
				return function_ptr.ptr(args...);
			}
			break;

			case FunctionKind::Big_Lambda:
			case FunctionKind::Small_Lambda: {
				return lambda.flattened_proc(this, args...);
			}
			break;

			default:
				assert(false);
				return ReturnType();
		}
	}

	void replace_with(Function other, CodeLocation loc = caller_loc()) {
		free();
		*this = other;
	}

	void operator<<=(auto other) {
		replace_with(grd_make_function(other));
	}

	Function copy(CodeLocation loc = caller_loc()) const {
		Function copied = *this;
		if (kind == FunctionKind::Big_Lambda) {
			void* new_mem = GrdMalloc(lambda.big_lambda.size, loc);
			memcpy(new_mem, lambda.big_lambda.mem, lambda.big_lambda.size);
			copied.lambda.big_lambda.mem = new_mem;
		}
		return copied;
	}

	void free(CodeLocation loc = caller_loc()) {
		assert(!is_using_custom_allocator);
		if (kind == FunctionKind::Big_Lambda){
			GrdFree(lambda.big_lambda.mem, loc);
		}
		kind = FunctionKind::None;
	}

	void free(GrdAllocator allocator, CodeLocation loc = caller_loc()) {
		assert(is_using_custom_allocator);
		if (kind == FunctionKind::Big_Lambda) {
			GrdFree(allocator, lambda.big_lambda.mem, loc);
		}
		kind = FunctionKind::None;
		is_using_custom_allocator = false;
	}
};


template <typename ReturnType, typename... ArgsTypes>
auto grd_make_function(ReturnType (*function_ptr)(ArgsTypes...)) {
	Function<ReturnType(ArgsTypes...)> result;
	result.kind = FunctionKind::Function_Pointer;
	result.function_ptr.ptr = function_ptr;
	return result;
}

// Assert that we do not use heap allocator.
auto grd_make_no_heap_function(auto lambda, CodeLocation loc = caller_loc()) {
	static_assert(sizeof(lambda) <= SMALL_LAMBDA_STORAGE_SIZE);
	return grd_make_function(lambda, loc);
}

auto grd_make_function(auto lambda, CodeLocation loc = caller_loc()) {
	using Lambda_Type = decltype(lambda);

	// static_assert(std::is_same_v<std::invoke_result_t<Lambda_Type, ArgsTypes...>, ReturnType>, "Lambda return type is incorrect");
	// using Decomposed = Decompose_Member_FunctionType<decltype(&Lambda_Type::operator())>;

	auto do_stuff = [&]
		<typename ReturnType, typename... ArgsTypes>
		( const DecomposedMemberFunctionType<Lambda_Type, ReturnType, ArgsTypes...>) {
		
		using FlattenedType = Function<ReturnType(ArgsTypes...)>;

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
			result.kind = FunctionKind::Big_Lambda;
		
		} else {
			memcpy(&result.lambda.small_lambda.storage, &lambda, sizeof(lambda));
			auto flattened_proc = +[](FlattenedType* flattened, ArgsTypes... args) -> ReturnType {
				auto lambda = (Lambda_Type*) &flattened->lambda.small_lambda;

				return lambda->operator()(args...);
			};

			result.lambda.flattened_proc = flattened_proc;
			result.kind = FunctionKind::Small_Lambda;
		}

		return result;
	};
	
	using Decomposed = decltype(decompose_member_function_type(&Lambda_Type::operator()));

	auto result = do_stuff(Decomposed());
	assert(result.kind != FunctionKind::None);
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

struct __cast2function {
	auto operator<<(auto f) {
		auto function = grd_make_function(f);
		return function;
	}
};

struct __cast2noheapfunction {
	auto operator<<(auto f) {
		auto function = grd_make_no_heap_function(f);
		return function;
	}
};

#define function_cast  __cast2function() << 
#define no_heap_function_cast  __cast2noheapfunction() << 

#define lambda(expr) [&] (auto... lambda_args) { auto lambda_args_tuple = grd_make_tuple(lambda_args...); return expr; }
#define $0 tuple_get<0>(lambda_args_tuple)
#define $1 tuple_get<1>(lambda_args_tuple)
#define $2 tuple_get<2>(lambda_args_tuple)
#define $3 tuple_get<3>(lambda_args_tuple)
#define $4 tuple_get<4>(lambda_args_tuple)
#define $5 tuple_get<5>(lambda_args_tuple)
#define $6 tuple_get<6>(lambda_args_tuple)


template <typename Candidate, typename FunctionType, typename D = DecomposedFunctionType<FunctionType>>
concept Callable = requires(Candidate f, FunctionType signature_type) {
	{ call_with_tuple(f, std::declval<typename D::ArgsTupleType>()) } -> std::same_as<typename D::ReturnType>;
};

auto call_with_tuple(auto f, auto tuple) {
	auto impl = []
		<s64... Indices>
		(auto f, auto tuple, std::integer_sequence<s64, Indices...>) {
		return f(tuple_get<Indices>(tuple)...);
	};
	return impl(f, tuple, std::grd_make_integer_sequence<s64, decltype(tuple)::size>{});
}
