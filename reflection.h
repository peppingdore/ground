#pragma once

#include "base.h"
#include "sync/atomics.h"
#include "sync/spinlock.h"
#include "pointer_math.h"
#include "compile_time_counter.h"
#include "coroutine.h"
#include "scoped.h"
#include "heap_sprintf.h"

using Type_Id = u32;

struct Formatter;

// TypeKind is safe to cast to C string.
using TypeKind = u64;

template <int N> requires(N <= sizeof(TypeKind))
constexpr TypeKind make_type_kind(const char (&str)[N]) {
	TypeKind kind = 0;
	for (int i = 0; i < (N - 1); i++) {
		kind |= u64(str[i]) << u64(i * 8);
	}
	return kind;
}

const char* type_kind_as_c_str(TypeKind* kind) {
	return (const char*) kind;
}

struct Type {
	const char* name    = "<unnamed>";
	TypeKind    kind    = 0;
	u32         size    = 0;
	Type_Id     id      = 0;

	template <typename T>
	T* as() {
		return kind == T::KIND ? (T*) this : NULL;
	}
};

struct Any {
	Type* type = NULL;
	void* ptr  = NULL;
};


template <typename T>
struct ReflectArray {
	T*  data  = NULL;
	s64 count = 0;

	T* begin() { return data; }
	T* end()   { return data + count; }
};

template <typename T>
void reflect_add_array(ReflectArray<T>* arr, s64* capacity, T thing) {
	if (arr->data == NULL) {
		*capacity = 8;
		arr->data = (T*) malloc(sizeof(T) * *capacity);
	} else if (arr->count == *capacity) {
		*capacity *= 2;
		arr->data = (T*) realloc(arr->data, sizeof(T) * *capacity);
	}

	arr->data[arr->count] = thing;
	arr->count += 1;
}



struct PrimitiveValue {
	union {
		u8       u8_value;
		u16      u16_value;
		u32      u32_value;
		u64      u64_value;
		s8       s8_value;
		s16      s16_value;	
		s32      s32_value;
		s64      s64_value;
		f32      f32_value;
		f64      f64_value;
		bool     bool_value;
		char     char_value;
		wchar_t  wchar_value;
		char16_t char16_value;
		char32_t char32_value;
		// @NewPrimitive
	};

	bool operator==(PrimitiveValue other) {
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
};

PrimitiveValue make_primitive_value(auto thing) {
	PrimitiveValue v = {};
	static_assert(sizeof(thing) <= sizeof(v));
	memcpy(&v, &thing, sizeof(thing));
	return v;
}

enum class PrimitiveKind: u32 {
	P_u8     = 0,
	P_s8     = 1,
	P_u16    = 2,
	P_s16    = 3,
	P_u32    = 4,
	P_s32    = 5,
	P_u64    = 6,
	P_s64    = 7,
	P_f32    = 8,
	P_f64    = 9,
	P_bool   = 10,
	P_void   = 11,
	P_char   = 12,
	P_wchar  = 13,
	P_char16 = 14,
	P_char32 = 15,
	// @NewPrimitive

	MAX_PRIMITIVE_COUNT,
};

struct PrimitiveType: Type {
	constexpr static auto KIND = make_type_kind("prim"); 

	PrimitiveKind primitive_kind;
};

struct PointerType: Type {
	constexpr static auto KIND = make_type_kind("pointer"); 

	Type* inner;
};

struct UnregisteredType: Type {
	constexpr static auto KIND = make_type_kind("unreg");
};


struct EnumValue {
	const char*       name = NULL;
	PrimitiveValue    value;
	ReflectArray<Any> tags;
};

struct EnumType: Type {
	constexpr static auto KIND = make_type_kind("enum");

	PrimitiveType*          base_type = NULL;
	bool                    is_flags  = false;
	ReflectArray<EnumValue> values;
};

enum StructMemberKind: s32 {
	STRUCT_MEMBER_KIND_PLAIN = 0,
	STRUCT_MEMBER_KIND_BASE  = 1,
};

struct StructMember {
	const char*         name   = NULL;
	Type*               type   = NULL;
	StructMemberKind    kind   = STRUCT_MEMBER_KIND_PLAIN;
	s32                 offset = 0;
	ReflectArray<Any>   tags;
};

struct StructType: Type {
	constexpr static auto KIND = make_type_kind("struct");

