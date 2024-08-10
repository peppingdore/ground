#pragma once

#include "../string.h"
#include "../defer.h"
#include "../format.h"
#include "../error.h"
#include "../panic.h"

struct PrepFile {
	UnicodeString src;
	UnicodeString path;
};

enum PrepNodeKind {
	PREP_NODE_KIND_NONE,
	PREP_NODE_KIND_FILE,
	PREP_NODE_KIND_MACRO,
};
REFLECT(PrepNodeKind) {
	ENUM_VALUE(PREP_NODE_KIND_NONE);
	ENUM_VALUE(PREP_NODE_KIND_FILE);
	ENUM_VALUE(PREP_NODE_KIND_MACRO);
}

struct Prep;

struct PrepNode {
	Prep*            p = NULL;
	PrepNodeKind     kind = PREP_NODE_KIND_NONE;
	s64              length = 0;
	PrepFile*        file = NULL;
	s64              file_start = 0;
	Array<PrepNode*> parents;

	REFLECT(PrepNode) {
		MEMBER(kind);
		MEMBER(length);
		MEMBER(file);
		MEMBER(file_start);
		MEMBER(parents);
	}
};

struct PrepMacroArg {
	UnicodeString    name;
	Array<PrepNode*> loc;

	REFLECT(PrepMacroArg) {
		MEMBER(name);
		MEMBER(loc);
	}
};

struct PrepMacro {
	UnicodeString       name;
	UnicodeString       body;
	Array<PrepNode*>    regs;
	Array<PrepMacroArg> args;
	bool                is_object = false;
	bool                is_variadic = false;
};

constexpr s64 PREP_MACRO_VARIADIC_ARG_COUNT = s64_max;

s64 get_prep_macro_arg_count(PrepMacro* macro) {
	if (macro->is_object) {
		return -1;
	}
	return macro->is_variadic ? PREP_MACRO_VARIADIC_ARG_COUNT : len(macro->args);
}

struct Prep {
	Allocator              allocator;
	AllocatedUnicodeString src;
	Array<PrepNode*>       regs;
	Array<PrepMacro*>      macros;
	Array<PrepMacro*>      macro_stack;
};

Tuple<s64, s64> get_reg_bounds(Span<PrepNode*> regs, s64 reg_idx) {
	s64 cursor = 0;
	for (auto i: range(reg_idx)) {
		cursor += regs[i]->length;
	}
	return {cursor, cursor + regs[reg_idx]->length};
}

Tuple<s64, s64> get_prep_node_idx(Span<PrepNode*> regs, s64 idx) {
	s64 cursor = 0;
	for (auto i: range(len(regs))) {
		if (idx >= cursor && idx < cursor + regs[i]->length) {
			return { cursor, i };
		}
		if (i == len(regs) - 1) {
			if (idx >= cursor && idx <= cursor + regs[i]->length) {
				return { cursor, i };
			}
		}
		cursor += regs[i]->length;
	}
	return { 0, -1 };
}

Array<PrepNode*> slice(Array<PrepNode*> regs, s64 start, s64 end);

PrepNode* slice(PrepNode* node, s64 start, s64 end) {
	if (end == start) {
		return NULL;
	}
	if (start == 0 && end == node->length) {
		return node;
	}
	auto nn = copy(node->p->allocator, node);
	if (nn->kind == PREP_NODE_KIND_FILE) {
		nn->file_start += start;
	} else {
		panic("slice(): Unsupported node kind");
	}
	nn->length = end - start;
	auto parents = slice(node->parents, start, end);
	nn->parents = parents;
	return nn;
}

Array<PrepNode*> slice(Array<PrepNode*> regs, s64 start, s64 end) {
	if (len(regs) == 0) {
		return {};
	}
	auto [start_reg_cursor, start_reg_idx] = get_prep_node_idx(regs, start);
	auto [end_reg_cursor, end_reg_idx]     = get_prep_node_idx(regs, end);
	Array<PrepNode*> res;
	if (start_reg_idx == end_reg_idx) {
		auto node = slice(regs[start_reg_idx], start - start_reg_cursor, end_reg_cursor - start_reg_cursor);
		if (node) {
			add(&res, node);
		}
	} else {
		auto start_node = slice(regs[start_reg_idx], start - start_reg_cursor, regs[start_reg_idx]->length);
		if (start_node) {
			add(&res, start_node);
		}
		for (auto i: range_from_to(start_reg_idx + 1, end_reg_idx)) {
			add(&res, regs[i]);
		}
		auto end_node = slice(regs[end_reg_idx], 0, end - end_reg_cursor);
		if (end_node) {
			add(&res, end_node);
		}
	}
	return res;
}

