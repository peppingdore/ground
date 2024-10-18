#pragma once

#include "../string.h"
#include "../grd_defer.h"
#include "../format.h"
#include "../error.h"
#include "../panic.h"
#include "../file.h"
#include "../log.h"
#include "../arena_allocator.h"
#include "../sub_allocator.h"
#include "../gnd_assert.h"

enum PrepTokenKind {
	PREP_TOKEN_KIND_NONE,
	PREP_TOKEN_KIND_EOF,
	PREP_TOKEN_KIND_LINE_BREAK,
	PREP_TOKEN_KIND_NUMBER,
	PREP_TOKEN_KIND_DIRECTIVE,
	PREP_TOKEN_KIND_STRING,
	PREP_TOKEN_KIND_IDENT,
	PREP_TOKEN_KIND_SPACE,
	PREP_TOKEN_KIND_PUNCT,
	PREP_TOKEN_KIND_GRD_CONCAT,
	PREP_TOKEN_KIND_GRD_CONCATTED,
	PREP_TOKEN_KIND_STRINGIZE,
	PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT,
};
GRD_REFLECT(PrepTokenKind) {
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_NONE);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_EOF);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_LINE_BREAK);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_NUMBER);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_DIRECTIVE);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_STRING);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_IDENT);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_SPACE);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_PUNCT);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_GRD_CONCAT);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_GRD_CONCATTED);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_STRINGIZE);
	GRD_ENUM_VALUE(PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
}

struct PrepFileMapping {
	s64 start;
	s64 real_start;
	s64 length;
};

struct MacroExp;

enum PrepTokenFlags {
	PREP_TOKEN_FLAG_NONE = 0,
	PREP_TOKEN_FLAG_CUSTOM_STR = 1 << 0,
	PREP_TOKEN_FLAG_FILE_MISSING_TRAILING_LINE_BREAK = 1 << 1,
};

enum PrepTokenSourceKind {
	PREP_TOKEN_SOURCE_NONE = 0,
	PREP_TOKEN_SOURCE_FILE_SOURCE = 1,
	PREP_TOKEN_SOURCE_STRINGIZE = 2,
	PREP_TOKEN_SOURCE_MACRO = 3,
	PREP_TOKEN_SOURCE_GRD_CONCATTED = 4,
	PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE = 5,
	PREP_TOKEN_SOURCE_PRESCAN_EXP = 6,
	PREP_TOKEN_SOURCE_INCLUDED_FILE = 7,
};
GRD_REFLECT(PrepTokenSourceKind) {
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_NONE);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_FILE_SOURCE);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_STRINGIZE);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_MACRO);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_GRD_CONCATTED);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_PRESCAN_EXP);
	GRD_ENUM_VALUE(PREP_TOKEN_SOURCE_INCLUDED_FILE);
}

struct IncludedFile;
struct PrepFileSource;

struct Token {
	PrepTokenKind          kind  = PREP_TOKEN_KIND_NONE;
	s64                    flags = PREP_TOKEN_FLAG_NONE;
	PrepTokenSourceKind    src_kind = PREP_TOKEN_SOURCE_NONE;
	AllocatedUnicodeString custom_str;
	PrepFileSource*        file_source = NULL;
	s64                    file_tok_idx = 0;
	IncludedFile*          included_file = NULL;
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
	MacroExp*              concat_exp = NULL;
	Token*                 prescan_exp_og = NULL;
	MacroExp*              prescan_exp = NULL;
	Token*                 included_file_og_tok = NULL;
};

struct IncludedFile {
	IncludedFile*   parent = NULL;
	Array<Token*>   site_toks;
	PrepFileSource* file = NULL;
	Array<Token*>   tokens;
};

// @TODO: rename back to PrepFile.
struct PrepFileSource {
	UnicodeString          fullpath;
	UnicodeString          og_src;
	UnicodeString          src;
	Array<PrepFileMapping> splice_mappings;
	Array<PrepFileMapping> comment_mappings;
	Array<Token*>          tokens;
	bool                   is_tokenized = false;
};

struct PrepMacro {
	IncludedFile* def_site = NULL;
	s64           start_tok_idx = 0;
	s64           end_tok_idx = 0;
	Token*        name;
	Array<Token*> arg_defs;
	Span<Token*>  tokens;
	bool          is_object = true;
};

struct PrepMacroArg {
	s64          def_tok_idx;
	s64          start;
	s64          end;
};

struct MacroExp {
	MacroExp*     parent = NULL;
	PrepMacro*    macro = NULL;
	Array<Token*> body;
	Array<Token*> replaced;
	Array<PrepMacroArg> args;
};

struct Prep {
	GrdAllocator         allocator;
	GrdAllocator         arena;
	Array<Token*>     tokens;
	Array<PrepFileSource*> files;
	Array<PrepMacro*> macros;
	MacroExp*         macro_exp = NULL;
	IncludedFile*     include_site = NULL;
	void*             aux_data = NULL;
	UnicodeString   (*resolve_fullpath_hook) (Prep* p, UnicodeString path, bool global) = NULL;
	PrepFileSource* (*load_file_hook)        (Prep* p, UnicodeString fullpath) = NULL;
};

struct PrepFileError: Error {
	Prep*           prep = NULL;
	PrepFileSource* file = NULL;
	s64             start = 0;
	s64             end   = 0;
};

struct PrepTokenError: Error {
	Prep*  prep = NULL;
	Token* tok = NULL;
};

PrepFileError* grd_make_prep_file_error(Prep* p, PrepFileSource* file, s64 start, s64 end, auto... args) {
	auto msg = sprint(p->allocator, args...);
	auto e = grd_make_error<PrepFileError>(msg);
	e->prep = p;
	e->file = file;
	e->start = start;
	e->end = end;
	return e;
}

Token* tok_file_tok(Token* tok);

PrepTokenError* grd_make_prep_file_error(Prep* p, Token* tok, auto... args) {
	auto msg = sprint(p->allocator, args...);
	auto e = grd_make_error<PrepTokenError>(msg);
	e->prep = p;
	e->tok = tok;
	return e;
}

s64 map_index(Span<PrepFileMapping> mappings, s64 index) {
	for (auto it: mappings) {
		if (index >= it.start && index < it.start + it.length) {
			return index - it.start + it.real_start;
		}
	}
	return mappings[-1].real_start + mappings[-1].length;
}