	ReflectArray<StructMember> unflattened_members;
	ReflectArray<StructMember> members;
	const char*                subkind = "";
};

struct ArrayType: Type {
	constexpr static auto KIND = make_type_kind("array");

	Type*       inner = NULL;
	const char* subkind = "";

	s64   (*get_count)   (void* arr) = NULL;
	s64   (*get_capacity)(void* arr) = NULL;
	void* (*get_item)    (void* arr, s64 index) = NULL;
};

struct MapType: Type {
	constexpr static auto KIND = make_type_kind("map");

	Type*       key     = NULL;
	Type*       value   = NULL;
	const char* subkind = "";

	struct Item {
		void* key;
		void* value;
	};

	s64              (*get_count)    (void* map) = NULL;
	s64              (*get_capacity) (void* map) = NULL;
	Generator<Item*> (*iterate)      (void* map) = NULL;
};

struct FixedArrayType: ArrayType {
	s64 array_size = 0;
};

struct FunctionType: Type {
	constexpr static auto KIND = make_type_kind("func");

	Type*               return_type = NULL;
	ReflectArray<Type*> arg_types;
};

#define REFLECTION_REFLECT_PRIMITIVE(TYPE, KIND) PrimitiveType* reflect_type(TYPE* x, PrimitiveType* type) {\
	type->name = #TYPE;\
	type->primitive_kind = PrimitiveKind::P_##KIND;\
	return type;\
}
REFLECTION_REFLECT_PRIMITIVE(u8,       u8);
REFLECTION_REFLECT_PRIMITIVE(s8,       s8);
REFLECTION_REFLECT_PRIMITIVE(u16,      u16);
REFLECTION_REFLECT_PRIMITIVE(s16,      s16);
REFLECTION_REFLECT_PRIMITIVE(u32,      u32);
REFLECTION_REFLECT_PRIMITIVE(s32,      s32);
REFLECTION_REFLECT_PRIMITIVE(u64,      u64);
REFLECTION_REFLECT_PRIMITIVE(s64,      s64);
REFLECTION_REFLECT_PRIMITIVE(f32,      f32);
REFLECTION_REFLECT_PRIMITIVE(f64,      f64);
REFLECTION_REFLECT_PRIMITIVE(char,     char);
REFLECTION_REFLECT_PRIMITIVE(wchar_t,  wchar);
REFLECTION_REFLECT_PRIMITIVE(char16_t, char16);
REFLECTION_REFLECT_PRIMITIVE(char32_t, char32);
#undef REFLECTION_REFLECT_PRIMITIVE
// @NewPrimitive

PrimitiveType* reflect_type(bool* x, PrimitiveType* type) {
	type->name = "bool";
	type->primitive_kind = PrimitiveKind::P_bool;
	return type;
}

template <typename T, s64 N>
FixedArrayType* reflect_type(T (*arr)[N], FixedArrayType* type);

template <typename T> requires(!std::is_function_v<T>)
PointerType* reflect_type(T** thing, PointerType* type);

template <typename T> requires(std::is_function_v<T>)
FunctionType* reflect_type(T* thing, FunctionType* type);

using ReflectionHookCounter = CTCounter<>;

template <u64 _N>
struct ReflectionHookIndexTag {
	constexpr static u64 N = _N;
};


template <typename T, u64 N>
struct CallNextHook {
	CallNextHook() {
		reflection_hook<T>(ReflectionHookIndexTag<N>{});
	}
};

#define REFLECTION_REFLECT_HOOK(type_name)\
template <typename type_name>\
void reflection_hook(\
	ReflectionHookIndexTag<ReflectionHookCounter::Increment<>> t,\
	CallNextHook<type_name, ReflectionHookCounter::Read<> + 1> next = { })

template <typename T, u64 N>
void reflection_hook(ReflectionHookIndexTag<N> tag, EmptyStruct x = {}) {
	if constexpr (N == 0) {
		reflection_hook<T>(ReflectionHookIndexTag<N + 1>{});
	}
}


template <typename T>
concept ReflectionMemberReflectable = requires(T a) { 
	{ T::reflect_type(&a, NULL) };
};

template <typename T>
concept ReflectionGlobalReflectable = requires(T a) { 
	{ reflect_type(&a, NULL) };
};


