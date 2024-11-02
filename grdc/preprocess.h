#pragma once

#include "../grd_string.h"
#include "../grd_defer.h"
#include "../grd_format.h"
#include "../grd_error.h"
#include "../grd_panic.h"
#include "../grd_file.h"
#include "../grd_log.h"
#include "../grd_arena_allocator.h"
#include "../grd_sub_allocator.h"
#include "../grd_assert.h"

enum GrdcPrepTokenKind {
	GRDC_PREP_TOKEN_KIND_NONE,
	GRDC_PREP_TOKEN_KIND_EOF,
	GRDC_PREP_TOKEN_KIND_LINE_BREAK,
	GRDC_PREP_TOKEN_KIND_NUMBER,
	GRDC_PREP_TOKEN_KIND_DIRECTIVE,
	GRDC_PREP_TOKEN_KIND_STRING,
	GRDC_PREP_TOKEN_KIND_IDENT,
	GRDC_PREP_TOKEN_KIND_SPACE,
	GRDC_PREP_TOKEN_KIND_PUNCT,
	GRDC_PREP_TOKEN_KIND_GRD_CONCAT,
	GRDC_PREP_TOKEN_KIND_GRD_CONCATTED,
	GRDC_PREP_TOKEN_KIND_STRINGIZE,
	GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT,
};
GRD_REFLECT(GrdcPrepTokenKind) {
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_NONE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_EOF);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_LINE_BREAK);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_NUMBER);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_DIRECTIVE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_STRING);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_IDENT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_SPACE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_PUNCT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_GRD_CONCAT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_GRD_CONCATTED);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_STRINGIZE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
}

struct PrepFileMapping {
	s64 start;
	s64 real_start;
	s64 length;
};

struct GrdcMacroExp;

enum GrdcPrepTokenFlags {
	GRDC_PREP_TOKEN_FLAG_NONE = 0,
	GRDC_PREP_TOKEN_FLAG_CUSTOM_STR = 1 << 0,
	GRDC_PREP_TOKEN_FLAG_FILE_MISSING_TRAILING_LINE_BREAK = 1 << 1,
};

enum GrdcPrepTokenSourceKind {
	GRDC_PREP_TOKEN_SOURCE_NONE = 0,
	GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE = 1,
	GRDC_PREP_TOKEN_SOURCE_STRINGIZE = 2,
	GRDC_PREP_TOKEN_SOURCE_MACRO = 3,
	GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED = 4,
	GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE = 5,
	GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP = 6,
	GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE = 7,
};
GRD_REFLECT(GrdcPrepTokenSourceKind) {
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_NONE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_STRINGIZE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_MACRO);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE);
}

struct GrdcIncludedFile;
struct GrdcPrepFileSource;

struct GrdcToken {
	GrdcPrepTokenKind          kind  = GRDC_PREP_TOKEN_KIND_NONE;
	s64                    flags = GRDC_PREP_TOKEN_FLAG_NONE;
	GrdcPrepTokenSourceKind    src_kind = GRDC_PREP_TOKEN_SOURCE_NONE;
	GrdAllocatedUnicodeString custom_str;
	GrdcPrepFileSource*        file_source = NULL;
	s64                    file_tok_idx = 0;
	GrdcIncludedFile*          included_file = NULL;
	s64                    file_start = 0;
	s64                    file_end = 0;
	GrdcMacroExp*              stringize_exp = NULL;
	GrdcToken*                 stringize_tok = NULL;
	GrdcMacroExp*              og_macro_exp = NULL;
	GrdcToken*                 og_macro_tok = NULL;
	GrdcToken*                 concat_lhs = NULL;
	GrdcToken*                 concat_rhs = NULL;
	GrdcToken*                 concat_lhs_content = NULL;
	GrdcToken*                 concat_rhs_content = NULL;
	GrdcToken*                 concat_residue_og = NULL;
	GrdcMacroExp*              concat_exp = NULL;
	GrdcToken*                 prescan_exp_og = NULL;
	GrdcMacroExp*              prescan_exp = NULL;
	GrdcToken*                 included_file_og_tok = NULL;
};

struct GrdcIncludedFile {
	GrdcIncludedFile*      parent = NULL;
	GrdArray<GrdcToken*>   site_toks;
	GrdcPrepFileSource*    file = NULL;
	GrdArray<GrdcToken*>   tokens;
};

// @TODO: rename back to GrdcPrepFile.
struct GrdcPrepFileSource {
	GrdUnicodeString          fullpath;
	GrdUnicodeString          og_src;
	GrdUnicodeString          src;
	GrdArray<PrepFileMapping> splice_mappings;
	GrdArray<PrepFileMapping> comment_mappings;
	GrdArray<GrdcToken*>      tokens;
	bool                      is_tokenized = false;
};

struct GrdcPrepMacro {
	GrdcIncludedFile*    def_site = NULL;
	s64                  start_tok_idx = 0;
	s64                  end_tok_idx = 0;
	GrdcToken*           name;
	GrdArray<GrdcToken*> arg_defs;
	GrdSpan<GrdcToken*>  tokens;
	bool                 is_object = true;
};

struct GrdcPrepMacroArg {
	s64 def_tok_idx;
	s64 start;
	s64 end;
};

struct GrdcMacroExp {
	GrdcMacroExp*     parent = NULL;
	GrdcPrepMacro*    macro = NULL;
	GrdArray<GrdcToken*> body;
	GrdArray<GrdcToken*> replaced;
	GrdArray<GrdcPrepMacroArg> args;
};

struct GrdcPrep {
	GrdAllocator               allocator;
	GrdAllocator               arena;
	GrdArray<GrdcToken*>       tokens;
	GrdArray<GrdcPrepFileSource*>  files;
	GrdArray<GrdcPrepMacro*>       macros;
	GrdcMacroExp*              macro_exp = NULL;
	GrdcIncludedFile*              include_site = NULL;
	void*                      aux_data = NULL;
	GrdUnicodeString         (*resolve_fullpath_hook) (GrdcPrep* p, GrdUnicodeString path, bool global) = NULL;
	GrdcPrepFileSource*          (*load_file_hook)        (GrdcPrep* p, GrdUnicodeString fullpath) = NULL;
};

struct GrdcPrepFileError: GrdError {
	GrdcPrep*        prep = NULL;
	GrdcPrepFileSource* file = NULL;
	s64             start = 0;
	s64             end   = 0;
};

struct GrdcPrepTokenError: GrdError {
	GrdcPrep*  prep = NULL;
	GrdcToken* tok = NULL;
};

GrdcPrepFileError* grdc_make_prep_file_error(GrdcPrep* p, GrdcPrepFileSource* file, s64 start, s64 end, auto... args) {
	auto msg = grd_sprint(p->allocator, args...);
	auto e = grd_make_error<GrdcPrepFileError>(msg);
	e->prep = p;
	e->file = file;
	e->start = start;
	e->end = end;
	return e;
}

GrdcToken* grdc_file_tok(GrdcToken* tok);

GrdcPrepTokenError* grdc_make_prep_file_error(GrdcPrep* p, GrdcToken* tok, auto... args) {
	auto msg = grd_sprint(p->allocator, args...);
	auto e = grd_make_error<GrdcPrepTokenError>(msg);
	e->prep = p;
	e->tok = tok;
	return e;
}

s64 grdc_map_index(GrdSpan<PrepFileMapping> mappings, s64 index) {
	for (auto it: mappings) {
		if (index >= it.start && index < it.start + it.length) {
			return index - it.start + it.real_start;
		}
	}
	return mappings[-1].real_start + mappings[-1].length;
}

s64 grdc_og_file_index(GrdcPrepFileSource* source, s64 index) {
	index = grdc_map_index(source->comment_mappings, index);
	index = grdc_map_index(source->splice_mappings, index);
	return index;
}