s64 og_file_index(PrepFileSource* source, s64 index) {
	index = map_index(source->comment_mappings, index);
	index = map_index(source->splice_mappings, index);
	return index;
}

s64 get_residual_lines_above(UnicodeString src, s64 anchor, s64 lines_count) {
	s64 count = 0;
	for (auto i: reverse(grd_range(anchor))) {
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

UnicodeString tok_str(Token* tok);

void print_file_range_highlighted(PrepFileSource* file, s64 h_start, s64 h_end, s64 color) {
	s64 mapped_start = og_file_index(file, h_start);
	s64 mapped_end = og_file_index(file, h_end);
	s64 start = get_residual_lines_above(file->og_src, mapped_start, 3);
	s64 end = get_residual_lines_below(file->og_src, mapped_end, 3);
	println();
	print(file->og_src[{start, mapped_start}]);
	print(U"\x1b[0;%m", 30 + color);
	// Length is 0, but we have to print something to see where the error is.
	if (mapped_start == mapped_end) {
		print(U"\x1b[%m", 40 + color);
		print(" ");
		print(U"\x1b[49m");
	}
	for (auto c: file->og_src[{mapped_start, mapped_end}]) {
		if (is_whitespace(c)) {
			print(U"\x1b[%m", 40 + color);
			print(c);
			print(U"\x1b[49m");
		} else {
			print(c);
		}
	}
	print(U"\x1b[0m");
	print(file->og_src[{mapped_end, end}]);
	println();
}

void print_prep_token_error(Token* tok, s64 color) {
	assert(tok->src_kind == PREP_TOKEN_SOURCE_FILE_SOURCE);
	auto file = tok->file_source;
	print_file_range_highlighted(file, tok->file_start, tok->file_end, color);
}

void print_single_error_token(Token* tok) {
	switch (tok->src_kind) {
		case PREP_TOKEN_SOURCE_FILE_SOURCE: {
			println("  at file: %", tok->file_source->fullpath);
			print_prep_token_error(tok, 1);
			break;
		}
		case PREP_TOKEN_SOURCE_MACRO: {
			print_single_error_token(tok->og_macro_tok);
			println("  expanded from macro: %", tok_str(tok->og_macro_exp->macro->name));
			println("  which is defined in file: %", tok->og_macro_exp->macro->def_site->file->fullpath);
			print_prep_token_error(tok->og_macro_exp->macro->tokens[0], 1);
			break;
		}
		case PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			println("  at concatenated token: %", tok_str(tok));
			println("lhs token: ");
			print_single_error_token(tok->concat_lhs);
			println("rhs token: ");
			print_single_error_token(tok->concat_rhs);
			break;
		}
		case PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			println("  at included file: %", tok->included_file->file->fullpath);
			print_prep_token_error(tok->included_file_og_tok, 1);
			break;
		}
		default: {
			println("Unknown token source kind: %", tok->src_kind);
			gnd_assert(false);
		}
	}
}

void print_debug_tok(Token* tok) {
	println("tok: %, \"%\"", tok->kind, tok_str(tok));
}


struct TokenSpan {
	Span<Token*> tokens;
	s64          color = 0;
};

struct DumpedFileRanges {
	Array<Tuple<s64, s64>> ranges;
};

struct TokenDumperFile {
	IncludedFile* file = NULL;
	HashMap<s64, DumpedFileRanges> color_ranges;
};

struct TokenDumper {
	Array<TokenDumperFile> files;
};

void print_token_dumper(TokenDumper* dumper) {
	for (auto& it: dumper->files) {
		println("File: %", it.file->file->fullpath);
		auto parent = it.file->parent;
		if (parent) {
			println("  Included from: ");
		}
		while (parent) {
			println("    %", parent->file->fullpath);
			parent = parent->parent;
		}
		for (auto entry: it.color_ranges) {
			for (auto range: entry->value.ranges) {
				print_file_range_highlighted(it.file->file, range._0, range._1, entry->key);
			}
		}
	}
}

void dump_file_grd_range(TokenDumper* dumper, IncludedFile* file, s64 start, s64 end, s64 color) {
	// println("dump_file_grd_range(): %, (%, %)", file->fullpath, start, end);
	TokenDumperFile* df = NULL;
	for (auto& it: dumper->files) {
		if (it.file == file) {
			df = &it;
			break;
		}
	}
	if (df == NULL) {
		df = add(&dumper->files, { file });
	}

	DumpedFileRanges* dfr = get(&df->color_ranges, color);
	if (!dfr) {
		dfr = put(&df->color_ranges, color, {});
	}

	grd_defer { 
		for (auto it: dfr->ranges) {
			// println("  %, %", it._0, it._1);
		}
	};

	s64 idx = 0;
	for (auto& it: dfr->ranges) {
		bool intersects_or_touches = max_s64(start, it._0) <= min_s64(end, it._1);
		if (intersects_or_touches) {
			it._0 = min_s64(it._0, start);
			it._1 = max_s64(it._1, end);
			// Coalesce following.
			for (s64 i = idx + 1; i < len(dfr->ranges); i++) {
				bool intersects_or_touches = max_s64(dfr->ranges[idx]._0, dfr->ranges[i]._0) <= min_s64(dfr->ranges[idx]._1, dfr->ranges[i]._1);
				if (intersects_or_touches) {
					dfr->ranges[idx]._0 = min_s64(dfr->ranges[i]._0, dfr->ranges[idx]._0);
					dfr->ranges[idx]._1 = max_s64(dfr->ranges[i]._1, dfr->ranges[idx]._1);
					remove_at_index(&dfr->ranges, i);
					i -= 1;
				}
			}
			return;
		}
		idx += 1;
	}
	add(&dfr->ranges, { start, end });
}

MacroExp* get_root_macro_exp(MacroExp* exp) {
	while (exp->parent) {
		exp = exp->parent;
	}
	return exp;
}