struct Reflection {
	Spinlock lock;
	Type**   types;
	s64      types_count;
	s64      types_capacity;


	template <typename T>
	struct TypeMapper {
		inline static Type* type = NULL;
	};

	template <typename T>
	constexpr u32 get_type_size() {
		return sizeof(T);
	}

	template <>
	constexpr u32 get_type_size<void>() {
		return 0;
	}

	template <typename T, typename TypeT>
	TypeT* add_type() {
		ScopedLock(lock);
		if (types == NULL) {
			types_capacity = 16;
			types = (Type**) malloc(sizeof(Type*) * types_capacity);
		} else if (types_count == types_capacity) {
			types_capacity *= 2;
			types = (Type**) realloc(types, sizeof(Type*) * types_capacity);
		}
		auto* type = new TypeT();
		TypeMapper<T>::type = type;
		types[types_count] = type;
		type->size = get_type_size<T>();
		type->id = types_count;
		type->kind = TypeT::KIND;
		types_count += 1;
		return type;
	}

	template <ReflectionMemberReflectable T>
	Type* inner_reflect() {
		using TypeT = std::remove_cvref_t<decltype(*T::reflect_type((T*) NULL, NULL))>;
		TypeT* type = add_type<T, TypeT>();
		T::reflect_type((T*) NULL, type);
		return type;
	}

	template <ReflectionGlobalReflectable T>
		requires (!ReflectionMemberReflectable<T>)
	Type* inner_reflect() {
		using TypeT = std::remove_cvref_t<decltype(*reflect_type((T*) NULL, NULL))>;
		TypeT* type = add_type<T, TypeT>();
		reflect_type((T*) NULL, type);
		return type;
	}

	// Every type pointer auto casts to void*, that's why
	//  we can't have global reflect_type(void*, ) to reflect type 'void'.
	template <typename T> requires (std::is_same_v<T, void>)
	Type* inner_reflect() {
		auto* type = add_type<void, PrimitiveType>();
		type->name = "void";
		type->primitive_kind = PrimitiveKind::P_void;
		return type;
	}

	template <typename T>
	Type* inner_reflect() {
		auto type = add_type<T, UnregisteredType>();
		type->name = "unregistered";
		return type;
	}

	template <typename T>
	struct remove_all_const: std::remove_const<T> {};
	template <typename T>
	struct remove_all_const<T*> {
		using type = typename remove_all_const<T>::type*;
	};
	template <typename T>
	struct remove_all_const<T* const> {
		using type = typename remove_all_const<T>::type*;
	};

	template <typename T>
	Type* type_of() {
		ScopedLock(lock);
		using XT = remove_all_const<T>::type;
		using Mapper = TypeMapper<XT>;
		if (!Mapper::type) {
			auto type = inner_reflect<XT>();
			reflection_hook<XT, 0>({});
		}
		assert(Mapper::type);
		return Mapper::type;
	}

	template <typename T>
	Type* type_of(T& value) {
		return type_of<std::remove_reference_t<T>>();
	}
};

inline static Reflection reflect;


Any make_any(Type* type, void* ptr) {
	return { .type = type, .ptr = ptr };
}

// Prevents Any containing Any, in variadic expressions like make_any(args)... .
Any make_any(Any any) {
	return any;
}
Any make_any(Any* any) {
	return make_any(*any);
}

template <typename T>
Any make_any(T* thing) {
	auto type = reflect.type_of<T>();
	return make_any(type, thing);
}

template <typename T>
Any make_any(T& thing) {
	return make_any(&thing);
}



template <typename T> requires(!std::is_function_v<T>)
PointerType* reflect_type(T** thing, PointerType* type) {
	type->inner = reflect.type_of<T>();
	auto inner_name_length = strlen(type->inner->name);
	auto name = (char*) malloc(inner_name_length + 2);
	memcpy(name, type->inner->name, inner_name_length);
	strcpy(name + inner_name_length, "*");
	type->name = name;
	return type;
}

template <typename T, s64 N>
FixedArrayType* reflect_type(T (*arr)[N], FixedArrayType* type) {
	type->inner = reflect.type_of<T>();
	type->array_size = N;
	type->subkind = "fixed_array";
	type->name = heap_sprintf("%s[%lld]", type->inner->name, N);
	type->get_count = +[](void* arr) {
		return N;
	};
	type->get_capacity = +[](void* arr) {
		return N;
	};
	type->get_item = +[](void* arr, s64 index) -> void* {
		return ptr_add(arr, sizeof(T) * index);
	};
	return type;
}

