#pragma once

#include "grd_base.h"
#include "sync/grd_atomics.h"
#include "sync/grd_spinlock.h"
#include "grd_pointer_math.h"
#include "grd_compile_time_counter.h"
#include "grd_coroutine.h"
#include "grd_scoped.h"
#include "grd_heap_sprintf.h"
#include "grd_code_location.h"

template <typename T>
struct GrdReflectGrdArray {
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


using GrdTypeId = u32;

// GrdTypeKind is safe to cast to C string.
struct GrdTypeKind {
	constexpr static u64 SZ = 8;
	char s[SZ] = {};

	bool operator==(GrdTypeKind rhs) {
		return memcmp(this, &rhs, sizeof(*this)) == 0;
	}
};

template <int N> requires(N <= sizeof(GrdTypeKind::SZ))
GRD_DEDUP constexpr GrdTypeKind grd_make_type_kind(const char (&str)[N]){
	GrdTypeKind kind;
	for (int i = 0; i < (N - 1); i++) {
		kind.s[i] = str[i];
	}
	return kind;
}

GRD_DEDUP const char* grd_type_kind_as_c_str(GrdTypeKind* kind) {
	return (const char*) kind->s;
}

struct GrdType {
	const char*    name    = "<unnamed>";
	GrdTypeKind    kind    = { 0 };
	u32            size    = 0;
	GrdTypeId      id      = 0;
};

struct GrdAny {
	GrdType* type = NULL;
	void*    ptr  = NULL;
};

struct GrdPrimitiveValue {
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

	bool operator==(GrdPrimitiveValue other) {
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
};

GRD_DEDUP GrdPrimitiveValue grd_make_primitive_value(auto thing) {
	GrdPrimitiveValue v = {};
	static_assert(sizeof(thing) <= sizeof(v));
	memcpy(&v, &thing, sizeof(thing));
	return v;
}

enum class GrdPrimitiveKind: u32 {
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

struct GrdPrimitiveType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("prim"); 

	GrdPrimitiveKind primitive_kind;
};

struct GrdPointerType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("pointer"); 

	GrdType* inner = NULL;
};

struct GrdUnregisteredType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("unreg");
};


struct GrdEnumValue {
	const char*             name = NULL;
	GrdPrimitiveValue       value;
	GrdReflectGrdArray<GrdAny> tags;
};

struct GrdEnumType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("enum");

	GrdPrimitiveType*             base_type = NULL;
	bool                          is_flags  = false;
	GrdReflectGrdArray<GrdEnumValue> values;
};

enum GrdStructMemberKind: s32 {
	GRD_STRUCT_MEMBER_KIND_PLAIN = 0,
	GRD_STRUCT_MEMBER_KIND_BASE  = 1,
};

struct GrdStructMember {
	const char*             name   = NULL;
	GrdType*                type   = NULL;
	GrdStructMemberKind     kind   = GRD_STRUCT_MEMBER_KIND_PLAIN;
	s32                     offset = 0;
	GrdReflectGrdArray<GrdAny> tags;
};

struct GrdStructType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("struct");

	GrdReflectGrdArray<GrdStructMember> unflattened_members;
	GrdReflectGrdArray<GrdStructMember> members;
	const char*                      subkind = "";
};

struct GrdSpanType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("span");

	GrdType*    inner = NULL;
	const char* subkind = "";

	s64   (*get_count)   (void* arr) = NULL;
	void* (*get_item)    (void* arr, s64 index) = NULL;
};

struct GrdMapType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("map");

	GrdType*    key     = NULL;
	GrdType*    value   = NULL;
	const char* subkind = "";

	struct Item {
		void* key;
		void* value;
	};

	s64                    (*get_count)    (void* map) = NULL;
	// @TODO: move get_capacity to more specific type.?
	s64                    (*get_capacity) (void* map) = NULL;
	GrdGenerator<Item*>    (*iterate)      (void* map) = NULL;
};

struct GrdFixedArrayType: GrdSpanType {
	s64 array_size = 0;
};

struct GrdFunctionType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("func");

	GrdType*                  return_type = NULL;
	GrdReflectGrdArray<GrdType*> arg_types;
};

