#pragma once

#include "string_conversion.h"
#include "reflect.h"
#include "hash_map.h"
#include "sort.h"
#include "defer.h"
#include "function.h"

struct Formatter;

template <typename T>
concept Custom_Formattable = requires (T a) {
	{ type_format((Formatter*) 0, &a, String()) } -> std::same_as<void>;
};

using Type_Format_Type_Erased = void(Formatter*, void*, String);

template <Custom_Formattable T>
auto pick_type_format() {
	Proc<void(Formatter*, T*, String)>* result = +[](Formatter* formatter, T* thing, String spec) {
		type_format(formatter, thing, spec);
	};
	return (Type_Format_Type_Erased*) result;
}

template <typename T>
auto pick_type_format() {
	return (Type_Format_Type_Erased*) NULL;
}

static HashMap<Type*, Type_Format_Type_Erased*>* type_format_procs = NULL;

REFLECTION_REFLECT_HOOK(T) {
	auto proc = pick_type_format<T>();
	if (proc) {
		if (!type_format_procs) {
			type_format_procs = make<HashMap<Type*, Type_Format_Type_Erased*>>();
		}
		type_format_procs->put(reflect_type_of<T>(), proc);
	}
}

// Use this as a tag on a member to specify how that member must be formatted.
struct Custom_Member_Format {
	void (*format_proc)(Formatter*, Any member);
};

Custom_Member_Format make_custom_member_format(auto proc) {
	return { .format_proc = proc };
}


enum FormatFlags: s64 {
	FORMAT_DEFAULT     = 0,
	FORMAT_EXTENDED    = 1 << 0,
	FORMAT_DEREFERENCE = 1 << 1,
};

// If builder is not unicode(ASCII),
//  characters out of ASCII range are encoded as utf-8 into the builder.
struct Formatter {
	void*                  builder;
	bool                   is_unicode_builder = false;
	FormatFlags            flags = FORMAT_DEFAULT;
	String                 indent_text = "\t"_b;
	s64                    indentation = 0;
	s64                    line_start = 0;
	s64                    builder_start;
	Array<void*>           cyclic_pointer_tracker;
	s64                    depth = -1;
	bool                   quote_inner_string = false;

	bool is_compact() {
		return !(flags & FORMAT_EXTENDED);
	}

	void append(UnicodeString str) {
		if (is_unicode_builder) {
			((UnicodeStringBuilder*) builder)->append(str);
		} else {
			s64 length = utf8_length(str);
			auto b = (StringBuilder<>*) builder;
			auto ptr = b->reserve(length);
			encode_utf8(str, ptr);
		}
	}

	void append(String str) {
		if (is_unicode_builder) {
			((UnicodeStringBuilder*) builder)->append(str);
		} else {
			((StringBuilder<>*) builder)->append(str);
		}
	}

	s64 builder_cursor() {
		s64 length;
		if (is_unicode_builder) {
			length = len(*(UnicodeStringBuilder*) builder);
		} else {
			length = len(*(StringBuilder<>*) builder);
		}
		return length - builder_start;
	}

	void free() {
		cyclic_pointer_tracker.free();
	}
};

template <StringChar T>
Formatter make_formatter(StringBuilder<T>* b, Allocator allocator) {
	Formatter result;
	result.builder = b;
	result.is_unicode_builder = std::is_same_v<T, char32_t>;
	result.cyclic_pointer_tracker.allocator = allocator;
	result.builder_start = len(*b);
	return result;
}

void formatter_indent(Formatter* formatter) {
	for (auto i: range(formatter->indentation)) {
		formatter->append(formatter->indent_text);
	}
}

template <typename T>
void formatter_append_indented(Formatter* formatter, BaseString<T> str) {
	s64 index = 0;
	for (auto line: iterate_lines(str)) {
		if (index > 0) {
			formatter->line_start = formatter->builder_cursor();
		}
		if (line.length > 0 && formatter->builder_cursor() == formatter->line_start) {
			formatter_indent(formatter);
		}
		formatter->append(line);
		index += 1;
	}
}

void format(Formatter* formatter, String str) {
	formatter_append_indented(formatter, str);
}

