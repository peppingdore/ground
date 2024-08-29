#pragma once

#include "../string.h"
#include "../defer.h"
#include "../format.h"
#include "../error.h"
#include "../panic.h"
#include "../one_dim_intersect.h"

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

struct GndAsxPrepRegion {
	s64 start = 0;
	s64 end   = 0;

	REFLECT(GndAsxPrepRegion) {
		MEMBER(start);
		MEMBER(end);
	}
};

struct PrepToken {
	PrepTokenKind    kind = PREP_TOKEN_KIND_NONE;
	GndAsxPrepRegion reg;
};

UnicodeString tok_str(PrepToken tok, UnicodeString src) {
	return src[{tok.reg.start, tok.reg.end}];
}

PrepToken make_tok(PrepTokenKind kind, s64 start, s64 end) {
	PrepToken tok;
	tok.kind = kind;
	tok.reg = { start, end };
	return tok;
}

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
	UnicodeString    str;
	PrepFile*        file = NULL;
	s64              file_start = 0;
	Array<PrepNode*> macro_replace_regs;
	MacroExpansion*  macro_exp = NULL;
	s64              macro_exp_offset = 0;
	Array<PrepNode*> stringsize_replace_regs;
	// Array<PrepNode*> parents;

	REFLECT(PrepNode) {
		MEMBER(p);
		MEMBER(kind);
		MEMBER(str);
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
	s64                 prev_offset = 0;
	s64                 length = 0;
	PrepMacroArg*       stringize_arg = NULL;
};

struct PrepMacro {
	UnicodeString           name;
	UnicodeString           body;
	Array<PrepNode*>        regs;
	Array<PrepToken>        tokens;
	Array<PrepMacroArg>     args;
	Array<PrepMacroRegion>  macro_regions;
	bool                    is_object = false;
	bool                    is_variadic = false;
};

struct MacroArgLoc {
	PrepMacroArg* arg; // NULL if variadic argument.
	s64           start;
	s64           end;
	
	REFLECT(MacroArgLoc) {
		MEMBER(arg);
		MEMBER(start);
		MEMBER(end);
	}
};

MacroArgLoc make_macro_arg_loc(PrepMacroArg* arg, s64 start, s64 end) {
	MacroArgLoc loc;
	loc.arg = arg;
	loc.start = start;
	loc.end = end;
	return loc;
}

struct MacroExpansion {
	AllocatedUnicodeString str;
	PrepMacro*             macro;
	Array<MacroArgLoc>     args;
	Array<PrepMacroRegion> exp_regions;
	MacroExpansion*        parent_exp = NULL;
};

struct Prep {
	Allocator              allocator;
	AllocatedUnicodeString src;
	Array<PrepNode*>       regs;
	Array<PrepMacro*>      macros;
	MacroExpansion*        expanding_macro = NULL;
};

enum GndAsxErrorTokenColor: u64 {
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_BLACK = 1,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_RED = 2,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_GREEN = 3,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_YELLOW = 4,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_BLUE = 5,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_MAGENTA = 6,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_CYAN = 7,
	GND_ASX_ERROR_TOKEN_COLOR_REGULAR_WHITE = 8,
};

struct GndAsxErrorHighlight {
	s64 start;
	s64 length;
	u64 color = 0;
};

struct GndAsxErrorPiece {
	AllocatedUnicodeString      message;
	Array<GndAsxErrorHighlight> highlight_regions;

	void free() {
		message.free();
		highlight_regions.free();
	}
};

struct GndAsxError: Error {
	Prep*                   p = NULL;
	Array<GndAsxErrorPiece> pieces;

	REFLECT(GndAsxError) {
		BASE_TYPE(Error);
		MEMBER(p);
		MEMBER(pieces);
	}
};

GndAsxError* make_asx_error(Prep* p, CodeLocation loc, auto... args) {
	auto str = sprint_unicode(p->allocator, args...);
	defer { Free(p->allocator, str.data); };
	auto error = format_error<GndAsxError>(loc, str);
	error->on_free = [](Error* uncasted) {
		auto e = (GndAsxError*) uncasted;
		for (auto it: e->pieces) {
			it.free();
		}
		e->pieces.free();
	};
	return error;
}

