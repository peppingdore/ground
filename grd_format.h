#pragma once

#include "grd_number_string_conversion.h"
#include "grd_reflect.h"
#include "grd_hash_map.h"
#include "grd_sort.h"
#include "grd_defer.h"
#include "grd_function.h"

struct GrdFormatter;

template <typename T>
concept GrdCustomFormattable = requires (T a) {
	{ grd_type_format((GrdFormatter*) 0, &a, GrdString()) } -> std::same_as<void>;
};

using GrdTypeFormatTypeErased = void(GrdFormatter*, void*, GrdString);

template <GrdCustomFormattable T>
auto grd_pick_type_format() {
	GrdProc<void(GrdFormatter*, T*, GrdString)>* result = +[](GrdFormatter* formatter, T* thing, GrdString spec) {
		grd_type_format(formatter, thing, spec);
	};
	return (GrdTypeFormatTypeErased*) result;
}

template <typename T>
auto grd_pick_type_format() {
	return (GrdTypeFormatTypeErased*) NULL;
}

static GrdHashMap<GrdType*, GrdTypeFormatTypeErased*>* GRD_TYPE_FORMAT_PROCS = NULL;

GRD_REFLECTION_REFLECT_HOOK(T) {
	auto proc = grd_pick_type_format<T>();
	if (proc) {
		if (!GRD_TYPE_FORMAT_PROCS) {
			GRD_TYPE_FORMAT_PROCS = grd_make<GrdHashMap<GrdType*, GrdTypeFormatTypeErased*>>();
		}
		grd_put(GRD_TYPE_FORMAT_PROCS, grd_reflect_type_of<T>(), proc);
	}
}

// Use this as a tag on a member to specify how that member must be formatted.
struct GrdCustomMemberFormat {
	void (*format_proc)(GrdFormatter*, GrdAny member);

	GRD_REFLECT(GrdCustomMemberFormat) {
		GRD_MEMBER(format_proc);
	}
};

GrdCustomMemberFormat grd_make_custom_member_format(auto proc) {
	return { .format_proc = proc };
}

enum GrdFormatFlags: s64 {
	FORMAT_DEFAULT     = 0,
	FORMAT_EXTENDED    = 1 << 0,
	FORMAT_DEREFERENCE = 1 << 1,
};

// If builder is not unicode(ASCII),
//  characters out of ASCII range are encoded as utf-8 into the builder.
struct GrdFormatter {
	void*                  builder;
	bool                   is_unicode_formatter = false;
	GrdFormatFlags         flags = FORMAT_DEFAULT;
	GrdString              indent_text = "\t"_b;
	s64                    indentation = 0;
	s64                    line_start = 0;
	s64                    builder_start;
	GrdArray<void*>        cyclic_pointer_tracker;
	s64                    depth = -1;
	bool                   quote_inner_string = false;

	GRD_REFLECT(GrdFormatter) {
		
	}


	bool is_compact() {
		return !(flags & FORMAT_EXTENDED);
	}

	void grd_append(GrdUnicodeString str) {
		if (is_unicode_formatter) {
			grd_add((GrdArray<char32_t>*) builder, str);
		} else {
			s64 length = grd_utf8_length(str);
			auto b = (GrdArray<char>*) builder;
			auto ptr = grd_reserve(b, length);
			grd_encode_utf8(str, ptr);
		}
	}

	void grd_append(GrdString str) {
		if (is_unicode_formatter) {
			::grd_append((GrdArray<char32_t>*) builder, str);
		} else {
			::grd_append((GrdArray<char>*) builder, str);
		}
	}

	s64 builder_cursor() {
		s64 length;
		if (is_unicode_formatter) {
			length = grd_len(*(GrdArray<char32_t>*) builder);
		} else {
			length = grd_len(*(GrdArray<char>*) builder);
		}
		return length - builder_start;
	}

	void free() {
		cyclic_pointer_tracker.free();
	}
};

template <GrdStringChar T>
GrdFormatter grd_make_formatter(GrdArray<T>* b, GrdAllocator allocator) {
	GrdFormatter result;
	result.builder = b;
	result.is_unicode_formatter = std::is_same_v<T, char32_t>;
	result.cyclic_pointer_tracker.allocator = allocator;
	result.builder_start = grd_len(*b);
	return result;
}