void format(Formatter* formatter, UnicodeString str) {
	formatter_append_indented(formatter, str);
}

void format(Formatter* formatter, const char* c_str) {
	format(formatter, make_string(c_str));
}

void format_item(Formatter* formatter, Type* type, void* thing, String spec);

void format_item(Formatter* formatter, Any any, String spec) {
	format_item(formatter, any.type, any.ptr, spec);
}



// Example of |fmt| string:
//    "% %[0] %(spec) %[0](spec)"
// [N] after argument specifies argument index, negative values are parsed and accepted,
//   but it's up to |insert_arg| to decide how to handle negative indices.
// Index at next '%' insertions is N + 1, N + 2, etc.
// (S) after argument specifies format specifition where S is specifition,
//  that's passed into type_format().
//
//  Escaping is not applied to characters in [] and in ().
//    Escapable characters are specified in the beginning of the functions in |escapable|.
//  Embedded parenthesis ((example(((fmt_spec))))) are allowed.

struct FormatFlag {
	String text;
	s64    flag;
	bool   unset = false;
};

template <typename Char>
void format_parser(
	BaseString<Char> fmt,
	ArrayView<String> short_specs,
	ArrayView<FormatFlag> format_flags,
	auto* flags,
	auto insert_char,
	auto insert_arg
) {
	
	constexpr char escapable[] = { '%', '[', '(' };

	sort(format_flags, lambda(len(format_flags[$1]->text) < len(format_flags[$2]->text)));
	reverse(format_flags);

	u32 arg_index = 0;
	for (s64 i = 0; i < fmt.length; i++) {
		auto c = fmt[i];
		if (c == '\\') {
			for (auto e: range(static_array_count(escapable))) {
				if (fmt.length > i + 1 && fmt[i + 1] == escapable[e]) {
					insert_char(escapable[e]);
					i += 1;
					continue;
				}
			}
		} else if (c == '%') {
			decltype(fmt) format_spec;

			for (auto it: short_specs) {
				if (starts_with(slice(fmt, i + 1), it)) {
					format_spec = slice(fmt, i + 1, it.length);
					i += it.length;
					break;
				}
			}

			if (starts_with(slice(fmt, i + 1), "["_b)) {
				auto [idx_str, found] = take_until(slice(fmt, i + 2), lambda($0 == ']'));
				if (found) {
					s64 new_idx;
					if (parse_integer(idx_str, &new_idx)) {
						arg_index = new_idx;
						i += idx_str.length + 2;
					}
				}
			}

			while (true) {
				auto start = slice(fmt, i + 1);
				for (auto it: format_flags) {
					if (starts_with(start, it.text)) {
						if (it.unset) {
							*flags &= ~it.flag;
						} else {
							*flags |= it.flag;
						}
						i += it.text.length;
						goto next;
					}
				}
				break;
			next:
				continue;
			}

			if (format_spec.length == 0 && starts_with(slice(fmt, i + 1), "("_b)) {
				s64 open_counter = 1;
				s64 closing_index = -1;
				for (auto j: range_from_to(i + 2, fmt.length)) {
					if (fmt[j] == '(') {
						open_counter += 1;
					}
					if (fmt[j] == ')') {
						open_counter -= 1;
						if (open_counter <= 0) {
							closing_index = j;
							break;
						}
					}
				}

				if (closing_index != -1) {
					auto str = slice(fmt, i + 2, closing_index - i - 2);
					format_spec = str;
					i = closing_index;
				}
			}

			insert_arg(arg_index, format_spec);
			arg_index += 1;
			continue;
		} else {
			insert_char(c);
		}
	}
}

Tuple<String, bool> formatter_resolve_spec(Formatter* formatter, UnicodeString spec) {
	return { encode_utf8(spec), true };
}

Tuple<String, bool> formatter_resolve_spec(Formatter* formatter, String spec) {
	return { spec, false };
}