Prep* make_prep(Allocator allocator, UnicodeString src, UnicodeString path) {
	auto p = make<Prep>(allocator);
	p->allocator = allocator;
	p->regs.allocator = allocator;
	p->macros.allocator = allocator;
	p->src = copy(allocator, src);
	auto file = make<PrepFile>(allocator);
	file->src = src;
	file->path = path;
	auto node = make<PrepNode>(allocator);
	node->p = p;
	node->kind = PREP_NODE_KIND_FILE;
	node->file = file;
	node->length = len(src);
	add(&p->regs, node, 0);
	return p;
}

void replace(Prep* p, Array<PrepNode*>* regs, s64 start, s64 end, PrepNode* node);
void print_prep_state(Prep* p);

void splice_lines(Prep* p, s64 start, s64 end) {
	s64 idx = 0;
	s64 cursor = start;
	while (cursor < end) {
		if (p->src[cursor] == '\\') {
			auto lb_len = get_line_break_len(p->src, cursor + 1);
			// @TODO: do not forget to adpd line break at the end of the files, if it is missing.
			//   because if last line ends with backslash,
			//   it's going to concat with the first line of the next file.
			if (lb_len > 0 || (cursor == end - 1)) {
				if (lb_len == 0) {
					int k = 43;
				}
				replace(p, &p->regs, cursor, cursor + lb_len + 1, NULL);
				remove_at_index(&p->src, cursor, lb_len + 1);
				end -= lb_len + 1;
				cursor += lb_len + 1;
				println("lb_len: %", lb_len);
				print_prep_state(p);
				continue;
			}
		}
		cursor += 1;
	}
}

enum PrepTokenKind {
	PREP_TOKEN_KIND_NONE,
	PREP_TOKEN_KIND_NUMBER,
	PREP_TOKEN_KIND_STRING,
	PREP_TOKEN_KIND_PUNCT,
	PREP_TOKEN_KIND_LINE_BREAK,
	PREP_TOKEN_KIND_PREP,
	PREP_TOKEN_KIND_IDENT,
	PREP_TOKEN_KIND_EOF,
};
REFLECT(PrepTokenKind) {
	ENUM_VALUE(PREP_TOKEN_KIND_NONE);
	ENUM_VALUE(PREP_TOKEN_KIND_NUMBER);
	ENUM_VALUE(PREP_TOKEN_KIND_STRING);
	ENUM_VALUE(PREP_TOKEN_KIND_PUNCT);
	ENUM_VALUE(PREP_TOKEN_KIND_LINE_BREAK);
	ENUM_VALUE(PREP_TOKEN_KIND_PREP);
	ENUM_VALUE(PREP_TOKEN_KIND_IDENT);
	ENUM_VALUE(PREP_TOKEN_KIND_EOF);
}

struct PrepToken {
	PrepTokenKind kind = PREP_TOKEN_KIND_NONE;
	s64           start = 0;
	s64           end = 0;
};

UnicodeString tok_str(PrepToken tok, UnicodeString src) {
	return src[{tok.start, tok.end}];
}

PrepToken make_tok(PrepTokenKind kind, s64 start, s64 end) {
	PrepToken tok;
	tok.kind = kind;
	tok.start = start;
	tok.end = end;
	return tok;
}

s64 prep_maybe_number_tok(UnicodeString src, s64 start) {
	bool got_dec_digit = false;
	for (s64 i = start; i < len(src); i++) {
		if (i == start && (src[i] == '.')) {
			continue;
		}
		if (!got_dec_digit) {
			if (src[i] >= '0' && src[i] <= '9') {
				got_dec_digit = true;
				continue;
			}
			return 0;
		}
		UnicodeString rem = src[{i, {}}];
		if (
			starts_with(rem, "e+") || starts_with(rem, "e-") ||
			starts_with(rem, "E+") || starts_with(rem, "E-") ||
			starts_with(rem, "p+") || starts_with(rem, "p-") ||
			starts_with(rem, "P+") || starts_with(rem, "P-")
		) {
			i += 1;
			continue;
		}
		if (src[i] >= '0' && src[i] <= '9') {
			continue;
		}
		if (src[i] >= 'a' && src[i] <= 'z') {
			continue;
		}
		if (src[i] >= 'A' && src[i] <= 'Z') {
			continue;
		}
		if (src[i] == '_') {
			continue;
		}
		if (src[i] == '.') {
			continue;
		}
		return i;
	}
	return len(src);
}

