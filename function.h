#pragma once

#include "base.h"
#include "tuple.h"
#include "allocator.h"
#include "code_location.h"

#include <utility>

template <typename... T>
struct Decomposed_Function_Type;

template <typename __Return_Type, typename... Args>
struct Decomposed_Function_Type<__Return_Type(Args...)> {
	using Return_Type = __Return_Type;
	using Args_Tuple_Type = Tuple<Args...>;
};

template <typename Class, typename __Return_Type, typename... Args>
struct Decomposed_Member_Function_Type {
	using Raw_Function_Type = Decomposed_Function_Type<__Return_Type(Args...)>;
	// using Return_Type = __Return_Type;
};

template <typename Return_Type, typename... Args>
Decomposed_Function_Type<Return_Type(Args...)> decompose_function_type( Return_Type (*) (Args...)) {
	return {};
}

template <typename Class, typename Return_Type, typename... Args>
Decomposed_Member_Function_Type<Class, Return_Type, Args...> decompose_member_function_type(Return_Type(Class::*)(Args...)) {
	return {};
}

template <typename Class, typename Return_Type, typename... Args>
Decomposed_Member_Function_Type<Class, Return_Type, Args...> decompose_member_function_type(Return_Type(Class::*)(Args...) const) {
	return {};
}

auto decompose_function_type(auto lambda) -> decltype(auto) {
	using Decomposed = decltype(decompose_member_function_type(&lambda.operator())());
	return typename Decomposed::Raw_Function_Type();
}

// Question of syntax taste.
// Allows us to use Proc<int(int)>* syntax instead of int (*ptr)(int).
template <typename T>
using Proc = T;

enum class Function_Kind: u8 {
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
template <typename Return_Type, typename... Args_Types>
struct Function<Return_Type(Args_Types...)> {
	union {
		struct {
			Return_Type (*ptr)(Args_Types...);
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

			Return_Type (*flattened_proc)(Function*, Args_Types...);
		} lambda;
	};

	Function_Kind kind                     : 4 = Function_Kind::None;
	bool          is_using_custom_allocator: 4 = false;

	bool is_empty() {
		return kind == Function_Kind::None;
	}

	Return_Type operator()(Args_Types... args) {
		switch (kind) {
			case Function_Kind::Function_Pointer: {
				return function_ptr.ptr(args...);
			}
			break;

			case Function_Kind::Big_Lambda:
			case Function_Kind::Small_Lambda: {
				return lambda.flattened_proc(this, args...);
			}
			break;

			default:
				assert(false);
				return Return_Type();
		}
	}

	void replace_with(Function other, CodeLocation loc = caller_loc()) {
		free();
		*this = other;
	}

	void operator<<=(auto other) {
		replace_with(make_function(other));
	}

	Function copy(CodeLocation loc = caller_loc()) const {
		Function copied = *this;
		if (kind == Function_Kind::Big_Lambda) {
			void* new_mem = c_allocator.alloc(lambda.big_lambda.size, loc);
			memcpy(new_mem, lambda.big_lambda.mem, lambda.big_lambda.size);
			copied.lambda.big_lambda.mem = new_mem;
		}
		return copied;
	}

	void free(CodeLocation loc = caller_loc()) {
		assert(!is_using_custom_allocator);
		if (kind == Function_Kind::Big_Lambda){
			c_allocator.free(lambda.big_lambda.mem, loc);
		}
		kind = Function_Kind::None;
	}