s64 grdc_get_residual_lines_above(GrdUnicodeString src, s64 anchor, s64 lines_count) {
	s64 count = 0;
	for (auto i: grd_reverse(grd_range(anchor))) {
		if (grd_is_line_break(src[i])) {
			count += 1;
			if (count >= lines_count) {
				return i + 1;
			}
		}
	}
	return 0;
}

s64 grdc_get_residual_lines_below(GrdUnicodeString src, s64 anchor, s64 lines_count) {
	s64 count = 0;
	for (auto i: grd_range_from_to(anchor, grd_len(src))) {
		if (grd_is_line_break(src[i])) {
			count += 1;
			if (count >= lines_count) {
				return i + 1;
			}
		}
	}
	return grd_len(src);
}

GrdUnicodeString grdc_tok_str(GrdcToken* tok);

void grdc_print_file_range_highlighted(GrdcPrepFileSource* file, s64 h_start, s64 h_end, s64 color) {
	s64 mapped_start = grdc_og_file_index(file, h_start);
	s64 mapped_end = grdc_og_file_index(file, h_end);
	s64 start = grdc_get_residual_lines_above(file->og_src, mapped_start, 3);
	s64 end = grdc_get_residual_lines_below(file->og_src, mapped_end, 3);
	grd_println();
	grd_print(file->og_src[{start, mapped_start}]);
	grd_print(U"\x1b[0;%m", 30 + color);
	// Length is 0, but we have to print something to see where the error is.
	if (mapped_start == mapped_end) {
		grd_print(U"\x1b[%m", 40 + color);
		grd_print(" ");
		grd_print(U"\x1b[49m");
	}
	for (auto c: file->og_src[{mapped_start, mapped_end}]) {
		if (grd_is_whitespace(c)) {
			grd_print(U"\x1b[%m", 40 + color);
			grd_print(c);
			grd_print(U"\x1b[49m");
		} else {
			grd_print(c);
		}
	}
	grd_print(U"\x1b[0m");
	grd_print(file->og_src[{mapped_end, end}]);
	grd_println();
}

void grdc_print_prep_token_error(GrdcToken* tok, s64 color) {
	assert(tok->src_kind == GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	auto file = tok->file_source;
	grdc_print_file_range_highlighted(file, tok->file_start, tok->file_end, color);
}

void grdc_print_single_error_token(GrdcToken* tok) {
	switch (tok->src_kind) {
		case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE: {
			grd_println("  at file: %", tok->file_source->fullpath);
			grdc_print_prep_token_error(tok, 1);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			grdc_print_single_error_token(tok->og_macro_tok);
			grd_println("  expanded from macro: %", grdc_tok_str(tok->og_macro_exp->macro->name));
			grd_println("  which is defined in file: %", tok->og_macro_exp->macro->def_site->file->fullpath);
			grdc_print_prep_token_error(tok->og_macro_exp->macro->tokens[0], 1);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			grd_println("  at concatenated token: %", grdc_tok_str(tok));
			grd_println("lhs token: ");
			grdc_print_single_error_token(tok->concat_lhs);
			grd_println("rhs token: ");
			grdc_print_single_error_token(tok->concat_rhs);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			grd_println("  at included file: %", tok->included_file->file->fullpath);
			grdc_print_prep_token_error(tok->included_file_og_tok, 1);
			break;
		}
		default: {
			grd_println("Unknown token source kind: %", tok->src_kind);
			grd_assert(false);
		}
	}
}

void grdc_print_debug_tok(GrdcToken* tok) {
	grd_println("tok: %, \"%\"", tok->kind, grdc_tok_str(tok));
}


struct GrdcTokenSpan {
	GrdSpan<GrdcToken*> tokens;
	s64                 color = 0;
};

struct GrdcDumpedFileRanges {
	GrdArray<GrdTuple<s64, s64>> ranges;
};

struct GrdcTokenDumperFile {
	GrdcIncludedFile* file = NULL;
	GrdHashMap<s64, GrdcDumpedFileRanges> color_ranges;
};

struct GrdcTokenDumper {
	GrdArray<GrdcTokenDumperFile> files;
};

void grdc_print_token_dumper(GrdcTokenDumper* dumper) {
	for (auto& it: dumper->files) {
		grd_println("File: %", it.file->file->fullpath);
		auto parent = it.file->parent;
		if (parent) {
			grd_println("  Included from: ");
		}
		while (parent) {
			grd_println("    %", parent->file->fullpath);
			parent = parent->parent;
		}
		for (auto entry: it.color_ranges) {
			for (auto range: entry->value.ranges) {
				grdc_print_file_range_highlighted(it.file->file, range._0, range._1, entry->key);
			}
		}
	}
}

void grdc_dump_file_range(GrdcTokenDumper* dumper, GrdcIncludedFile* file, s64 start, s64 end, s64 color) {
	// grd_println("grdc_dump_file_range(): %, (%, %)", file->fullpath, start, end);
	GrdcTokenDumperFile* df = NULL;
	for (auto& it: dumper->files) {
		if (it.file == file) {
			df = &it;
			break;
		}
	}
	if (df == NULL) {
		df = grd_add(&dumper->files, { file });
	}

	GrdcDumpedFileRanges* dfr = grd_get(&df->color_ranges, color);
	if (!dfr) {
		dfr = grd_put(&df->color_ranges, color, {});
	}

	grd_defer { 
		for (auto it: dfr->ranges) {
			// grd_println("  %, %", it._0, it._1);
		}
	};

	s64 idx = 0;
	for (auto& it: dfr->ranges) {
		bool intersects_or_touches = grd_max(start, it._0) <= grd_min(end, it._1);
		if (intersects_or_touches) {
			it._0 = grd_min(it._0, start);
			it._1 = grd_max(it._1, end);
			// Coalesce following.
			for (s64 i = idx + 1; i < grd_len(dfr->ranges); i++) {
				bool intersects_or_touches = grd_max(dfr->ranges[idx]._0, dfr->ranges[i]._0) <= grd_min(dfr->ranges[idx]._1, dfr->ranges[i]._1);
				if (intersects_or_touches) {
					dfr->ranges[idx]._0 = grd_min(dfr->ranges[i]._0, dfr->ranges[idx]._0);
					dfr->ranges[idx]._1 = grd_max(dfr->ranges[i]._1, dfr->ranges[idx]._1);
					grd_remove(&dfr->ranges, i);
					i -= 1;
				}
			}
			return;
		}
		idx += 1;
	}
	grd_add(&dfr->ranges, { start, end });
}

GrdcMacroExp* grdc_get_root_macro_exp(GrdcMacroExp* exp) {
	while (exp->parent) {
		exp = exp->parent;
	}
	return exp;
}

void grdc_dump_token(GrdcTokenDumper* dumper, GrdcToken* tok, s64 color) {
	switch (tok->src_kind) {
		// case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE:{
		// 	grdc_dump_file_range(dumper, tok->file_source, tok->file_start, tok->file_end, color);
		// }
		// break;
		case GRDC_PREP_TOKEN_SOURCE_STRINGIZE: {
			auto exp = grdc_get_root_macro_exp(tok->stringize_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				grdc_dump_token(dumper, it, color);
			}
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			auto exp = grdc_get_root_macro_exp(tok->og_macro_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				grdc_dump_token(dumper, it, color);
			}
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			auto exp = grdc_get_root_macro_exp(tok->concat_exp);
			// Lazy as fuck.
			for (auto it: exp->replaced) {
				grdc_dump_token(dumper, it, color);
			}
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE: {
			auto exp = grdc_get_root_macro_exp(tok->concat_exp);
			for (auto it: exp->replaced) {
				grdc_dump_token(dumper, it, color);
			}
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP: {
			auto exp = grdc_get_root_macro_exp(tok->prescan_exp);
			for (auto it: exp->replaced) {
				grdc_dump_token(dumper, it, color);
			}
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			grdc_dump_file_range(dumper, tok->included_file, tok->included_file_og_tok->file_start, tok->included_file_og_tok->file_end, color);
			// grdc_dump_token(dumper, tok->included_file_og_tok, color);
		}
		break;
		default: {
			grd_panic("Unknown token source kind: %", tok->src_kind);
		}
	}
}

struct GrdcDpSpan {
	s64 start;
	s64 end;
	s64 color;
};

struct GrdcDetailedPrinter {
	GrdSpan<GrdcToken*>  tokens;
	GrdArray<GrdcDpSpan> spans;
};

bool grdc_do_token_sources_match(GrdcToken* a, GrdcToken* b) {
	if (a->src_kind != b->src_kind) {
		return false;
	}
	switch (a->src_kind) {
		case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE: {
			return a->file_source == b->file_source;
		}
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			return a->og_macro_exp == b->og_macro_exp;
		}
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			return a->concat_exp == b->concat_exp;
		}
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE: {
			return a->concat_exp == b->concat_exp;
		}
		case GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP: {
			return a->prescan_exp == b->prescan_exp;
		}
		case GRDC_PREP_TOKEN_SOURCE_STRINGIZE: {
			return a->stringize_exp == b->stringize_exp;
		}
		case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			return a->included_file == b->included_file;
		}
		default: {
			grd_panic("Unknown token source kind: %", a->src_kind);
			return false;
		}
	}
}