s64 prep_maybe_string_tok(UnicodeString src, s64 start) {
	for (s64 i = start; i < len(src); i++) {
		if (i == start && src[i] != '"') {
			return 0;
		}
		if (src[i] == '"') {
			return i + 1;
		}
		if (starts_with(src[{i, {}}], R"xx(\")xx")) {
			i += 1;
			continue;
		}
	}
	return 0;
}

s64 prep_maybe_char_tok(UnicodeString src, s64 start) {
	for (s64 i = start; i < len(src); i++) {
		if (i == start && src[i] != '\'') {
			return 0;
		}
		if (src[i] == '\'') {
			return i + 1;
		}
		if (starts_with(src[{i, {}}], R"xx(\')xx")) {
			i += 1;
			continue;
		}
	}
	return 0;
}

s64 prep_is_valid_ident_symbol(char32_t c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c >= '0' && c <= '9' || c == '$';
}

s64 prep_is_valid_first_ident_symbol(char32_t c) {
	return prep_is_valid_ident_symbol(c) && !(c >= '0' && c <= '9');
}

s64 prep_maybe_prep_token(UnicodeString src, s64 start) {
	if (start < len(src) && src[start] == '#') {
		// concat token.
		if (start + 1 < len(src) && src[start + 1] == '#') {
			if (start + 2 < len(src) && src[start + 2] == '#') {
				return 0;
			}
			return start + 2;
		}
		for (s64 i = start + 1; i < len(src); i++) {
			if (is_whitespace(src[i])) {
				if (i == start + 1) {
					return 0;
				}
				return i;
			}
			if (!prep_is_valid_ident_symbol(src[i])) {
				return 0;
			}
		}
	}
	return 0;
}

bool prep_is_punct(char32_t c) {
	for (auto x: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>', '[', ']', '&', '#'}) {
		if (c == x) {
			return true;
		}
	}
	return false;
}

s64 prep_maybe_ident(UnicodeString src, s64 start) {
	for (s64 i = start; i < len(src); i++) {
		if (i == start) {
			if (!prep_is_valid_first_ident_symbol(src[i])) {
				return 0;
			}
		} else {
			if (is_whitespace(src[i]) || prep_is_punct(src[i])) {
				return i;
			}
			if (!prep_is_valid_ident_symbol(src[i])) {
				return 0;
			}
		}
	}
	return start < len(src) ? len(src) : 0;
}

PrepToken prep_get_token_at(UnicodeString src, s64 start) {
	s64 number_end = prep_maybe_number_tok(src, start);
	if (number_end > 0) {
		return make_tok(PREP_TOKEN_KIND_NUMBER, start, number_end);
	}
	s64 string_end = prep_maybe_string_tok(src, start);
	if (string_end > 0) {
		return make_tok(PREP_TOKEN_KIND_STRING, start, string_end);
	}
	s64 char_end = prep_maybe_char_tok(src, start);
	if (char_end > 0) {
		return make_tok(PREP_TOKEN_KIND_STRING, start, char_end);
	}
	s64 prep_tok_end = prep_maybe_prep_token(src, start);
	if (prep_tok_end > 0) {
		return make_tok(PREP_TOKEN_KIND_PREP, start, prep_tok_end);
	}
	s64 ident_end = prep_maybe_ident(src, start);
	if (ident_end > 0) {
		return make_tok(PREP_TOKEN_KIND_IDENT, start, ident_end);
	}
	if (prep_is_punct(src[start])) {
		return make_tok(PREP_TOKEN_KIND_PUNCT, start, start + 1); 
	}
	if (start == len(src)) {
		return make_tok(PREP_TOKEN_KIND_EOF, len(src), len(src));
	}
	panic("Invalid token at %", start);
	return {};
}

enum PrepNextTokenFlags {
	PREP_NEXT_TOKEN_FLAGS_NONE = 0,
	PREP_NEXT_TOKEN_TAKE_LINE_BREAK = 1 << 0,
};