struct GndAsxErrorToken {
	Optional<GndAsxPrepRegion> reg;
	u64                        color = 0;
};

bool sort_parser_error_toks(Span<GndAsxErrorToken> toks, s64 a, s64 b) {
	assert(toks[a].reg.has_value);
	assert(toks[b].reg.has_value);
	if (toks[a].reg.value.start < toks[b].reg.value.start) {
		return true;
	}
	if (toks[a].reg.value.start == toks[b].reg.value.start) {
		// It's actually '>', to make larger token go first in the sorted array,
		//   which lowers it's priority for highlighting.
		return toks[a].reg.value.end > toks[b].reg.value.end;  
	}
	return false;
}

void add_site(GndAsxError* error, auto... tok_arglist) {
	GndAsxErrorToken toks_list[] = { tok_arglist... }; 

	Array<GndAsxErrorToken> toks = { .allocator = error->p->allocator };
	defer { toks.free(); };
	for (auto it: toks_list) {
		if (it.reg.has_value) {
			add(&toks, it);
		}
	}

	// Stupid hacks to get rid of empty regions.
	for (auto& it: toks) {
		if (it.reg.value.end == it.reg.value.start) {
			if (it.reg.value.end < len(error->p->src)) {
				it.reg.value.end += 1;
			} else if (it.reg.value.start > 0) {
				it.reg.value.start -= 1;
			}
		}
	}

	if (len(toks) == 0) {
		return;
	}

	sort(toks, sort_parser_error_toks);

	s64 toks_start = toks[0].reg.value.start;
	s64 toks_end = toks[-1].reg.value.end;

	assert(toks_start >= 0);
	assert(toks_end <= len(error->p->src));

	constexpr s64 RESIDUAL_LINES_COUNT = 3;

	s64 bottom_lines_count = 0;
	s64 top_lines_count = 0;
	s64 region_start = 0;
	s64 region_end = len(error->p->src);
	for (auto i: reverse(range(toks_start))) {
		if (is_line_break(error->p->src[i])) {
			top_lines_count += 1;
			if (top_lines_count >= 3) {
				region_start = i;
				break;
			}
		}
	}
	for (auto i: range_from_to(toks_end, len(error->p->src))) {
		if (is_line_break(error->p->src[i])) {
			bottom_lines_count += 1;
			if (bottom_lines_count >= RESIDUAL_LINES_COUNT) {
				region_end = i + 1;
				break;
			}
		}
	}

	Array<GndAsxErrorHighlight> result_highlights;
	add(&result_highlights, GndAsxErrorHighlight{
		.start = region_start,
		.length = region_end - region_start,
	});

	for (auto tok: toks) {
		auto get_region = [&](s64 i) -> OneDimRegion {
			return OneDimRegion{
				.start = result_highlights[i].start,
				.length = result_highlights[i].length,
			};
		};

		auto resize = [&](s64 i, s64 start, s64 end) {
			result_highlights[i].start = start;
			result_highlights[i].length = end - start;
		};

		auto insert = [&](s64 i, s64 start, s64 end, s64 index) {
			auto highlight = GndAsxErrorHighlight{
				.start = start,
				.length = end - start,
				.color = index == -1 ? tok.color : result_highlights[index].color,
			};

			add(&result_highlights, highlight, i);
		};

		auto remove = [&](s64 i) {
			remove_at_index(&result_highlights, i);
		};

		one_dim_patch(len(result_highlights), get_region, resize, insert, remove, tok.reg.value.start, tok.reg.value.end - tok.reg.value.start);
	}

	GndAsxErrorPiece piece = {
		.highlight_regions = result_highlights,
	};
	add(&error->pieces, piece);
}

void add_text(GndAsxError* error, auto... args) {
	GndAsxErrorPiece piece;
	piece.message = sprint_unicode(args...);
	add(&error->pieces, piece);
}

void print_ast_error(Error* e) {
	if (auto x = reflect_cast<GndAsxError>(e)) {
		print(x->text);
		for (auto it: x->pieces) {
			if (len(it.message) > 0) {
				println(it.message);
			}
			for (auto reg: it.highlight_regions) {
				auto text = x->p->src[reg.start, reg.start + reg.length];
				u64 regular_color = reg.color & 0xf;
				if (regular_color) {
					print("\x1b[0;%m", 30 + regular_color - 1);
				}
				print(text);
				print("\x1b[0m");
			}
		}
	} else {
		println(e->text);
	}
	println("Generated at %", e->loc);
}