template <typename T>
GRD_DEDUP GrdPrimitiveType* grd_reflect_primitive_type(const char* name, GrdPrimitiveKind kind);

template <typename T, typename TypeT>
GRD_DEDUP TypeT* grd_reflect_register_type(const char* name);

#define GRD_REFLECTION_REFLECT_PRIMITIVE(TYPE, KIND)\
GRD_DEDUP GrdPrimitiveType* grd_reflect_create_type(TYPE* x) {\
	return grd_reflect_register_type<TYPE, GrdPrimitiveType>("");\
}\
GRD_DEDUP void grd_reflect_type(TYPE* x, GrdPrimitiveType* type) {\
	type->name = #TYPE;\
	type->primitive_kind = GrdPrimitiveKind::P_##KIND;\
}
GRD_REFLECTION_REFLECT_PRIMITIVE(u8,       u8);
GRD_REFLECTION_REFLECT_PRIMITIVE(s8,       s8);
GRD_REFLECTION_REFLECT_PRIMITIVE(u16,      u16);
GRD_REFLECTION_REFLECT_PRIMITIVE(s16,      s16);
GRD_REFLECTION_REFLECT_PRIMITIVE(u32,      u32);
GRD_REFLECTION_REFLECT_PRIMITIVE(s32,      s32);
GRD_REFLECTION_REFLECT_PRIMITIVE(u64,      u64);
GRD_REFLECTION_REFLECT_PRIMITIVE(s64,      s64);
GRD_REFLECTION_REFLECT_PRIMITIVE(f32,      f32);
GRD_REFLECTION_REFLECT_PRIMITIVE(f64,      f64);
GRD_REFLECTION_REFLECT_PRIMITIVE(char,     char);
GRD_REFLECTION_REFLECT_PRIMITIVE(wchar_t,  wchar);
GRD_REFLECTION_REFLECT_PRIMITIVE(char16_t, char16);
GRD_REFLECTION_REFLECT_PRIMITIVE(char32_t, char32);
#if GRD_IS_POSIX
	GRD_REFLECTION_REFLECT_PRIMITIVE(unsigned long, u64);
	GRD_REFLECTION_REFLECT_PRIMITIVE(long, s64);
#endif
#undef GRD_REFLECTION_REFLECT_PRIMITIVE
// @NewPrimitive

using GrdReflectionHookCounter = GrdCompileTimeCounter<>;

template <u64 _N>
struct GrdReflectHookIndexTag {
	constexpr static u64 N = _N;
};

template <typename T, u64 N>
struct GrdCallNextReflectHook {
	GrdCallNextReflectHook() {
		grd_reflect_hook<T>(GrdReflectHookIndexTag<N>{});
	}
};

#define GRD_REFLECTION_REFLECT_HOOK(type_name)\
template <typename type_name>\
GRD_DEDUP void grd_reflect_hook(\
	GrdReflectHookIndexTag<GrdReflectionHookCounter::Increment<>> t,\
	GrdCallNextReflectHook<type_name, GrdReflectionHookCounter::Read<> + 1> next = { })

template <typename T, u64 N>
GRD_DEDUP void grd_reflect_hook(GrdReflectHookIndexTag<N> tag, GrdEmptyStruct x = {}) {
	if constexpr (N == 0) {
		grd_reflect_hook<T>(GrdReflectHookIndexTag<N + 1>{});
	}
}


struct GrdReflection {
	GrdSpinlock lock;
	GrdType**   types;
	s64         types_count;
	s64         types_capacity;
};
GRD_DEDUP static GrdReflection GRD_REFLECT;

template <typename T>
struct GrdTypeMapper {
	GRD_DEDUP static GrdType* type = NULL;
};

template <typename T>
GRD_DEDUP constexpr u32 grd_reflect_get_type_size() {
	return sizeof(T);
}

template <>
GRD_DEDUP constexpr u32 grd_reflect_get_type_size<void>() {
	return 0;
}