template <StringChar T, typename... Args>
void format(Formatter* formatter, BaseString<T> format_str, Args... args) {

	Any things[] = { make_any(&args)... };

	String short_specs[] = { "p"_b, "P"_b, "h"_b, "H"_b, "b"_b, "B"_b };

	FormatFlag flags[] = {
		{
			.text = "*"_b,
			.flag = (s64) FORMAT_DEREFERENCE,
		},
		{
			.text = "*-"_b,
			.flag = (s64) FORMAT_DEREFERENCE,
			.unset = true,
		},
		{
			.text = "+"_b,
			.flag = (s64) FORMAT_EXTENDED,
		},
		{
			.text = "+-"_b,
			.flag = (s64) FORMAT_EXTENDED,
			.unset = true,
		},
	};

	auto specs_view = make_array_view(short_specs);
	auto flags_view = make_array_view(flags);

	auto insert_char = [&] (auto c) {
		formatter_append_indented(formatter, make_string(&c, 1));
	};

	auto insert_arg = [&] (s64 idx, auto spec) {
		if (idx >= 0 && idx < static_array_count(things)) {
			auto [resolved_spec, have_to_free] = formatter_resolve_spec(formatter, spec);
			format_item(formatter, things[idx], resolved_spec);
			if (have_to_free) {
				resolved_spec.free();
			}
		}
	};

	format_parser(
		format_str,
		specs_view,
		flags_view,
		(s64*) &formatter->flags,
		insert_char,
		insert_arg
	);
}

template <StringChar T, typename... Args>
void format(Formatter* formatter, const T* c_str, Args... args) {
	format(formatter, make_string(c_str), args...);
}

template <typename... Args>
void format(Formatter* formatter, Args... args) {

	constexpr auto N = sizeof...(args);

	char buf[N * 3 + 1];
	int  cursor = 0;
	for (auto i: range(N)) {
		const char* str = (i == N - 1) ? "%*" : "%*, ";
		memcpy(buf + cursor, str, strlen(str));
		cursor += strlen(str);
	}
	buf[cursor] = '\0';
	format(formatter, make_string(buf, cursor), args...);
}

struct Struct_Printer {
	Formatter* formatter;
	s64        member_index = 0;

	void head(String name) {
		if (formatter->is_compact()) {
			format(formatter, "{ "_b);
		} else {
			format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void member_head(String type, String name) {
		if (formatter->is_compact()) {
			if (member_index > 0 ) {
				format(formatter, ", "_b);
			}
			format(formatter, "% = ", name);
			formatter->quote_inner_string = true;
		} else {
			format(formatter, "% = ", name);
			formatter->quote_inner_string = true;
		}
	}

	void member_tail() {
		if (!formatter->is_compact()) {
			format(formatter, "\n");
		}
		member_index += 1;
	}

	void member(String type, String name, auto value) {
		member_head(type, name);
		format(formatter, "%", value);
		member_tail();
	}

	void member(void* ptr, StructMember member) {
		member_head(make_string(member.type->name), make_string(member.name));
		auto custom_member_fmt_tag = find_tag<Custom_Member_Format>(&member);
		auto any = make_any(member.type, ptr_add(ptr, member.offset));
		if (custom_member_fmt_tag) {
			custom_member_fmt_tag->format_proc(formatter, any);
		} else {
			format(formatter, "%", any);
		}
		member_tail();
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			format(formatter, " }"_b);
		} else {
			format(formatter, "}"_b);
		}
	}
};

Struct_Printer make_struct_printer(Formatter* formatter) {
	return { .formatter = formatter };
}

struct Array_Printer {
	Formatter* formatter;
	s64        index = 0;

	void head(String name) {
		if (formatter->is_compact()) {
			format(formatter, "["_b);
		} else {
			format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void item(auto value) {
		if (formatter->is_compact()) {
			if (index > 0) {
				format(formatter, ", "_b);
			}
			formatter->quote_inner_string = true;
			format(formatter, "%", value);
		} else {
			if (index > 0) {
				format(formatter, ",\n"_b);
			}
			format(formatter, "[%] = ", index);
			formatter->quote_inner_string = true;
			format(formatter, "%", value);
		}
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			format(formatter, "]"_b);
		} else {
			format(formatter, "\n}"_b);
		}
	}
};

Array_Printer make_array_printer(Formatter* formatter) {
	return { .formatter = formatter };
}

struct Tuple_Printer {
	Formatter* formatter;
	s64        index = 0;