template <typename R, typename... Args>
FunctionType* reflect_type(R (**function)(Args...), FunctionType* type) {
	type->return_type = reflect.type_of<R>();
	Type* arg_types[] = { reflect.type_of<Args>()... };
	s64 capacity = 0;
	for (Type* arg_type: arg_types) {
		reflect_add_array(&type->arg_types, &capacity, arg_type);		
	}
	return type;
}

Type* get_pointer_inner_type_with_indirection_level(PointerType* pointer_type, s32* result_indirection_level) {
	
	s32 indirection_level = 1;
	while (pointer_type->inner->kind == make_type_kind("pointer")) {
		indirection_level += 1;
		pointer_type = (PointerType*) pointer_type->inner;
	}

	if (result_indirection_level) {
		*result_indirection_level = indirection_level;
	}
	return pointer_type->inner;
}

bool does_type_inherit(Type* derived, Type* base) {
	if (derived->kind != StructType::KIND ||
		base->kind    != StructType::KIND) {
		return false;
	}

	auto der = (StructType*) derived;
	auto b   = (StructType*) base;

	for (auto it: der->unflattened_members) {
		if (it.kind == STRUCT_MEMBER_KIND_BASE) {
			if (it.type == b) {
				return true;
			}
			if (does_type_inherit(it.type, b)) {
				return true;
			}
		}
	}
	return false;
}

bool does_type_convert_to(Type* derived, Type* base) {
	if (derived == base) {
		return true;
	}
	return does_type_inherit(derived, base);
}

// @TODO: we have to add or subtract base member offset here, not just cast.
template <typename T>
T* as(auto* x) {
	if (does_type_convert_to(x->type, reflect.type_of<T>())) {
		return (T*) x;
	}
	return NULL;
}

template <typename T>
T* as(Any any) {
	if (does_type_convert_to(any.type, reflect.type_of<T>())) {
		return (T*) any.ptr;
	}
	return NULL;
}

template <typename T>
T* as(Any* any) {
	if (does_type_convert_to(any->type, reflect.type_of<T>())) {
		return (T*) any->ptr;
	}
	return NULL;
}

template <typename T, typename X>
	requires(
		std::is_same_v<X, StructMember> ||
		std::is_same_v<X, EnumValue>)
T* find_tag(X* x) {
	for (auto it: x->tags) {
		auto casted = as<T>(it);
		if (casted) {
			return casted;
		}
	}
	return NULL;
}

void reflect_struct_type_add_member(Type* type, s64* capacity, StructMember member) {
	if (type->kind != StructType::KIND) {
		return;
	}
	auto t = (StructType*) type;
	reflect_add_array(&t->unflattened_members, capacity, member);
}

void reflect_enum_type_add_value(Type* type, s64* capacity, EnumValue v) {
	if (type->kind != EnumType::KIND) {
		return;
	}
	auto t = (EnumType*) type;
	reflect_add_array(&t->values, capacity, v);
}

template <typename From, typename To, typename = void>
struct can_static_cast: std::false_type { };
template <typename From, typename To>
struct can_static_cast<From, To, decltype(static_cast<To>(std::declval<From>()), void())>: std::true_type { };
template <typename Base, typename Derived, typename = void>
struct is_virtual_base_of: std::false_type { };
template <typename Base, typename Derived>
struct is_virtual_base_of<Base, Derived, typename std::enable_if<
    std::is_base_of<Base, Derived>::value && 
    !can_static_cast<Base*, Derived*>::value
>::type>: std::true_type{ };

void fill_struct_type_members(StructType* type, s32 offset, StructMember* members, s64* count) {
	for (auto member: type->unflattened_members) {
		if (member.kind == STRUCT_MEMBER_KIND_BASE) {
			if (member.type->kind == make_type_kind("struct")) {
				fill_struct_type_members((StructType*) member.type, offset + member.offset, members, count);
			}
		} else if (member.kind == STRUCT_MEMBER_KIND_PLAIN) {
			if (members) {
				member.offset += offset;
				members[*count] = member;
			}
			*count += 1;
		}
	}
}