PrepToken prep_next_token(UnicodeString src, s64* cursor, PrepNextTokenFlags flags = PREP_NEXT_TOKEN_FLAGS_NONE) {
	println("prep_next_token: %, %", *cursor, len(src));
	println("src: %", src);
	while (*cursor < len(src)) {
		auto c = src[*cursor];
		s64 lb_len = get_line_break_len(src, *cursor);
		if (lb_len > 0) {
			if (flags & PREP_NEXT_TOKEN_TAKE_LINE_BREAK) {
				return make_tok(PREP_TOKEN_KIND_LINE_BREAK, *cursor, *cursor + lb_len);
			} else {
				println("skip line break at %, %", *cursor, lb_len);
				*cursor += lb_len;
				continue;
			}
		}
		if (is_whitespace(c)) {
			println("skip whitespace at %", *cursor);
			*cursor += 1;
			continue;
		} else {
			break;
		}
	}
	auto tok = prep_get_token_at(src, *cursor);
	println("tok: %, %, (%: %)", tok.kind, tok_str(tok, src), tok.start, tok.end);
	*cursor += tok.end - tok.start;
	return tok;
}

PrepToken prep_next_token(Prep* p, s64* cursor, PrepNextTokenFlags flags = PREP_NEXT_TOKEN_FLAGS_NONE) {
	return prep_next_token(p->src, cursor, flags);
}

void replace(Prep* p, Array<PrepNode*>* regs, s64 start, s64 end, PrepNode* node) {
	if (len(*regs) == 0) {
		return;
	}
	auto [start_reg_cursor, start_reg_idx] = get_prep_node_idx(*regs, start);
	auto [end_reg_cursor, end_reg_idx]     = get_prep_node_idx(*regs, end);
	if (start_reg_idx == end_reg_idx) {
		auto reg = (*regs)[start_reg_idx];
		auto first = slice(reg, 0, start - start_reg_cursor);
		auto second = slice(reg, end - start_reg_cursor, reg->length);
		remove_at_index(regs, start_reg_idx);
		s64 add_idx = start_reg_idx;
		if (first) {
			add(regs, first, add_idx++);
		}
		if (node) {
			add(regs, node, add_idx++);
		}
		if (second) {
			add(regs, second, add_idx++);
		}
	} else {
		auto first = slice((*regs)[start_reg_idx], 0, start - start_reg_cursor);
		auto second = slice((*regs)[end_reg_idx], end - end_reg_cursor, (*regs)[end_reg_idx]->length);
		for (s64 i = start_reg_idx; i <= end_reg_idx; i++) {
			remove_at_index(regs, i);
		}
		s64 add_idx = start_reg_idx;
		if (first) {
			add(regs, first, add_idx++);
		}
		if (node) {
			add(regs, node, add_idx++);
		}
		if (second) {
			add(regs, second, add_idx++);
		}
	}
}

Error* parse_macro(Prep* p, s64* cursor, s64 define_start, s64* end) {
	auto name_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	// @TODO: report if name is not a valid identifier.
	Array<PrepMacroArg> args;
	s64 after_name = *cursor;
	auto first_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	if (tok_str(first_tok, p->src) == "(" && first_tok.start == name_tok.end) {
		while (true) {
			auto macro_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			if (tok_str(macro_tok, p->src) == ")") {
				break;	
			}
			if (len(args) > 0) {
				if (tok_str(macro_tok, p->src) != ",") {
					panic("Expected ',' after macro argument");
				}
				macro_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			}
			if (macro_tok.kind != PREP_TOKEN_KIND_IDENT) {
				panic("Expected ident tok for macro argument");
			}
			auto name = copy(p->allocator, tok_str(macro_tok, p->src));
			auto loc = slice(p->regs, macro_tok.start, macro_tok.end);
			add(&args, PrepMacroArg{name, loc});
		}
	}
	println("args: %", args);
	while (true) {
		auto macro_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
		if (macro_tok.kind == PREP_TOKEN_KIND_LINE_BREAK || macro_tok.kind == PREP_TOKEN_KIND_EOF) {
			auto body = p->src[{after_name, macro_tok.start}];
			body = copy(p->allocator, body);
			auto macro = make<PrepMacro>(p->allocator);
			macro->name = copy(p->allocator, tok_str(name_tok, p->src));
			macro->body = body;
			macro->regs = slice(p->regs, define_start, macro_tok.end);
			add(&p->macros, macro);
			replace(p, &p->regs, define_start, macro_tok.end, NULL);
			remove_at_index(&p->src, define_start, macro_tok.end - define_start);
			*end -= macro_tok.end - define_start;
			// @TODO: verify macro tokens.
			break;
		}
	}
	return NULL;
}

struct MacroArg {
	s64 start;
	s64 end;
};