// GRD_DEDUP void grd_reflect_add_type_impl(GrdType* type, u64 tp_size, GrdTypeKind tp_kind) {
// 	GrdScopedLock(GRD_REFLECT.lock);
// 	if (GRD_REFLECT.types == NULL) {
// 		GRD_REFLECT.types_capacity = 16;
// 		GRD_REFLECT.types = (GrdType**) malloc(sizeof(GrdType*) * GRD_REFLECT.types_capacity);
// 	} else if (GRD_REFLECT.types_count == GRD_REFLECT.types_capacity) {
// 		GRD_REFLECT.types_capacity *= 2;
// 		GRD_REFLECT.types = (GrdType**) realloc(GRD_REFLECT.types, sizeof(GrdType*) * GRD_REFLECT.types_capacity);
// 	}
// 	GRD_REFLECT.types[GRD_REFLECT.types_count] = type;
// 	type->size = tp_size;
// 	type->id = GRD_REFLECT.types_count;
// 	type->kind = tp_kind;
// 	GRD_REFLECT.types_count += 1;
// }

template <typename T>
struct grd_remove_all_const: std::remove_const<T> {};
template <typename T>
struct grd_remove_all_const<T*> {
	using type = typename grd_remove_all_const<T>::type*;
};
template <typename T>
struct grd_remove_all_const<T* const> {
	using type = typename grd_remove_all_const<T>::type*;
};

template <typename T>
using grd_reflect_clean_type = typename std::remove_reference_t<typename grd_remove_all_const<T>::type>;

template <typename T, typename TypeT>
GRD_DEF grd_reflect_register_type(const char* name) -> TypeT* {
	GrdScopedLock(GRD_REFLECT.lock);
	static TypeT type;
	type.name = name;
	type.id = GRD_REFLECT.types_count++;
	type.kind = TypeT::KIND;
	type.size = grd_reflect_get_type_size<T>();
	return &type;
}

template <typename T>
GRD_DEDUP GrdUnregisteredType* grd_reflect_create_type(T* x) {
	return grd_reflect_register_type<T, GrdUnregisteredType>("UnregType");
}

template <typename T>
GRD_DEDUP void grd_reflect_type(T* x, GrdUnregisteredType* tp) {
}

GRD_DEDUP GrdPrimitiveType* grd_reflect_create_type(bool* x) {
	return grd_reflect_register_type<bool, GrdPrimitiveType>("");
}

GRD_DEDUP void grd_reflect_type(bool* x, GrdPrimitiveType* type) {
	type->name = "bool";
	type->primitive_kind = GrdPrimitiveKind::P_bool;
}

template <typename T>
GRD_DEDUP GrdPointerType* grd_reflect_create_type(T** thing) {
	return grd_reflect_register_type<T*, GrdPointerType>("");
}

template <typename T>
concept GrdReflectTypeOfMember =
	requires (grd_reflect_clean_type<T> x) { grd_reflect_clean_type<T>::grd_reflect_create_type(&x); };

template <typename T>
concept GrdReflectTypeOfGlobal =
	!GrdReflectTypeOfMember<T> &&
	requires (grd_reflect_clean_type<T> x) { grd_reflect_create_type(&x); };

template <typename T>
concept GrdReflectTypeOf = GrdReflectTypeOfGlobal<T> || GrdReflectTypeOfMember<T>;

template <GrdReflectTypeOfMember T>
GRD_DEDUP GrdType* grd_reflect_type_of();
template <GrdReflectTypeOfGlobal X>
GRD_DEDUP GrdType* grd_reflect_type_of();
template <typename T> requires (std::is_void_v<T>)
GRD_DEDUP GrdType* grd_reflect_type_of();


template <typename T>
GRD_DEDUP void grd_reflect_type(T** thing, GrdPointerType* type) {
	type->inner = grd_reflect_type_of<T>();
	auto inner_name_length = strlen(type->inner->name);
	auto name = (char*) malloc(inner_name_length + 2);
	memcpy(name, type->inner->name, inner_name_length);
	strcpy(name + inner_name_length, "*");
	type->name = name;
}

template <typename T, s64 N>
GRD_DEDUP GrdFixedArrayType* grd_reflect_create_type(T (*arr)[N]) {
	return grd_reflect_register_type<grd_reflect_clean_type<decltype(*arr)>, GrdFixedArrayType>("");
}