GndAsxError* simple_parser_error(Prep* p, CodeLocation loc, Optional<GndAsxPrepRegion> reg, auto... args) {
	auto error = make_asx_error(p, loc, args...);
	if (reg.has_value && reg.value.end > reg.value.start) {
		add_site(error, GndAsxErrorToken {
			.reg = reg,
			.color = GND_ASX_ERROR_TOKEN_COLOR_REGULAR_RED,
		}); 
	}
	return error;
}

Tuple<s64, s64> get_reg_bounds(Span<PrepNode*> regs, s64 reg_idx) {
	s64 cursor = 0;
	for (auto i: range(reg_idx)) {
		cursor += len(regs[i]->str);
	}
	return {cursor, cursor + len(regs[reg_idx]->str)};
}

Tuple<s64, s64> get_prep_node_idx(Span<PrepNode*> regs, s64 idx) {
	s64 cursor = 0;
	for (auto i: range(len(regs))) {
		if (idx >= cursor && idx <= cursor + len(regs[i]->str)) {
			return { cursor, i };
		}
		// if (i == len(regs) - 1) {
		// 	println("ABOBA");
		// 	if (idx >= cursor && idx <= cursor + regs[i]->length) {
		// 		return { cursor, i };
		// 	}
		// }
		cursor += len(regs[i]->str);
	}
	return { 0, -1 };
}

Array<PrepNode*> slice(Array<PrepNode*> regs, s64 start, s64 end);
void print_prep_state(Prep* p);
void print_regs(Span<PrepNode*> regs);

PrepNode* make_prep_node(Prep* p, PrepNodeKind kind, UnicodeString str) {
	auto node = make<PrepNode>(p->allocator);
	node->p = p;
	node->kind = kind;
	node->str = str;
	return node;
}

PrepNode* slice(PrepNode* node, s64 start, s64 end) {
	println("slice node: len, start, end, % %:%", len(node->str), start, end);
	if (end == start) {	
		return NULL;
	}
	if (start == 0 && end == len(node->str)) {
		return node;
	}
	auto nn = copy_value(node->p->allocator, *node);
	if (nn->kind == PREP_NODE_KIND_FILE) {
		nn->file_start += start;
	} else if (nn->kind == PREP_NODE_KIND_MACRO) {
		nn->macro_exp_offset += start;
	} else {
		panic("slice(): Unsupported node kind: %", nn->kind);
	}
	nn->str = nn->str[{start, end}];
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
		auto start_node = slice(regs[start_reg_idx], start - start_reg_cursor, len(regs[start_reg_idx]->str));
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
	p->src = copy_string(allocator, src);
	auto file = make<PrepFile>(allocator);
	file->src = src;
	file->path = path;
	auto node = make_prep_node(p, PREP_NODE_KIND_FILE, src);
	node->file = file;
	node->file_start = 0;
	add(&p->regs, node, 0);
	return p;
}

void replace(Prep* p, s64 start, s64 end, PrepNode* node);