GrdcDetailedPrinter* grdc_make_detailed_printer(GrdSpan<GrdcToken*> tokens) {
	auto dp = grd_make<GrdcDetailedPrinter>();
	dp->tokens = tokens;
	return dp;
}

s64 grdc_get_residual_tokens(GrdSpan<GrdcToken*> tokens, s64 cursor, s64 dir) {
	s64 lines = 0;
	bool fwd = dir >= 0;
	dir = fwd ? 1 : -1;
	for (s64 i = cursor + dir; (i >= 0 && i < grd_len(tokens)); i += dir) {
		if (tokens[i]->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
			lines += 1;
			if (lines >= 3) {
				return i + 1;
			}
		}
	}
	return fwd ? grd_len(tokens) : 0;
}

void grdc_do_detailed_print(GrdcDetailedPrinter* dp, bool expand_site) {
	if (grd_len(dp->spans) == 0) {
		grd_println("Empty DP");
	}

	grd_sort(dp->spans, [] (auto spans, s64 a, s64 b) {
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

	global_start = grdc_get_residual_tokens(dp->tokens, global_start, -1);
	global_end   = grdc_get_residual_tokens(dp->tokens, global_end, 1);

	GrdArray<GrdTuple<s64, s64, bool, s64>> regs;
	grd_add(&regs, { global_start, -1, true });
	for (auto idx: grd_range_from_to(global_start, global_end)) {
		bool match = grdc_do_token_sources_match(dp->tokens[idx], dp->tokens[idx - 1]);
		if (!match) {
			regs[-1]._1 = idx;
			grd_add(&regs, { idx, -1, true });
		}
	}
	regs[-1]._1 = global_end;

	for (s64 idx = 0; idx < grd_len(regs); idx++) {
		if (regs[idx]._0 == regs[idx]._1) {
			grd_remove(&regs, idx);
			idx -= 1;
			continue;
		}
		// auto tok = dp->tokens[regs[idx]._1];
		// if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE) {
		// 	if (tok->included_file == first_file) {
		// 		grd_remove(&regs, idx);
		// 		idx -= 1;
		// 	}
		// }
	}

	for (s64 i = 0; i < grd_len(regs); i++) {
		s64 left_end = i - 1 >= 0 ? regs[i - 1]._1 : 0;
		if (left_end < regs[i]._0) {
			grd_add(&regs, { left_end, regs[i]._0, false }, i);
			i += 1;
		}
		s64 right_start = i + 1 < grd_len(regs) ? regs[i + 1]._0 : grd_len(dp->tokens);
		if (regs[i]._1 < right_start) {
			grd_add(&regs, { regs[i]._1, right_start, false }, i + 1);
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
	// 	grd_println("reg: %, %, %", it._0, it._1, it._2);
	// }

	// if (first_file) {
	// 	// @TODO: dump include path.
	// 	grd_println("  Path: %", first_file->file->fullpath);
	// }

	GrdAllocatedUnicodeString line_double;
	grd_defer { line_double.free(); };
	for (auto [start, end, need_exp, zone]: regs) {
		s64 printed_zone_len = 0;
		auto get_zone_char = [&] () -> char {
			auto zone_num = grd_to_string(zone);
			if (printed_zone_len < grd_len(zone_num)) {
				return zone_num[printed_zone_len++];
			} else {
				return '~';
			}
		};
		for (s64 idx: grd_range_from_to(start, end)) {
			for (auto span: dp->spans) {
				if (idx >= span.start && idx < span.end) {
					grd_print("\x1b[0;%m", 30 + span.color);
				}
			}
			auto it = dp->tokens[idx];
			if (it->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
				grd_println();
				if (expand_site) {
					grd_print(line_double);
					grd_clear(&line_double);
					grd_println();
				}
				continue;
			}
			if (need_exp) {
				grd_print(grdc_tok_str(it));
				for (auto i: grd_range(grd_len(grdc_tok_str(it)))) grd_add(&line_double, get_zone_char());
			} else {
				grd_print(grdc_tok_str(it));
				for (auto i: grd_range(grd_len(grdc_tok_str(it)))) grd_add(&line_double, ' ');
			}
			grd_print("\x1b[0m");
		}
	}
	if (expand_site) {
		grd_println();
		grd_print(line_double);
		grd_println();
	}
	grd_println();

	if (expand_site) {
		for (auto [start, end, need_exp, zone]: regs) {
			if (end <= start) {
				continue;
			}
			GrdcToken* tok = dp->tokens[start];
			grd_println("Site %:", zone);
			void grdc_print_dp_site(GrdcToken* tok);
			grdc_print_dp_site(tok);
		}
	}	
}

void grdc_print_include_site(GrdcIncludedFile* inc) {
	while (inc) {
		grd_println("%", inc->file->fullpath);
		inc = inc->parent;
	}
}

void grdc_print_dp_site(GrdcToken* tok) {
	switch (tok->src_kind) {
		case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE: {
			auto dp = grdc_make_detailed_printer(tok->file_source->tokens);
			grd_add(&dp->spans, { tok->file_tok_idx, tok->file_tok_idx + 1, 1 });
			grdc_do_detailed_print(dp, false);
		}
		break;
		// case GRDC_PREP_TOKEN_SOURCE_STRINGIZE
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			auto macro = tok->og_macro_exp->macro;
			grd_println("Expanded from macro: %", grdc_tok_str(macro->name));
			grd_println("Defined in:");
			grdc_print_include_site(macro->def_site);
			grd_println();
			auto dp = grdc_make_detailed_printer(macro->def_site->tokens);
			grd_add(&dp->spans, { macro->start_tok_idx, macro->end_tok_idx, 1 });
			grdc_do_detailed_print(dp, false);
		}
		break;
		// case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED
		// case GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE
		// case GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP
		// case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE

		default: {
			grd_panic("Unknown token source kind: %", tok->src_kind);
		}
	}
}

struct GrdcPrepDpErrorNode {
	GrdcDetailedPrinter* dp = NULL;
	GrdUnicodeString    str;
};

struct GrdcPrepDetailedError: GrdError {
	GrdArray<GrdcPrepDpErrorNode> nodes;
};

GrdcPrepDetailedError* grdc_make_prep_dp_error(GrdcPrep* p, auto... args) {
	auto msg = grd_sprint(args...);
	auto e = grd_make_error<GrdcPrepDetailedError>(msg);
	return e;
}

void grdc_add_dp(GrdcPrepDetailedError* e, GrdcDetailedPrinter* dp) {
	grd_add(&e->nodes, { dp });
}

void grdc_add_message(GrdcPrepDetailedError* e, auto... args) {
	auto msg = grd_sprint(args...);
	grd_add(&e->nodes, { msg });
}

void grdc_print_token_spans(GrdcTokenDumper* dumper, GrdSpan<GrdcTokenSpan> spans) {
	for (auto it: spans) {
		for (auto tok: it.tokens) {
			grdc_dump_token(dumper, tok, it.color);
		}
	}
}

void grdc_print_prep_error(GrdError* e) {
	grd_println();
	grd_println("Error: %", e->text);
	if (auto x = grd_reflect_cast<GrdcPrepFileError>(e)) {
		grd_println();
		grd_println("   at %", x->file->fullpath);
		grdc_print_file_range_highlighted(x->file, x->start, x->end, 1);
		grd_println();
		grd_println();
	} else if (auto x = grd_reflect_cast<GrdcPrepTokenError>(e)) {
		grd_println();
		GrdcTokenDumper dumper;
		grdc_dump_token(&dumper, x->tok, 1);
		grdc_print_token_dumper(&dumper);
		// grdc_print_single_error_token(x->tok);
	} else if (auto x = grd_reflect_cast<GrdcPrepDetailedError>(e)) {
		for (auto node: x->nodes) {
			if (node.dp) {
				grdc_do_detailed_print(node.dp, true);
			} else {
				grd_println(node.str);
			}
		}
	} else {
		grd_println(e);
	}
}

GrdcToken* grdc_make_token(GrdcPrep* p, GrdcPrepTokenKind kind, GrdcPrepTokenSourceKind src_kind) {
	auto* tok = grd_make<GrdcToken>(p->arena);
	tok->kind = kind;
	tok->custom_str.allocator = null_allocator;
	tok->src_kind = src_kind;
	return tok;
}

GrdcToken* grdc_make_file_token(GrdcPrep* p, GrdcPrepTokenKind kind, GrdcPrepFileSource* file, s64 file_start, s64 file_end) {
	auto tok = grdc_make_token(p, kind, GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	tok->file_source = file;
	tok->file_start = file_start;
	tok->file_end = file_end;
	return tok;
}

void grdc_prep_use_custom_str(GrdcPrep* p, GrdcToken* tok) {
	tok->flags |= GRDC_PREP_TOKEN_FLAG_CUSTOM_STR;
	tok->custom_str.allocator = p->allocator;
}

void grdc_prep_push_mapping(GrdArray<PrepFileMapping>* mappings, GrdcPrep* p, GrdcPrepFileSource* file, s64* cursor, s64 end, s64 rm_len, s64* removed_len, s64* i) {
	PrepFileMapping mapping;
	mapping.start = *cursor - *removed_len;
	mapping.real_start = *cursor;
	mapping.length = end - *cursor;
	grd_add(mappings, mapping);
	*removed_len += rm_len;
	*cursor = end;
	if (i) {
		*i = end - 1;
	}
	grd_remove(&file->src, end, rm_len);
}

void grdc_prep_remove_file_splices(GrdcPrep* p, GrdcPrepFileSource* file) {
	s64 cursor = 0;
	s64 removed_len = 0;
	for (s64 i = 0; i < grd_len(file->src); i++) {
		if (file->src[i] == '\\') {
			auto lb_len = grd_get_line_break_len(file->src, i + 1);
			if (lb_len > 0) {
				grdc_prep_push_mapping(&file->splice_mappings, p, file, &cursor, i, lb_len + 1, &removed_len, &i);
			}
		}
	}
	grdc_prep_push_mapping(&file->splice_mappings, p, file, &cursor, grd_len(file->src), 0, &removed_len, NULL);
}

void grdc_prep_remove_comments(GrdcPrep* p, GrdcPrepFileSource* file) {
	s64 cursor = 0;
	s64 removed_len = 0;
	for (s64 i = 0; i < grd_len(file->src); i++) {
		if (grd_starts_with(file->src[{i, {}}], "//")) {
			s64 comment_end = -1;
			for (s64 j: grd_range_from_to(i, grd_len(file->src))) {
				auto lb_len = grd_get_line_break_len(file->src, j);
				if (lb_len > 0) {
					comment_end = j;
					break;	
				}
			}
			if (comment_end == -1) {
				comment_end = grd_len(file->src);
			}
			grdc_prep_push_mapping(&file->comment_mappings, p, file, &cursor, i, comment_end - i, &removed_len, &i);
		}

		if (grd_starts_with(file->src[{i, {}}], "/*")) {
			s64 level = 0;
			s64 comment_end = -1;
			for (s64 j: grd_range_from_to(i + 2, grd_len(file->src))) {
				if (grd_starts_with(file->src[{j, {}}], "*/")) {
					level -= 1;
					if (level == 0) {
						comment_end = j + 2;
						break;
					}
				} else if (grd_starts_with(file->src[{j, {}}], "/*")) {
					level += 1;
				}
			}
			if (comment_end == -1) {
				comment_end = grd_len(file->src);
			}
			grdc_prep_push_mapping(&file->comment_mappings, p, file, &cursor, i, comment_end - i, &removed_len, &i);
		}
	}
	grdc_prep_push_mapping(&file->comment_mappings, p, file, &cursor, grd_len(file->src), 0, &removed_len, NULL);
}

s64 grdc_prep_maybe_number_tok(GrdUnicodeString src, s64 start) {
	bool got_dec_digit = false;
	for (s64 i = start; i < grd_len(src); i++) {
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
		GrdUnicodeString rem = src[{i, {}}];
		if (
			grd_starts_with(rem, "e+") || grd_starts_with(rem, "e-") ||
			grd_starts_with(rem, "E+") || grd_starts_with(rem, "E-") ||
			grd_starts_with(rem, "p+") || grd_starts_with(rem, "p-") ||
			grd_starts_with(rem, "P+") || grd_starts_with(rem, "P-")
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
	return grd_len(src);
}

s64 grdc_prep_maybe_string_tok(GrdUnicodeString src, s64 start) {
	for (s64 i = start; i < grd_len(src); i++) {
		if (i == start) {
			if (src[i] != '"') {
				return 0;
			}
		} else {
			if (src[i] == '"') {
				return i + 1;
			}
			if (grd_starts_with(src[{i, {}}], R"xx(\")xx")) {
				i += 1;
				continue;
			}
		}
	}
	return 0;
}

s64 grdc_prep_is_valid_ident_symbol(char32_t c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c >= '0' && c <= '9' || c == '$';
}

s64 grdc_prep_is_valid_first_ident_symbol(char32_t c) {
	return grdc_prep_is_valid_ident_symbol(c) && !(c >= '0' && c <= '9');
}

bool grdc_prep_is_punct(char32_t c) {
	for (auto x: {',',';','/','.','-','=','(',')','?', '!', ':','+','*','-','<','>', '[', ']', '&', '#'}) {
		if (c == x) {
			return true;
		}
	}
	return false;
}

s64 grdc_prep_maybe_ident(GrdUnicodeString src, s64 start) {
	for (s64 i = start; i < grd_len(src); i++) {
		if (i == start) {
			if (!grdc_prep_is_valid_first_ident_symbol(src[i])) {
				return 0;
			}
		} else {
			if (grd_is_whitespace(src[i]) || grdc_prep_is_punct(src[i])) {
				return i;
			}
			if (!grdc_prep_is_valid_ident_symbol(src[i])) {
				return 0;
			}
		}
	}
	return start < grd_len(src) ? grd_len(src) : 0;
}

GrdTuple<s64, GrdcPrepTokenKind> grdc_get_str_token_at(GrdUnicodeString src, s64 cursor) {
	if (grdc_prep_is_punct(src[cursor])) {
		return { cursor + 1, GRDC_PREP_TOKEN_KIND_PUNCT };
	}
	s64 num_end = grdc_prep_maybe_number_tok(src, cursor);
	if (num_end > 0) {
		return { num_end, GRDC_PREP_TOKEN_KIND_NUMBER };
	}
	s64 string_end = grdc_prep_maybe_string_tok(src, cursor);
	if (string_end > 0) {
		return { string_end, GRDC_PREP_TOKEN_KIND_STRING };
	}
	s64 ident_end = grdc_prep_maybe_ident(src, cursor);
	if (ident_end > 0) {
		return { ident_end, GRDC_PREP_TOKEN_KIND_IDENT };
	}
	return { 0 };
}

GrdTuple<GrdcToken*, GrdError*> grdc_get_token_at(GrdcPrep* p, GrdcPrepFileSource* file, s64 cursor) {
	if (file->src[cursor] == '#') {
		if (grd_starts_with(file->src[{cursor, {}}], "##")) {
			return { grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_GRD_CONCAT, file, cursor, cursor + 2) };
		}
		s64 dir_end = grd_len(file->src);
		for (s64 i: grd_range_from_to(cursor + 1, grd_len(file->src))) {
			if (grd_is_whitespace(file->src[i]) || grd_is_line_break(file->src[i])) {
				dir_end = i;
				break;
			}
		}
		if (dir_end == cursor + 1) {
			return { {}, grdc_make_prep_file_error(p, file, cursor, cursor + 1, "Empty preprocessor directive") };
		}
		auto str = file->src[{cursor, dir_end}];
		if (str == "#include" || str == "#define") {
			return { grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_DIRECTIVE, file, cursor, dir_end) };
		}
		return { grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_STRINGIZE, file, cursor, dir_end) };
	}
	auto [tok_end, tok_kind] = grdc_get_str_token_at(file->src, cursor);
	if (tok_end != 0) {
		return { grdc_make_file_token(p, tok_kind, file, cursor, tok_end) };
	}
	return { NULL, grdc_make_prep_file_error(p, file, cursor, grd_len(file->src), "Invalid token. First char: %", file->src[cursor]) };
}

GrdTuple<GrdcToken*, GrdError*> grdc_prep_file_next_token(GrdcPrep* p, GrdcPrepFileSource* file, s64* cursor) {
	while (*cursor < grd_len(file->src)) {
		auto lb_len = grd_get_line_break_len(file->src, *cursor);
		if (lb_len > 0) {
			grd_defer { *cursor += lb_len; };
			return { grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_LINE_BREAK, file, *cursor, *cursor + lb_len) };
		}
		if (grd_is_whitespace(file->src[*cursor])) {
			grd_defer { *cursor += 1; };
			return { grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_SPACE, file, *cursor, *cursor + 1) };
		}
		break;
	}
	if (*cursor == grd_len(file->src)) {
		return { NULL, NULL };
	}
	auto [tok, e] = grdc_get_token_at(p, file, *cursor);
	if (e) {
		return { {}, e };
	}
	assert(tok->src_kind == GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	*cursor = tok->file_end;
	return { tok, NULL };
}

GrdUnicodeString grdc_tok_str(GrdcToken* tok) {
	if (tok->flags & GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
		return U"\n"_b;
	}
	if (tok->flags & GRDC_PREP_TOKEN_FLAG_CUSTOM_STR) {
		return tok->custom_str;
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_MACRO) {
		return grdc_tok_str(tok->og_macro_tok);
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE) {
		return grdc_tok_str(tok->concat_residue_og);
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP) {
		return grdc_tok_str(tok->prescan_exp_og);
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE) {
		return tok->file_source->src[{tok->file_start, tok->file_end}];
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE) {
		return grdc_tok_str(tok->included_file_og_tok);
	}
	assert(false);
	return {};
}

GrdUnicodeString grdc_tok_str_content(GrdcToken* tok) {
	assert(tok->kind == GRDC_PREP_TOKEN_KIND_STRING);
	return grdc_tok_str(tok)[{1, -1}];
}

void grdc_push_source_file_token(GrdcPrepFileSource* file, GrdcToken* tok) {
	tok->file_tok_idx = grd_len(file->tokens);
	grd_add(&file->tokens, tok);
}

GrdError* grdc_tokenize(GrdcPrep* p, GrdcPrepFileSource* file) {
	if (file->is_tokenized) {
		return NULL;
	}
	file->is_tokenized = true;
	file->src = grd_copy_string(p->allocator, file->og_src);
	grdc_prep_remove_file_splices(p, file);
	grdc_prep_remove_comments(p, file);
	s64 cursor = 0;
	while (true) {
		auto [tok, e] = grdc_prep_file_next_token(p, file, &cursor);
		if (e) {
			return e;
		}
		if (!tok) {
			break;
		}
		grdc_push_source_file_token(file, tok);
	}
	if (grd_len(file->tokens) == 0 || file->tokens[-1]->kind != GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
		auto tok = grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_LINE_BREAK, file, grd_len(file->src), grd_len(file->src));
		tok->flags |= GRDC_PREP_TOKEN_FLAG_FILE_MISSING_TRAILING_LINE_BREAK;
		grdc_push_source_file_token(file, tok);
	}
	auto eof = grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_EOF, file, grd_len(file->src), grd_len(file->src));
	grdc_push_source_file_token(file, eof);
	return NULL;
}

