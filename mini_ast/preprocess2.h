#pragma once

#include "../string.h"
#include "../defer.h"
#include "../format.h"
#include "../error.h"
#include "../panic.h"
#include "../file.h"

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
	PREP_TOKEN_KIND_STRINGIZE,
	PREP_TOKEN_KIND_STRINGIZED,
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
	ENUM_VALUE(PREP_TOKEN_KIND_STRINGIZE);
	ENUM_VALUE(PREP_TOKEN_KIND_STRINGIZED);
}

struct PrepFileMapping {
	s64 start;
	s64 real_start;
	s64 length;
};

struct PrepFile;

struct Token {
	PrepTokenKind kind = PREP_TOKEN_KIND_NONE;
	s64           file_start = 0;
	s64           file_end = 0;
	PrepFile*     file = NULL;
	UnicodeString custom_str;
};

struct PrepFile {
	UnicodeString          fullpath;
	UnicodeString          og_src;
	UnicodeString          src;
	Array<PrepFileMapping> splice_mappings;
	Array<PrepFileMapping> comment_mappings;
	Array<Token>           tokens;
};

struct PrepMacro {
	Token        start;
	Token        end;
	Token        name;
	Array<Token> args;
	Span<Token>  tokens;
};

struct Prep {
	Allocator         allocator;
	Array<Token>      tokens;
	Array<PrepFile*>  files;
	Array<PrepMacro*> macros;
	Array<PrepFile*>  include_stack;
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

PrepFileError* make_prep_file_error(Prep* p, Token tok, auto... args) {
	return make_prep_file_error(p, tok.file, tok.file_start, tok.file_end, args...);
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

Token make_token(Prep* p, PrepTokenKind kind, PrepFile* file, s64 file_start, s64 file_end) {
	Token tok;
	tok.kind = kind;
	tok.file = file;
	tok.file_start = file_start;
	tok.file_end = file_end;
	return tok;
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

Tuple<Token, Error*> get_token_at(Prep* p, PrepFile* file, s64 cursor) {
	if (cursor == len(file->src)) {
		return { make_token(p, PREP_TOKEN_KIND_EOF, file, cursor, cursor) };
	}
	if (file->src[cursor] == '#') {
		if (starts_with(file->src[{cursor, {}}], "##")) {
			return { make_token(p, PREP_TOKEN_KIND_CONCAT, file, cursor, cursor + 2) };
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
			return { make_token(p, PREP_TOKEN_KIND_DIRECTIVE, file, cursor, dir_end) };
		}
		return { make_token(p, PREP_TOKEN_KIND_STRINGIZE, file, cursor, dir_end) };
	}
	if (prep_is_punct(file->src[cursor])) {
		return { make_token(p, PREP_TOKEN_KIND_PUNCT, file, cursor, cursor + 1) };
	}
	s64 num_end = prep_maybe_number_tok(file->src, cursor);
	if (num_end > 0) {
		return { make_token(p, PREP_TOKEN_KIND_NUMBER, file, cursor, num_end) };
	}
	s64 string_end = prep_maybe_string_tok(file->src, cursor);
	if (string_end > 0) {
		return { make_token(p, PREP_TOKEN_KIND_STRING, file, cursor, string_end) };
	}
	s64 ident_end = prep_maybe_ident(file->src, cursor);
	if (ident_end > 0) {
		return { make_token(p, PREP_TOKEN_KIND_IDENT, file, cursor, ident_end) };
	}
	return { {}, make_prep_file_error(p, file, cursor, len(file->src), "Invalid token. First char: %", file->src[cursor]) };
}

Tuple<Token, Error*> prep_file_next_token(Prep* p, PrepFile* file, s64* cursor) {
	while (*cursor < len(file->src)) {
		auto lb_len = get_line_break_len(file->src, *cursor);
		if (lb_len > 0) {
			defer { *cursor += lb_len; };
			return { make_token(p, PREP_TOKEN_KIND_LINE_BREAK, file, *cursor, *cursor + lb_len) };
		}
		if (is_whitespace(file->src[*cursor])) {
			*cursor += 1;
			continue;
		}
		break;
	}
	auto [tok, e] = get_token_at(p, file, *cursor);
	if (e) {
		return { {}, e };
	}
	*cursor = tok.file_end;
	return { tok, NULL };
}

UnicodeString tok_str(Token tok) {
	if (tok.kind == PREP_TOKEN_KIND_STRINGIZED) {
		return tok.custom_str;
	}
	return tok.file->src[{tok.file_start, tok.file_end}];
}

UnicodeString tok_str_content(Token tok) {
	assert(tok.kind == PREP_TOKEN_KIND_STRING);
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
		println("file: %, cursor: %, tok: %, '%'", file->fullpath, cursor, tok.kind, tok_str(tok));
		add(&file->tokens, tok);
		if (tok.kind == PREP_TOKEN_KIND_EOF) {
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
	p->files.allocator = allocator;
	p->tokens.allocator = allocator;
	p->include_stack.allocator = allocator;
	p->load_file_hook = prep_default_load_file_hook;
	p->resolve_fullpath_hook = prep_default_resolve_fullpath_hook;
	return p;
}

AllocatedUnicodeString prep_str(Prep* p) {
	AllocatedUnicodeString str;
	for (auto it: p->tokens) {
		append(&str, tok_str(it));
		append(&str, U" "_b);
	}
	return str;
}

// After get_next_token() cursor points to the returned token.
Token get_next_token(Span<Token> tokens, s64* cursor) {
	while (++(*cursor) < len(tokens)) {
		auto tok = tokens[*cursor];
		if (tok.kind != PREP_TOKEN_KIND_SPACE && tok.kind != PREP_TOKEN_KIND_LINE_BREAK) {
			return tok;
		}
	}
	assert(len(tokens) > 0);
	assert(tokens[-1].kind == PREP_TOKEN_KIND_EOF);
	return tokens[-1];
}

Optional<Token> find_macro_arg(PrepMacro* macro, UnicodeString name) {
	for (auto it: macro->args) {
		if (tok_str(it) == name) {
			return it;
		}
	}
	return {};
}

Error* preprocess(Prep* p, PrepFile* file) {
	auto e = tokenize(p, file);
	if (e) {
		return e;
	}
	s64 cursor = -1;
	while (++cursor < len(file->tokens)) {
		auto tok = file->tokens[cursor];
		if (tok.kind == PREP_TOKEN_KIND_DIRECTIVE) {
			if (tok_str(tok) == "#include") {
				// @TODO: do not skip newline.
				// @TODO: do computed includes.
				auto next_tok = get_next_token(file->tokens, &cursor);
				if (next_tok.kind != PREP_TOKEN_KIND_STRING) {
					return make_prep_file_error(p, next_tok, "Expected file path after #include");
				}
				auto fullpath = p->resolve_fullpath_hook(p, tok_str_content(next_tok), false);
				if (fullpath == "") {
					return make_prep_file_error(p, next_tok, "Failed to resolve #include file '%'", fullpath);
				}
				auto included_file = p->load_file_hook(p, fullpath);
				if (!included_file) {
					return make_prep_file_error(p, next_tok, "Failed to find #include file '%'", fullpath);
				}
				auto e = preprocess(p, included_file);
				if (e) {
					return e;
				}
			} else if (tok_str(tok) == "#define") {
				auto ident_tok = get_next_token(file->tokens, &cursor);
				if (ident_tok.kind != PREP_TOKEN_KIND_IDENT) {
					return make_prep_file_error(p, ident_tok, "Expected an identifier after #define");
				}
				auto macro = make<PrepMacro>(p->allocator);
				macro->start = tok;
				macro->name = ident_tok;
				add(&p->macros, macro);

				auto paren_tok = get_next_token(file->tokens, &cursor);
				if (tok_str(paren_tok) == "(" && paren_tok.file_start == ident_tok.file_end) {
					auto arg_tok = get_next_token(file->tokens, &cursor);
					if (tok_str(arg_tok) == ")") {
						macro->end = arg_tok;
						cursor += 1;
						break;
					}
					if (arg_tok.kind != PREP_TOKEN_KIND_IDENT) {
						return make_prep_file_error(p, arg_tok, "Expected an identifier as a macro argument");
					}
					if (len(macro->args) >= 1) {
						if (tok_str(arg_tok) != ",") {
							return make_prep_file_error(p, arg_tok, "Expected ',' after macro argument");
						}
					}
					for (auto it: macro->args) {
						if (tok_str(it) == tok_str(arg_tok)) {
							return make_prep_file_error(p, arg_tok, "Duplicate macro argument");
						}
					}
					if (len(macro->args) >= 1 && tok_str(macro->args[-1]) == "...") {
						return make_prep_file_error(p, arg_tok, "Unexpected macro argument after '...'");
					}
					add(&macro->args, arg_tok);
				}
				s64 first_macro_body_tok = cursor;
				while (cursor < len(file->tokens)) {
					auto macro_tok = file->tokens[cursor];
					if (macro_tok.kind == PREP_TOKEN_KIND_EOF || macro_tok.kind == PREP_TOKEN_KIND_LINE_BREAK) {
						break;
					} else if (macro_tok.kind == PREP_TOKEN_KIND_STRINGIZE) {
						auto [arg, found] = find_macro_arg(macro, tok_str(macro_tok)[{1, {}}]);
						if (!found) {
							return make_prep_file_error(p, macro_tok, "No macro argument is named for stringizing");
						}
					} else if (macro_tok.kind == PREP_TOKEN_KIND_DIRECTIVE) {
						return make_prep_file_error(p, macro_tok, "Unexpected preprocessor directive in a macro");
					}
					assert(macro_tok.kind != PREP_TOKEN_KIND_NONE);
					cursor += 1;
				}
				macro->tokens = file->tokens[{first_macro_body_tok, cursor}];
			} else {
				return make_prep_file_error(p, tok, "Unknown preprocessor directive '%'", tok_str(tok));
			}
		} else if (tok.kind == PREP_TOKEN_KIND_EOF) {
			break;
		} else {
			add(&p->tokens, tok);
		}
	}
	return NULL;
}
