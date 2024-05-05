#pragma once

#include "base.h"
#include "sync/atomics.h"
#include "sync/spinlock.h"
#include "pointer_math.h"
#include "compile_time_counter.h"
#include "coroutine.h"
#include "scoped.h"
#include "heap_sprintf.h"

template <typename T>
struct ReflectArray {
	T*  data     = NULL;
	s64 count    = 0;
	s64 capacity = 0;

	T* operator[](s64 index) {
		assert(index >= 0 && index < count);
		return data + index;
	}

	void add(T thing) {
		if (data == NULL) {
			capacity = 8;
			data = (T*) malloc(sizeof(T) * capacity);
		} else if (count == capacity) {
			capacity *= 2;
			data = (T*) realloc(data, sizeof(T) * capacity);
		}
		data[count] = thing;
		count += 1;
	}

	T* begin() { return data; }
	T* end()   { return data + count; }
};


using TypeId = u32;

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
	TypeId      id      = 0;

	template <typename T>
	T* as() {
		return kind == T::KIND ? (T*) this : NULL;
	}
};

struct Any {
	Type* type = NULL;
	void* ptr  = NULL;
};

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

struct SpanType: Type {
	constexpr static auto KIND = make_type_kind("span");

	Type*       inner = NULL;
	const char* subkind = "";

	s64   (*get_count)   (void* arr) = NULL;
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
	// @TODO: move get_capacity to more specific type.?
	s64              (*get_capacity) (void* map) = NULL;
	Generator<Item*> (*iterate)      (void* map) = NULL;
};

struct FixedArrayType: SpanType {
	s64 array_size = 0;
};

struct FunctionType: Type {
	constexpr static auto KIND = make_type_kind("func");

	Type*               return_type = NULL;
	ReflectArray<Type*> arg_types;
};

template <typename T>
PrimitiveType* reflect_primitive_type(const char* name, PrimitiveKind kind);

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


PrimitiveType* reflect_type(bool* x, PrimitiveType* type);

template <typename T, s64 N>
FixedArrayType* reflect_type(T (*arr)[N], FixedArrayType* type);

template <typename T> requires(!std::is_function_v<T>)
PointerType* reflect_type(T** thing, PointerType* type);

template <typename T> requires(std::is_function_v<T>)
FunctionType* reflect_type(T* thing, FunctionType* type);

using ReflectionHookCounter = CTCounter<>;

template <u64 _N>
struct ReflectHookIndexTag {
	constexpr static u64 N = _N;
};

template <typename T, u64 N>
struct CallNextReflectHook {
	CallNextReflectHook() {
		reflect_hook<T>(ReflectHookIndexTag<N>{});
	}
};

#define REFLECTION_REFLECT_HOOK(type_name)\
template <typename type_name>\
void reflect_hook(\
	ReflectHookIndexTag<ReflectionHookCounter::Increment<>> t,\
	CallNextReflectHook<type_name, ReflectionHookCounter::Read<> + 1> next = { })

template <typename T, u64 N>
void reflect_hook(ReflectHookIndexTag<N> tag, EmptyStruct x = {}) {
	if constexpr (N == 0) {
		reflect_hook<T>(ReflectHookIndexTag<N + 1>{});
	}
}


template <typename T>
concept MemberReflect = requires(T a) { 
	{ T::reflect_type(&a, NULL) };
};

template <typename T>
concept GlobalReflect = requires(T a) { 
	{ reflect_type(&a, NULL) };
};

struct Reflection {
	Spinlock                lock;
	Type**                  types;
	s64                     types_count;
	s64                     types_capacity;
};
inline static Reflection reflect;

template <typename T>
struct TypeMapper {
	inline static Type* type = NULL;
};

template <typename T>
constexpr u32 reflect_get_type_size() {
	return sizeof(T);
}

template <>
constexpr u32 reflect_get_type_size<void>() {
	return 0;
}