void dump_token(TokenDumper* dumper, Token* tok, s64 color) {
	switch (tok->src_kind) {
		// case PREP_TOKEN_SOURCE_FILE_SOURCE:{
		// 	dump_file_grd_range(dumper, tok->file_source, tok->file_start, tok->file_end, color);
		// }
		// break;
		case PREP_TOKEN_SOURCE_STRINGIZE: {
			auto exp = get_root_macro_exp(tok->stringize_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				dump_token(dumper, it, color);
			}
		}
		break;
		case PREP_TOKEN_SOURCE_MACRO: {
			auto exp = get_root_macro_exp(tok->og_macro_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				dump_token(dumper, it, color);
			}
		}
		break;
		case PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			auto exp = get_root_macro_exp(tok->concat_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				dump_token(dumper, it, color);
			}
		}
		break;
		case PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE: {
			auto exp = get_root_macro_exp(tok->concat_exp);
			for (auto it: exp->replaced) {
				dump_token(dumper, it, color);
			}
		}
		break;
		case PREP_TOKEN_SOURCE_PRESCAN_EXP: {
			auto exp = get_root_macro_exp(tok->prescan_exp);
			for (auto it: exp->replaced) {
				dump_token(dumper, it, color);
			}
		}
		break;
		case PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			dump_file_grd_range(dumper, tok->included_file, tok->included_file_og_tok->file_start, tok->included_file_og_tok->file_end, color);
			// dump_token(dumper, tok->included_file_og_tok, color);
		}
		break;
		default: {
			panic("Unknown token source kind: %", tok->src_kind);
		}
	}
}

struct DpSpan {
	s64 start;
	s64 end;
	s64 color;
};

struct DetailedPrinter {
	Span<Token*>  tokens;
	Array<DpSpan> spans;
};

bool do_token_sources_match(Token* a, Token* b) {
	if (a->src_kind != b->src_kind) {
		return false;
	}
	switch (a->src_kind) {
		case PREP_TOKEN_SOURCE_FILE_SOURCE: {
			return a->file_source == b->file_source;
		}
		case PREP_TOKEN_SOURCE_MACRO: {
			return a->og_macro_exp == b->og_macro_exp;
		}
		case PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			return a->concat_exp == b->concat_exp;
		}
		case PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE: {
			return a->concat_exp == b->concat_exp;
		}
		case PREP_TOKEN_SOURCE_PRESCAN_EXP: {
			return a->prescan_exp == b->prescan_exp;
		}
		case PREP_TOKEN_SOURCE_STRINGIZE: {
			return a->stringize_exp == b->stringize_exp;
		}
		case PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			return a->included_file == b->included_file;
		}
		default: {
			panic("Unknown token source kind: %", a->src_kind);
			return false;
		}
	}
}

DetailedPrinter* grd_make_detailed_printer(Span<Token*> tokens) {
	auto dp = grd_make<DetailedPrinter>();
	dp->tokens = tokens;
	return dp;
}

s64 get_residual_tokens(Span<Token*> tokens, s64 cursor, s64 dir) {
	s64 lines = 0;
	bool fwd = dir >= 0;
	dir = fwd ? 1 : -1;
	for (s64 i = cursor + dir; (i >= 0 && i < len(tokens)); i += dir) {
		if (tokens[i]->kind == PREP_TOKEN_KIND_LINE_BREAK) {
			lines += 1;
			if (lines >= 3) {
				return i + 1;
			}
		}
	}
	return fwd ? len(tokens) : 0;
}

void do_detailed_print(DetailedPrinter* dp, bool expand_site) {
	if (len(dp->spans) == 0) {
		println("Empty DP");
	}

	sort(dp->spans, [] (auto spans, s64 a, s64 b) {
		if (spans[a].start < spans[b].start) {
			return true;
		}
		if (spans[a].start == spans[b].start) {
			return spans[a].end > spans[b].end;
		}
		return false;
	});

	s64 global_start = dp->spans[0].start;
	s64 global_end = dp->spans[-1].end;

	global_start = get_residual_tokens(dp->tokens, global_start, -1);
	global_end   = get_residual_tokens(dp->tokens, global_end, 1);

	Array<Tuple<s64, s64, bool, s64>> regs;
	add(&regs, { global_start, -1, true });
	for (auto idx: range_from_to(global_start, global_end)) {
		bool match = do_token_sources_match(dp->tokens[idx], dp->tokens[idx - 1]);
		if (!match) {
			regs[-1]._1 = idx;
			add(&regs, { idx, -1, true });
		}
	}
	regs[-1]._1 = global_end;

	for (s64 idx = 0; idx < len(regs); idx++) {
		if (regs[idx]._0 == regs[idx]._1) {
			remove_at_index(&regs, idx);
			idx -= 1;
			continue;
		}
		// auto tok = dp->tokens[regs[idx]._1];
		// if (tok->src_kind == PREP_TOKEN_SOURCE_INCLUDED_FILE) {
		// 	if (tok->included_file == first_file) {
		// 		remove_at_index(&regs, idx);
		// 		idx -= 1;
		// 	}
		// }
	}

	for (s64 i = 0; i < len(regs); i++) {
		s64 left_end = i - 1 >= 0 ? regs[i - 1]._1 : 0;
		if (left_end < regs[i]._0) {
			add(&regs, { left_end, regs[i]._0, false }, i);
			i += 1;
		}
		s64 right_start = i + 1 < len(regs) ? regs[i + 1]._0 : len(dp->tokens);
		if (regs[i]._1 < right_start) {
			add(&regs, { regs[i]._1, right_start, false }, i + 1);
			i += 1;
		}
	}

	s64 zone_counter = 1;
	for (auto& it: regs) {
		if (it._2 == true) {
			it._3 = zone_counter;
			zone_counter += 1;
		}
	}

	// for (auto it: regs) {
	// 	// debug.
	// 	println("reg: %, %, %", it._0, it._1, it._2);
	// }

	// if (first_file) {
	// 	// @TODO: dump include path.
	// 	println("  Path: %", first_file->file->fullpath);
	// }

	AllocatedUnicodeString line_double;
	grd_defer { line_double.free(); };
	for (auto [start, end, need_exp, zone]: regs) {
		s64 printed_zone_len = 0;
		auto get_zone_char = [&] () -> char {
			auto zone_num = to_string(zone);
			if (printed_zone_len < len(zone_num)) {
				return zone_num[printed_zone_len++];
			} else {
				return '~';
			}
		};
		for (s64 idx: range_from_to(start, end)) {
			for (auto span: dp->spans) {
				if (idx >= span.start && idx < span.end) {
					print("\x1b[0;%m", 30 + span.color);
				}
			}
			auto it = dp->tokens[idx];
			if (it->kind == PREP_TOKEN_KIND_LINE_BREAK) {
				println();
				if (expand_site) {
					print(line_double);
					clear(&line_double);
					println();
				}
				continue;
			}
			if (need_exp) {
				print(tok_str(it));
				for (auto i: grd_range(len(tok_str(it)))) add(&line_double, get_zone_char());
			} else {
				print(tok_str(it));
				for (auto i: grd_range(len(tok_str(it)))) add(&line_double, ' ');
			}
			print("\x1b[0m");
		}
	}
	if (expand_site) {
		println();
		print(line_double);
		println();
	}
	println();

	if (expand_site) {
		for (auto [start, end, need_exp, zone]: regs) {
			if (end <= start) {
				continue;
			}
			Token* tok = dp->tokens[start];
			println("Site %:", zone);
			void print_dp_site(Token* tok);
			print_dp_site(tok);
		}
	}	
}