void reflect_add_tag(Type* type, auto value, s64* tags_capacity) {
	auto copied = new decltype(value)();
	*copied = value;
	auto tag = make_any(copied);

	if (auto casted = type->as<StructType>()) {
		if (casted->unflattened_members.count > 0) {
			auto* member = &casted->unflattened_members.data[casted->unflattened_members.count - 1];
			reflect_add_array(&member->tags, tags_capacity, tag);
		}
	} else if (auto casted = type->as<EnumType>()) {
		if (casted->values.count > 0) {
			auto* val = &casted->values.data[casted->values.count - 1];
			reflect_add_array(&val->tags, tags_capacity, tag);
		}
	}
}

template <typename T>
using ReflectMacroPickType = std::conditional_t<std::is_class_v<T>, StructType, EnumType>;


#define REFLECT_NAME(_type, _name)\
template <typename __T = _type> requires(std::is_class_v<__T> && std::is_same_v<__T, _type>)\
inline static StructType* reflect_type(__T* x, StructType* type) {\
	type->name = _name;\
	s64 capacity = 0;\
	s64 tags_capacity = 0;\
	reflect_fill_type(x, type, &capacity, &tags_capacity);\
	s64 members_capacity = 0;\
	fill_struct_type_members(type, 0, NULL, &members_capacity);\
	type->members.data = (StructMember*) malloc(sizeof(StructMember) * members_capacity);\
	type->members.count = members_capacity;\
	members_capacity = 0;\
	fill_struct_type_members(type, 0, type->members.data, &members_capacity);\
	return type;\
}\
template <typename __T = _type> requires(std::is_enum_v<__T> && std::is_same_v<__T, _type>)\
inline static EnumType* reflect_type(__T* x, EnumType* type) {\
	type->name = #_type;\
	s64 capacity = 0;\
	s64 tags_capacity = 0;\
	reflect_fill_type(x, type, &capacity, &tags_capacity);\
	return type;\
}\
inline static void reflect_fill_type(_type* x, ReflectMacroPickType<_type>* type, s64* capacity, s64* tags_capacity)

#define REFLECT(_type) REFLECT_NAME(_type, #_type)

#define MEMBER_AS(member, member_type) \
static_assert(sizeof(member_type) == sizeof(x->member));\
reflect_struct_type_add_member(type, capacity, StructMember{\
	#member,\
	reflect.type_of<member_type>(),\
	STRUCT_MEMBER_KIND_PLAIN,\
	(s32) OFFSETOF(std::remove_pointer_t<decltype(x)>, member)\
});\
*tags_capacity = 0;

#define MEMBER(member) MEMBER_AS(member, decltype(x->member))

#define TAG(value) reflect_add_tag(type, value, tags_capacity);

#define ENUM_VALUE(value) reflect_enum_type_add_value(type, capacity, EnumValue{ #value, make_primitive_value(std::remove_pointer_t<decltype(x)>::value) });\
*tags_capacity = 0;

#define BASE_TYPE(base)\
assert((((s32) ((u8*) ((base*) ((decltype(x)) 0x100000)) - (u8*) ((decltype(x)) 0x100000))) == 0) && "non-zero offset base is not supported");\
static_assert(std::is_base_of_v<base, std::remove_pointer_t<decltype(x)>>);\
static_assert(!is_virtual_base_of<base, std::remove_pointer_t<decltype(x)>>::value);\
reflect_struct_type_add_member(type, capacity, StructMember{\
	reflect.type_of<base>()->name,\
	reflect.type_of<base>(),\
	STRUCT_MEMBER_KIND_BASE,\
	((s32) ((u8*) ((base*) ((decltype(x)) 0x100000)) - (u8*) ((decltype(x)) 0x100000))),\
});


REFLECT(Type) {
	MEMBER(name);
	MEMBER(kind);
	MEMBER(size);
	MEMBER(id);
}

struct RealTypeMember {};

Type* get_real_type(Type* type, void* ptr) {
	if (ptr != NULL) {
		if (type->kind == StructType::KIND) {
			auto casted = (StructType*) type;
			for (auto it: casted->members) {
				auto tag = find_tag<RealTypeMember>(&it);
				if (tag) {
					auto candidate = *(Type**) ptr_add(ptr, it.offset);
					if (candidate) {
						return candidate;
					}
				}
			}
		}
	}
	return type;
}