void validate_regs_integrity(Prep* p) {
	s64 sum = 0;
	for (auto it: p->regs) {
		sum += len(it->str);
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
				replace(p, cursor, cursor + lb_len + 1, NULL);
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

Optional<PrepToken> prep_get_token_at(UnicodeString src, s64 start) {
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
	return {};
}

enum PrepNextTokenFlags {
	PREP_NEXT_TOKEN_FLAGS_NONE = 0,
	PREP_NEXT_TOKEN_TAKE_LINE_BREAK = 1 << 0,
};

Tuple<PrepToken, Error*> prep_next_token(Prep* p, s64* cursor, PrepNextTokenFlags flags = PREP_NEXT_TOKEN_FLAGS_NONE) {
	println("prep_next_token: %, %", *cursor, len(p->src));
	println("src: %", p->src);
	while (*cursor < len(p->src)) {
		auto c = p->src[*cursor];
		s64 lb_len = get_line_break_len(p->src, *cursor);
		if (lb_len > 0) {
			if (flags & PREP_NEXT_TOKEN_TAKE_LINE_BREAK) {
				auto tok = make_tok(PREP_TOKEN_KIND_LINE_BREAK, *cursor, *cursor + lb_len);
				*cursor = tok.reg.end;
				return { tok, NULL };
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
	auto [tok, has_value] = prep_get_token_at(p->src, *cursor);
	if (!has_value) {
		GndAsxPrepRegion reg = { .start = *cursor, .end = *cursor };
		return { {}, simple_parser_error(p, current_loc(), reg, "Invalid token") };
	}
	println("tok: %, %, (%: %)", tok.kind, tok_str(tok, p->src), tok.reg.start, tok.reg.end);
	*cursor = tok.reg.end;
	return { tok, NULL };
}

void replace(Prep* p, s64 start, s64 end, PrepNode* node) {
	assert(end >= start);
	if (len(p->regs) == 0) {
		return;
	}
	validate_regs_integrity(p);
	auto [start_reg_cursor, start_reg_idx] = get_prep_node_idx(p->regs, start);
	auto [end_reg_cursor, end_reg_idx]     = get_prep_node_idx(p->regs, end);
	if (start_reg_idx == end_reg_idx) {
		auto reg = (p->regs)[start_reg_idx];
		auto first = slice(reg, 0, start - start_reg_cursor);
		auto second = slice(reg, end - start_reg_cursor, len(reg->str));
		remove_at_index(&p->regs, start_reg_idx);
		s64 add_idx = start_reg_idx;
		if (first) {
			add(&p->regs, first, add_idx++);
		}
		if (node) {
			add(&p->regs, node, add_idx++);
		}
		if (second) {
			add(&p->regs, second, add_idx++);
		}
	} else {
		auto first = slice((p->regs)[start_reg_idx], 0, start - start_reg_cursor);
		auto second = slice((p->regs)[end_reg_idx], end - end_reg_cursor, len((p->regs)[end_reg_idx]->str));
		remove_at_index(&p->regs, start_reg_idx, end_reg_idx - start_reg_idx + 1);
		s64 add_idx = start_reg_idx;
		if (first) {
			add(&p->regs, first, add_idx++);
		}
		if (node) {
			add(&p->regs, node, add_idx++);
		}
		if (second) {
			add(&p->regs, second, add_idx++);
		}
	}
	remove_at_index(&p->src, start, end - start);
	if (node) {
		add(&p->src, node->str, start);
	}
	validate_regs_integrity(p);
}

PrepMacroArg* find_macro_arg(PrepMacro* macro, UnicodeString name) {
	for (auto& it: macro->args) {
		if (it.name == name) {
			return &it;
		}
	}
	return NULL;
}

Error* parse_macro(Prep* p, s64* cursor, s64 define_start, s64* end) {
	println("parse macro");
	PrepMacro* macro = make<PrepMacro>(p->allocator);
	macro->args.allocator = p->allocator;
	macro->regs.allocator = p->allocator;
	macro->macro_regions.allocator = p->allocator;
	auto [name_tok, e] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	if (e) {
		return e;
	}
	if (name_tok.kind != PREP_TOKEN_KIND_IDENT) {
		return simple_parser_error(p, current_loc(), name_tok.reg, "Expected ident after macro name");
	}
	macro->name = copy_string(p->allocator, tok_str(name_tok, p->src));
	s64 body_start = *cursor;
	auto [first_tok, e1] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
	if (e1) {
		return e1;
	}
	macro->is_object = true;
	if (tok_str(first_tok, p->src) == "(" && first_tok.reg.start == name_tok.reg.end) {
		macro->is_object = false;
		while (true) {
			auto [macro_tok, e2] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			if (e2) {
				return e2;
			}
			if (tok_str(macro_tok, p->src) == ")") {
				body_start = macro_tok.reg.end;
				break;
			}
			if (len(macro->args) > 0) {
				if (tok_str(macro_tok, p->src) != ",") {
					panic("Expected ',' after macro argument");
				}
				auto [mt, e3] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
				if (e3) {
					return e3;
				}
				macro_tok = mt;
			}
			if (macro_tok.kind != PREP_TOKEN_KIND_IDENT) {
				panic("Expected ident tok for macro argument");
			}
			auto name = copy_string(p->allocator, tok_str(macro_tok, p->src));
			auto loc = slice(p->regs, macro_tok.reg.start, macro_tok.reg.end);
			add(&macro->args, PrepMacroArg{name, loc});
		}
	}
	println("args: %", macro->args);
	s64 last_macro_region_end = body_start;
	while (true) {
		auto [macro_tok, e] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
		if (e) {
			return e;
		}
		if (macro_tok.kind == PREP_TOKEN_KIND_LINE_BREAK || macro_tok.kind == PREP_TOKEN_KIND_EOF) {
			auto body = p->src[{body_start, macro_tok.reg.start}];
			macro->body = copy_string(p->allocator, body);
			macro->regs = slice(p->regs, define_start, macro_tok.reg.end);
			replace(p, define_start, macro_tok.reg.end, NULL);
			*end -= macro_tok.reg.end - define_start;
			// @TODO: verify macro tokens.
			break;
		} else if (tok_str(macro_tok, p->src) == "#") {
			auto [name_tok, e1] = prep_next_token(p, cursor, PREP_NEXT_TOKEN_TAKE_LINE_BREAK);
			if (e1) {
				return e1;
			}
			if (name_tok.kind != PREP_TOKEN_KIND_IDENT) {
				panic("Stringized token must be an ident");
			}
			if (name_tok.reg.start != macro_tok.reg.end) {
				panic("Stringized token must immediately follow '#'.");
			}
			auto arg = find_macro_arg(macro, tok_str(name_tok, p->src));
			if (!arg) {
				panic("Unknown macro arg '%' after '#'", tok_str(name_tok, p->src));
			}
			auto reg = PrepMacroRegion {
				.kind = PREP_MACRO_TOKEN_REGION_STRINGIZE,
				.prev_offset = name_tok.reg.start - last_macro_region_end, 
				.length = name_tok.reg.end - macro_tok.reg.start,
				.stringize_arg = arg,
			};
			last_macro_region_end = name_tok.reg.end;
			add(&macro->macro_regions, reg);
		}
	}
	add(&p->macros, macro);
	return NULL;
}

Error* process_stringize_ops(Prep* p, MacroExpansion* exp, s64* diff, s64 macro_start) {
	s64 cursor = macro_start;
	for (auto& reg: exp->exp_regions) {
		cursor += reg.prev_offset;
		if (reg.kind == PREP_MACRO_TOKEN_REGION_STRINGIZE) {
			MacroArgLoc* arg_loc = NULL;
			for (auto it: exp->args) {
				if (it.arg == reg.stringize_arg) {
					arg_loc = &it;
					break;
				}
			}
			s64 start = arg_loc->start;
			s64 end = arg_loc->end;
			if (reg.stringize_arg->name == "__VA_ARGS__") {
				end = exp->args[-1].end;
			}

			AllocatedUnicodeString str = { .allocator = p->allocator };
			add(&str, '"');
			// @TODO: escape.
			add(&str, p->src[{start, end}]);
			add(&str, '"');

			auto node = make_prep_node(p, PREP_NODE_KIND_STRINGIZE, str);
			node->stringsize_replace_regs = slice(p->regs, cursor, cursor + reg.length);
			replace(p, cursor, cursor + reg.length, node);

			*diff += len(str) - reg.length; 
			reg.length = len(str);
		}
		cursor += reg.length;
	}
	return NULL;
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
	exp->exp_regions = copy_array(p->allocator, macro->macro_regions);
	add(&exp->str, ' ');
	add(&exp->str, macro->body);
	add(&exp->str, ' ');

	p->expanding_macro = exp;
	defer { p->expanding_macro = p->expanding_macro->parent_exp; };

	auto replace_slice = slice(p->regs, rep_start, rep_end);
	auto macro_node = make_prep_node(p, PREP_NODE_KIND_MACRO, exp->str);
	macro_node->macro_replace_regs = replace_slice;
	macro_node->macro_exp = exp;
	macro_node->macro_exp_offset = 0;
	replace(p, rep_end, rep_end, macro_node);
	
	println("src after insertion: %", p->src);
	print_regs(p->regs);
	
	s64 diff = len(exp->str) - (rep_end - rep_start);
	auto e = process_stringize_ops(p, exp, &diff, rep_end);

	// Remove macro call.
	replace(p, rep_start, rep_end, NULL);

	println("src after removal: %", p->src);
	print_regs(p->regs);

	*end += diff;
	*cursor = rep_end + diff;
	return NULL;
}

Tuple<Error*, Array<MacroArgLoc>> parse_macro_args(Prep* p, s64* cursor, PrepMacro* macro, PrepToken macro_name_tok) {
	Array<MacroArgLoc> args;
	args.allocator = p->allocator;
	s64 macro_end = macro_name_tok.reg.end;
	if (!macro->is_object) {
		auto [paren, e1] = prep_next_token(p, cursor);
		if (e1) {
			return { e1, args };
		}
		if (tok_str(paren, p->src) != "(") {
			panic("Expected '(' after function like macro");
		}
		if (paren.reg.start != macro_name_tok.reg.end) {
			panic("'(' must immediately follow macro name");
		}
		s64 arg_start = *cursor;
		while (true) {
			s64 paren_level = 0;
			auto [arg_tok, e2] = prep_next_token(p, cursor);
			if (e2) {
				return { e2, args };
			}
			if (arg_tok.kind == PREP_TOKEN_KIND_EOF) {
				panic("Unexpected EOF");
			}
			auto t_str = tok_str(arg_tok, p->src);
			if (t_str == ")") {
				if (paren_level > 0) {
					paren_level -= 1;
					continue;
				}
			} else if (t_str == "(") {
				paren_level += 1;
				continue;
			} else if (t_str == ",") {
				if (paren_level > 0) {
					continue;
				}
			} else {
				continue;
			}
			if (!macro->is_variadic) {
				if (len(args) >= len(macro->args)) {
					panic("Too many macro args");
				}
			}
			s64 idx = len(args);
			PrepMacroArg* arg = NULL;
			if (idx < len(macro->args)) {
				arg = &macro->args[idx];
			}
			auto arg_loc = make_macro_arg_loc(arg, arg_start, arg_tok.reg.start);
			add(&args, arg_loc);
			arg_start = arg_tok.reg.end;
			if (t_str == ")") {
				macro_end = arg_tok.reg.end;
				break;
			}
		}
	}
	return { NULL, args };
}

PrepMacro* find_macro(Prep* p, UnicodeString name) {
	for (auto it: p->macros) {
		if (it->name == name) {
			return it;
		}
	}
	return NULL;
}

Error* preprocess(Prep* p, s64 start, s64 end) {
	s64 cursor = start;
	while (cursor < end) {
		s64 before_main_tok = cursor;
		auto [tok, e1] = prep_next_token(p, &cursor);
		if (e1) {
			return e1;
		}
		if (tok_str(tok, p->src) == "#define") {
			auto e = parse_macro(p, &cursor, tok.reg.start, &end);
			if (e) {
				return e;
			}
			cursor = before_main_tok;
			continue;
		}
		if (tok.kind == PREP_TOKEN_KIND_IDENT) {
			auto macro = find_macro(p, tok_str(tok, p->src));
			if (macro) {
				println("matched macro: %", macro->name);
				auto [e, args] = parse_macro_args(p, &cursor, macro, tok);
				if (e) {
					return e;
				}
				auto e1 = expand_macro(p, macro, args, &cursor, &end, tok.reg.start, cursor);
				if (e1) {
					return e1;
				}
				continue;
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
		println("   '%'", node->file->src[{node->file_start, node->file_start + len(node->str)}]);
	} else if (node->kind == PREP_NODE_KIND_MACRO) {
		println("   '%'", node->macro_exp->str[{node->macro_exp_offset, node->macro_exp_offset + len(node->str)}]);
	} else if (node->kind == PREP_NODE_KIND_STRINGIZE) {
		print_ident();
		println("   '%'", node->p->src[{node_start, node_start + len(node->str)}]);
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
		cursor += len(it->str);
	}
	println();
}

void print_prep_state(Prep* p) {
	println("0: %", len(p->src));
	print_regs(p->regs);
}