template <typename T, s64 N>
GRD_DEDUP void grd_reflect_type(T (*arr)[N], GrdFixedArrayType* type) {
	type->inner = grd_reflect_type_of<T>();
	type->array_size = N;
	type->subkind = "fixed_array";
	type->name = grd_heap_sprintf("%s[%lld]", type->inner->name, N);
	type->get_count = +[](void* arr) {
		return N;
	};
	type->get_item = +[](void* arr, s64 index) -> void* {
		return grd_ptr_add(arr, sizeof(T) * index);
	};
}

template <typename R, typename... Args>
GRD_DEDUP GrdFunctionType* grd_reflect_create_type(R (**function)(Args...)) {
	return grd_reflect_register_type<R (*)(Args...), GrdFunctionType>("");
}

template <typename R, typename... Args>
GRD_DEDUP void grd_reflect_type(R (**function)(Args...), GrdFunctionType* type) {
	type->return_type = grd_reflect_type_of<R>();
	GrdType* arg_types[] = { grd_reflect_type_of<Args>()... };
	s64 capacity = 0;
	for (GrdType* arg_type: arg_types) {
		type->arg_types.add(arg_type);
	}
	s64 str_len = 16;
	str_len += strlen(type->name);
	for (GrdType* arg_type: type->arg_types) {
		str_len += strlen(arg_type->name);
	}
	char* str = (char*) malloc(str_len);
	str[0] = '\0';
	strcat(str, type->return_type->name);
	strcat(str, "(");
	for (s64 i = 0; i < type->arg_types.count; i++) {
		if (i > 0) {
			strcat(str, ", ");
		}
		strcat(str, type->arg_types.data[i]->name);
	}
	strcat(str, ")");
	type->name = str;
}

GRD_DEDUP void grd_reflect_maybe_flatten_struct_type(GrdType* type);

template <GrdReflectTypeOfMember T>
GRD_DEDUP GrdType* grd_reflect_type_of() {
	GrdScopedLock(GRD_REFLECT.lock);
	using XT = grd_reflect_clean_type<T>;
	using Mapper = GrdTypeMapper<XT>;
	if (!Mapper::type) {\
		auto type = XT::grd_reflect_create_type((XT*) NULL);
		Mapper::type = type;
		XT::grd_reflect_type((XT*) NULL, type);
		grd_reflect_maybe_flatten_struct_type(type);
		grd_reflect_hook<XT, 0>({});
	}
	assert(Mapper::type);
	return Mapper::type;
}

template <GrdReflectTypeOfGlobal T>
GRD_DEDUP GrdType* grd_reflect_type_of() {
	GrdScopedLock(GRD_REFLECT.lock);
	using XT = grd_reflect_clean_type<T>;
	using Mapper = GrdTypeMapper<XT>;
	if (!Mapper::type) {
		auto type = grd_reflect_create_type((XT*) NULL);
		Mapper::type = type;
		grd_reflect_type((XT*) NULL, type);
		grd_reflect_maybe_flatten_struct_type(type);
		grd_reflect_hook<XT, 0>({});
	}
	assert(Mapper::type);
	return Mapper::type;
}

template <typename T> requires (std::is_void_v<T>)
GRD_DEDUP GrdType* grd_reflect_type_of() {
	GrdScopedLock(GRD_REFLECT.lock);
	using Mapper = GrdTypeMapper<void>;
	if (!Mapper::type) {\
		auto type = grd_reflect_register_type<void, GrdPrimitiveType>("void");
		Mapper::type = type;
		type->primitive_kind = GrdPrimitiveKind::P_void;
		grd_reflect_maybe_flatten_struct_type(type);
		grd_reflect_hook<void, 0>({});
	}
	assert(Mapper::type);
	return Mapper::type;
}

template <typename T>
GRD_DEDUP GrdType* grd_reflect_type_of(T& value) {
	return grd_reflect_type_of<std::remove_reference_t<T>>();
}

GRD_DEDUP GrdAny grd_make_any(GrdType* type, void* ptr) {
	return { .type = type, .ptr = ptr };
}

// Prevents GrdAny containing GrdAny, in variadic expressions like grd_make_any(args)... .
GRD_DEDUP GrdAny grd_make_any(GrdAny any) {
	return any;
}
GRD_DEDUP GrdAny grd_make_any(GrdAny* any) {
	return grd_make_any(*any);
}

template <typename T>
GRD_DEDUP GrdAny grd_make_any(T* thing) {
	auto type = grd_reflect_type_of<T>();
	return grd_make_any(type, thing);
}