	void free(Allocator allocator, CodeLocation loc = caller_loc()) {
		assert(is_using_custom_allocator);
		if (kind == Function_Kind::Big_Lambda) {
			allocator.free(lambda.big_lambda.mem, loc);
		}
		kind = Function_Kind::None;
		is_using_custom_allocator = false;
	}
};


template <typename Return_Type, typename... Args_Types>
auto make_function(Return_Type (*function_ptr)(Args_Types...)) {
	Function<Return_Type(Args_Types...)> result;
	result.kind = Function_Kind::Function_Pointer;
	result.function_ptr.ptr = function_ptr;
	return result;
}

// Assert that we do not use heap allocator.
auto make_no_heap_function(auto lambda, CodeLocation loc = caller_loc()) {
	static_assert(sizeof(lambda) <= SMALL_LAMBDA_STORAGE_SIZE);
	return make_function(lambda, loc);
}

auto make_function(auto lambda, CodeLocation loc = caller_loc()) {
	using Lambda_Type = decltype(lambda);

	// static_assert(std::is_same_v<std::invoke_result_t<Lambda_Type, Args_Types...>, Return_Type>, "Lambda return type is incorrect");
	// using Decomposed = Decompose_Member_Function_Type<decltype(&Lambda_Type::operator())>;

	auto do_stuff = [&]
		<typename Return_Type, typename... Args_Types>
		( const Decomposed_Member_Function_Type<Lambda_Type, Return_Type, Args_Types...>) {
		
		using Flattened_Type = Function<Return_Type(Args_Types...)>;

		Flattened_Type result;

		if constexpr (SMALL_LAMBDA_STORAGE_SIZE < sizeof(lambda)) {
		
			void* mem = c_allocator.alloc(sizeof(lambda), loc);
			memcpy(mem, &lambda, sizeof(lambda));

			auto flattened_proc = +[](Flattened_Type* flattened, Args_Types... args) -> Return_Type {
				auto lambda = (Lambda_Type*) flattened->lambda.big_lambda.mem;
				return lambda->operator()(args...);
			};

			result.lambda.flattened_proc = flattened_proc;
			result.lambda.big_lambda.mem = mem;
			result.lambda.big_lambda.size = sizeof(lambda);
			result.kind = Function_Kind::Big_Lambda;
		
		} else {
			memcpy(&result.lambda.small_lambda.storage, &lambda, sizeof(lambda));
			auto flattened_proc = +[](Flattened_Type* flattened, Args_Types... args) -> Return_Type {
				auto lambda = (Lambda_Type*) &flattened->lambda.small_lambda;

				return lambda->operator()(args...);
			};

			result.lambda.flattened_proc = flattened_proc;
			result.kind = Function_Kind::Small_Lambda;
		}

		return result;
	};
	
	using Decomposed = decltype(decompose_member_function_type(&Lambda_Type::operator()));

	auto result = do_stuff(Decomposed());
	assert(result.kind != Function_Kind::None);
	return result;
}

//  If you do not like wrapping lambdas in the function call:
//
//      Function<int(int, int)> f = make_function([&](int x, int y) {
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
		auto function = make_function(f);
		return function;
	}
};

struct __cast2noheapfunction {
	auto operator<<(auto f) {
		auto function = make_no_heap_function(f);
		return function;
	}
};

#define function_cast  __cast2function() << 
#define no_heap_function_cast  __cast2noheapfunction() << 

#define lambda(expr) [&] (auto... lambda_args) { auto lambda_args_tuple = make_tuple(lambda_args...); return expr; }
#define $0 tuple_get<0>(lambda_args_tuple)
#define $1 tuple_get<1>(lambda_args_tuple)
#define $2 tuple_get<2>(lambda_args_tuple)
#define $3 tuple_get<3>(lambda_args_tuple)
#define $4 tuple_get<4>(lambda_args_tuple)
#define $5 tuple_get<5>(lambda_args_tuple)
#define $6 tuple_get<6>(lambda_args_tuple)


template <typename Candidate, typename Function_Type, typename D = Decomposed_Function_Type<Function_Type>>
concept Callable = requires(Candidate f, Function_Type signature_type) {
	{ call_with_tuple(f, std::declval<typename D::Args_Tuple_Type>()) } -> std::same_as<typename D::Return_Type>;
};

auto call_with_tuple(auto f, auto tuple) {
	auto impl = []
		<s64... Indices>
		(auto f, auto tuple, std::integer_sequence<s64, Indices...>) {
		return f(tuple_get<Indices>(tuple)...);
	};
	return impl(f, tuple, std::make_integer_sequence<s64, decltype(tuple)::size>{});
}