void grd_formatter_indent(GrdFormatter* formatter) {
	for (auto i: grd_range(formatter->indentation)) {
		formatter->grd_append(formatter->indent_text);
	}
}

template <GrdStringChar T>
void grd_formatter_append_indented(GrdFormatter* formatter, GrdSpan<T> str) {
	s64 index = 0;
	for (auto line: grd_iterate_lines(str)) {
		if (index > 0) {
			formatter->line_start = formatter->builder_cursor();
		}
		if (grd_len(line) > 0 && formatter->builder_cursor() == formatter->line_start) {
			grd_formatter_indent(formatter);
		}
		formatter->grd_append(line);
		index += 1;
	}
}

void grd_format(GrdFormatter* formatter, GrdString str) {
	grd_formatter_append_indented(formatter, str);
}

void grd_format(GrdFormatter* formatter, GrdUnicodeString str) {
	grd_formatter_append_indented(formatter, str);
}

void grd_format(GrdFormatter* formatter, const char* c_str) {
	grd_format(formatter, grd_make_string(c_str));
}

void grd_format_item(GrdFormatter* formatter, GrdType* type, void* thing, GrdString spec);

void grd_format_item(GrdFormatter* formatter, GrdAny any, GrdString spec) {
	grd_format_item(formatter, any.type, any.ptr, spec);
}



// Example of |fmt| string:
//    "% %[0] %(spec) %[0](spec)"
// [N] after argument specifies argument index, negative values are parsed and accepted,
//   but it's up to |insert_arg| to decide how to handle negative indices.
// Index at next '%' insertions is N + 1, N + 2, etc.
// (S) after argument specifies format specifition where S is specifition,
//  that's passed into type_grd_format().
//
//  Escaping is not applied to characters in [] and in ().
//    Escapable characters are specified in the beginning of the functions in |escapable|.
//  Embedded parenthesis ((example(((fmt_spec))))) are allowed.

struct GrdFormatFlag {
	GrdString text;
	s64    flag;
	bool   unset = false;
};