template <typename T, typename TypeT>
TypeT* reflect_add_type(TypeT* type) {
	ScopedLock(reflect.lock);
	if (reflect.types == NULL) {
		reflect.types_capacity = 16;
		reflect.types = (Type**) malloc(sizeof(Type*) * reflect.types_capacity);
	} else if (reflect.types_count == reflect.types_capacity) {
		reflect.types_capacity *= 2;
		reflect.types = (Type**) realloc(reflect.types, sizeof(Type*) * reflect.types_capacity);
	}
	TypeMapper<T>::type = type;
	reflect.types[reflect.types_count] = type;
	type->size = reflect_get_type_size<T>();
	type->id = reflect.types_count;
	type->kind = TypeT::KIND;
	reflect.types_count += 1;
	return type;
}

template <typename T, typename TypeT>
TypeT* reflect_add_type_named(const char* name) {
	ScopedLock(reflect.lock);
	static TypeT type;
	reflect_add_type<T, TypeT>(&type);
	type.name = name;
	return &type;
}

template <MemberReflect T>
Type* reflect_type_internal() {
	using TypeT = std::remove_cvref_t<decltype(*T::reflect_type((T*) NULL, NULL))>;
	auto type = reflect_add_type_named<T, TypeT>("");
	return T::reflect_type((T*) NULL, type);
}

template <GlobalReflect T>
	requires (!MemberReflect<T>)
Type* reflect_type_internal() {
	using TypeT = std::remove_cvref_t<decltype(*reflect_type((T*) NULL, NULL))>;
	auto type = reflect_add_type_named<T, TypeT>("");
	return reflect_type((T*) NULL, type);
}

// Every type pointer auto casts to void*, that's why
//  we can't have global reflect_type(void*, ) to reflect type 'void'.
template <typename T> requires (std::is_same_v<T, void>)
Type* reflect_type_internal() {
	auto* type = reflect_add_type_named<void, PrimitiveType>("");
	type->name = "void";
	type->primitive_kind = PrimitiveKind::P_void;
	return type;
}

template <typename T>
Type* reflect_type_internal() {
	auto type = reflect_add_type_named<T, UnregisteredType>("");
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
Type* reflect_type_of() {
	ScopedLock(reflect.lock);
	using XT = remove_all_const<T>::type;
	using Mapper = TypeMapper<XT>;
	if (!Mapper::type) {
		auto type = reflect_type_internal<XT>();
		reflect_hook<XT, 0>({});
	}
	assert(Mapper::type);
	return Mapper::type;
}

template <typename T>
Type* reflect_type_of(T& value) {
	return reflect_type_of<std::remove_reference_t<T>>();
}

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
	auto type = reflect_type_of<T>();
	return make_any(type, thing);
}

template <typename T>
Any make_any(T& thing) {
	return make_any(&thing);
}

const char* resolve_type_name_alias(const char* name) {
	if (strcmp(name, "int") == 0) return "s32";
	if (strcmp(name, "long") == 0) return "s32";
	if (strcmp(name, "long long") == 0) return "s64";
	if (strcmp(name, "unsigned") == 0) return "u32";
	if (strcmp(name, "unsigned int") == 0) return "u32";
	if (strcmp(name, "unsigned long") == 0) return "u32";
	if (strcmp(name, "char") == 0) return "s8";
	if (strcmp(name, "short") == 0) return "s16";
	if (strcmp(name, "unsigned short") == 0) return "u16";
	if (strcmp(name, "char") == 0) return "s8";
	if (strcmp(name, "unsigned char") == 0) return "u8";
	if (strcmp(name, "float") == 0) return "f32";
	if (strcmp(name, "double") == 0) return "f64";
	if (strcmp(name, "unsigned long long") == 0) return "u64";
	return name;
}

PrimitiveType* reflect_type(bool* x, PrimitiveType* type) {
	type->name = "bool";
	type->primitive_kind = PrimitiveKind::P_bool;
	return type;
}

template <typename T> requires(!std::is_function_v<T>)
PointerType* reflect_type(T** thing, PointerType* type) {
	type->inner = reflect_type_of<T>();
	auto inner_name_length = strlen(type->inner->name);
	auto name = (char*) malloc(inner_name_length + 2);
	memcpy(name, type->inner->name, inner_name_length);
	strcpy(name + inner_name_length, "*");
	type->name = name;
	return type;
}