GrdUnicodeString grdc_prep_default_resolve_fullpath_hook(GrdcPrep* p, GrdUnicodeString path, bool global) {
	return path;
}

GrdcPrepFileSource* grdc_make_mem_prep_file(GrdcPrep* p, GrdUnicodeString src, GrdUnicodeString fullpath) {
	auto* file = grd_make<GrdcPrepFileSource>(p->allocator);
	file->og_src = src;
	file->fullpath = fullpath;
	file->comment_mappings.allocator = p->allocator;
	file->splice_mappings.allocator = p->allocator;
	file->tokens.allocator = p->allocator;
	return file;
}

GrdcPrepFileSource* grdc_prep_default_load_file_hook(GrdcPrep* p, GrdUnicodeString fullpath) {
	auto [text, e] = grd_read_text_at_path(p->allocator, fullpath);
	if (e) {
		return NULL;
	}
	auto utf32 = grd_decode_utf8(p->allocator, text);
	text.free();
	return grdc_make_mem_prep_file(p, utf32, fullpath);
}

GrdcPrep* grdc_make_prep(GrdAllocator allocator = c_allocator) {
	allocator = grd_make_sub_allocator(allocator);
	auto p = grd_make<GrdcPrep>(allocator);
	p->allocator = allocator;
	p->arena = grd_make_arena_allocator(allocator);
	p->files.allocator = allocator;
	p->tokens.allocator = allocator;
	p->macros.allocator = allocator;
	p->load_file_hook = grdc_prep_default_load_file_hook;
	p->resolve_fullpath_hook = grdc_prep_default_resolve_fullpath_hook;
	return p;
}