template <GrdStringChar Char>
void format_parser(
	GrdSpan<Char> fmt,
	GrdSpan<GrdString> short_specs,
	GrdSpan<GrdFormatFlag> format_flags,
	auto* flags,
	auto insert_char,
	auto insert_arg
) {
	
	constexpr char escapable[] = { '%', '[', '(' };

	grd_sort(format_flags, grd_lambda(_, x, y, grd_len(format_flags[x].text) < grd_len(format_flags[y].text))); 
	grd_reverse(format_flags);

	u32 arg_index = 0;
	for (s64 i = 0; i < grd_len(fmt); i++) {
		auto c = fmt[i];
		if (c == '\\') {
			bool insert_escapable = false;
			for (auto e: grd_range(grd_static_array_count(escapable))) {
				if (grd_len(fmt) > i + 1 && fmt[i + 1] == escapable[e]) {
					insert_char(escapable[e]);
					i += 1;
					insert_escapable = true;
					break;
				}
			}
			if (!insert_escapable) {
				insert_char(c);
			}
		} else if (c == '%') {
			decltype(fmt) format_spec;

			for (auto it: short_specs) {
				if (grd_starts_with(fmt[i + 1, grd_len(fmt)], it)) {
					format_spec = fmt[i + 1, i + 1 + grd_len(it)];
					i += grd_len(it);
					break;
				}
			}

			if (grd_starts_with(fmt[i + 1, grd_len(fmt)], "["_b)) {
				auto [idx_str, found] = grd_take_until(fmt[i + 2, grd_len(fmt)], grd_lambda(x, x == ']'));
				if (found) {
					s64 new_idx;
					if (grd_parse_integer(idx_str, &new_idx)) {
						arg_index = new_idx;
						i += grd_len(idx_str) + 2; 
					}
				}
			}

			while (true) {
				auto start = fmt[i + 1, grd_len(fmt)];
				for (auto it: format_flags) {
					if (grd_starts_with(start, it.text)) {
						if (it.unset) {
							*flags &= ~it.flag;
						} else {
							*flags |= it.flag;
						}
						i += grd_len(it.text);
						goto next;
					}
				}
				break;
			next:
				continue;
			}

			if (grd_len(format_spec) == 0 && grd_starts_with(fmt[i + 1, {}], "("_b)) {
				s64 open_counter = 1;
				s64 closing_index = -1;
				for (auto j: grd_range_from_to(i + 2, grd_len(fmt))) {
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
					auto str = fmt[i + 2, closing_index]; 
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

GrdTuple<GrdString, bool> formatter_resolve_spec(GrdFormatter* formatter, GrdUnicodeString spec) {
	return { grd_encode_utf8(spec), true };
}

GrdTuple<GrdString, bool> formatter_resolve_spec(GrdFormatter* formatter, GrdString spec) {
	return { spec, false };
}

template <GrdStringChar T>
void grd_format_impl(GrdFormatter* formatter, GrdSpan<T> format_str, std::initializer_list<GrdAny> things) {

	GrdString short_specs[] = { "p"_b, "P"_b, "h"_b, "H"_b, "b"_b, "B"_b };

	GrdFormatFlag flags[] = {
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

	auto specs_view = grd_make_span(short_specs);
	auto flags_view = grd_make_span(flags);

	auto insert_char = [&] (auto c) {
		grd_formatter_append_indented(formatter, grd_make_string(&c, 1));
	};

	auto insert_arg = [&] (s64 idx, auto spec) {
		if (idx >= 0 && idx < things.size()) {
			auto [resolved_spec, have_to_free] = formatter_resolve_spec(formatter, spec);
			grd_format_item(formatter, things.begin()[idx], resolved_spec);
			if (have_to_free) {
				GrdFree(resolved_spec.data);
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

template <GrdStringChar T, typename... Args>
void grd_format(GrdFormatter* formatter, const T* fmt_str, Args... args) {
	grd_format_impl(formatter, grd_make_string(fmt_str), { grd_make_any(&args)... });
}

template <GrdStringChar T, typename... Args>
void grd_format(GrdFormatter* formatter, GrdSpan<T> fmt_str, Args... args) {
	grd_format_impl(formatter, fmt_str, { grd_make_any(&args)... });
}

template <typename... Args>
void grd_format(GrdFormatter* formatter, Args... args) {

	constexpr auto N = sizeof...(args);

	char buf[N * 3 + 1];
	int  cursor = 0;
	for (auto i: grd_range(N)) {
		const char* str = (i == N - 1) ? "%*" : "%*, ";
		memcpy(buf + cursor, str, strlen(str));
		cursor += strlen(str);
	}
	buf[cursor] = '\0';
	grd_format_impl(formatter, grd_make_string(buf, cursor), { grd_make_any(&args)... });
}

struct Struct_Printer {
	GrdFormatter* formatter;
	s64        member_index = 0;

	void head(GrdString name) {
		if (formatter->is_compact()) {
			grd_format(formatter, "{ "_b);
		} else {
			grd_format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void member_head(GrdString type, GrdString name) {
		if (formatter->is_compact()) {
			if (member_index > 0 ) {
				grd_format(formatter, ", "_b);
			}
			grd_format(formatter, "% = ", name);
			formatter->quote_inner_string = true;
		} else {
			grd_format(formatter, "% = ", name);
			formatter->quote_inner_string = true;
		}
	}

	void member_tail() {
		if (!formatter->is_compact()) {
			grd_format(formatter, "\n");
		}
		member_index += 1;
	}

	void member(GrdString type, GrdString name, auto value) {
		member_head(type, name);
		grd_format(formatter, "%", value);
		member_tail();
	}

	void member(void* ptr, GrdStructMember member) {
		member_head(grd_make_string(member.type->name), grd_make_string(member.name));
		auto custom_member_fmt_tag = grd_find_tag<GrdCustomMemberFormat>(&member);
		auto any = grd_make_any(member.type, grd_ptr_add(ptr, member.offset));
		if (custom_member_fmt_tag) {
			custom_member_fmt_tag->format_proc(formatter, any);
		} else {
			grd_format(formatter, "%", any);
		}
		member_tail();
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			grd_format(formatter, " }"_b);
		} else {
			grd_format(formatter, "}"_b);
		}
	}
};

Struct_Printer grd_make_struct_printer(GrdFormatter* formatter) {
	return { .formatter = formatter };
}

struct GrdArray_Printer {
	GrdFormatter* formatter;
	s64        index = 0;

	void head(GrdString name) {
		if (formatter->is_compact()) {
			grd_format(formatter, "["_b);
		} else {
			grd_format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void item(auto value) {
		if (formatter->is_compact()) {
			if (index > 0) {
				grd_format(formatter, ", "_b);
			}
			formatter->quote_inner_string = true;
			grd_format(formatter, "%", value);
		} else {
			if (index > 0) {
				grd_format(formatter, ",\n"_b);
			}
			grd_format(formatter, "[%] = ", index);
			formatter->quote_inner_string = true;
			grd_format(formatter, "%", value);
		}
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			grd_format(formatter, "]"_b);
		} else {
			grd_format(formatter, "\n}"_b);
		}
	}
};

GrdArray_Printer grd_make_array_printer(GrdFormatter* formatter) {
	return { .formatter = formatter };
}

struct Tuple_Printer {
	GrdFormatter* formatter;
	s64        index = 0;

	void head() {
		if (formatter->is_compact()) {
			grd_format(formatter, "("_b);
		} else {
			grd_format(formatter, "("_b);
		}
		formatter->indentation += 1;
	}

	void item(auto value) {
		if (formatter->is_compact()) {
			grd_format(formatter, ", "_b);
		} else {
			grd_format(formatter, ",\n"_b);
		}
		formatter->quote_inner_string = true;
		grd_format(formatter, "%", value);
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		grd_format(formatter, ")"_b);
	}
};

Tuple_Printer grd_make_tuple_printer(GrdFormatter* formatter) {
	return { .formatter = formatter };
}

struct Map_Printer {
	GrdFormatter* formatter;
	s64        index = 0;

	void head(GrdString name) {
		if (formatter->is_compact()) {
			grd_format(formatter, "{"_b);
		} else {
			grd_format(formatter, "% {\n", name);
		}
		formatter->indentation += 1;
	}

	void item(auto key, auto value) {
		if (formatter->is_compact()) {
			if (index > 0) {
				grd_format(formatter, ", "_b);
			}
			grd_format(formatter, "%=", key);
			formatter->quote_inner_string = true;
			grd_format(formatter, "%", value);
		} else {
			if (index > 0) {
				grd_format(formatter, ",\n"_b);
			}
			grd_format(formatter, "% = ", key);
			formatter->quote_inner_string = true;
			grd_format(formatter, "%", value);
		}
		index += 1;
	}

	void tail() {
		formatter->indentation -= 1;
		if (formatter->is_compact()) {
			grd_format(formatter, "}"_b);
		} else {
			grd_format(formatter, "\n}"_b);
		}
	}
};

Map_Printer grd_make_map_printer(GrdFormatter* formatter) {
	return { .formatter = formatter };
}

// @TODO: add grd prefixes. and remove underscores.
void format_struct(GrdFormatter* formatter, GrdStructType* type, void* thing) {
	auto printer = grd_make_struct_printer(formatter);
	printer.head(grd_make_string(type->name));
	for (auto it: type->members) {
		printer.member(thing, it);
	}
	printer.tail();
}

template <typename T>
void format_c_string(GrdFormatter* formatter, T* c_str, GrdString spec) {
	if (c_str == NULL) {
		grd_format(formatter, "NULL");
		return;
	}

	bool quote = grd_contains(spec, "quote"_b) || formatter->quote_inner_string;
	formatter->quote_inner_string = false;
	if (quote) {
		grd_format(formatter, "\"");
	}

	char32_t buf[128];

	s64 length = 0;
	s64 start  = 0;
	while (true) {
		if ((length - start) == grd_static_array_count(buf)) {
			grd_format(formatter, grd_make_string(buf, length - start));
			start = length;
		}
		if (length >= 4096 || c_str[length] == '\0') {
			grd_format(formatter, grd_make_string(buf, length - start));
			break;
		}
		buf[length - start] = c_str[length];
		length += 1;
	}

	if (quote) {
		grd_format(formatter, "\"");
	}
}

void format_string(GrdFormatter* formatter, GrdSpanType* type, void* thing, GrdString spec) {
	bool quote = grd_contains(spec, "quote"_b) || formatter->quote_inner_string;
	formatter->quote_inner_string = false;
	if (quote) {
		grd_format(formatter, "\"");
	}

	if (type->inner == grd_reflect_type_of<char>()) {
		grd_format(formatter, *(GrdString*) thing);
	} else if (type->inner == grd_reflect_type_of<char32_t>()) {
		grd_format(formatter, *(GrdUnicodeString*) thing);
	} else {
		assert(false);
	}

	if (quote) {
		grd_format(formatter, "\"");
	}
}

void format_span(GrdFormatter* formatter, GrdSpanType* type, void* thing, GrdString spec) {
	if (type->inner == grd_reflect_type_of<char>() ||
		type->inner == grd_reflect_type_of<char32_t>()) {
		format_string(formatter, type, thing, spec);
		return;
	}

	auto printer = grd_make_array_printer(formatter);
	printer.head(grd_make_string(type->name));
	for (auto i: grd_range(type->get_count(thing))) {
		void* item = type->get_item(thing, i);
		printer.item(grd_make_any(type->inner, item));
	}
	printer.tail();
}

void format_tuple(GrdFormatter* formatter, GrdStructType* type, void* thing) {
	auto printer = grd_make_tuple_printer(formatter);
	printer.head();
	for (auto it: type->members) {
		printer.item(grd_make_any(it.type, grd_ptr_add(thing, it.offset)));
	}
	printer.tail();
}

void format_map(GrdFormatter* formatter, GrdMapType* type, void* thing) {
	auto printer = grd_make_map_printer(formatter);
	printer.head(grd_make_string(type->name));
	for (auto* item: type->iterate(thing)) {
		auto key   = grd_make_any(type->key,   item->key);
		auto value = grd_make_any(type->value, item->value);
		printer.item(key, value);
	}
	printer.tail();
}

template <typename T> requires (std::numeric_limits<T>::is_integer)
void format_integer_primitive(GrdFormatter* formatter, T num, GrdString spec) {
	if (spec == "sz") {
		if (num >= 1024ULL * 1024 * 1024 * 1024) {
			grd_format(formatter, "% TB", f64(num) / f64(1024ULL * 1024 * 1024 * 1024));
		} else if (num >= 1024ULL * 1024 * 1024) {
			grd_format(formatter, "% GB", f64(num) / f64(1024ULL * 1024 * 1024));
		} else if (num >= 1024ULL * 1024) {
			grd_format(formatter, "% MB", f64(num) / f64(1024ULL * 102));
		} else if (num >= 1024) {
			grd_format(formatter, "% KB", f64(num) / f64(1024ULL));
		} else {
			grd_format(formatter, "% B", num);
		}
		return;
	}

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

	GrdSmallString num_str = grd_to_string(num, p);
	grd_format(formatter, grd_as_str(&num_str));
}

void format_float_primitive(GrdFormatter* formatter, f64 num, GrdString spec) {
	GrdSmallString num_str = grd_to_string(num);
	grd_format(formatter, grd_as_str(&num_str));
}

void format_primitive(GrdFormatter* formatter, GrdPrimitiveType* type, void* thing, GrdString spec) {

	switch (type->primitive_kind) {
		case GrdPrimitiveKind::P_u8:
			format_integer_primitive(formatter, *(u8*)  thing, spec);
			break;
		case GrdPrimitiveKind::P_s8:
			format_integer_primitive(formatter, *(s8*)  thing, spec);
			break;
		case GrdPrimitiveKind::P_u16:
			format_integer_primitive(formatter, *(u16*) thing, spec);
			break;
		case GrdPrimitiveKind::P_s16:
			format_integer_primitive(formatter, *(s16*) thing, spec);
			break;
		case GrdPrimitiveKind::P_u32:
			format_integer_primitive(formatter, *(u32*) thing, spec);
			break;
		case GrdPrimitiveKind::P_s32:
			format_integer_primitive(formatter, *(s32*) thing, spec);
			break;
		case GrdPrimitiveKind::P_u64:
			format_integer_primitive(formatter, *(u64*) thing, spec);
			break;
		case GrdPrimitiveKind::P_s64:
			format_integer_primitive(formatter, *(s64*) thing, spec);
			break;

		case GrdPrimitiveKind::P_f32:
			format_float_primitive(formatter, *(f32*)  thing, spec);
			break;
		case GrdPrimitiveKind::P_f64:
			format_float_primitive(formatter, *(f64*)  thing, spec);
			break;

		case GrdPrimitiveKind::P_bool: {
			GrdSmallString str = grd_to_string(*(bool*) thing);
			grd_format(formatter, grd_as_str(&str));
		}
		break;

		case GrdPrimitiveKind::P_char: {
			grd_format(formatter, "%", GrdString((char*) thing, 1));
		}
		break;
		case GrdPrimitiveKind::P_wchar: {
			char32_t c = (char32_t) (*(wchar_t*) thing);
			grd_format(formatter, "%", GrdUnicodeString(&c, 1));
		}
		break;
		case GrdPrimitiveKind::P_char16: {
			char32_t c = *(char16_t*) thing;
			grd_format(formatter, "%", GrdUnicodeString(&c, 1));
		}
		break;
		case GrdPrimitiveKind::P_char32: {
			grd_format(formatter, "%", GrdUnicodeString((char32_t*) thing, 1));
		}
		break;

		case GrdPrimitiveKind::P_void:
			// Should not happen.
			grd_format(formatter, "(void)");
			break;
		// @NewPrimitive

		default:
			grd_format(formatter, "(unknown primitive)");
			break;
	}
}

bool is_flag_set(void* dst, void* src, u64 size) {
	for (auto i: grd_range(size)) {
		u8 dst_v = *(u8*) dst;
		u8 src_v = *(u8*) src;

		if ((dst_v & src_v) != src_v) {
			return false;
		}
	}
	return true;
}

void format_enum(GrdFormatter* formatter, GrdEnumType* type, void* thing) {

	if (type->is_flags) {

		s32  matched = 0;
		for (auto it: type->values) {
			if (is_flag_set(thing, &it.value, type->size)) {
				if (matched > 0) {
					grd_format(formatter, '|');
				}
				grd_format(formatter, it.name);
				matched += 1;		
			}
		}
		if (matched != 0) {
			return;
		}
	} else {
		for (auto it: type->values) {
			if (memcmp(&it.value, thing, type->size) == 0) {
				grd_format(formatter, it.name);
				return;				
			}
		}
	}

	grd_format(formatter, "%(%)", type->name, grd_make_any(type->base_type, thing));
}

void format_pointer(GrdFormatter* formatter, GrdPointerType* type, void* thing, GrdString spec) {
	s32  indir_level;
	auto inner_type = grd_reflect_get_pointer_inner_type_with_indirection_level(type, &indir_level);

	char stars_buf[64];
	auto stars_count = grd_min_u32(grd_static_array_count(stars_buf), indir_level);
	for (auto i: grd_range(stars_count)) {
		stars_buf[i] = '*';
	}
	auto stars = grd_make_string(stars_buf, stars_count);

	bool did_deref_before = false;
	bool did_add_to_cycle_tracker = false;

	void* ptr_value = *(void**) thing;

	if (formatter->flags & FORMAT_DEREFERENCE) {
		did_deref_before = grd_contains(formatter->cyclic_pointer_tracker, ptr_value);
		if (!did_deref_before) {
			grd_add(&formatter->cyclic_pointer_tracker, ptr_value);
			did_add_to_cycle_tracker = true;
		}
	}
	grd_defer { 
		if (did_add_to_cycle_tracker) {
			grd_find_and_remove(&formatter->cyclic_pointer_tracker, ptr_value); 
		}
	};

	bool deref = (formatter->flags & FORMAT_DEREFERENCE) && !did_deref_before;

	if (spec == "*"_b) {
		deref = true;
	}

	void* ptr = thing;
	// Detect NULL pointer.
	if (deref) {
		for (auto i: grd_range(indir_level)) {
			ptr = *(void**)ptr;
			if (!ptr) {
				deref = false;
				break;
			}
		}
	}
	// Can't deref void type.
	if (deref) {
		if (inner_type == grd_reflect_type_of<void>()) {
			deref = false;
		}
	}

	if (!deref) {
		auto num = grd_bitcast<u64>(*(void**) thing);
		grd_format(formatter, "(%%) %h", grd_make_string(inner_type->name), stars, num);
		return;
	}

	if (inner_type == grd_reflect_type_of<char>()) {
		format_c_string(formatter, (char*) ptr, spec);
	} else if (inner_type == grd_reflect_type_of<wchar_t>()) {
		format_c_string(formatter, (wchar_t*) ptr, spec);
	} else if (inner_type == grd_reflect_type_of<char16_t>()) {
		format_c_string(formatter, (char16_t*) ptr, spec);
	} else if (inner_type == grd_reflect_type_of<char32_t>()) {
		format_c_string(formatter, (char32_t*) ptr, spec);
	} else {
		auto inner = grd_make_any(inner_type, ptr);

		if (formatter->is_compact()) {
			grd_format(formatter, "%", inner);
		} else {
			grd_format(formatter, "%", stars);
			grd_format(formatter, "%", inner);
		}
	}
}

void grd_format_function(GrdFormatter* formatter, GrdFunctionType* type, void* thing, GrdString spec) {
	grd_format(formatter, "% (*)(", type->return_type->name);
	for (auto i: grd_range(type->arg_types.count)) {
		grd_format(formatter, type->arg_types.data[i]->name);
		if (i != type->arg_types.count - 1) {
			grd_format(formatter, ", ");
		}
	}
	grd_format(formatter, ")");
}

void grd_format_item(GrdFormatter* formatter, GrdType* type, void* thing, GrdString spec) {
	type = grd_get_real_type(type, thing);

	if (GRD_TYPE_FORMAT_PROCS) {
		if (auto* fmt_proc = grd_get(GRD_TYPE_FORMAT_PROCS, type)) {
			(*fmt_proc)(formatter, thing, spec);
			return;
		}
	}

	formatter->depth += 1;
	grd_defer { formatter->depth -= 1; };
	auto saved_quote_string = formatter->quote_inner_string;
	formatter->quote_inner_string = false;

	auto real_type = grd_get_real_type(type, thing);

	switch (real_type->kind) {
		case GrdStructType::KIND: {
			auto casted = (GrdStructType*) real_type;
			if (strcmp(casted->subkind, "tuple") == 0) {
				format_tuple(formatter, casted, thing);
			} else {
				format_struct(formatter, casted, thing);
			}
		}
		break;

		case GrdPrimitiveType::KIND: {
			auto casted = (GrdPrimitiveType*) real_type;
			format_primitive(formatter, casted, thing, spec);
		}
		break;

		case GrdEnumType::KIND: {
			auto casted = (GrdEnumType*) real_type;
			format_enum(formatter, casted, thing);
		}
		break;

		case GrdSpanType::KIND: {
			auto casted = (GrdArrayType*) real_type;
			format_span(formatter, casted, thing, spec);
		}
		break;

		case GrdPointerType::KIND: {
			auto casted = (GrdPointerType*) real_type;
			auto inner_type = grd_reflect_get_pointer_inner_type_with_indirection_level(casted, NULL);
			if (inner_type == grd_reflect_type_of<char>() ||
				inner_type == grd_reflect_type_of<wchar_t>() ||
				inner_type == grd_reflect_type_of<char16_t>() ||
				inner_type == grd_reflect_type_of<char32_t>()) {
				formatter->quote_inner_string = saved_quote_string;
			}
			format_pointer(formatter, casted, thing, spec);
		}
		break;

		case GrdFunctionType::KIND: {
			auto casted = (GrdFunctionType*) real_type;
			grd_format_function(formatter, casted, thing, spec);
		}
		break;

		case GrdUnregisteredType::KIND: {
			grd_format(formatter, "(Unregistered type)"_b);
		}
		break;

		default: {
			grd_format(formatter, "(Unknown type)"_b);
		}
		break;
	}
}

template <GrdStringChar T>
void grd_format(GrdArray<T>* builder, GrdAllocator allocator, auto... args) {
	auto formatter = grd_make_formatter(builder, allocator);
	grd_format(&formatter, args...);
	formatter.free();
}

template <GrdStringChar T>
void grd_format(GrdArray<T>* builder, auto... args) {
	grd_format(builder, c_allocator, args...);
}

void grd_formatln(GrdFormatter* formatter, auto... args) {
	grd_format(formatter, args...);
	grd_format(formatter, "\n"_b);
}

template <GrdStringChar T>
void grd_formatln(GrdArray<T>* builder, GrdAllocator allocator, auto... args) {
	grd_format(builder, allocator, args...);
	grd_format(builder, allocator, "\n");
}

template <GrdStringChar T>
void grd_formatln(GrdArray<T>* builder, auto... args) {
	grd_formatln(builder, c_allocator, args...);
}

void grd_print_to_stdout(GrdString text) {
	fwrite(text.data, grd_len(text), 1, stdout);
}

void grd_print_to_stdout(GrdUnicodeString text) {
	auto utf8 = grd_encode_utf8(text);
	grd_print_to_stdout(utf8);
	GrdFree(utf8.data);
}

void grd_print(auto... args) {
	GrdArray<char32_t> builder;
	grd_defer { builder.free(); };
	grd_format(&builder, c_allocator, args...);
	grd_print_to_stdout(builder);
}

void grd_println(auto... args) {
	grd_print(args...);
	grd_print(U"\n"_b);
}

template <GrdStringChar T = char>
GrdArray<T> grd_sprint(GrdAllocator allocator, auto... args) {
	GrdArray<T> builder = { .allocator = allocator };
	grd_format(&builder, allocator, args...);
	return builder;
}

template <GrdStringChar T = char>
GrdArray<T> grd_sprint(auto... args) {
	return grd_sprint(c_allocator, args...);
}

GrdAllocatedUnicodeString grd_sprint_unicode(GrdAllocator allocator, auto... args) {
	return grd_sprint<char32_t>(allocator, args...);
}

GrdAllocatedUnicodeString grd_sprint_unicode(auto... args) {
	return grd_sprint<char32_t>(c_allocator, args...);
}


void grd_format_fixed_array_as_c_string(GrdFormatter* formatter, GrdAny thing) {
	if (thing.type->kind == GrdArrayType::KIND) {
		auto casted = (GrdArrayType*) thing.type;
		if (strcmp(casted->subkind, "fixed_array") == 0) {
			auto casted = (GrdFixedArrayType*) thing.type;
			if (casted->inner == grd_reflect_type_of<char>()) {
				auto ptr = (char*) thing.ptr;
				s64 length = 0;
				for (auto i: grd_range(casted->array_size)) {
					if (ptr[i] == '\0') {
						break;
					}
					length += 1;
				}
				auto str = grd_make_string(ptr, length);
				grd_format(formatter, "%", str);
				return;
			}
		}
	}

	grd_format(formatter, "%", thing);
}

template <typename T>
void grd_type_format(GrdFormatter* formatter, GrdOptional<T>* opt, GrdString spec) {
	if (opt->has_value) {
		grd_format(formatter, "GrdOptional{%}", opt->value);
	} else {
		grd_format(formatter, "GrdOptional{}");
	}
}

void grd_type_format(GrdFormatter* formatter, GrdCodeLoc* loc, GrdString spec) {
	grd_format(formatter, "%: %", grd_make_string(loc->file), loc->line);
}

void grd_type_format(GrdFormatter* formatter, GrdSmallString* str, GrdString spec) {
	grd_format(formatter, grd_as_str(str));
}

GRD_REFLECT(GrdCodeLoc) {
	GRD_MEMBER(file);
	GRD_MEMBER(line);
}