template <typename T, s64 N>
FixedArrayType* reflect_type(T (*arr)[N], FixedArrayType* type) {
	type->inner = reflect_type_of<T>();
	type->array_size = N;
	type->subkind = "fixed_array";
	type->name = heap_sprintf("%s[%lld]", type->inner->name, N);
	type->get_count = +[](void* arr) {
		return N;
	};
	type->get_item = +[](void* arr, s64 index) -> void* {
		return ptr_add(arr, sizeof(T) * index);
	};
	return type;
}

template <typename R, typename... Args>
FunctionType* reflect_type(R (**function)(Args...), FunctionType* type) {
	// @TODO: set name.
	type->return_type = reflect_type_of<R>();
	Type* arg_types[] = { reflect_type_of<Args>()... };
	s64 capacity = 0;
	for (Type* arg_type: arg_types) {
		type->arg_types.add(arg_type);
	}
	return type;
}

Type* reflect_get_pointer_inner_type_with_indirection_level(PointerType* pointer_type, s32* result_indirection_level) {
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

bool reflect_does_type_inherit(Type* derived, Type* base, s32* out_offset) {
	if (derived->kind != StructType::KIND ||
		base->kind    != StructType::KIND) {
		return false;
	}

	auto der = (StructType*) derived;
	auto b   = (StructType*) base;

	for (auto it: der->unflattened_members) {
		if (it.kind == STRUCT_MEMBER_KIND_BASE) {
			if (it.type == b) {
				*out_offset = it.offset;
				return true;
			}
			if (reflect_does_type_inherit(it.type, b, out_offset)) {
				*out_offset = *out_offset + it.offset;
				return true;
			}
		}
	}
	return false;
}

bool reflect_can_cast(Type* derived, Type* base, s32* out_offset) {
	if (derived == base) {
		return true;
	}
	return reflect_does_type_inherit(derived, base, out_offset);
}

template <typename T>
T* reflect_cast(auto* x) {
	s32 offset;
	if (reflect_can_cast(x->type, reflect_type_of<T>(), &offset)) {
		return (T*) x;
	}
	return NULL;
}

template <typename T>
T* reflect_cast(Any any) {
	s32 offset;
	if (reflect_can_cast(any.type, reflect_type_of<T>(), &offset)) {
		return (T*) ptr_add(any.ptr, -offset);
	}
	return NULL;
}

template <typename T>
T* reflect_cast(Any* any) {
	return reflect_cast<T>(*any);
}

template <typename T, typename X>
	requires(
		std::is_same_v<X, StructMember> ||
		std::is_same_v<X, EnumValue>)
T* find_tag(X* x) {
	for (auto it: x->tags) {
		auto casted = reflect_cast<T>(it);
		if (casted) {
			return casted;
		}
	}
	return NULL;
}

void reflect_flatten_struct_members(Type* type, s32 offset, StructMember member) {
	if (type->kind != StructType::KIND) {
		return;
	}
	auto t = (StructType*) type;
	if (member.kind == STRUCT_MEMBER_KIND_BASE) {
		if (auto member_type = member.type->as<StructType>()) {
			for (auto it: member_type->unflattened_members) {
				reflect_flatten_struct_members(type, offset + it.offset, it);
			}
		}
	} else if (member.kind == STRUCT_MEMBER_KIND_PLAIN) {
		member.offset = offset;
		t->members.add(member);
	}
}

void reflect_struct_type_add_member(Type* type, StructMember member) {
	if (type->kind != StructType::KIND) {
		return;
	}
	auto t = (StructType*) type;
	t->unflattened_members.add(member);
}

void reflect_maybe_flatten_struct_type(Type* type) {
	if (type->kind == StructType::KIND) {
		auto casted = (StructType*) type;
		for (auto it: casted->unflattened_members) {
			reflect_flatten_struct_members(type, it.offset, it);
		}
	}
}

void reflect_enum_type_add_value(Type* type, EnumValue v) {
	if (type->kind != EnumType::KIND) {
		return;
	}
	auto t = (EnumType*) type;
	t->values.add(v);
}

void reflect_add_tag(Type* type, auto value) {
	auto copied = new decltype(value)();
	*copied = value;
	auto tag = make_any(copied);

	if (auto casted = type->as<StructType>()) {
		if (casted->unflattened_members.count > 0) {
			auto* member = &casted->unflattened_members.data[casted->unflattened_members.count - 1];
			member->tags.add(tag);
		}
	} else if (auto casted = type->as<EnumType>()) {
		if (casted->values.count > 0) {
			auto* val = &casted->values.data[casted->values.count - 1];
			val->tags.add(tag);
		}
	}
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

template <typename T>
using ReflectMacroPickType = std::conditional_t<std::is_class_v<T>, StructType, EnumType>;

#define REFLECT_NAME(_T, _name)\
template <typename __T = _T> requires(std::is_convertible_v<__T, _T>)\
inline static ReflectMacroPickType<__T>* reflect_type(__T* x, ReflectMacroPickType<__T>* type) {\
	type->name = _name;\
	reflect_fill_type(x, type);\
	reflect_maybe_flatten_struct_type(type);\
	return type;\
}\
inline static void reflect_fill_type(_T* x, ReflectMacroPickType<_T>* type)

#define REFLECT(_T) REFLECT_NAME(_T, #_T)

#define MEMBER_AS(member, member_type) \
static_assert(sizeof(member_type) == sizeof(x->member));\
reflect_struct_type_add_member(type, StructMember{\
	#member,\
	reflect_type_of<member_type>(),\
	STRUCT_MEMBER_KIND_PLAIN,\
	(s32) OFFSETOF(std::remove_pointer_t<decltype(x)>, member)\
});

#define MEMBER(member) MEMBER_AS(member, decltype(x->member))

#define TAG(value) reflect_add_tag(type, value);

#define ENUM_VALUE(value) reflect_enum_type_add_value(type, EnumValue{ #value, make_primitive_value(std::remove_pointer_t<decltype(x)>::value) });

#define BASE_TYPE(base)\
/*assert((((s32) ((u8*) ((base*) ((decltype(x)) 0x10000000)) - (u8*) ((decltype(x)) 0x10000000))) == 0) && "non-zero offset base is not supported");*/\
static_assert(std::is_base_of_v<base, std::remove_pointer_t<decltype(x)>>);\
static_assert(!is_virtual_base_of<base, std::remove_pointer_t<decltype(x)>>::value);\
reflect_struct_type_add_member(type, StructMember{\
	reflect_type_of<base>()->name,\
	reflect_type_of<base>(),\
	STRUCT_MEMBER_KIND_BASE,\
	((s32) ((u8*) ((base*) ((decltype(x)) 0x10000000)) - (u8*) ((decltype(x)) 0x10000000))),\
});

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

REFLECT(Type) {
	MEMBER(name);
	MEMBER(kind);
	MEMBER(size);
	MEMBER(id);
}

struct ReflectArrayType: SpanType {
	s64 (*get_capacity)(void* arr);
};

template <typename T>
ReflectArrayType* reflect_type(ReflectArray<T>* x, ReflectArrayType* type) {
	type->inner = reflect_type_of<T>();
	type->subkind = "reflect_array";
	type->name = heap_sprintf("ReflectArray<%s>", type->inner->name);
	type->get_count = [](void* arr) {
		return ((ReflectArray<T>*) arr)->count;
	};
	type->get_capacity = [](void* arr) {
		return ((ReflectArray<T>*) arr)->capacity;
	};
	type->get_item = [](void* arr, s64 index) -> void* {
		return ((ReflectArray<T>*) arr)->operator[](index);
	};
	return type;
}

REFLECT(StructMemberKind) {
	ENUM_VALUE(STRUCT_MEMBER_KIND_PLAIN);
	ENUM_VALUE(STRUCT_MEMBER_KIND_BASE);
}

REFLECT(StructMember) {
	MEMBER(name);
	MEMBER(type);
	MEMBER(kind);
	MEMBER(offset);
	MEMBER(tags);
}

REFLECT(StructType) {
	BASE_TYPE(Type);
	MEMBER(unflattened_members);
	MEMBER(members);
	MEMBER(subkind);
}

REFLECT(Any) {
	MEMBER(type);
	MEMBER(ptr);
}