void print_include_site(IncludedFile* inc) {
	while (inc) {
		println("%", inc->file->fullpath);
		inc = inc->parent;
	}
}

void print_dp_site(Token* tok) {
	switch (tok->src_kind) {
		case PREP_TOKEN_SOURCE_FILE_SOURCE: {
			auto dp = grd_make_detailed_printer(tok->file_source->tokens);
			add(&dp->spans, { tok->file_tok_idx, tok->file_tok_idx + 1, 1 });
			do_detailed_print(dp, false);
		}
		break;
		// case PREP_TOKEN_SOURCE_STRINGIZE
		case PREP_TOKEN_SOURCE_MACRO: {
			auto macro = tok->og_macro_exp->macro;
			println("Expanded from macro: %", tok_str(macro->name));
			println("Defined in:");
			print_include_site(macro->def_site);
			println();
			auto dp = grd_make_detailed_printer(macro->def_site->tokens);
			add(&dp->spans, { macro->start_tok_idx, macro->end_tok_idx, 1 });
			do_detailed_print(dp, false);
		}
		break;
		// case PREP_TOKEN_SOURCE_GRD_CONCATTED
		// case PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE
		// case PREP_TOKEN_SOURCE_PRESCAN_EXP
		// case PREP_TOKEN_SOURCE_INCLUDED_FILE

		default: {
			panic("Unknown token source kind: %", tok->src_kind);
		}
	}
}

struct PrepDpErrorNode {
	DetailedPrinter* dp = NULL;
	UnicodeString    str;
};

struct PrepDetailedError: Error {
	Array<PrepDpErrorNode> nodes;
};

PrepDetailedError* grd_make_prep_dp_error(Prep* p, auto... args) {
	auto msg = sprint(args...);
	auto e = grd_make_error<PrepDetailedError>(msg);
	return e;
}

void add_dp(PrepDetailedError* e, DetailedPrinter* dp) {
	add(&e->nodes, { dp });
}

void add_message(PrepDetailedError* e, auto... args) {
	auto msg = sprint(args...);
	add(&e->nodes, { msg });
}

void print_token_spans(TokenDumper* dumper, Span<TokenSpan> spans) {
	for (auto it: spans) {
		for (auto tok: it.tokens) {
			dump_token(dumper, tok, it.color);
		}
	}
}

void print_prep_error(Error* e) {
	println();
	println("Error: %", e->text);
	if (auto x = grd_reflect_cast<PrepFileError>(e)) {
		println();
		println("   at %", x->file->fullpath);
		print_file_range_highlighted(x->file, x->start, x->end, 1);
		println();
		println();
	} else if (auto x = grd_reflect_cast<PrepTokenError>(e)) {
		println();
		TokenDumper dumper;
		dump_token(&dumper, x->tok, 1);
		print_token_dumper(&dumper);
		// print_single_error_token(x->tok);
	} else if (auto x = grd_reflect_cast<PrepDetailedError>(e)) {
		for (auto node: x->nodes) {
			if (node.dp) {
				do_detailed_print(node.dp, true);
			} else {
				println(node.str);
			}
		}
	} else {
		println(e);
	}
}

Token* grd_make_token(Prep* p, PrepTokenKind kind, PrepTokenSourceKind src_kind) {
	Token* tok = grd_make<Token>(p->arena);
	tok->kind = kind;
	tok->custom_str.allocator = null_allocator;
	tok->src_kind = src_kind;
	return tok;
}

Token* grd_make_file_token(Prep* p, PrepTokenKind kind, PrepFileSource* file, s64 file_start, s64 file_end) {
	auto tok = grd_make_token(p, kind, PREP_TOKEN_SOURCE_FILE_SOURCE);
	tok->file_source = file;
	tok->file_start = file_start;
	tok->file_end = file_end;
	return tok;
}

void prep_use_custom_str(Prep* p, Token* tok) {
	tok->flags |= PREP_TOKEN_FLAG_CUSTOM_STR;
	tok->custom_str.allocator = p->allocator;
}