s64 process_stringize_ops(Prep* p, s64 start, s64 end, PrepMacro* m, Span<MacroArg> args) {
	s64 cursor = start;
	s64 len_diff = 0;
	while (cursor < end) {
		auto tok = prep_next_token(p, &cursor);
		if (tok_str(tok, p->src) == "#") {
			auto name_tok = prep_get_token_at(p->src, cursor);
			auto str = tok_str(name_tok, p->src);
			for (auto i: range(len(m->args))) {
				if (m->args[i].name == str) {
					
					replace(p, &p->regs, tok.start, name_tok.end);

				}
			}
		}
	}
}

Error* expand_macro(Prep* p, PrepMacro* macro, Span<MacroArg> args, s64 *cursor, s64* end, PrepToken replace_tok) {
	add(&p->macro_stack, macro);
	defer_x(pop(&p->macro_stack));

	auto replace_slice = slice(p->regs, replace_tok.start, replace_tok.end);
	auto macro_node = make<PrepNode>(p->allocator);
	macro_node->p = p;
	macro_node->kind = PREP_NODE_KIND_MACRO;
	macro_node->length = len(macro->body) + 2;
	macro_node->parents = replace_slice;

	replace(p, &p->regs, replace_tok.start, replace_tok.end, macro_node);
	remove_at_index(&p->src, replace_tok.start, replace_tok.end - replace_tok.start);
	
	auto dst = reserve(&p->src, len(macro->body) + 2);
	add(&p->src, macro->body, replace_tok.start);
	dst[0] = ' ';
	memcpy(dst + 1, macro->body.data, len(macro->body) * sizeof(macro->body[0]));
	dst[len(macro->body) * sizeof(macro->body[0]) + 1] = ' ';

	process_stringize_ops(p, replace_tok.start, replace_tok.start + len(macro->body) + 2, macro, args);
	
	*end -= replace_tok.end - replace_tok.start;
	*end += len(macro->body) + 2;
}

Error* preprocess(Prep* p, s64 start, s64 end) {
	s64 cursor = start;
	while (cursor < end) {
		s64 before_main_tok = cursor;
		auto tok = prep_next_token(p, &cursor);
		println("tok: %, %, (%: %)", tok.kind, tok_str(tok, p->src), tok.start, tok.end);
		if (tok_str(tok, p->src) == "#define") {
			auto e = parse_macro(p, &cursor, tok.start, &end);
			if (e) {
				return e;
			}
			cursor = before_main_tok;
			continue;
		}
		if (tok.kind == PREP_TOKEN_KIND_IDENT) {
			for (auto it: p->macros) {
				if (it->name == tok_str(tok, p->src)) {
					Array<MacroArg> args;
					defer_x(args.free());
					s64 arg_count = get_prep_macro_arg_count(it);
					if (arg_count != -1) {
						auto paren = prep_get_token_at(p->src, tok.end);
						if (tok_str(paren, p->src) != "(") {
							panic("Expected '(' after macro name");
						}
						cursor = paren.end;
						auto arg_start = cursor;
						while (true) {
							s64 paren_level = 0;
							auto arg_tok = prep_next_token(p, &cursor);
							if (arg_tok.kind == PREP_TOKEN_KIND_EOF) {
								panic("Unexpected EOF");
							}
							auto t_str = tok_str(arg_tok, p->src);
							if (t_str == ")") {
								if (paren_level > 0) {
									paren_level -= 1;
									continue;
								}
								break;
							}
							if (t_str == "(") {
								paren_level += 1;
							}
							if (t_str == ",") {
								if (paren_level == 0) {
									add(&args, MacroArg{arg_start, arg_tok.start});
								}
							}
						}
					}
					auto e = expand_macro(p, it, &cursor, &end, tok);
					if (e) {
						return e;
					}
					// @TODO: move cursor to the end of the macro.
					continue;
				}
			}
		}
	}
	return NULL;
}

void print_prep_node(PrepNode* node, s64 offset) {
	print("{%:%", offset, node->length + offset);
	if (len(node->parents) > 0) {
		print(":(");
		s64 cursor = offset;
		for (auto it: node->parents) {
			print_prep_node(it, cursor);
			cursor += it->length;
		}
		print(")");
	}
	print("}");
}

void print_prep_state(Prep* p) {
	println("0: %", len(p->src));
	s64 cursor = 0;
	for (auto it: p->regs) {
		print_prep_node(it, cursor);
		cursor += it->length;
	}
	println();
}