template <typename T>
GRD_DEDUP GrdAny grd_make_any(T& thing) {
	return grd_make_any(&thing);
}

template <typename T>
GRD_DEDUP T* grd_type_as(GrdType* tp) {
	if (!tp) {
		return NULL;
	}
	if (tp->kind != T::KIND) {
		return NULL;
	}
	return (T*) tp;
}

GRD_DEDUP const char* grd_resolve_type_name_alias(const char* name) {
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

GRD_DEDUP GrdType* grd_reflect_get_pointer_inner_type_with_indirection_level(GrdPointerType* pointer_type, s32* result_indirection_level) {
	s32 indirection_level = 1;
	while (pointer_type->inner->kind == grd_make_type_kind("pointer")) {
		indirection_level += 1;
		pointer_type = (GrdPointerType*) pointer_type->inner;
	}

	if (result_indirection_level) {
		*result_indirection_level = indirection_level;
	}
	return pointer_type->inner;
}

GRD_DEDUP bool grd_reflect_does_type_inherit(GrdType* derived, GrdType* base, s32* out_offset) {
	if (derived->kind != GrdStructType::KIND ||
		base->kind    != GrdStructType::KIND) {
		return false;
	}

	auto der = (GrdStructType*) derived;
	auto b   = (GrdStructType*) base;

	for (auto it: der->unflattened_members) {
		if (it.kind == GRD_STRUCT_MEMBER_KIND_BASE) {
			if (it.type == b) {
				*out_offset = it.offset;
				return true;
			}
			if (grd_reflect_does_type_inherit(it.type, b, out_offset)) {
				*out_offset = *out_offset + it.offset;
				return true;
			}
		}
	}
	return false;
}

GRD_DEDUP bool grd_reflect_can_cast(GrdType* derived, GrdType* base, s32* out_offset) {
	if (derived == base) {
		*out_offset = 0;
		return true;
	}
	return grd_reflect_does_type_inherit(derived, base, out_offset);
}

template <typename T>
GRD_DEDUP T* grd_reflect_cast(auto* x) {
	s32 offset;
	if (grd_reflect_can_cast(x->type, grd_reflect_type_of<T>(), &offset)) {
		return (T*) x;
	}
	return NULL;
}

template <typename T>
GRD_DEDUP T* grd_reflect_cast(GrdAny any) {
	s32 offset = 0;
	if (grd_reflect_can_cast(any.type, grd_reflect_type_of<T>(), &offset)) {
		return (T*) grd_ptr_add(any.ptr, -offset);
	}
	return NULL;
}

template <typename T>
GRD_DEDUP T* grd_reflect_cast(GrdAny* any) {
	return grd_reflect_cast<T>(*any);
}

template <typename T, typename X>
	requires(
		std::is_same_v<X, GrdStructMember> ||
		std::is_same_v<X, GrdEnumValue>)
GRD_DEDUP T* grd_find_tag(X* x) {
	for (auto it: x->tags) {
		auto casted = grd_reflect_cast<T>(it);
		if (casted) {
			return casted;
		}
	}
	return NULL;
}

GRD_DEDUP void grd_reflect_flatten_struct_members(GrdType* type, s32 offset, GrdStructMember member) {
	if (type->kind != GrdStructType::KIND) {
		return;
	}
	auto t = (GrdStructType*) type;
	if (member.kind == GRD_STRUCT_MEMBER_KIND_BASE) {
		if (auto member_type = grd_type_as<GrdStructType>(member.type)) {
			for (auto it: member_type->unflattened_members) {
				grd_reflect_flatten_struct_members(type, offset + it.offset, it);
			}
		}
	} else if (member.kind == GRD_STRUCT_MEMBER_KIND_PLAIN) {
		member.offset = offset;
		t->members.add(member);
	}
}

GRD_DEDUP void grd_reflect_struct_type_add_member(GrdType* type, GrdStructMember member) {
	if (type->kind != GrdStructType::KIND) {
		return;
	}
	auto t = (GrdStructType*) type;
	t->unflattened_members.add(member);
}

GRD_DEDUP void grd_reflect_maybe_flatten_struct_type(GrdType* type) {
	if (type->kind == GrdStructType::KIND) {
		auto casted = (GrdStructType*) type;
		for (auto it: casted->unflattened_members) {
			grd_reflect_flatten_struct_members(type, it.offset, it);
		}
	}
}

GRD_DEDUP void grd_reflect_enum_type_add_value(GrdType* type, GrdEnumValue v) {
	if (type->kind != GrdEnumType::KIND) {
		return;
	}
	auto t = (GrdEnumType*) type;
	t->values.add(v);
}

GRD_DEDUP void grd_reflect_add_tag(GrdType* type, auto value) {
	auto copied = new decltype(value)();
	*copied = value;
	auto tag = grd_make_any(copied);

	if (auto casted = grd_type_as<GrdStructType>(type)) {
		if (casted->unflattened_members.count > 0) {
			auto* member = &casted->unflattened_members.data[casted->unflattened_members.count - 1];
			member->tags.add(tag);
		}
	} else if (auto casted = grd_type_as<GrdEnumType>(type)) {
		if (casted->values.count > 0) {
			auto* val = &casted->values.data[casted->values.count - 1];
			val->tags.add(tag);
		}
	}
}

template <typename From, typename To, typename = void>
struct grd_can_static_cast: std::false_type { };
template <typename From, typename To>
struct grd_can_static_cast<From, To, decltype(static_cast<To>(std::declval<From>()), void())>: std::true_type { };
template <typename Base, typename Derived, typename = void>
struct grd_is_virtual_base_of: std::false_type { };
template <typename Base, typename Derived>
struct grd_is_virtual_base_of<Base, Derived, typename std::enable_if<
    std::is_base_of<Base, Derived>::value && 
    !grd_can_static_cast<Base*, Derived*>::value
>::type>: std::true_type{ };

template <typename T>
using GrdReflectMacroPickType = std::conditional_t<std::is_class_v<T>, GrdStructType, GrdEnumType>;

// #define GRD_REFLECT_NAME(__X, _name)\
// template <typename __T = __X> requires (std::is_same_v<__X, __T>)\
// static GrdReflectMacroPickType<__T>* grd_reflect_create_type(__T* x) { return grd_reflect_register_type<__T, GrdReflectMacroPickType<__T>>(_name); }\
// template <typename __T = __X> requires (std::is_same_v<__X, __T>)\
// static void grd_reflect_type(__T* x, GrdReflectMacroPickType<__T>* type)

// grd_reflect_type() must be a template to make sure,
//   grd_reflect_type_of<T> returns the correct type if occurs earlier in the code than grd_reflect_type(T*).
#define GRD_REFLECT_NAME(__T, _name)\
template <int _N = 0> requires (_N == 0)\
GRD_DEDUP static GrdReflectMacroPickType<__T>* grd_reflect_create_type(__T* x) { return grd_reflect_register_type<__T, GrdReflectMacroPickType<__T>>(_name); }\
template <int _N = 0> requires (_N == 0)\
GRD_DEDUP static void grd_reflect_type(__T* x, GrdReflectMacroPickType<__T>* type)


#define GRD_REFLECT(_T) GRD_REFLECT_NAME(_T, #_T)

#define GRD_MEMBER_AS(member, member_type) \
static_assert(sizeof(member_type) == sizeof(x->member));\
grd_reflect_struct_type_add_member(type, GrdStructMember{\
	#member,\
	grd_reflect_type_of<member_type>(),\
	GRD_STRUCT_MEMBER_KIND_PLAIN,\
	(s32) GRD_OFFSETOF(std::remove_pointer_t<decltype(x)>, member)\
});

#define GRD_MEMBER(member) GRD_MEMBER_AS(member, decltype(x->member))

#define GRD_TAG(value) grd_reflect_add_tag(type, value);

#define GRD_ENUM_VALUE(value) grd_reflect_enum_type_add_value(type, GrdEnumValue{ #value, grd_make_primitive_value(std::remove_pointer_t<decltype(x)>::value) });

#define GRD_BASE_TYPE(base)\
/*assert((((s32) ((u8*) ((base*) ((decltype(x)) 0x10000000)) - (u8*) ((decltype(x)) 0x10000000))) == 0) && "non-zero offset base is not supported");*/\
static_assert(std::is_base_of_v<base, std::remove_pointer_t<decltype(x)>>);\
static_assert(!grd_is_virtual_base_of<base, std::remove_pointer_t<decltype(x)>>::value);\
grd_reflect_struct_type_add_member(type, GrdStructMember{\
	grd_reflect_type_of<base>()->name,\
	grd_reflect_type_of<base>(),\
	GRD_STRUCT_MEMBER_KIND_BASE,\
	((s32) ((u8*) ((base*) ((decltype(x)) 0x10000000)) - (u8*) ((decltype(x)) 0x10000000))),\
});

struct GrdRealTypeMember {
	GRD_REFLECT(GrdRealTypeMember) {}
};

GRD_DEDUP GrdType* grd_get_real_type(GrdType* type, void* ptr) {
	if (ptr != NULL) {
		if (type->kind == GrdStructType::KIND) {
			auto casted = (GrdStructType*) type;
			for (auto it: casted->members) {
				auto tag = grd_find_tag<GrdRealTypeMember>(&it);
				if (tag) {
					auto candidate = *(GrdType**) grd_ptr_add(ptr, it.offset);
					if (candidate) {
						return candidate;
					}
				}
			}
		}
	}
	return type;
}

GRD_REFLECT(GrdType) {
	GRD_MEMBER(name);
	GRD_MEMBER(kind);
	GRD_MEMBER(size);
	GRD_MEMBER(id);
}

struct GrdReflectGrdArrayType: GrdSpanType {
	s64 (*get_capacity)(void* arr);
};

template <typename T>
GRD_DEDUP GrdReflectGrdArrayType* grd_reflect_create_type(GrdReflectGrdArray<T>* x) {
	return grd_reflect_register_type<GrdReflectGrdArray<T>, GrdReflectGrdArrayType>("");
}

template <typename T>
GRD_DEDUP void grd_reflect_type(GrdReflectGrdArray<T>* x, GrdReflectGrdArrayType* type) {
	type->inner = grd_reflect_type_of<T>();
	type->subkind = "reflect_array";
	type->name = grd_heap_sprintf("GrdReflectGrdArray<%s>", type->inner->name);
	type->get_count = [](void* arr) {
		return ((GrdReflectGrdArray<T>*) arr)->count;
	};
	type->get_capacity = [](void* arr) {
		return ((GrdReflectGrdArray<T>*) arr)->capacity;
	};
	type->get_item = [](void* arr, s64 index) -> void* {
		return ((GrdReflectGrdArray<T>*) arr)->operator[](index);
	};
}

GRD_REFLECT(GrdStructMemberKind) {
	GRD_ENUM_VALUE(GRD_STRUCT_MEMBER_KIND_PLAIN);
	GRD_ENUM_VALUE(GRD_STRUCT_MEMBER_KIND_BASE);
}

GRD_REFLECT(GrdStructMember) {
	GRD_MEMBER(name);
	GRD_MEMBER(type);
	GRD_MEMBER(kind);
	GRD_MEMBER(offset);
	GRD_MEMBER(tags);
}

GRD_REFLECT(GrdStructType) {
	GRD_BASE_TYPE(GrdType);
	GRD_MEMBER(unflattened_members);
	GRD_MEMBER(members);
	GRD_MEMBER(subkind);
}

GRD_REFLECT(GrdAny) {
	GRD_MEMBER(type);
	GRD_MEMBER(ptr);
}

struct GrdGeneratorType: GrdType {
	constexpr static auto KIND = grd_make_type_kind("genr"); 

	GrdType* gen_type = NULL;
};

template <typename T>
GRD_DEDUP GrdGeneratorType* grd_reflect_create_type(GrdGenerator<T>* x) {
	return grd_reflect_register_type<GrdGenerator<T>, GrdGeneratorType>("");
}

template <typename T>
GRD_DEDUP void grd_reflect_type(GrdGenerator<T>* x, GrdGeneratorType* type) {
	type->gen_type = grd_reflect_type_of<T>();
}

GRD_REFLECT(GrdTypeKind) {
	GRD_MEMBER(s);
}

GRD_REFLECT(GrdCodeLoc) {
	GRD_MEMBER(file);
	GRD_MEMBER(line);
}