// String is allocated separately from GrdcPrep.
GrdAllocatedUnicodeString grdc_prep_str(GrdcPrep* p) {
	GrdAllocatedUnicodeString str;
	for (auto it: p->tokens) {
		grd_append(&str, grdc_tok_str(it));
	}
	return str;
}

s64 grdc_get_next_token_idx(GrdSpan<GrdcToken*> tokens, s64 cursor) {
	while (++cursor < grd_len(tokens)) {
		auto tok = tokens[cursor];
		if (tok->kind != GRDC_PREP_TOKEN_KIND_SPACE &&
			tok->kind != GRDC_PREP_TOKEN_KIND_LINE_BREAK
		) {
			return cursor;
		}
	}
	return grd_len(tokens);
}

// After grdc_get_next_token() cursor points to the returned token.
GrdcToken* grdc_get_next_token(GrdSpan<GrdcToken*> tokens, s64* cursor) {
	*cursor = grdc_get_next_token_idx(tokens, *cursor);
	if (*cursor >= grd_len(tokens)) {
		assert(grd_len(tokens) > 0);
		assert(tokens[-1]->kind == GRDC_PREP_TOKEN_KIND_EOF);
		return tokens[-1];
	}
	return tokens[*cursor];
}

bool grdc_is_prep_macro_variadic(GrdcPrepMacro* macro) {
	return grd_len(macro->arg_defs) > 0 && grdc_tok_str(macro->arg_defs[-1]) == "...";
}