	void head() {
		if (formatter->is_compact()) {
			format(formatter, "("_b);
		} else {
			format(formatter, "("_b);
		}
		formatter->indentation += 1;
	}

	void item(auto value) {
		if (formatter->is_compact()) {
			format(formatter, ", "_b);
		} else {
			format(formatter, ",\n"_b);
		}
		formatter->quote_inner_string = true;
		format(formatter, "%", value);
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		format(formatter, ")"_b);
	}
};

Tuple_Printer make_tuple_printer(Formatter* formatter) {
	return { .formatter = formatter };
}

struct Map_Printer {
	Formatter* formatter;
	s64        index = 0;

	void head(String name) {
		if (formatter->is_compact()) {
			format(formatter, "{"_b);
		} else {
			format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void item(auto key, auto value) {
		if (formatter->is_compact()) {
			if (index > 0) {
				format(formatter, ", "_b);
			}
			format(formatter, "%=", key);
			formatter->quote_inner_string = true;
			format(formatter, "%", value);
		} else {
			if (index > 0) {
				format(formatter, ",\n"_b);
			}
			format(formatter, "% = ", key);
			formatter->quote_inner_string = true;
			format(formatter, "%", value);
		}
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			format(formatter, "}"_b);
		} else {
			format(formatter, "\n}"_b);
		}
	}
};

Map_Printer make_map_printer(Formatter* formatter) {
	return { .formatter = formatter };
}

void format_struct(Formatter* formatter, StructType* type, void* thing) {
	auto printer = make_struct_printer(formatter);
	printer.head(make_string(type->name));
	for (auto it: type->members) {
		printer.member(thing, it);
	}
	printer.tail();
}

void format_array(Formatter* formatter, ArrayType* type, void* thing) {
	auto printer = make_array_printer(formatter);
	printer.head(make_string(type->name));
	for (auto i: range(type->get_count(thing))) {
		void* item = type->get_item(thing, i);
		printer.item(make_any(type->inner, item));
	}
	printer.tail();
}

void format_tuple(Formatter* formatter, StructType* type, void* thing) {
	auto printer = make_tuple_printer(formatter);
	printer.head();
	for (auto it: type->members) {
		printer.item(make_any(it.type, ptr_add(thing, it.offset)));
	}
	printer.tail();
}

void format_map(Formatter* formatter, MapType* type, void* thing) {
	auto printer = make_map_printer(formatter);
	printer.head(make_string(type->name));
	for (auto* item: type->iterate(thing)) {
		auto key   = make_any(type->key,   item->key);
		auto value = make_any(type->value, item->value);
		printer.item(key, value);
	}
	printer.tail();
}

template <typename T> requires (std::numeric_limits<T>::is_integer)
void format_integer_primitive(Formatter* formatter, T num, String spec) {
	IntegerStringParams p;
	if (spec == "b"_b) {
		p.base = 2;
	} else if (spec == "B"_b) {
		p.base = 2;
		p.uppercase = true;
	} else if (spec == "h"_b) {
		p.base = 16;
	} else if (spec == "H"_b) {
		p.base = 16;
		p.uppercase = true;
	}

	SmallString num_str = to_string(num, p);
	format(formatter, as_str(&num_str));
}

void format_float_primitive(Formatter* formatter, f64 num, String spec) {
	SmallString num_str = to_string(num);
	format(formatter, as_str(&num_str));
}

void format_primitive(Formatter* formatter, PrimitiveType* type, void* thing, String spec) {

	switch (type->primitive_kind) {
		case PrimitiveKind::P_u8:
			format_integer_primitive(formatter, *(u8*)  thing, spec);
			break;
		case PrimitiveKind::P_s8:
			format_integer_primitive(formatter, *(s8*)  thing, spec);
			break;
		case PrimitiveKind::P_u16:
			format_integer_primitive(formatter, *(u16*) thing, spec);
			break;
		case PrimitiveKind::P_s16:
			format_integer_primitive(formatter, *(s16*) thing, spec);
			break;
		case PrimitiveKind::P_u32:
			format_integer_primitive(formatter, *(u32*) thing, spec);
			break;
		case PrimitiveKind::P_s32:
			format_integer_primitive(formatter, *(s32*) thing, spec);
			break;
		case PrimitiveKind::P_u64:
			format_integer_primitive(formatter, *(u64*) thing, spec);
			break;
		case PrimitiveKind::P_s64:
			format_integer_primitive(formatter, *(s64*) thing, spec);
			break;

		case PrimitiveKind::P_f32:
			format_float_primitive(formatter, *(f32*)  thing, spec);
			break;
		case PrimitiveKind::P_f64:
			format_float_primitive(formatter, *(f64*)  thing, spec);
			break;

		case PrimitiveKind::P_bool: {
			SmallString str = to_string(*(bool*) thing);
			format(formatter, as_str(&str));
		}
		break;

		case PrimitiveKind::P_char: {
			format(formatter, "'%()'", String((char*) thing, 1));
		}
		break;
		case PrimitiveKind::P_wchar: {
			char32_t c = (char32_t) (*(wchar_t*) thing);
			format(formatter, "'%()'", UnicodeString(&c, 1));
		}
		break;
		case PrimitiveKind::P_char16: {
			char32_t c = *(char16_t*) thing;
			format(formatter, "'%()'", UnicodeString(&c, 1));
		}
		break;
		case PrimitiveKind::P_char32: {
			format(formatter, "'%()'", UnicodeString((char32_t*) thing, 1));
		}
		break;

		case PrimitiveKind::P_void:
			// Should not happen.
			format(formatter, "(void)");
			break;
		// @NewPrimitive

		default:
			format(formatter, "(unknown primitive)");
			break;
	}
}

void format_string(Formatter* formatter, StringType* type, void* thing, String spec) {
	bool quote = contains(spec, "quote"_b) || formatter->quote_inner_string;
	formatter->quote_inner_string = false;
	if (quote) {
		format(formatter, "\"");
	}

	if (type->string_kind == StringKind::Ascii) {
		format(formatter, *(String*) thing);
	} else if (type->string_kind == StringKind::Utf32) {
		format(formatter, *(UnicodeString*) thing);
	} else {
		assert(false);
	}

	if (quote) {
		format(formatter, "\"");
	}
}

bool is_flag_set(void* dst, void* src, u64 size) {
	for (auto i: range(size)) {
		u8 dst_v = *(u8*) dst;
		u8 src_v = *(u8*) src;

		if ((dst_v & src_v) != src_v) {
			return false;
		}
	}
	return true;
}

void format_enum(Formatter* formatter, EnumType* type, void* thing) {

	if (type->is_flags) {

		s32  matched = 0;
		for (auto it: type->values) {
			if (is_flag_set(thing, &it.value, type->size)) {
				if (matched > 0) {
					format(formatter, '|');
				}
				format(formatter, it.name);
				matched += 1;		
			}
		}
		if (matched != 0) {
			return;
		}
	} else {
		for (auto it: type->values) {
			if (memcmp(&it.value, thing, type->size) == 0) {
				format(formatter, it.name);
				return;				
			}
		}
	}

	format(formatter, "%(%)", type->name, make_any(type->base_type, thing));
}

template <typename T>
void format_c_string(Formatter* formatter, T* c_str, String spec) {
	if (c_str == NULL) {
		format(formatter, "NULL");
		return;
	}

	bool quote = contains(spec, "quote"_b) || formatter->quote_inner_string;
	formatter->quote_inner_string = false;
	if (quote) {
		format(formatter, "\"");
	}

	char32_t buf[128];

	s64 length = 0;
	s64 start  = 0;
	while (true) {
		if ((length - start) == static_array_count(buf)) {
			format(formatter, make_string(buf, length - start));
			start = length;
		}
		if (length >= 4096 || c_str[length] == '\0') {
			format(formatter, make_string(buf, length - start));
			break;
		}
		buf[length - start] = c_str[length];
		length += 1;
	}

	if (quote) {
		format(formatter, "\"");
	}
}


void format_pointer(Formatter* formatter, PointerType* type, void* thing, String spec) {
	s32  indir_level;
	auto inner_type = reflect_get_pointer_inner_type_with_indirection_level(type, &indir_level);

	char stars_buf[64];
	auto stars_count = min_u32(static_array_count(stars_buf), indir_level);
	for (auto i: range(stars_count)) {
		stars_buf[i] = '*';
	}
	auto stars = make_string(stars_buf, stars_count);

	bool did_deref_before = false;
	bool did_add_to_cycle_tracker = false;

	void* ptr_value = *(void**) thing;

	if (formatter->flags & FORMAT_DEREFERENCE) {
		did_deref_before = formatter->cyclic_pointer_tracker.contains(ptr_value);
		if (!did_deref_before) {
			formatter->cyclic_pointer_tracker.add(ptr_value);
			did_add_to_cycle_tracker = true;
		}
	}
	defer { 
		if (did_add_to_cycle_tracker) {
			formatter->cyclic_pointer_tracker.remove(ptr_value);
		}
	};

	bool deref = (formatter->flags & FORMAT_DEREFERENCE) && !did_deref_before;

	if (spec == "*"_b) {
		deref = true;
	}

	void* ptr = thing;
	// Detect NULL pointer.
	if (deref) {
		for (auto i: range(indir_level)) {
			ptr = *(void**)ptr;
			if (!ptr) {
				deref = false;
				break;
			}
		}
	}
	// Can't deref void type.
	if (deref) {
		if (inner_type == reflect_type_of<void>()) {
			deref = false;
		}
	}

	if (!deref) {
		auto num = bitcast<u64>(*(void**) thing);
		format(formatter, "(%%) %h", make_string(inner_type->name), stars, num);
		return;
	}

	if (inner_type == reflect_type_of<char>()) {
		format_c_string(formatter, (char*) ptr, spec);
	} else if (inner_type == reflect_type_of<wchar_t>()) {
		format_c_string(formatter, (wchar_t*) ptr, spec);
	} else if (inner_type == reflect_type_of<char16_t>()) {
		format_c_string(formatter, (char16_t*) ptr, spec);
	} else if (inner_type == reflect_type_of<char32_t>()) {
		format_c_string(formatter, (char32_t*) ptr, spec);
	} else {
		auto inner = make_any(inner_type, ptr);

		if (formatter->is_compact()) {
			format(formatter, "%", inner);
		} else {
			format(formatter, "%", stars);
			format(formatter, "%", inner);
		}
	}
}

void format_function(Formatter* formatter, FunctionType* type, void* thing, String spec) {
	format(formatter, "% (*)(", type->return_type->name);
	for (auto i: range(type->arg_types.count)) {
		format(formatter, type->arg_types.data[i]->name);
		if (i != type->arg_types.count - 1) {
			format(formatter, ", ");
		}
	}
	format(formatter, ")");
}

void format_item(Formatter* formatter, Type* type, void* thing, String spec) {

	if (type_format_procs) {
		if (auto* fmt_proc = type_format_procs->get(type)) {
			(*fmt_proc)(formatter, thing, spec);
			return;
		}
	}

	formatter->depth += 1;
	defer { formatter->depth -= 1; };
	auto saved_quote_string = formatter->quote_inner_string;
	formatter->quote_inner_string = false;

	auto real_type = get_real_type(type, thing);

	switch (real_type->kind) {
		case StructType::KIND: {
			auto casted = (StructType*) real_type;
			if (strcmp(casted->subkind, "tuple") == 0) {
				format_tuple(formatter, casted, thing);
			} else {
				format_struct(formatter, casted, thing);
			}
		}
		break;

		case PrimitiveType::KIND: {
			auto casted = (PrimitiveType*) real_type;
			format_primitive(formatter, casted, thing, spec);
		}
		break;

		case StringType::KIND: {
			auto casted = (StringType*) real_type;
			formatter->quote_inner_string = saved_quote_string;
			format_string(formatter, casted, thing, spec);
		}
		break;

		case EnumType::KIND: {
			auto casted = (EnumType*) real_type;
			format_enum(formatter, casted, thing);
		}
		break;

		case ArrayType::KIND: {
			auto casted = (ArrayType*) real_type;
			format_array(formatter, casted, thing);
		}
		break;

		case PointerType::KIND: {
			auto casted = (PointerType*) real_type;
			auto inner_type = reflect_get_pointer_inner_type_with_indirection_level(casted, NULL);
			if (inner_type == reflect_type_of<char>() ||
				inner_type == reflect_type_of<wchar_t>() ||
				inner_type == reflect_type_of<char16_t>() ||
				inner_type == reflect_type_of<char32_t>()) {
				formatter->quote_inner_string = saved_quote_string;
			}
			format_pointer(formatter, casted, thing, spec);
		}
		break;

		case FunctionType::KIND: {
			auto casted = (FunctionType*) real_type;
			format_function(formatter, casted, thing, spec);
		}
		break;

		case UnregisteredType::KIND: {
			format(formatter, "(Unregistered type)"_b);
		}
		break;

		default: {
			format(formatter, "(Unknown type)"_b);
		}
		break;
	}
}

template <StringChar T>
void format(StringBuilder<T>* builder, Allocator allocator, auto... args) {
	auto formatter = make_formatter(builder, allocator);
	format(&formatter, args...);
	formatter.free();
}

template <StringChar T>
void format(StringBuilder<T>* builder, auto... args) {
	format(builder, c_allocator, args...);
}

void formatln(Formatter* formatter, auto... args) {
	format(formatter, args...);
	format(formatter, "\n"_b);
}

template <StringChar T>
void formatln(StringBuilder<T>* builder, Allocator allocator, auto... args) {
	format(builder, allocator, args...);
	format(builder, allocator, "\n");
}

template <StringChar T>
void formatln(StringBuilder<T>* builder, auto... args) {
	formatln(builder, c_allocator, args...);
}

void print_to_stdout(String text) {
	fwrite(text.data, text.length, 1, stdout);
}

void print_to_stdout(UnicodeString text) {
	auto utf8 = encode_utf8(text);
	print_to_stdout(utf8);
	utf8.free();
}

void print(auto... args) {
	auto builder = build_unicode_string();
	format(&builder, c_allocator, args...);
	auto text = builder.get_string();
	print_to_stdout(text);
}

void println(auto... args) {
	print(args...);
	print(U"\n"_b);
}

template <StringChar T = char>
BaseString<T> sprint(Allocator allocator, auto... args) {
	auto builder = build_string<T>();
	format(&builder, allocator, args...);
	return builder.get_string();
}

template <StringChar T = char>
BaseString<T> sprint(auto... args) {
	return sprint(c_allocator, args...);
}

UnicodeString sprint_unicode(Allocator allocator, auto... args) {
	return sprint<char32_t>(allocator, args...);
}

UnicodeString sprint_unicode(auto... args) {
	return sprint<char32_t>(c_allocator, args...);
}


void format_fixed_array_as_c_string(Formatter* formatter, Any thing) {
	if (thing.type->kind == ArrayType::KIND) {
		auto casted = (ArrayType*) thing.type;
		if (strcmp(casted->subkind, "fixed_array") == 0) {
			auto casted = (FixedArrayType*) thing.type;
			if (casted->inner == reflect_type_of<char>()) {
				auto ptr = (char*) thing.ptr;
				s64 length = 0;
				for (auto i: range(casted->array_size)) {
					if (ptr[i] == '\0') {
						break;
					}
					length += 1;
				}
				auto str = make_string(ptr, length);
				format(formatter, "%", str);
				return;
			}
		}
	}

	format(formatter, "%", thing);
}

template <typename T>
void type_format(Formatter* formatter, Optional<T>* opt, String spec) {
	if (opt->has_value) {
		format(formatter, "Optional{%}", opt->value);
	} else {
		format(formatter, "Optional{}");
	}
}

void type_format(Formatter* formatter, CodeLocation* loc, String spec) {
	format(formatter, "%: %", make_string(loc->file), loc->line);
}

REFLECT(CodeLocation) {
	MEMBER(file);
	MEMBER(line);
}