void prep_push_mapping(Array<PrepFileMapping>* mappings, Prep* p, PrepFileSource* file, s64* cursor, s64 end, s64 rm_len, s64* removed_len, s64* i) {
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

void prep_remove_file_splices(Prep* p, PrepFileSource* file) {
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

void prep_remove_comments(Prep* p, PrepFileSource* file) {
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

Tuple<Token*, Error*> get_token_at(Prep* p, PrepFileSource* file, s64 cursor) {
	if (file->src[cursor] == '#') {
		if (starts_with(file->src[{cursor, {}}], "##")) {
			return { grd_make_file_token(p, PREP_TOKEN_KIND_GRD_CONCAT, file, cursor, cursor + 2) };
		}
		s64 dir_end = len(file->src);
		for (s64 i: range_from_to(cursor + 1, len(file->src))) {
			if (is_whitespace(file->src[i]) || is_line_break(file->src[i])) {
				dir_end = i;
				break;
			}
		}
		if (dir_end == cursor + 1) {
			return { {}, grd_make_prep_file_error(p, file, cursor, cursor + 1, "Empty preprocessor directive") };
		}
		auto str = file->src[{cursor, dir_end}];
		if (str == "#include" || str == "#define") {
			return { grd_make_file_token(p, PREP_TOKEN_KIND_DIRECTIVE, file, cursor, dir_end) };
		}
		return { grd_make_file_token(p, PREP_TOKEN_KIND_STRINGIZE, file, cursor, dir_end) };
	}
	auto [tok_end, tok_kind] = get_str_token_at(file->src, cursor);
	if (tok_end != 0) {
		return { grd_make_file_token(p, tok_kind, file, cursor, tok_end) };
	}
	return { NULL, grd_make_prep_file_error(p, file, cursor, len(file->src), "Invalid token. First char: %", file->src[cursor]) };
}

Tuple<Token*, Error*> prep_file_next_token(Prep* p, PrepFileSource* file, s64* cursor) {
	while (*cursor < len(file->src)) {
		auto lb_len = get_line_break_len(file->src, *cursor);
		if (lb_len > 0) {
			grd_defer { *cursor += lb_len; };
			return { grd_make_file_token(p, PREP_TOKEN_KIND_LINE_BREAK, file, *cursor, *cursor + lb_len) };
		}
		if (is_whitespace(file->src[*cursor])) {
			grd_defer { *cursor += 1; };
			return { grd_make_file_token(p, PREP_TOKEN_KIND_SPACE, file, *cursor, *cursor + 1) };
		}
		break;
	}
	if (*cursor == len(file->src)) {
		return { NULL, NULL };
	}
	auto [tok, e] = get_token_at(p, file, *cursor);
	if (e) {
		return { {}, e };
	}
	assert(tok->src_kind == PREP_TOKEN_SOURCE_FILE_SOURCE);
	*cursor = tok->file_end;
	return { tok, NULL };
}

UnicodeString tok_str(Token* tok) {
	if (tok->flags & PREP_TOKEN_KIND_LINE_BREAK) {
		return U"\n"_b;
	}
	if (tok->flags & PREP_TOKEN_FLAG_CUSTOM_STR) {
		return tok->custom_str;
	}
	if (tok->src_kind == PREP_TOKEN_SOURCE_MACRO) {
		return tok_str(tok->og_macro_tok);
	}
	if (tok->src_kind == PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE) {
		return tok_str(tok->concat_residue_og);
	}
	if (tok->src_kind == PREP_TOKEN_SOURCE_PRESCAN_EXP) {
		return tok_str(tok->prescan_exp_og);
	}
	if (tok->src_kind == PREP_TOKEN_SOURCE_FILE_SOURCE) {
		return tok->file_source->src[{tok->file_start, tok->file_end}];
	}
	if (tok->src_kind == PREP_TOKEN_SOURCE_INCLUDED_FILE) {
		return tok_str(tok->included_file_og_tok);
	}
	assert(false);
	return {};
}

UnicodeString tok_str_content(Token* tok) {
	assert(tok->kind == PREP_TOKEN_KIND_STRING);
	return tok_str(tok)[{1, -1}];
}

void push_source_file_token(PrepFileSource* file, Token* tok) {
	tok->file_tok_idx = len(file->tokens);
	add(&file->tokens, tok);
}

Error* tokenize(Prep* p, PrepFileSource* file) {
	if (file->is_tokenized) {
		return NULL;
	}
	file->is_tokenized = true;
	file->src = copy_string(p->allocator, file->og_src);
	prep_remove_file_splices(p, file);
	prep_remove_comments(p, file);
	s64 cursor = 0;
	while (true) {
		auto [tok, e] = prep_file_next_token(p, file, &cursor);
		if (e) {
			return e;
		}
		if (!tok) {
			break;
		}
		push_source_file_token(file, tok);
	}
	if (len(file->tokens) == 0 || file->tokens[-1]->kind != PREP_TOKEN_KIND_LINE_BREAK) {
		auto tok = grd_make_file_token(p, PREP_TOKEN_KIND_LINE_BREAK, file, len(file->src), len(file->src));
		tok->flags |= PREP_TOKEN_FLAG_FILE_MISSING_TRAILING_LINE_BREAK;
		push_source_file_token(file, tok);
	}
	auto eof = grd_make_file_token(p, PREP_TOKEN_KIND_EOF, file, len(file->src), len(file->src));
	push_source_file_token(file, eof);
	return NULL;
}

UnicodeString prep_default_resolve_fullpath_hook(Prep* p, UnicodeString path, bool global) {
	return path;
}

PrepFileSource* grd_make_mem_prep_file(Prep* p, UnicodeString src, UnicodeString fullpath) {
	auto* file = grd_make<PrepFileSource>(p->allocator);
	file->og_src = src;
	file->fullpath = fullpath;
	file->comment_mappings.allocator = p->allocator;
	file->splice_mappings.allocator = p->allocator;
	file->tokens.allocator = p->allocator;
	return file;
}

PrepFileSource* prep_default_load_file_hook(Prep* p, UnicodeString fullpath) {
	auto [text, e] = read_text_at_path(p->allocator, fullpath);
	if (e) {
		return NULL;
	}
	auto utf32 = decode_utf8(p->allocator, text);
	text.free();
	return grd_make_mem_prep_file(p, utf32, fullpath);
}

Prep* grd_make_prep(GrdAllocator allocator = c_allocator) {
	allocator = grd_make_sub_allocator(allocator);
	auto p = grd_make<Prep>(allocator);
	p->allocator = allocator;
	p->arena = grd_make_arena_allocator(allocator);
	p->files.allocator = allocator;
	p->tokens.allocator = allocator;
	p->macros.allocator = allocator;
	p->load_file_hook = prep_default_load_file_hook;
	p->resolve_fullpath_hook = prep_default_resolve_fullpath_hook;
	return p;
}

// String is allocated separately from Prep.
AllocatedUnicodeString prep_str(Prep* p) {
	AllocatedUnicodeString str;
	for (auto it: p->tokens) {
		append(&str, tok_str(it));
	}
	return str;
}

s64 get_next_token_idx(Span<Token*> tokens, s64 cursor) {
	while (++cursor < len(tokens)) {
		auto tok = tokens[cursor];
		if (tok->kind != PREP_TOKEN_KIND_SPACE &&
			tok->kind != PREP_TOKEN_KIND_LINE_BREAK
		) {
			return cursor;
		}
	}
	return len(tokens);
}

// After get_next_token() cursor points to the returned token.
Token* get_next_token(Span<Token*> tokens, s64* cursor) {
	*cursor = get_next_token_idx(tokens, *cursor);
	if (*cursor >= len(tokens)) {
		assert(len(tokens) > 0);
		assert(tokens[-1]->kind == PREP_TOKEN_KIND_EOF);
		return tokens[-1];
	}
	return tokens[*cursor];
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
		for (auto idx: grd_range(len(macro->arg_defs))) {
			if (tok_str(macro->arg_defs[idx]) == name) {
				return idx;
			}
		}
	}
	return -1;
}

Error* preprocess_file(Prep* p, PrepFileSource* file);

bool does_include_stack_contain_file(Prep* p, PrepFileSource* file) {
	auto entry = p->include_site;
	while (entry) {
		if (entry->file == file) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

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
				return { grd_make_prep_file_error(p, arg_tok, "Expected % arguments at most for a macro", len(macro->arg_defs)) };
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
			return { grd_make_prep_file_error(p, paren_tok, "Expected % arguments at least for a macro", len(macro->arg_defs)) };
		}
	} else {
		if (len(args) != len(macro->arg_defs)) {
			return { grd_make_prep_file_error(p, paren_tok, "Expected % arguments for a macro", len(macro->arg_defs)) };
		}
	}
	return { NULL, args };
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

PrepMacro* find_macro(Prep* p, UnicodeString name) {
	for (auto it: p->macros) {
		if (tok_str(it->name) == name) {
			return it;
		}
	}
	return NULL;
}

struct TokenSlice {
	Span<Token*> tokens;
	s64          start = 0;
	s64          end = 0;

	Token*& operator[](s64 idx) {
		return tokens[idx + start];
	}

	TokenSlice operator[](Tuple<s64, s64> x) {
		return { .tokens = tokens, .start = x._0 + start, .end = x._1 + start };
	}

	Span<Token*> span() {
		return tokens[{start, end}];
	}
};

s64 len(TokenSlice slice) {
	return slice.end - slice.start;
}

TokenSlice grd_make_tok_slice(Span<Token*> tokens, s64 start, s64 end) {
	return { .tokens = tokens, .start = start, .end = end };
}

TokenSlice grd_make_tok_slice(Span<Token*> tokens) {
	return grd_make_tok_slice(tokens, 0, len(tokens));
}

Optional<TokenSlice> get_arg_tokens(PrepMacro* macro, Span<Token*> tokens, Span<PrepMacroArg> args, UnicodeString name) {
	auto def_idx = find_macro_arg_def_tok_idx(macro, name);
	if (def_idx == -1) {
		return {};
	}
	if (tok_str(macro->arg_defs[def_idx]) == "...") {
		if (def_idx < len(args)) {
			return grd_make_tok_slice(tokens, args[def_idx].start, args[-1].end);
		} else {
			return grd_make_tok_slice(tokens, len(tokens), len(tokens));
		}
	} else {
		auto arg = args[def_idx];
		return grd_make_tok_slice(tokens, arg.start, arg.end);
	}
}

MaybeExpandedMacro maybe_expand_macro(Prep* p, TokenSlice tokens, s64* cursor) {
	auto exp_start = *cursor;
	auto name_tok = tokens[*cursor];
	assert(name_tok->kind == PREP_TOKEN_KIND_IDENT || name_tok->kind == PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
	auto macro = find_macro(p, tok_str(name_tok));
	if (!macro) {
		return { };
	}
	Token* paren_tok = NULL;
	if (!macro->is_object) {
		paren_tok = get_next_token(tokens.span(), cursor);
		if (tok_str(paren_tok) != "(" || paren_tok->file_start != name_tok->file_end) {
			return { };
		}
	}

	if (does_macro_stack_contain_macro(p, macro)) {
		return { };
	}

	auto exp = grd_make<MacroExp>(p->arena);
	exp->macro = macro;
	exp->parent = p->macro_exp;
	exp->body.allocator = p->allocator;
	p->macro_exp = exp;
	grd_defer { p->macro_exp = exp->parent; };

	for (auto tok: macro->tokens) {
		auto nt = grd_make_token(p, tok->kind, PREP_TOKEN_SOURCE_MACRO);
		nt->og_macro_tok = tok;
		nt->og_macro_exp = exp;
		add(&exp->body, nt);
	}

	if (!macro->is_object) {
		auto [e, args] = parse_macro_args(p, macro, tokens.span(), cursor, paren_tok);
		if (e) {
			return { e };
		}
		exp->args = args;
	}
	exp->replaced = copy_array(p->allocator, tokens[{exp_start, *cursor + 1}].span());
	if (!macro->is_object) {
		// Stringize.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_STRINGIZE) {
				auto str_tok = grd_make_token(p, PREP_TOKEN_KIND_STRING, PREP_TOKEN_SOURCE_STRINGIZE);
				str_tok->stringize_exp = exp;
				str_tok->stringize_tok = exp->body[i];
				prep_use_custom_str(p, str_tok);
				append(&str_tok->custom_str, U"\"");
				auto [arg_tokens, found] = get_arg_tokens(macro, tokens.span(), exp->args, tok_str(exp->body[i])[{1, {}}]);
				if (!found) {
					return { grd_make_prep_file_error(p, exp->body[i], "Macro argument is not found for stringizing") };
				}
				for (auto tok: arg_tokens.span()) {
					prep_stringize_tok(str_tok, tok);
				}
				append(&str_tok->custom_str, U"\"");
				exp->body[i] = str_tok;
			}
		}
		// Concat.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_GRD_CONCAT) {
				if (i - 1 < 0 || contains({PREP_TOKEN_KIND_SPACE, PREP_TOKEN_KIND_EOF}, exp->body[i - 1]->kind)) {
					return { grd_make_prep_file_error(p, exp->body[i], "Missing left argument for ## operator") };
				}
				if (i + 1 >= len(exp->body) || contains({PREP_TOKEN_KIND_SPACE, PREP_TOKEN_KIND_EOF}, exp->body[i + 1]->kind)) {
					return { grd_make_prep_file_error(p, exp->body[i], "Missing right argument for ## operator") };
				}
				Token* tok = grd_make_token(p, PREP_TOKEN_KIND_NONE, PREP_TOKEN_SOURCE_GRD_CONCATTED);
				tok->concat_exp = exp;
				prep_use_custom_str(p, tok);
				auto lhs = grd_make_tok_slice(exp->body, i - 1, i);
				auto rhs = grd_make_tok_slice(exp->body, i + 1, i + 2);
				tok->concat_lhs = lhs[0];
				tok->concat_rhs = rhs[0];
				auto [lhs_toks, lhs_found] = get_arg_tokens(macro, tokens.span(), exp->args, tok_str(lhs[0]));
				if (lhs_found) {
					lhs = lhs_toks;
				}
				auto [rhs_toks, rhs_found] = get_arg_tokens(macro, tokens.span(), exp->args, tok_str(rhs[0]));
				if (rhs_found) {
					rhs = rhs_toks;
				}
				TokenSlice   lhs_residue;
				TokenSlice   rhs_residue;
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
					auto e = grd_make_prep_dp_error(p, "Concatenated string '%' doesn't form a valid token", tok_str(tok));
					auto dp = grd_make_detailed_printer(exp->body);
					add(&dp->spans, { i, i + 1, 1 });
					add_dp(e, dp);
					return { e };
				}
				for (auto& it: lhs_residue.span()) {
					auto t = grd_make_token(p, it->kind, PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					it = t;
				}
				for (auto& it: rhs_residue.span()) {
					auto t = grd_make_token(p, it->kind, PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					it = t;
				}
				tok->kind = kind;
				remove_at_index(&exp->body, i - 1, 3);
				add(&exp->body, lhs_residue.span(), i - 1);
				add(&exp->body, tok,         i - 1 + len(lhs_residue));
				add(&exp->body, rhs_residue.span(), i - 1 + len(lhs_residue) + 1);
				i = i - 1 + len(lhs_residue) + len(rhs_residue);
			}
		}
		// Prescan.
		for (s64 i = 0; i < len(exp->body); i++) {
			if (exp->body[i]->kind == PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT) {
				auto [arg_tokens, found] = get_arg_tokens(macro, tokens.span(), exp->args, tok_str(exp->body[i]));
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
						auto t = grd_make_token(p, arg_tokens[arg_cursor]->kind, PREP_TOKEN_SOURCE_PRESCAN_EXP);
						t->prescan_exp = exp;
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

Error* handle_prep_directive(Prep* p, TokenSlice tokens, s64* cursor) {
	auto first_directive_tok = tokens[*cursor];
	if (tok_str(first_directive_tok) == "#include") {
		*cursor += 1;
		s64 start = *cursor;
		s64 end = len(tokens);
		s64 include_trailing_newline_idx = -1;
		while (*cursor < len(tokens)) {
			if (tokens[*cursor]->kind == PREP_TOKEN_KIND_LINE_BREAK ||
				tokens[*cursor]->kind == PREP_TOKEN_KIND_EOF)
			{
				if (tokens[*cursor]->kind == PREP_TOKEN_KIND_LINE_BREAK) {
					include_trailing_newline_idx = *cursor;
				}
				end = *cursor;
				break;
			}
			*cursor += 1;
		}

		Array<Token*> path_toks;
		path_toks.allocator = p->allocator;
		grd_defer { path_toks.free(); };

		s64 exp_cursor = start;
		while (exp_cursor < end) {
			if (tokens[exp_cursor]->kind == PREP_TOKEN_KIND_IDENT) {
				auto exp = maybe_expand_macro(p, tokens, &exp_cursor);
				if (exp.e) {
					return exp.e;
				} else if (exp.exp) {
					add(&path_toks, exp.exp->body);
				} else {
					continue;
				}
			}
			add(&path_toks, tokens[exp_cursor]);
			exp_cursor += 1;
		}

		// Remove preceeding and trailing spaces.
		for (s64 i = 0; i < len(path_toks); i++) {
			if (path_toks[i]->kind != PREP_TOKEN_KIND_SPACE) {
				break;
			}
			remove_at_index(&path_toks, i);
			i -= 1;
		}
		for (s64 i = len(path_toks) - 1; i >= 0; i--) {
			if (path_toks[i]->kind != PREP_TOKEN_KIND_SPACE) {
				break;
			}
			remove_at_index(&path_toks, i);
		}
		if (len(path_toks) == 0) {
			return grd_make_prep_file_error(p, first_directive_tok, "Empty #include path");
		}
		UnicodeString path;
		bool          is_global = false;
		if (path_toks[0]->kind == PREP_TOKEN_KIND_STRING) {
			for (auto i: range_from_to(1, len(path_toks) - 1)) {
				if (path_toks[i]->kind != PREP_TOKEN_KIND_SPACE) {
					return grd_make_prep_file_error(p, path_toks[i], "Unexpected token after #include path");
				}
			}
			path = tok_str_content(path_toks[0]);
		} else {
			// Check for starting < and trailing >
			if (tok_str(path_toks[0]) != "<") {
				return grd_make_prep_file_error(p, path_toks[0], "Expected '<' at the start of #include path");
			}
			if (tok_str(path_toks[-1]) != ">") {
				return grd_make_prep_file_error(p, path_toks[-1], "Expected '>' at the end of #include path");
			}
			remove_at_index(&path_toks, 0);
			remove_at_index(&path_toks, -1);
			AllocatedUnicodeString composed_include_path;
			composed_include_path.allocator = p->allocator;
			for (auto tok: path_toks) {
				append(&composed_include_path, tok_str(tok));
			}
			path = composed_include_path;
			is_global = true;
		}

		auto fullpath = p->resolve_fullpath_hook(p, path, is_global);
		if (fullpath == "") {
			if (!is_global) {
				return grd_make_prep_file_error(p, first_directive_tok, "Failed to resolve fullpath for #include \"%\"", path);
			} else {
				return grd_make_prep_file_error(p, first_directive_tok, "Failed to resolve fullpath for #include <%>", path);
			}
		}
		PrepFileSource* source = NULL;
		for (auto loaded: p->files) {
			if (loaded->fullpath == fullpath) {
				source = loaded;
				break;
			}
		}
		if (!source) {
			source = p->load_file_hook(p, fullpath);
			if (!source) {
				return grd_make_prep_file_error(p, first_directive_tok, "Failed to find #include file with fullpath '%'", fullpath);
			}
			add(&p->files, source);
		}
		if (!does_include_stack_contain_file(p, source)) {
// S			inc_file->site_toks = copy_array(p->allocator, path_toks);
// 			path_toks = {};
			auto e = preprocess_file(p, source);
			if (e) {
				return e;
			}
		}
		*cursor = end - 1;
		// Eat line break if present, but don't eat EOF.
		// *cursor = include_trailing_newline_idx == -1 ? end - 1 : end;
	} else if (tok_str(first_directive_tok) == "#define") {
		s64 start_tok_idx = *cursor;
		auto ident_tok = get_next_token(tokens.span(), cursor);
		if (ident_tok->kind != PREP_TOKEN_KIND_IDENT) {
			return grd_make_prep_file_error(p, ident_tok, "Expected an identifier after #define");
		}
		auto macro = grd_make<PrepMacro>(p->allocator);
		macro->def_site = p->include_site;
		macro->start_tok_idx = start_tok_idx;
		macro->name = ident_tok;
		add(&p->macros, macro);

		s64 first_macro_body_tok = *cursor + 1;
		if (first_macro_body_tok < len(tokens) &&
			tok_str(tokens[first_macro_body_tok]) == "(" &&
			tokens[first_macro_body_tok]->file_start == ident_tok->file_end)
		{
			*cursor += 1;
			macro->is_object = false;
			while (true) {
				auto arg_tok = get_next_token(tokens.span(), cursor);
				if (tok_str(arg_tok) == ")") {
					*cursor += 1;
					break;
				}
				if (len(macro->arg_defs) >= 1) {
					if (tok_str(arg_tok) != ",") {
						return grd_make_prep_file_error(p, arg_tok, "Expected ',' after macro argument");
					}
					arg_tok = get_next_token(tokens.span(), cursor);
				}
				if (arg_tok->kind != PREP_TOKEN_KIND_IDENT) {
					return grd_make_prep_file_error(p, arg_tok, "Expected an identifier as a macro argument");
				}
				for (auto it: macro->arg_defs) {
					if (tok_str(it) == tok_str(arg_tok)) {
						return grd_make_prep_file_error(p, arg_tok, "Duplicate macro argument");
					}
				}
				if (len(macro->arg_defs) >= 1 && tok_str(macro->arg_defs[-1]) == "...") {
					return grd_make_prep_file_error(p, arg_tok, "Unexpected macro argument after '...'");
				}
				add(&macro->arg_defs, arg_tok);
			}
			first_macro_body_tok = *cursor;
		}
		while (*cursor < len(tokens)) {
			auto macro_tok = tokens[*cursor];
			if (macro_tok->kind == PREP_TOKEN_KIND_EOF || macro_tok->kind == PREP_TOKEN_KIND_LINE_BREAK) {
				break;
			} else if (macro_tok->kind == PREP_TOKEN_KIND_STRINGIZE) {
				auto def_tok_idx = find_macro_arg_def_tok_idx(macro, tok_str(macro_tok)[{1, {}}]);
				if (def_tok_idx == -1) {
					return grd_make_prep_file_error(p, macro_tok, "Macro argument is not found for stringizing");
				}
			} else if (macro_tok->kind == PREP_TOKEN_KIND_DIRECTIVE) {
				return grd_make_prep_file_error(p, macro_tok, "Unexpected preprocessor directive in a macro");
			}
			assert(macro_tok->kind != PREP_TOKEN_KIND_NONE);
			*cursor += 1;
		}
		macro->end_tok_idx = *cursor;
		macro->tokens = copy_array(p->allocator, tokens[{first_macro_body_tok, *cursor}].span());
		for (auto& it: macro->tokens) {
			if (it->kind == PREP_TOKEN_KIND_IDENT) {
				it->kind = PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT;
			}
		}
		LogTrace("Expanding macro {%}, tokens:", tok_str(macro->name));
		for (auto it: macro->tokens) {
			LogTrace("  %, %: %, %", tok_str(it), it->file_start, it->file_end, it->kind);
		}
	} else {
		return grd_make_prep_file_error(p, first_directive_tok, "Unknown preprocessor directive '%'", tok_str(first_directive_tok));
	}
	return NULL;
}

Error* preprocess_file(Prep* p, PrepFileSource* source_file) {
	auto e = tokenize(p, source_file);
	if (e) {
		return e;
	}
	auto included = grd_make<IncludedFile>(p->arena);
	included->file = source_file;
	included->parent = p->include_site;
	p->include_site = included;
	// @TODO: set include site tokens.
	grd_defer { p->include_site = included->parent; };
	included->tokens.allocator = p->allocator;
	for (auto tok: source_file->tokens) {
		auto nt = grd_make_token(p, tok->kind, PREP_TOKEN_SOURCE_INCLUDED_FILE);
		nt->included_file = included;
		nt->included_file_og_tok = tok;
		add(&included->tokens, nt);
	}
	//
	s64 cursor = -1;
	while (++cursor < len(included->tokens)) {
		auto tok = included->tokens[cursor];
		if (tok->kind == PREP_TOKEN_KIND_DIRECTIVE) {
			auto e = handle_prep_directive(p, grd_make_tok_slice(included->tokens), &cursor);
			if (e) {
				return e;
			}
			continue;
		}
		if (tok->kind == PREP_TOKEN_KIND_IDENT) {
			auto mb = maybe_expand_macro(p, grd_make_tok_slice(included->tokens), &cursor);
			if (mb.e) {
				return mb.e;
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

Tuple<Error*, Prep*> preprocess(UnicodeString str) {
	auto p = grd_make_prep();
	auto file = grd_make_mem_prep_file(p, str, U"<root_file>"_b);
	auto e = preprocess_file(p, file);
	return { e, p };
}

Tuple<Error*, Prep*, UnicodeString> preprocess_to_string(UnicodeString str) {
	auto [e, p] = preprocess(str);
	if (e) {
		return { e };
	}
	return { NULL, p, prep_str(p) };
}