s64 grdc_find_macro_arg_def_tok_idx(GrdcPrepMacro* macro, GrdUnicodeString name) {
	if (name == "__VA_ARGS__") {
		if (grdc_is_prep_macro_variadic(macro)) {
			return grd_len(macro->arg_defs) - 1;
		}
	} else {
		for (auto idx: grd_range(grd_len(macro->arg_defs))) {
			if (grdc_tok_str(macro->arg_defs[idx]) == name) {
				return idx;
			}
		}
	}
	return -1;
}

GrdError* grdc_preprocess_file(GrdcPrep* p, GrdcPrepFileSource* file);

bool grdc_does_include_stack_contain_file(GrdcPrep* p, GrdcPrepFileSource* file) {
	auto entry = p->include_site;
	while (entry) {
		if (entry->file == file) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

void grdc_grdc_prep_stringize_tok(GrdcToken* dst, GrdcToken* arg) {
	// @TODO: escape.
	grd_add(&dst->custom_str, grdc_tok_str(arg));
}

GrdTuple<GrdError*, GrdArray<GrdcPrepMacroArg>> grdc_parse_macro_args(GrdcPrep* p, GrdcPrepMacro* macro, GrdSpan<GrdcToken*> tokens, s64* cursor, GrdcToken* paren_tok) {
	GrdArray<GrdcPrepMacroArg> args = { .capacity = grd_len(macro->arg_defs), .allocator = p->allocator };
	s64 paren_level = 0;
	s64 arg_start = -1;
	s64 arg_end = -1;
	while (true) {
		auto arg_tok = grdc_get_next_token(tokens, cursor);
		if (arg_start == -1) {
			arg_start = *cursor;
		}
		if (grdc_tok_str(arg_tok) == ")") {
			if (paren_level > 0) {
				paren_level -= 1;
				continue;
			}
		} else if (grdc_tok_str(arg_tok) == ",") {
			if (paren_level > 0) {
				continue;
			}
		} else if (grdc_tok_str(arg_tok) == "(") {
			paren_level += 1;
			continue;
		} else {
			arg_end = *cursor + 1;
			continue;
		}
		s64 def_tok_idx = -1;
		if (grd_len(args) >= grd_len(macro->arg_defs)) {
			if (!grdc_is_prep_macro_variadic(macro)) {
				return { grdc_make_prep_file_error(p, arg_tok, "Expected % arguments at most for a macro", grd_len(macro->arg_defs)) };
			} else {
				def_tok_idx = -1;
			}
		} else {
			def_tok_idx = grd_len(args);
		}
		grd_add(&args, { def_tok_idx, arg_start, arg_end == -1 ? arg_start : arg_end });
		arg_start = -1;
		arg_end = -1;
		if (grdc_tok_str(arg_tok) == ")") {
			break;
		}
	}
	if (grdc_is_prep_macro_variadic(macro)) {
		if (grd_len(args) < grd_len(macro->arg_defs)) {
			return { grdc_make_prep_file_error(p, paren_tok, "Expected % arguments at least for a macro", grd_len(macro->arg_defs)) };
		}
	} else {
		if (grd_len(args) != grd_len(macro->arg_defs)) {
			return { grdc_make_prep_file_error(p, paren_tok, "Expected % arguments for a macro", grd_len(macro->arg_defs)) };
		}
	}
	return { NULL, args };
}

struct GrdcMaybeExpandedMacro {
	GrdError*     e = NULL;
	GrdcMacroExp* exp = NULL;
	s64           exp_start = 0;
	s64           exp_end = 0;
};

bool grdc_does_macro_stack_contain_macro(GrdcPrep* p, GrdcPrepMacro* macro) {
	auto entry = p->macro_exp;
	while (entry) {
		if (entry->macro == macro) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

GrdcPrepMacro* grdc_find_macro(GrdcPrep* p, GrdUnicodeString name) {
	for (auto it: p->macros) {
		if (grdc_tok_str(it->name) == name) {
			return it;
		}
	}
	return NULL;
}

struct GrdcTokenSlice {
	GrdSpan<GrdcToken*> tokens;
	s64                 start = 0;
	s64                 end = 0;

	GrdcToken*& operator[](s64 idx) {
		return tokens[idx + start];
	}

	GrdcTokenSlice operator[](GrdTuple<s64, s64> x) {
		return { .tokens = tokens, .start = x._0 + start, .end = x._1 + start };
	}

	GrdSpan<GrdcToken*> span() {
		return tokens[{start, end}];
	}
};

s64 grd_len(GrdcTokenSlice slice) {
	return slice.end - slice.start;
}

GrdcTokenSlice grdc_make_tok_slice(GrdSpan<GrdcToken*> tokens, s64 start, s64 end) {
	return { .tokens = tokens, .start = start, .end = end };
}

GrdcTokenSlice grdc_make_tok_slice(GrdSpan<GrdcToken*> tokens) {
	return grdc_make_tok_slice(tokens, 0, grd_len(tokens));
}

GrdOptional<GrdcTokenSlice> grdc_get_arg_tokens(GrdcPrepMacro* macro, GrdSpan<GrdcToken*> tokens, GrdSpan<GrdcPrepMacroArg> args, GrdUnicodeString name) {
	auto def_idx = grdc_find_macro_arg_def_tok_idx(macro, name);
	if (def_idx == -1) {
		return {};
	}
	if (grdc_tok_str(macro->arg_defs[def_idx]) == "...") {
		if (def_idx < grd_len(args)) {
			return grdc_make_tok_slice(tokens, args[def_idx].start, args[-1].end);
		} else {
			return grdc_make_tok_slice(tokens, grd_len(tokens), grd_len(tokens));
		}
	} else {
		auto arg = args[def_idx];
		return grdc_make_tok_slice(tokens, arg.start, arg.end);
	}
}

GrdcMaybeExpandedMacro grdc_maybe_expand_macro(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor) {
	auto exp_start = *cursor;
	auto name_tok = tokens[*cursor];
	assert(name_tok->kind == GRDC_PREP_TOKEN_KIND_IDENT || name_tok->kind == GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
	auto macro = grdc_find_macro(p, grdc_tok_str(name_tok));
	if (!macro) {
		return { };
	}
	GrdcToken* paren_tok = NULL;
	if (!macro->is_object) {
		paren_tok = grdc_get_next_token(tokens.span(), cursor);
		if (grdc_tok_str(paren_tok) != "(" || paren_tok->file_start != name_tok->file_end) {
			return { };
		}
	}

	if (grdc_does_macro_stack_contain_macro(p, macro)) {
		return { };
	}

	auto exp = grd_make<GrdcMacroExp>(p->arena);
	exp->macro = macro;
	exp->parent = p->macro_exp;
	exp->body.allocator = p->allocator;
	p->macro_exp = exp;
	grd_defer { p->macro_exp = exp->parent; };

	for (auto tok: macro->tokens) {
		auto nt = grdc_make_token(p, tok->kind, GRDC_PREP_TOKEN_SOURCE_MACRO);
		nt->og_macro_tok = tok;
		nt->og_macro_exp = exp;
		grd_add(&exp->body, nt);
	}

	if (!macro->is_object) {
		auto [e, args] = grdc_parse_macro_args(p, macro, tokens.span(), cursor, paren_tok);
		if (e) {
			return { e };
		}
		exp->args = args;
	}
	exp->replaced = grd_clear_array(p->allocator, tokens[{exp_start, *cursor + 1}].span());
	if (!macro->is_object) {
		// Stringize.
		for (s64 i = 0; i < grd_len(exp->body); i++) {
			if (exp->body[i]->kind == GRDC_PREP_TOKEN_KIND_STRINGIZE) {
				auto str_tok = grdc_make_token(p, GRDC_PREP_TOKEN_KIND_STRING, GRDC_PREP_TOKEN_SOURCE_STRINGIZE);
				str_tok->stringize_exp = exp;
				str_tok->stringize_tok = exp->body[i];
				grdc_prep_use_custom_str(p, str_tok);
				grd_append(&str_tok->custom_str, U"\"");
				auto [arg_tokens, found] = grdc_get_arg_tokens(macro, tokens.span(), exp->args, grdc_tok_str(exp->body[i])[{1, {}}]);
				if (!found) {
					return { grdc_make_prep_file_error(p, exp->body[i], "Macro argument is not found for stringizing") };
				}
				for (auto tok: arg_tokens.span()) {
					grdc_grdc_prep_stringize_tok(str_tok, tok);
				}
				grd_append(&str_tok->custom_str, U"\"");
				exp->body[i] = str_tok;
			}
		}
		// Concat.
		for (s64 i = 0; i < grd_len(exp->body); i++) {
			if (exp->body[i]->kind == GRDC_PREP_TOKEN_KIND_GRD_CONCAT) {
				if (i - 1 < 0 || grd_contains({GRDC_PREP_TOKEN_KIND_SPACE, GRDC_PREP_TOKEN_KIND_EOF}, exp->body[i - 1]->kind)) {
					return { grdc_make_prep_file_error(p, exp->body[i], "Missing left argument for ## operator") };
				}
				if (i + 1 >= grd_len(exp->body) || grd_contains({GRDC_PREP_TOKEN_KIND_SPACE, GRDC_PREP_TOKEN_KIND_EOF}, exp->body[i + 1]->kind)) {
					return { grdc_make_prep_file_error(p, exp->body[i], "Missing right argument for ## operator") };
				}
				GrdcToken* tok = grdc_make_token(p, GRDC_PREP_TOKEN_KIND_NONE, GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED);
				tok->concat_exp = exp;
				grdc_prep_use_custom_str(p, tok);
				auto lhs = grdc_make_tok_slice(exp->body, i - 1, i);
				auto rhs = grdc_make_tok_slice(exp->body, i + 1, i + 2);
				tok->concat_lhs = lhs[0];
				tok->concat_rhs = rhs[0];
				auto [lhs_toks, lhs_found] = grdc_get_arg_tokens(macro, tokens.span(), exp->args, grdc_tok_str(lhs[0]));
				if (lhs_found) {
					lhs = lhs_toks;
				}
				auto [rhs_toks, rhs_found] = grdc_get_arg_tokens(macro, tokens.span(), exp->args, grdc_tok_str(rhs[0]));
				if (rhs_found) {
					rhs = rhs_toks;
				}
				GrdcTokenSlice   lhs_residue;
				GrdcTokenSlice   rhs_residue;
				if (grd_len(lhs) > 0) {
					tok->concat_lhs_content = lhs[-1];
					grd_append(&tok->custom_str, grdc_tok_str(lhs[-1]));
					lhs_residue = lhs[{0, -1}];
				}
				if (grd_len(rhs) > 0) {
					tok->concat_rhs_content = rhs[0];
					grd_append(&tok->custom_str, grdc_tok_str(rhs[0]));
					rhs_residue = rhs[{1, {}}];
				}
				auto [end, kind] = grdc_get_str_token_at(tok->custom_str, 0);
				if (end != grd_len(tok->custom_str)) {
					auto e = grdc_make_prep_dp_error(p, "Concatenated string '%' doesn't form a valid token", grdc_tok_str(tok));
					auto dp = grdc_make_detailed_printer(exp->body);
					grd_add(&dp->spans, { i, i + 1, 1 });
					grdc_add_dp(e, dp);
					return { e };
				}
				for (auto& it: lhs_residue.span()) {
					auto t = grdc_make_token(p, it->kind, GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					it = t;
				}
				for (auto& it: rhs_residue.span()) {
					auto t = grdc_make_token(p, it->kind, GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					it = t;
				}
				tok->kind = kind;
				grd_remove(&exp->body, i - 1, 3);
				grd_add(&exp->body, lhs_residue.span(), i - 1);
				grd_add(&exp->body, tok,         i - 1 + grd_len(lhs_residue));
				grd_add(&exp->body, rhs_residue.span(), i - 1 + grd_len(lhs_residue) + 1);
				i = i - 1 + grd_len(lhs_residue) + grd_len(rhs_residue);
			}
		}
		// Prescan.
		for (s64 i = 0; i < grd_len(exp->body); i++) {
			if (exp->body[i]->kind == GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT) {
				auto [arg_tokens, found] = grdc_get_arg_tokens(macro, tokens.span(), exp->args, grdc_tok_str(exp->body[i]));
				if (found) {
					s64 body_cursor = i;
					grd_remove(&exp->body, i);
					s64 arg_cursor = 0;
					while (arg_cursor < grd_len(arg_tokens)) {
						if (arg_tokens[arg_cursor]->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
							auto mb = grdc_maybe_expand_macro(p, arg_tokens, &arg_cursor);
							if (mb.e) {
								return { mb.e };
							}
							if (mb.exp) {
								grd_add(&exp->body, mb.exp->body, body_cursor);
								body_cursor += grd_len(mb.exp->body);
								arg_cursor += 1;
								continue;
							}
						}
						auto t = grdc_make_token(p, arg_tokens[arg_cursor]->kind, GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP);
						t->prescan_exp = exp;
						t->prescan_exp_og = arg_tokens[arg_cursor];
						grd_add(&exp->body, t, body_cursor);
						body_cursor += 1;
						arg_cursor += 1;
					}
				}
			}
		}
	}

	GrdcMaybeExpandedMacro res = {
		.exp = exp,
		.exp_start = exp_start,
		.exp_end = *cursor,
	};
	return res;
}

GrdError* grdc_handle_prep_directive(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor) {
	auto first_directive_tok = tokens[*cursor];
	if (grdc_tok_str(first_directive_tok) == "#include") {
		*cursor += 1;
		s64 start = *cursor;
		s64 end = grd_len(tokens);
		s64 include_trailing_newline_idx = -1;
		while (*cursor < grd_len(tokens)) {
			if (tokens[*cursor]->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK ||
				tokens[*cursor]->kind == GRDC_PREP_TOKEN_KIND_EOF)
			{
				if (tokens[*cursor]->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
					include_trailing_newline_idx = *cursor;
				}
				end = *cursor;
				break;
			}
			*cursor += 1;
		}

		GrdArray<GrdcToken*> path_toks;
		path_toks.allocator = p->allocator;
		grd_defer { path_toks.free(); };

		s64 exp_cursor = start;
		while (exp_cursor < end) {
			if (tokens[exp_cursor]->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
				auto exp = grdc_maybe_expand_macro(p, tokens, &exp_cursor);
				if (exp.e) {
					return exp.e;
				} else if (exp.exp) {
					grd_add(&path_toks, exp.exp->body);
				} else {
					continue;
				}
			}
			grd_add(&path_toks, tokens[exp_cursor]);
			exp_cursor += 1;
		}

		// Remove preceeding and trailing spaces.
		for (s64 i = 0; i < grd_len(path_toks); i++) {
			if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				break;
			}
			grd_remove(&path_toks, i);
			i -= 1;
		}
		for (s64 i = grd_len(path_toks) - 1; i >= 0; i--) {
			if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				break;
			}
			grd_remove(&path_toks, i);
		}
		if (grd_len(path_toks) == 0) {
			return grdc_make_prep_file_error(p, first_directive_tok, "Empty #include path");
		}
		GrdUnicodeString path;
		bool          is_global = false;
		if (path_toks[0]->kind == GRDC_PREP_TOKEN_KIND_STRING) {
			for (auto i: grd_range_from_to(1, grd_len(path_toks) - 1)) {
				if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
					return grdc_make_prep_file_error(p, path_toks[i], "Unexpected token after #include path");
				}
			}
			path = grdc_tok_str_content(path_toks[0]);
		} else {
			// Check for starting < and trailing >
			if (grdc_tok_str(path_toks[0]) != "<") {
				return grdc_make_prep_file_error(p, path_toks[0], "Expected '<' at the start of #include path");
			}
			if (grdc_tok_str(path_toks[-1]) != ">") {
				return grdc_make_prep_file_error(p, path_toks[-1], "Expected '>' at the end of #include path");
			}
			grd_remove(&path_toks, 0);
			grd_remove(&path_toks, -1);
			GrdAllocatedUnicodeString composed_include_path;
			composed_include_path.allocator = p->allocator;
			for (auto tok: path_toks) {
				grd_append(&composed_include_path, grdc_tok_str(tok));
			}
			path = composed_include_path;
			is_global = true;
		}

		auto fullpath = p->resolve_fullpath_hook(p, path, is_global);
		if (fullpath == "") {
			if (!is_global) {
				return grdc_make_prep_file_error(p, first_directive_tok, "Failed to resolve fullpath for #include \"%\"", path);
			} else {
				return grdc_make_prep_file_error(p, first_directive_tok, "Failed to resolve fullpath for #include <%>", path);
			}
		}
		GrdcPrepFileSource* source = NULL;
		for (auto loaded: p->files) {
			if (loaded->fullpath == fullpath) {
				source = loaded;
				break;
			}
		}
		if (!source) {
			source = p->load_file_hook(p, fullpath);
			if (!source) {
				return grdc_make_prep_file_error(p, first_directive_tok, "Failed to find #include file with fullpath '%'", fullpath);
			}
			grd_add(&p->files, source);
		}
		if (!grdc_does_include_stack_contain_file(p, source)) {
// S			inc_file->site_toks = grd_clear_array(p->allocator, path_toks);
// 			path_toks = {};
			auto e = grdc_preprocess_file(p, source);
			if (e) {
				return e;
			}
		}
		*cursor = end - 1;
		// Eat line break if present, but don't eat EOF.
		// *cursor = include_trailing_newline_idx == -1 ? end - 1 : end;
	} else if (grdc_tok_str(first_directive_tok) == "#define") {
		s64 start_tok_idx = *cursor;
		auto ident_tok = grdc_get_next_token(tokens.span(), cursor);
		if (ident_tok->kind != GRDC_PREP_TOKEN_KIND_IDENT) {
			return grdc_make_prep_file_error(p, ident_tok, "Expected an identifier after #define");
		}
		auto macro = grd_make<GrdcPrepMacro>(p->allocator);
		macro->def_site = p->include_site;
		macro->start_tok_idx = start_tok_idx;
		macro->name = ident_tok;
		grd_add(&p->macros, macro);

		s64 first_macro_body_tok = *cursor + 1;
		if (first_macro_body_tok < grd_len(tokens) &&
			grdc_tok_str(tokens[first_macro_body_tok]) == "(" &&
			tokens[first_macro_body_tok]->file_start == ident_tok->file_end)
		{
			*cursor += 1;
			macro->is_object = false;
			while (true) {
				auto arg_tok = grdc_get_next_token(tokens.span(), cursor);
				if (grdc_tok_str(arg_tok) == ")") {
					*cursor += 1;
					break;
				}
				if (grd_len(macro->arg_defs) >= 1) {
					if (grdc_tok_str(arg_tok) != ",") {
						return grdc_make_prep_file_error(p, arg_tok, "Expected ',' after macro argument");
					}
					arg_tok = grdc_get_next_token(tokens.span(), cursor);
				}
				if (arg_tok->kind != GRDC_PREP_TOKEN_KIND_IDENT) {
					return grdc_make_prep_file_error(p, arg_tok, "Expected an identifier as a macro argument");
				}
				for (auto it: macro->arg_defs) {
					if (grdc_tok_str(it) == grdc_tok_str(arg_tok)) {
						return grdc_make_prep_file_error(p, arg_tok, "Duplicate macro argument");
					}
				}
				if (grd_len(macro->arg_defs) >= 1 && grdc_tok_str(macro->arg_defs[-1]) == "...") {
					return grdc_make_prep_file_error(p, arg_tok, "Unexpected macro argument after '...'");
				}
				grd_add(&macro->arg_defs, arg_tok);
			}
			first_macro_body_tok = *cursor;
		}
		while (*cursor < grd_len(tokens)) {
			auto macro_tok = tokens[*cursor];
			if (macro_tok->kind == GRDC_PREP_TOKEN_KIND_EOF || macro_tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
				break;
			} else if (macro_tok->kind == GRDC_PREP_TOKEN_KIND_STRINGIZE) {
				auto def_tok_idx = grdc_find_macro_arg_def_tok_idx(macro, grdc_tok_str(macro_tok)[{1, {}}]);
				if (def_tok_idx == -1) {
					return grdc_make_prep_file_error(p, macro_tok, "Macro argument is not found for stringizing");
				}
			} else if (macro_tok->kind == GRDC_PREP_TOKEN_KIND_DIRECTIVE) {
				return grdc_make_prep_file_error(p, macro_tok, "Unexpected preprocessor directive in a macro");
			}
			assert(macro_tok->kind != GRDC_PREP_TOKEN_KIND_NONE);
			*cursor += 1;
		}
		macro->end_tok_idx = *cursor;
		macro->tokens = grd_clear_array(p->allocator, tokens[{first_macro_body_tok, *cursor}].span());
		for (auto& it: macro->tokens) {
			if (it->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
				it->kind = GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT;
			}
		}
		GrdLogTrace("Expanding macro {%}, tokens:", grdc_tok_str(macro->name));
		for (auto it: macro->tokens) {
			GrdLogTrace("  %, %: %, %", grdc_tok_str(it), it->file_start, it->file_end, it->kind);
		}
	} else {
		return grdc_make_prep_file_error(p, first_directive_tok, "Unknown preprocessor directive '%'", grdc_tok_str(first_directive_tok));
	}
	return NULL;
}

GrdError* grdc_preprocess_file(GrdcPrep* p, GrdcPrepFileSource* source_file) {
	auto e = grdc_tokenize(p, source_file);
	if (e) {
		return e;
	}
	auto included = grd_make<GrdcIncludedFile>(p->arena);
	included->file = source_file;
	included->parent = p->include_site;
	p->include_site = included;
	// @TODO: set include site tokens.
	grd_defer { p->include_site = included->parent; };
	included->tokens.allocator = p->allocator;
	for (auto tok: source_file->tokens) {
		auto nt = grdc_make_token(p, tok->kind, GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE);
		nt->included_file = included;
		nt->included_file_og_tok = tok;
		grd_add(&included->tokens, nt);
	}
	//
	s64 cursor = -1;
	while (++cursor < grd_len(included->tokens)) {
		auto tok = included->tokens[cursor];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_DIRECTIVE) {
			auto e = grdc_handle_prep_directive(p, grdc_make_tok_slice(included->tokens), &cursor);
			if (e) {
				return e;
			}
			continue;
		}
		if (tok->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
			auto mb = grdc_maybe_expand_macro(p, grdc_make_tok_slice(included->tokens), &cursor);
			if (mb.e) {
				return mb.e;
			}
			if (mb.exp) {
				grd_add(&p->tokens, mb.exp->body);
				continue;
			}
		}
		if (tok->kind == GRDC_PREP_TOKEN_KIND_EOF) {
			break;
		}
		grd_add(&p->tokens, tok);
	}
	return NULL;
}

GrdTuple<GrdError*, GrdcPrep*> grdc_preprocess(GrdUnicodeString str) {
	auto p = grdc_make_prep();
	auto file = grdc_make_mem_prep_file(p, str, U"<root_file>"_b);
	auto e = grdc_preprocess_file(p, file);
	return { e, p };
}

GrdTuple<GrdError*, GrdcPrep*, GrdUnicodeString> grdc_preprocess_to_string(GrdUnicodeString str) {
	auto [e, p] = grdc_preprocess(str);
	if (e) {
		return { e };
	}
	return { NULL, p, grdc_prep_str(p) };
}
