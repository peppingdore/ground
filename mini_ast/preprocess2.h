#pragma once

#include "../string.h"
#include "../defer.h"
#include "../format.h"
#include "../error.h"
#include "../panic.h"
#include "../file.h"
#include "../log.h"
#include "../arena_allocator.h"

enum PrepTokenKind {
	PREP_TOKEN_KIND_NONE,
	PREP_TOKEN_KIND_LINE_BREAK,
	PREP_TOKEN_KIND_NUMBER,
	PREP_TOKEN_KIND_EOF,
	PREP_TOKEN_KIND_DIRECTIVE,
	PREP_TOKEN_KIND_STRING,
	PREP_TOKEN_KIND_IDENT,
	PREP_TOKEN_KIND_SPACE,
	PREP_TOKEN_KIND_PUNCT,
	PREP_TOKEN_KIND_CONCAT,
	PREP_TOKEN_KIND_CONCATTED,
	PREP_TOKEN_KIND_STRINGIZE,
	PREP_TOKEN_KIND_STRINGIZED,
	PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT,
};
REFLECT(PrepTokenKind) {
	ENUM_VALUE(PREP_TOKEN_KIND_NONE);
	ENUM_VALUE(PREP_TOKEN_KIND_LINE_BREAK);
	ENUM_VALUE(PREP_TOKEN_KIND_NUMBER);
	ENUM_VALUE(PREP_TOKEN_KIND_EOF);
	ENUM_VALUE(PREP_TOKEN_KIND_DIRECTIVE);
	ENUM_VALUE(PREP_TOKEN_KIND_STRING);
	ENUM_VALUE(PREP_TOKEN_KIND_IDENT);
	ENUM_VALUE(PREP_TOKEN_KIND_SPACE);
	ENUM_VALUE(PREP_TOKEN_KIND_PUNCT);
	ENUM_VALUE(PREP_TOKEN_KIND_CONCAT);
	ENUM_VALUE(PREP_TOKEN_KIND_CONCATTED);
	ENUM_VALUE(PREP_TOKEN_KIND_STRINGIZE);
	ENUM_VALUE(PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
}

struct PrepFileMapping {
	s64 start;
	s64 real_start;
	s64 length;
};

struct PrepFile;
struct MacroExp;

enum PrepTokenFlags {
	PREP_TOKEN_FLAG_NONE = 0,
	PREP_TOKEN_FLAG_CUSTOM_STR = 1 << 0,
	PREP_TOKEN_FLAG_SOURCE_FILE = 1 << 1,
	PREP_TOKEN_FLAG_SOURCE_STRINGIZE = 1 << 2,
	PREP_TOKEN_FLAG_SOURCE_MACRO = 1 << 3,
	PREP_TOKEN_FLAG_CONCATTED = 1 << 4,
	PREP_TOKEN_FLAG_CONCAT_RESIDUE = 1 << 5,
	PREP_TOKEN_FLAG_PRESCAN_EXP = 1 << 6,
};

struct Token {
	PrepTokenKind          kind  = PREP_TOKEN_KIND_NONE;
	s64                    flags = PREP_TOKEN_FLAG_NONE;
	AllocatedUnicodeString custom_str;
	PrepFile*              file = NULL;
	s64                    file_start = 0;
	s64                    file_end = 0;
	MacroExp*              stringize_exp = NULL;
	Token*                 stringize_tok = NULL;
	MacroExp*              og_macro_exp = NULL;
	Token*                 og_macro_tok = NULL;
	Token*                 concat_lhs = NULL;
	Token*                 concat_rhs = NULL;
	Token*                 concat_lhs_content = NULL;
	Token*                 concat_rhs_content = NULL;
	Token*                 concat_residue_og = NULL;
	Token*                 prescan_exp_og = NULL;
};

struct IncludeSite {
	IncludeSite*  parent = NULL;
	Array<Token*> site_toks;
	PrepFile*     file = NULL;
};

struct PrepFile {
	UnicodeString          fullpath;
	UnicodeString          og_src;
	UnicodeString          src;
	Array<PrepFileMapping> splice_mappings;
	Array<PrepFileMapping> comment_mappings;
	Array<Token*>          tokens;
};

struct PrepMacro {
	IncludeSite*  def_site = NULL;
	s64           start_tok_idx = 0;
	s64           end_tok_idx = 0;
	Token*        name;
	Array<Token*> arg_defs;
	Span<Token*>  tokens;
	bool          is_object = true;
};

struct MacroExp {
	MacroExp*     parent = NULL;
	PrepMacro*    macro = NULL;
	Array<Token*> body;
};

struct Prep {
	Allocator         allocator;
	Allocator         arena;
	Array<Token*>     tokens;
	Array<PrepFile*>  files;
	Array<PrepMacro*> macros;
	MacroExp*         macro_exp = NULL;
	IncludeSite*      include_site = NULL;
	UnicodeString   (*resolve_fullpath_hook) (Prep* p, UnicodeString path, bool global) = NULL;
	PrepFile*       (*load_file_hook)        (Prep* p, UnicodeString fullpath) = NULL;
};

struct PrepFileError: Error {
	Prep*     prep = NULL;
	PrepFile* file = NULL;
	s64       start = 0;
	s64       end   = 0;
};

PrepFileError* make_prep_file_error(Prep* p, PrepFile* file, s64 start, s64 end, auto... args) {
	auto msg = sprint(p->allocator, args...);
	auto e = make_error<PrepFileError>(msg);
	e->prep = p;
	e->file = file;
	e->start = start;
	e->end = end;
	return e;
}

PrepFileError* make_prep_file_error(Prep* p, Token* tok, auto... args) {
	return make_prep_file_error(p, tok->file, tok->file_start, tok->file_end, args...);
}

s64 map_index(Span<PrepFileMapping> mappings, s64 index) {
	for (auto it: mappings) {
		if (index >= it.start && index < it.start + it.length) {
			return index - it.start + it.real_start;
		}
	}
	return mappings[-1].real_start + mappings[-1].length;
}

s64 og_file_index(PrepFile* file, s64 index) {
	index = map_index(file->comment_mappings, index);
	index = map_index(file->splice_mappings, index);
	return index;
}

s64 get_residual_lines_above(UnicodeString src, s64 anchor, s64 lines_count) {
	s64 count = 0;
	for (auto i: reverse(range(anchor))) {
		if (is_line_break(src[i])) {
			count += 1;
			if (count >= lines_count) {
				return i + 1;
			}
		}
	}
	return 0;
}

s64 get_residual_lines_below(UnicodeString src, s64 anchor, s64 lines_count) {
	s64 count = 0;
	for (auto i: range_from_to(anchor, len(src))) {
		if (is_line_break(src[i])) {
			count += 1;
			if (count >= lines_count) {
				return i + 1;
			}
		}
	}
	return len(src);
}

void print_prep_error(Error* e) {
	if (auto x = reflect_cast<PrepFileError>(e)) {
		println(x->text);
		s64 mapped_start = og_file_index(x->file, x->start);
		s64 mapped_end = og_file_index(x->file, x->end);
		s64 start = get_residual_lines_above(x->file->og_src, mapped_start, 3);
		s64 end = get_residual_lines_below(x->file->og_src, mapped_end, 3);
		println("%", x->file->fullpath);
		print(x->file->og_src[{start, mapped_start}]);
		print(U"\x1b[0;31m");
		print(x->file->og_src[{mapped_start, mapped_end}]);
		print(U"\x1b[0m");
		print(x->file->og_src[{mapped_end, end}]);
		println();
	} else {
		println(e);
	}
}

Token* make_file_token(Prep* p, PrepTokenKind kind, PrepFile* file, s64 file_start, s64 file_end) {
	Token* tok = make<Token>(p->arena);
	tok->flags |= PREP_TOKEN_FLAG_SOURCE_FILE;
	tok->kind = kind;
	tok->file = file;
	tok->file_start = file_start;
	tok->file_end = file_end;
	return tok;
}

Token* make_token(Prep* p, PrepTokenKind kind) {
	Token* tok = make<Token>(p->arena);
	tok->kind = kind;
	tok->custom_str.allocator = null_allocator;
	return tok;
}

void prep_use_custom_str(Prep* p, Token* tok) {
	tok->flags |= PREP_TOKEN_FLAG_CUSTOM_STR;
	tok->custom_str.allocator = p->allocator;
}

void prep_push_mapping(Array<PrepFileMapping>* mappings, Prep* p, PrepFile* file, s64* cursor, s64 end, s64 rm_len, s64* removed_len, s64* i) {
	PrepFileMapping mapping;
	mapping.start = *cursor - *removed_len;
	mapping.real_start = *cursor;
	mapping.length = end - *cursor;
	add(mappings, mapping);
	*removed_len += rm_len;
	*cursor = end;
	if (i) {
		*i = end - 1;
	}
	remove_at_index(&file->src, end, rm_len);
}

void prep_remove_file_splices(Prep* p, PrepFile* file) {
	s64 cursor = 0;
	s64 removed_len = 0;
	for (s64 i = 0; i < len(file->src); i++) {
		if (file->src[i] == '\\') {
			auto lb_len = get_line_break_len(file->src, i + 1);
			if (lb_len > 0) {
				prep_push_mapping(&file->splice_mappings, p, file, &cursor, i, lb_len + 1, &removed_len, &i);
			}
		}
	}
	prep_push_mapping(&file->splice_mappings, p, file, &cursor, len(file->src), 0, &removed_len, NULL);
}

void prep_remove_comments(Prep* p, PrepFile* file) {
	s64 cursor = 0;
	s64 removed_len = 0;
	for (s64 i = 0; i < len(file->src); i++) {
		if (starts_with(file->src[{i, {}}], "//")) {
			s64 comment_end = -1;
			for (s64 j: range_from_to(i, len(file->src))) {
				auto lb_len = get_line_break_len(file->src, j);
				if (lb_len > 0) {
					comment_end = j;
					break;	
				}
			}
			if (comment_end == -1) {
				comment_end = len(file->src);
			}
			prep_push_mapping(&file->comment_mappings, p, file, &cursor, i, comment_end - i, &removed_len, &i);
		}

		if (starts_with(file->src[{i, {}}], "/*")) {
			s64 level = 0;
			s64 comment_end = -1;
			for (s64 j: range_from_to(i + 2, len(file->src))) {
				if (starts_with(file->src[{j, {}}], "*/")) {
					level -= 1;
					if (level == 0) {
						comment_end = j + 2;
						break;
					}
				} else if (starts_with(file->src[{j, {}}], "/*")) {
					level += 1;
				}
			}
			if (comment_end == -1) {
				comment_end = len(file->src);
			}
			prep_push_mapping(&file->comment_mappings, p, file, &cursor, i, comment_end - i, &removed_len, &i);
		}
	}
	prep_push_mapping(&file->comment_mappings, p, file, &cursor, len(file->src), 0, &removed_len, NULL);
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
		if (i == start) {
			if (src[i] != '"') {
				return 0;
			}
		} else {
			if (src[i] == '"') {
				return i + 1;
			}
			if (starts_with(src[{i, {}}], R"xx(\")xx")) {
				i += 1;
				continue;
			}
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

Tuple<s64, PrepTokenKind> get_str_token_at(UnicodeString src, s64 cursor) {
	if (prep_is_punct(src[cursor])) {
		return { cursor + 1, PREP_TOKEN_KIND_PUNCT };
	}
	s64 num_end = prep_maybe_number_tok(src, cursor);
	if (num_end > 0) {
		return { num_end, PREP_TOKEN_KIND_NUMBER };
	}
	s64 string_end = prep_maybe_string_tok(src, cursor);
	if (string_end > 0) {
		return { string_end, PREP_TOKEN_KIND_STRING };
	}
	s64 ident_end = prep_maybe_ident(src, cursor);
	if (ident_end > 0) {
		return { ident_end, PREP_TOKEN_KIND_IDENT };
	}
	return { 0 };
}

Tuple<Token*, Error*> get_token_at(Prep* p, PrepFile* file, s64 cursor) {
	if (cursor == len(file->src)) {
		return { make_file_token(p, PREP_TOKEN_KIND_EOF, file, cursor, cursor) };
	}
	if (file->src[cursor] == '#') {
		if (starts_with(file->src[{cursor, {}}], "##")) {
			return { make_file_token(p, PREP_TOKEN_KIND_CONCAT, file, cursor, cursor + 2) };
		}
		s64 dir_end = len(file->src);
		for (s64 i: range_from_to(cursor + 1, len(file->src))) {
			if (is_whitespace(file->src[i]) || is_line_break(file->src[i])) {
				dir_end = i;
				break;
			}
		}
		if (dir_end == cursor + 1) {
			return { {}, make_prep_file_error(p, file, cursor, cursor + 1, "Empty preprocessor directive") };
		}
		auto str = file->src[{cursor, dir_end}];
		if (str == "#include" || str == "#define") {
			return { make_file_token(p, PREP_TOKEN_KIND_DIRECTIVE, file, cursor, dir_end) };
		}
		return { make_file_token(p, PREP_TOKEN_KIND_STRINGIZE, file, cursor, dir_end) };
	}
	auto [tok_end, tok_kind] = get_str_token_at(file->src, cursor);
	if (tok_end != 0) {
		return { make_file_token(p, tok_kind, file, cursor, tok_end) };
	}
	return { {}, make_prep_file_error(p, file, cursor, len(file->src), "Invalid token. First char: %", file->src[cursor]) };
}

Tuple<Token*, Error*> prep_file_next_token(Prep* p, PrepFile* file, s64* cursor) {
	while (*cursor < len(file->src)) {
		auto lb_len = get_line_break_len(file->src, *cursor);
		if (lb_len > 0) {
			defer { *cursor += lb_len; };
			return { make_file_token(p, PREP_TOKEN_KIND_LINE_BREAK, file, *cursor, *cursor + lb_len) };
		}
		if (is_whitespace(file->src[*cursor])) {
			defer { *cursor += 1; };
			return { make_file_token(p, PREP_TOKEN_KIND_SPACE, file, *cursor, *cursor + 1) };
		}
		break;
	}
	auto [tok, e] = get_token_at(p, file, *cursor);
	if (e) {
		return { {}, e };
	}
	assert(tok->flags & PREP_TOKEN_FLAG_SOURCE_FILE);
	*cursor = tok->file_end;
	return { tok, NULL };
}

UnicodeString tok_str(Token* tok) {
	if (tok->flags & PREP_TOKEN_FLAG_CUSTOM_STR) {
		return tok->custom_str;
	}
	if (tok->flags & PREP_TOKEN_FLAG_SOURCE_MACRO) {
		return tok_str(tok->og_macro_tok);
	}
	if (tok->flags & PREP_TOKEN_FLAG_CONCAT_RESIDUE) {
		return tok_str(tok->concat_residue_og);
	}
	if (tok->flags & PREP_TOKEN_FLAG_PRESCAN_EXP) {
		return tok_str(tok->prescan_exp_og);
	}
	assert(tok->flags & PREP_TOKEN_FLAG_SOURCE_FILE);
	return tok->file->src[{tok->file_start, tok->file_end}];
}

UnicodeString tok_str_content(Token* tok) {
	assert(tok->kind == PREP_TOKEN_KIND_STRING);
	return tok_str(tok)[{1, -1}];
}

Error* tokenize(Prep* p, PrepFile* file) {
	file->src = copy_string(p->allocator, file->og_src);
	prep_remove_file_splices(p, file);
	prep_remove_comments(p, file);
	s64 cursor = 0;
	while (true) {
		auto [tok, e] = prep_file_next_token(p, file, &cursor);
		if (e) {
			return e;
		}
		println("file: %, cursor: %, tok: %, '%'", file->fullpath, cursor, tok->kind, tok_str(tok));
		add(&file->tokens, tok);
		if (tok->kind == PREP_TOKEN_KIND_EOF) {
			break;
		}
	}
	return NULL;
}

UnicodeString prep_default_resolve_fullpath_hook(Prep* p, UnicodeString path, bool global) {
	return path;
}

PrepFile* make_mem_prep_file(Prep* p, UnicodeString src, UnicodeString fullpath) {
	PrepFile* file = make<PrepFile>(p->allocator);
	file->og_src = src;
	file->fullpath = fullpath;
	file->comment_mappings.allocator = p->allocator;
	file->splice_mappings.allocator = p->allocator;
	file->tokens.allocator = p->allocator;
	return file;
}

PrepFile* prep_default_load_file_hook(Prep* p, UnicodeString fullpath) {
	auto [text, e] = read_text_at_path(p->allocator, fullpath);
	if (e) {
		return NULL;
	}
	auto utf32 = decode_utf8(p->allocator, text);
	text.free();
	return make_mem_prep_file(p, utf32, fullpath);
}

Prep* make_prep(Allocator allocator) {
	auto p = make<Prep>(allocator);
	p->allocator = allocator;
	p->arena = make_arena_allocator(allocator);
	p->files.allocator = allocator;
	p->tokens.allocator = allocator;
	p->macros.allocator = allocator;
	p->load_file_hook = prep_default_load_file_hook;
	p->resolve_fullpath_hook = prep_default_resolve_fullpath_hook;
	return p;
}

AllocatedUnicodeString prep_str(Prep* p) {
	AllocatedUnicodeString str;
	for (auto it: p->tokens) {
		append(&str, tok_str(it));
	}
	return str;
}

// After get_next_token() cursor points to the returned token.
Token* get_next_token(Span<Token*> tokens, s64* cursor) {
	while (++(*cursor) < len(tokens)) {
		auto tok = tokens[*cursor];
		if (tok->kind != PREP_TOKEN_KIND_SPACE &&
			tok->kind != PREP_TOKEN_KIND_LINE_BREAK
		) {
			return tok;
		}
	}
	assert(len(tokens) > 0);
	assert(tokens[-1]->kind == PREP_TOKEN_KIND_EOF);
	return tokens[-1];
}

bool is_prep_macro_variadic(PrepMacro* macro) {
	return len(macro->arg_defs) > 0 && tok_str(macro->arg_defs[-1]) == "...";
}

s64 find_macro_arg_def_tok_idx(PrepMacro* macro, UnicodeString name) {
	if (name == "__VA_ARGS__") {
		if (is_prep_macro_variadic(macro)) {
			return len(macro->arg_defs) - 1;
		}
	} else {
		for (auto idx: range(len(macro->arg_defs))) {
			if (tok_str(macro->arg_defs[idx]) == name) {
				return idx;
			}
		}
	}
	return -1;
}

Error* preprocess(Prep* p, PrepFile* file);

bool does_include_stack_contain_file(Prep* p, PrepFile* file) {
	auto entry = p->include_site;
	while (entry) {
		if (entry->file == file) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

Error* handle_prep_directive(Prep* p, PrepFile* file, s64* cursor) {
	auto first_directive_tok = file->tokens[*cursor];
	if (tok_str(first_directive_tok) == "#include") {
		s64 inc_start_tok_idx = *cursor;
		// @TODO: do not skip newline.
		// @TODO: do computed includes.
		auto next_tok = get_next_token(file->tokens, cursor);
		if (next_tok->kind != PREP_TOKEN_KIND_STRING) {
			return make_prep_file_error(p, next_tok, "Expected file path after #include");
		}
		auto fullpath = p->resolve_fullpath_hook(p, tok_str_content(next_tok), false);
		if (fullpath == "") {
			return make_prep_file_error(p, next_tok, "Failed to resolve #include file '%'", fullpath);
		}
		PrepFile* included_file = NULL;
		for (auto loaded: p->files) {
			if (loaded->fullpath == fullpath) {
				included_file = loaded;
				break;
			}
		}
		if (!included_file) {
			included_file = p->load_file_hook(p, fullpath);
			if (!included_file) {
				return make_prep_file_error(p, next_tok, "Failed to find #include file '%'", fullpath);
			}
			add(&p->files, included_file);
		}
		if (!does_include_stack_contain_file(p, included_file)) {
			auto inc_site = make<IncludeSite>(p->arena);
			inc_site->file = included_file;
			inc_site->parent = p->include_site;
			// @TODO: set site_toks properly if we do computed includes.
			inc_site->site_toks = copy_array(p->allocator, file->tokens[{inc_start_tok_idx, *cursor}]);
			p->include_site = inc_site;
			defer { p->include_site = inc_site->parent; };
			auto e = preprocess(p, included_file);
			if (e) {
				return e;
			}
		}
	} else if (tok_str(first_directive_tok) == "#define") {
		s64 start_tok_idx = *cursor;
		auto ident_tok = get_next_token(file->tokens, cursor);
		if (ident_tok->kind != PREP_TOKEN_KIND_IDENT) {
			return make_prep_file_error(p, ident_tok, "Expected an identifier after #define");
		}
		auto macro = make<PrepMacro>(p->allocator);
		macro->def_site = p->include_site;
		macro->start_tok_idx = start_tok_idx;
		macro->name = ident_tok;
		add(&p->macros, macro);

		s64 first_macro_body_tok = *cursor + 1;
		if (first_macro_body_tok < len(file->tokens) &&
			tok_str(file->tokens[first_macro_body_tok]) == "(" &&
			file->tokens[first_macro_body_tok]->file_start == ident_tok->file_end)
		{
			*cursor += 1;
			macro->is_object = false;
			while (true) {
				auto arg_tok = get_next_token(file->tokens, cursor);
				if (tok_str(arg_tok) == ")") {
					macro->end_tok_idx = *cursor;
					*cursor += 1;
					break;
				}
				if (len(macro->arg_defs) >= 1) {
					if (tok_str(arg_tok) != ",") {
						return make_prep_file_error(p, arg_tok, "Expected ',' after macro argument");
					}
					arg_tok = get_next_token(file->tokens, cursor);
				}
				if (arg_tok->kind != PREP_TOKEN_KIND_IDENT) {
					return make_prep_file_error(p, arg_tok, "Expected an identifier as a macro argument");
				}
				for (auto it: macro->arg_defs) {
					if (tok_str(it) == tok_str(arg_tok)) {
						return make_prep_file_error(p, arg_tok, "Duplicate macro argument");
					}
				}
				if (len(macro->arg_defs) >= 1 && tok_str(macro->arg_defs[-1]) == "...") {
					return make_prep_file_error(p, arg_tok, "Unexpected macro argument after '...'");
				}
				add(&macro->arg_defs, arg_tok);
			}
			first_macro_body_tok = *cursor;
		}
		while (*cursor < len(file->tokens)) {
			auto macro_tok = file->tokens[*cursor];
			if (macro_tok->kind == PREP_TOKEN_KIND_EOF || macro_tok->kind == PREP_TOKEN_KIND_LINE_BREAK) {
				break;
			} else if (macro_tok->kind == PREP_TOKEN_KIND_STRINGIZE) {
				auto def_tok_idx = find_macro_arg_def_tok_idx(macro, tok_str(macro_tok)[{1, {}}]);
				if (def_tok_idx == -1) {
					return make_prep_file_error(p, macro_tok, "Macro argument is not found for stringizing");
				}
			} else if (macro_tok->kind == PREP_TOKEN_KIND_DIRECTIVE) {
				return make_prep_file_error(p, macro_tok, "Unexpected preprocessor directive in a macro");
			}
			assert(macro_tok->kind != PREP_TOKEN_KIND_NONE);
			*cursor += 1;
		}
		macro->tokens = copy_array(p->allocator, file->tokens[{first_macro_body_tok, *cursor}]);
		for (auto& it: macro->tokens) {
			if (it->kind == PREP_TOKEN_KIND_IDENT) {
				it->kind = PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT;
			}
		}
		LogTrace("Macro tokens {%}", tok_str(macro->name));
		for (auto it: macro->tokens) {
			LogTrace("  %, %: %, %", tok_str(it), it->file_start, it->file_end, it->kind);
		}
	} else {
		return make_prep_file_error(p, first_directive_tok, "Unknown preprocessor directive '%'", tok_str(first_directive_tok));
	}
	return NULL;
}

PrepMacro* find_macro(Prep* p, UnicodeString name) {
	for (auto it: p->macros) {
		if (tok_str(it->name) == name) {
			return it;
		}
	}
	return NULL;
}

struct PrepMacroArg {
	s64          def_tok_idx;
	s64          start;
	s64          end;
};

void prep_stringize_tok(Token* dst, Token* arg) {
	// @TODO: escape.
	add(&dst->custom_str, tok_str(arg));
}

Tuple<Error*, Array<PrepMacroArg>> parse_macro_args(Prep* p, PrepMacro* macro, Span<Token*> tokens, s64* cursor, Token* paren_tok) {
	Array<PrepMacroArg> args = { .capacity = len(macro->arg_defs), .allocator = p->allocator };
	s64 paren_level = 0;
	s64 arg_start = -1;
	s64 arg_end = -1;
	while (true) {
		auto arg_tok = get_next_token(tokens, cursor);
		if (arg_start == -1) {
			arg_start = *cursor;
		}
		if (tok_str(arg_tok) == ")") {
			if (paren_level > 0) {
				paren_level -= 1;
				continue;
			}
		} else if (tok_str(arg_tok) == ",") {
			if (paren_level > 0) {
				continue;
			}
		} else if (tok_str(arg_tok) == "(") {
			paren_level += 1;
			continue;
		} else {
			arg_end = *cursor + 1;
			continue;
		}
		s64 def_tok_idx = -1;
		if (len(args) >= len(macro->arg_defs)) {
			if (!is_prep_macro_variadic(macro)) {
				return { make_prep_file_error(p, arg_tok, "Expected % arguments at most for a macro", len(macro->arg_defs)) };
			} else {
				def_tok_idx = -1;
			}
		} else {
			def_tok_idx = len(args);
		}
		add(&args, { def_tok_idx, arg_start, arg_end == -1 ? arg_start : arg_end });
		arg_start = -1;
		arg_end = -1;
		if (tok_str(arg_tok) == ")") {
			break;
		}
	}
	if (is_prep_macro_variadic(macro)) {
		if (len(args) < len(macro->arg_defs)) {
			return { make_prep_file_error(p, paren_tok, "Expected % arguments at least for a macro", len(macro->arg_defs)) };
		}
	} else {
		if (len(args) != len(macro->arg_defs)) {
			return { make_prep_file_error(p, paren_tok, "Expected % arguments for a macro", len(macro->arg_defs)) };
		}
	}
	return { NULL, args };
}

Optional<Span<Token*>> get_arg_tokens(PrepMacro* macro, Span<Token*> tokens, Span<PrepMacroArg> args, UnicodeString name) {
	auto def_idx = find_macro_arg_def_tok_idx(macro, name);
	if (def_idx == -1) {
		return {};
	}
	if (tok_str(macro->arg_defs[def_idx]) == "...") {
		if (def_idx < len(args)) {
			return tokens[{args[def_idx].start, args[-1].end}];
		} else {
			return make_optional<Span<Token*>>({});
		}
	} else {
		auto arg = args[def_idx];
		return tokens[{arg.start, arg.end}];
	}
}

struct MaybeExpandedMacro {
	Error*       e = NULL;
	MacroExp*    exp = NULL;
	s64          exp_start = 0;
	s64          exp_end = 0;
};

bool does_macro_stack_contain_macro(Prep* p, PrepMacro* macro) {
	auto entry = p->macro_exp;
	while (entry) {
		if (entry->macro == macro) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

MaybeExpandedMacro maybe_expand_macro(Prep* p, Span<Token*> tokens, s64* cursor) {
	auto exp_start = *cursor;
	auto name_tok = tokens[*cursor];
	assert(name_tok->kind == PREP_TOKEN_KIND_IDENT || name_tok->kind == PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
	auto macro = find_macro(p, tok_str(name_tok));
	if (!macro) {
		return { };
	}
	Token* paren_tok = NULL;
	if (!macro->is_object) {
		paren_tok = get_next_token(tokens, cursor);
		if (tok_str(paren_tok) != "(" || paren_tok->file_start != name_tok->file_end) {
			return { };
		}
	}

	if (does_macro_stack_contain_macro(p, macro)) {
		return { };
	}

	auto exp = make<MacroExp>(p->arena);
	exp->macro = macro;
	exp->parent = p->macro_exp;
	exp->body.allocator = p->allocator;
	p->macro_exp = exp;
	defer { p->macro_exp = exp->parent; };

	for (auto tok: macro->tokens) {
		auto nt = make_token(p, tok->kind);
		nt->flags |= PREP_TOKEN_FLAG_SOURCE_MACRO;
		nt->og_macro_tok = tok;
		nt->og_macro_exp = exp;
		add(&exp->body, nt);
	}

	if (!macro->is_object) {
		auto [e, args] = parse_macro_args(p, macro, tokens, cursor, paren_tok);
		if (e) {
			return { e };
		}
		// Stringize.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_STRINGIZE) {
				auto str_tok = make_token(p, PREP_TOKEN_KIND_STRING);
				str_tok->flags |= PREP_TOKEN_FLAG_SOURCE_STRINGIZE;
				str_tok->stringize_exp = exp;
				str_tok->stringize_tok = exp->body[i];
				prep_use_custom_str(p, str_tok);
				append(&str_tok->custom_str, U"\"");
				auto [arg_tokens, found] = get_arg_tokens(macro, tokens, args, tok_str(exp->body[i])[{1, {}}]);
				if (!found) {
					return { make_prep_file_error(p, exp->body[i], "Macro argument is not found for stringizing") };
				}
				for (auto tok: arg_tokens) {
					prep_stringize_tok(str_tok, tok);
				}
				append(&str_tok->custom_str, U"\"");
				exp->body[i] = str_tok;
			}
		}
		// Concat.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_CONCAT) {
				if (i - 1 < 0 || contains({PREP_TOKEN_KIND_SPACE, PREP_TOKEN_KIND_EOF}, exp->body[i - 1]->kind)) {
					return { make_prep_file_error(p, exp->body[i], "Missing left argument for ## operator") };
				}
				if (i + 1 >= len(exp->body) || contains({PREP_TOKEN_KIND_SPACE, PREP_TOKEN_KIND_EOF}, exp->body[i + 1]->kind)) {
					return { make_prep_file_error(p, exp->body[i], "Missing right argument for ## operator") };
				}
				Token* tok = make_token(p, PREP_TOKEN_KIND_NONE);
				tok->flags |= PREP_TOKEN_FLAG_CONCATTED;
				prep_use_custom_str(p, tok);
				auto lhs_tok = exp->body[i - 1];
				Span<Token*> lhs = exp->body[{i - 1, i}];
				Span<Token*> rhs = exp->body[{i + 1, i + 2}];
				tok->concat_lhs = lhs[0];
				tok->concat_rhs = rhs[0];
				auto [lhs_toks, lhs_found] = get_arg_tokens(macro, tokens, args, tok_str(lhs[0]));
				if (lhs_found) {
					lhs = lhs_toks;
				}
				auto [rhs_toks, rhs_found] = get_arg_tokens(macro, tokens, args, tok_str(rhs[0]));
				if (rhs_found) {
					rhs = rhs_toks;
				}
				Span<Token*> lhs_residue;
				Span<Token*> rhs_residue;
				if (len(lhs) > 0) {
					tok->concat_lhs_content = lhs[-1];
					append(&tok->custom_str, tok_str(lhs[-1]));
					lhs_residue = lhs[{0, -1}];
				}
				if (len(rhs) > 0) {
					tok->concat_rhs_content = rhs[0];
					append(&tok->custom_str, tok_str(rhs[0]));
					rhs_residue = rhs[{1, {}}];
				}
				auto [end, kind] = get_str_token_at(tok->custom_str, 0);
				if (end != len(tok->custom_str)) {
					return { make_prep_file_error(p, exp->body[i], "Concatenated string '%' doesn't form a valid token", tok_str(tok)) };
				}
				for (auto& it: lhs_residue) {
					auto t = make_token(p, it->kind);
					t->flags |= PREP_TOKEN_FLAG_CONCAT_RESIDUE;
					t->concat_residue_og = it;
					it = t;
				}
				for (auto& it: rhs_residue) {
					auto t = make_token(p, it->kind);
					t->flags |= PREP_TOKEN_FLAG_CONCAT_RESIDUE;
					t->concat_residue_og = it;
					it = t;
				}
				tok->kind = kind;
				remove_at_index(&exp->body, i - 1, 3);
				add(&exp->body, lhs_residue, i - 1);
				add(&exp->body, tok,         i - 1 + len(lhs_residue));
				add(&exp->body, rhs_residue, i - 1 + len(lhs_residue) + 1);
				i = i - 1 + len(lhs_residue) + len(rhs_residue);
			}
		}
		// Prescan.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT) {
				auto [arg_tokens, found] = get_arg_tokens(macro, tokens, args, tok_str(exp->body[i]));
				println("found: %", found);
				if (found) {
					s64 body_cursor = i;
					remove_at_index(&exp->body, i);
					s64 arg_cursor = 0;
					while (arg_cursor < len(arg_tokens)) {
						if (arg_tokens[arg_cursor]->kind == PREP_TOKEN_KIND_IDENT) {
							auto mb = maybe_expand_macro(p, arg_tokens, &arg_cursor);
							if (mb.e) {
								return { mb.e };
							}
							if (mb.exp) {
								add(&exp->body, mb.exp->body, body_cursor);
								body_cursor += len(mb.exp->body);
								arg_cursor += 1;
								continue;
							}
						}
						auto t = make_token(p, arg_tokens[arg_cursor]->kind);
						t->flags |= PREP_TOKEN_FLAG_PRESCAN_EXP;
						t->prescan_exp_og = arg_tokens[arg_cursor];
						add(&exp->body, t, body_cursor);
						body_cursor += 1;
						arg_cursor += 1;
					}
				}
			}
		}
	}

	MaybeExpandedMacro res = {
		.exp = exp,
		.exp_start = exp_start,
		.exp_end = *cursor,
	};
	return res;
}

Error* preprocess(Prep* p, PrepFile* file) {
	auto e = tokenize(p, file);
	if (e) {
		return e;
	}
	s64 cursor = -1;
	while (++cursor < len(file->tokens)) {
		auto tok = file->tokens[cursor];
		if (tok->kind == PREP_TOKEN_KIND_DIRECTIVE) {
			auto e = handle_prep_directive(p, file, &cursor);
			if (e) {
				return e;
			}
			continue;
		}
		if (tok->kind == PREP_TOKEN_KIND_IDENT) {
			auto mb = maybe_expand_macro(p, file->tokens, &cursor);
			if (mb.e) {
				return e;
			}
			if (mb.exp) {
				add(&p->tokens, mb.exp->body);
				continue;
			}
		}
		if (tok->kind == PREP_TOKEN_KIND_EOF) {
			break;
		}
		add(&p->tokens, tok);
	}
	return NULL;
}
