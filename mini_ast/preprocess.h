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
	PREP_NODE_KIND_STRINGIZE,
};
REFLECT(PrepNodeKind) {
	ENUM_VALUE(PREP_NODE_KIND_NONE);
	ENUM_VALUE(PREP_NODE_KIND_FILE);
	ENUM_VALUE(PREP_NODE_KIND_MACRO);
	ENUM_VALUE(PREP_NODE_KIND_STRINGIZE);
}

struct Prep;
struct MacroExpansion;

struct PrepNode {
	Prep*            p = NULL;
	PrepNodeKind     kind = PREP_NODE_KIND_NONE;
	s64              length = 0;
	PrepFile*        file = NULL;
	s64              file_start = 0;
	Array<PrepNode*> macro_replace_regs;
	MacroExpansion*  macro_exp = NULL;
	s64              macro_exp_offset = 0;
	Array<PrepNode*> stringsize_replace_regs;
	// Array<PrepNode*> parents;

	REFLECT(PrepNode) {
		MEMBER(kind);
		MEMBER(length);
		MEMBER(file);
		MEMBER(file_start);
		MEMBER(macro_replace_regs);
		MEMBER(macro_exp);
		MEMBER(macro_exp_offset);
		MEMBER(stringsize_replace_regs);
		// MEMBER(parents);
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

enum PrepMacroRegionKind { 
	PREP_MACRO_TOKEN_REGION_REGULAR = 0,
	PREP_MACRO_TOKEN_REGION_STRINGIZE = 1,
	PREP_MACRO_TOKEN_REGION_ARG = 2,
};

struct PrepMacroRegion {
	PrepMacroRegionKind kind = PREP_MACRO_TOKEN_REGION_REGULAR;
	s64                 length = 0;
};

struct PrepMacro {
	UnicodeString           name;
	UnicodeString           body;
	Array<PrepNode*>        regs;
	Array<PrepToken>        tokens;
	Array<PrepMacroArg> args;
	Array<PrepMacroRegion>  macro_regions;
	bool                    is_object = false;
	bool                    is_variadic = false;
};

struct MacroArgLoc {
	s64 start;
	s64 end;
	
	REFLECT(MacroArgLoc) {
		MEMBER(start);
		MEMBER(end);
	}
};

struct MacroExpansion {
	AllocatedUnicodeString str;
	PrepMacro*             macro;
	Array<MacroArgLoc>        args;
	MacroExpansion*        parent_exp = NULL;
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
	MacroExpansion*        expanding_macro = NULL;
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
		if (idx >= cursor && idx <= cursor + regs[i]->length) {
			return { cursor, i };
		}
		// if (i == len(regs) - 1) {
		// 	println("ABOBA");
		// 	if (idx >= cursor && idx <= cursor + regs[i]->length) {
		// 		return { cursor, i };
		// 	}
		// }
		cursor += regs[i]->length;
	}
	return { 0, -1 };
}

Array<PrepNode*> slice(Array<PrepNode*> regs, s64 start, s64 end);
void print_prep_state(Prep* p);
void print_regs(Span<PrepNode*> regs);

PrepNode* make_prep_node(Prep* p, PrepNodeKind kind, s64 length) {
	auto node = make<PrepNode>(p->allocator);
	node->p = p;
	node->kind = kind;
	node->length = length;
	return node;
}

PrepNode* slice(PrepNode* node, s64 start, s64 end) {
	println("slice node: len, start, end, % %:%", node->length, start, end);
	if (end == start) {	
		return NULL;
	}
	if (start == 0 && end == node->length) {
		return node;
	}
	auto nn = copy(node->p->allocator, node);
	if (nn->kind == PREP_NODE_KIND_FILE) {
		nn->file_start += start;
	} else if (nn->kind == PREP_NODE_KIND_MACRO) {
		nn->macro_exp_offset += start;
	} else {
		panic("slice(): Unsupported node kind: %", nn->kind);
	}
	nn->length = end - start;
	// auto parents = slice(node->parents, start, end);
	// nn->parents = parents;
	return nn;
}

Array<PrepNode*> slice(Array<PrepNode*> regs, s64 start, s64 end) {
	if (len(regs) == 0) {
		return {};
	}
	println("slice regs start, end: %, %", start, end);
	print("   ");
	// print_regs(regs);
	auto [start_reg_cursor, start_reg_idx] = get_prep_node_idx(regs, start);
	auto [end_reg_cursor, end_reg_idx]     = get_prep_node_idx(regs, end);
	Array<PrepNode*> res;
	println("start idx, cursor: %, %", start_reg_idx, start_reg_cursor);
	println("end idx, cursor: %, %", end_reg_idx, end_reg_cursor);
	if (start_reg_idx == end_reg_idx) {
		auto node = slice(regs[start_reg_idx], start - start_reg_cursor, end - start_reg_cursor);
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
	print("sliced ");
	// print_regs(res);
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
	auto node = make_prep_node(p, PREP_NODE_KIND_FILE, len(src));
	node->file = file;
	node->file_start = 0;
	add(&p->regs, node, 0);
	return p;
}

void replace(Prep* p, Array<PrepNode*>* regs, s64 start, s64 end, PrepNode* node);

void validate_regs_integrity(Prep* p) {
	s64 sum = 0;
	for (auto it: p->regs) {
		sum += it->length;
	}
	if (sum != len(p->src)) {
		panic("Regs sum (%) does not match src len (%)", sum, len(p->src));
	}
}

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
				validate_regs_integrity(p);
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
	PREP_TOKEN_KIND_CONCAT,
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
	ENUM_VALUE(PREP_TOKEN_KIND_CONCAT);
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
	for (auto x: {',',';','/','.','-','=','(',')','?', '!', ':','+','*','-','<','>', '[', ']', '&', '#'}) {
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
	if (starts_with(src[{start, {}}], "##")) {
		return make_tok(PREP_TOKEN_KIND_CONCAT, start, start + 2);
	}
	s64 prep_tok_end = prep_maybe_prep_token(src, start);
	if (prep_tok_end > 0) {
		auto str = src[{start, prep_tok_end}];
		if (str == "#include" || str == "#pragma" || str == "#define") {
			return make_tok(PREP_TOKEN_KIND_PREP, start, prep_tok_end);
		}
		// # token
		return make_tok(PREP_TOKEN_KIND_PUNCT, start, start + 1);
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
	println("Failed start: %", src[{start, {}}]);
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
				auto tok = make_tok(PREP_TOKEN_KIND_LINE_BREAK, *cursor, *cursor + lb_len);
				*cursor = tok.end;
				return tok;
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
	*cursor = tok.end;
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
		remove_at_index(regs, start_reg_idx, end_reg_idx - start_reg_idx + 1);
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

PrepMacroArg* find_macro_arg_desc()

Error* parse_macro(Prep* p, s64* cursor, s64 define_start, s64* end) {
	println("parse macro");
	auto name_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	// @TODO: report if name is not a valid identifier.
	Array<PrepMacroArg> args;
	args.allocator = p->allocator;
	s64 body_start = *cursor;
	auto first_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	bool is_object = true;
	if (tok_str(first_tok, p->src) == "(" && first_tok.start == name_tok.end) {
		is_object = false;
		while (true) {
			auto macro_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			if (tok_str(macro_tok, p->src) == ")") {
				body_start = macro_tok.end;
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
	Array<PrepMacroRegion> macro_regions;
	macro_regions.allocator = p->allocator;
	s64 macro_region_start = body_start;
	while (true) {
		auto macro_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
		if (macro_tok.kind == PREP_TOKEN_KIND_LINE_BREAK || macro_tok.kind == PREP_TOKEN_KIND_EOF) {
			auto body = p->src[{body_start, macro_tok.start}];
			body = copy(p->allocator, body);
			auto macro = make<PrepMacro>(p->allocator);
			macro->name = copy(p->allocator, tok_str(name_tok, p->src));
			macro->body = body;
			macro->regs = slice(p->regs, define_start, macro_tok.end);
			macro->is_object = is_object;
			macro->args = args;
			add(&p->macros, macro);
			validate_regs_integrity(p);
			replace(p, &p->regs, define_start, macro_tok.end, NULL);
			remove_at_index(&p->src, define_start, macro_tok.end - define_start);
			validate_regs_integrity(p);
			*end -= macro_tok.end - define_start;
			// @TODO: verify macro tokens.
			break;
		}
		if (tok_str(macro_tok, p->src) == "#") {
			s64 str_start = macro_tok.start;
			auto name_tok = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			if (name_tok.kind != PREP_TOKEN_KIND_IDENT) {
				panic("Stringized token must be an ident");
			}
			if (name_tok.start != macro_tok.end) {
				panic("Stringized token must immediately follow '#'.");
			}
			
		}
	}
	return NULL;
}

s64 process_stringize_ops(Prep* p, s64 start, s64 end, MacroExpansion* exp, Span<MacroArgLoc> args) {
	s64 cursor = start;
	s64 len_diff = 0;
	while (cursor < end) {
		auto tok = prep_next_token(p, &cursor);
		if (tok_str(tok, p->src) == "#") {
			auto name_tok = prep_next_token(p, &cursor);
			if (name_tok.kind != PREP_TOKEN_KIND_IDENT) {
				panic("Stringized token must be an ident");
			}
			if (name_tok.start != tok.end) {
				panic("Stringized token must immediately proceed '#'.");
			}
			auto str = tok_str(name_tok, p->src);
			bool found = false;
			for (auto i: range(len(exp->macro->args))) {
				if (exp->macro->args[i].name == str) {
					auto arg = args[i];
					auto arg_str = p->src[{arg.start, arg.end}];
					// @TODO: do escaping.
					AllocatedUnicodeString str;
					str.allocator = p->allocator;
					add(&str, '"');
					add(&str, arg_str);
					add(&str, '"');
					auto node = make_prep_node(p, PREP_NODE_KIND_STRINGIZE, len(str));
					node->stringsize_replace_regs = slice(p->regs, tok.start, name_tok.end);
					remove_at_index(&p->src, tok.start, name_tok.end - tok.start);
					add(&p->src, str, tok.start);
					replace(p, &p->regs, tok.start, name_tok.end, node);
					validate_regs_integrity(p);
					s64 diff = len(str) - (name_tok.end - tok.start);
					len_diff += diff;
					cursor = tok.start + len(str);
					end += diff;
					found = true;
					break;
				}
			}
			if (!found) {
				panic("Unknown macro arg '%'", str);
			}
		}
	}
	return len_diff;
}

Error* expand_macro(Prep* p, PrepMacro* macro, Array<MacroArgLoc> args, s64 *cursor, s64* end, s64 rep_start, s64 rep_end) {
	println("macro: %, body: %", macro->name, macro->body);
	println("macro args: %", args);
	print_regs(p->regs);

	auto exp = make<MacroExpansion>(p->allocator);
	exp->macro = macro;
	exp->args = args;
	exp->str.allocator = p->allocator;
	exp->str.capacity = len(macro->body) + 2;
	exp->parent_exp = p->expanding_macro;
	add(&exp->str, ' ');
	add(&exp->str, macro->body);
	add(&exp->str, ' ');

	p->expanding_macro = exp;
	defer { p->expanding_macro = p->expanding_macro->parent_exp; };

	auto replace_slice = slice(p->regs, rep_start, rep_end);
	auto macro_node = make_prep_node(p, PREP_NODE_KIND_MACRO, len(exp->str));
	macro_node->macro_replace_regs = replace_slice;
	macro_node->macro_exp = exp;
	macro_node->macro_exp_offset = 0;
	replace(p, &p->regs, rep_end, rep_end, macro_node);
	add(&p->src, exp->str, rep_end);
	validate_regs_integrity(p); 
	
	println("src after insertion: %", p->src);
	print_regs(p->regs);
	
	s64 stringize_diff = process_stringize_ops(p, rep_end, rep_end + len(exp->str), exp, args);

	replace(p, &p->regs, rep_start, rep_end, NULL);
	remove_at_index(&p->src, rep_start, rep_end - rep_start);
	validate_regs_integrity(p);

	println("src after removal: %", p->src);
	print_regs(p->regs);

	s64 macro_diff = len(exp->str) - (rep_end - rep_start);

	*end += macro_diff + stringize_diff;
	*cursor += macro_diff + stringize_diff;
	return NULL;
}

Error* preprocess(Prep* p, s64 start, s64 end) {
	s64 cursor = start;
	while (cursor < end) {
		s64 before_main_tok = cursor;
		auto tok = prep_next_token(p, &cursor);
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
					println("matched macro");
					Array<MacroArgLoc> args;
					args.allocator = p->allocator;
					s64 arg_count = get_prep_macro_arg_count(it);
					s64 macro_end = tok.end;
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
								macro_end = arg_tok.end;
								// If there's something in between arg_start and ')'.start
								//   we have to treat it as an argument.
								s64 new_cursor = arg_start;
								auto first_tok_after_arg_start = prep_next_token(p, &new_cursor);
								if (first_tok_after_arg_start.start < arg_tok.start) {
									add(&args, MacroArgLoc{arg_start, arg_tok.start});
								}
								break;
							}
							if (t_str == "(") {
								paren_level += 1;
							}
							if (t_str == ",") {
								if (paren_level == 0) {
									add(&args, MacroArgLoc{arg_start, arg_tok.start});
									arg_start = arg_tok.end;
								}
							}
						}
						if (len(args) != arg_count) {
							panic("Expected at least % macro args, got %", arg_count, len(args));
						}
					}
					auto e = expand_macro(p, it, args, &cursor, &end, tok.start, macro_end);
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

void print_prep_node(PrepNode* node, s64 ident, s64 node_start) {
	auto print_ident = [&]() {
		for (auto i: range(ident)) {
			print("  ");
		}
	};

	auto kind_str = sprint("%", node->kind);
	defer { kind_str.free(); };
	auto wo_prefix = remove_prefix(kind_str, U"PREP_NODE_KIND_"_b);
	print_ident();
	println("{");
	print_ident();
	println("   %", wo_prefix);
	print_ident();
	if (node->kind == PREP_NODE_KIND_FILE) {
		println("   '%'", node->file->src[{node->file_start, node->file_start + node->length}]);
	} else if (node->kind == PREP_NODE_KIND_MACRO) {
		println("   '%'", node->macro_exp->str[{node->macro_exp_offset, node->macro_exp_offset + node->length}]);
	} else if (node->kind == PREP_NODE_KIND_STRINGIZE) {
		print_ident();
		println("   '%'", node->p->src[{node_start, node_start + node->length}]);
		ident += 1;
		print_ident();
		println(" Stringized regs {");
		for (auto it: node->stringsize_replace_regs) {
			print_prep_node(it, ident, -1);
		}
		print_ident();
		println("}");
		ident -= 1;
	} else {
		print_ident();
		println("Unknown node kind: %", node->kind);
	}
	print_ident();
	println("}");
	print("}");
}

void print_regs(Span<PrepNode*> regs) {
	s64 cursor = 0;
	for (auto it: regs) {
		print_prep_node(it, 0, cursor);
		cursor += it->length;
	}
	println();
}

void print_prep_state(Prep* p) {
	println("0: %", len(p->src));
	print_regs(p->regs);
}
