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


enum GrdcAstOperatorFlags {
	GRDC_AST_OP_FLAG_LEFT_ASSOC = 1 << 0,
	GRDC_AST_OP_FLAG_MOD_ASSIGN = 1 << 1,
	GRDC_AST_OP_FLAG_BOOL       = 1 << 3,
	GRDC_AST_OP_FLAG_INT        = 1 << 4,
	GRDC_AST_OP_FLAG_PRIMITIVE  = 1 << 5,
	GRDC_AST_OP_FLAG_NUMERIC    = 1 << 6,
	GRDC_AST_OP_FLAG_POSTFIX    = 1 << 7,
	GRDC_AST_OP_FLAG_PREFIX     = 1 << 8,
	GRDC_AST_OP_FLAG_PREP       = 1 << 9,
};
GRD_REFLECT(GrdcAstOperatorFlags) {
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_LEFT_ASSOC);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_MOD_ASSIGN);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_BOOL);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_INT);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_PRIMITIVE);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_NUMERIC);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_POSTFIX);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_PREFIX);
	GRD_ENUM_VALUE(GRDC_AST_OP_FLAG_PREP);
}

struct GrdcAstOperator {
	GrdUnicodeString op;
	s32              prec;
	s32              flags = 0;

	GRD_REFLECT(GrdcAstOperator) {
		GRD_MEMBER(op);
		GRD_MEMBER(prec);
		GRD_MEMBER(flags);
	}
};

GRD_DEDUP GrdcAstOperator GRDC_AST_BINARY_OPERATORS_UNSORTED[] = { 
	{ U","_b, 10, GRDC_AST_OP_FLAG_LEFT_ASSOC },
	{ U"="_b, 11, 0 },
	{ U"|="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"^="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"&="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"<<="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U">>="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"+="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"-="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"*="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"/="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"%="_b, 11, GRDC_AST_OP_FLAG_MOD_ASSIGN },
	{ U"?"_b, 12, 0 },
	{ U"||"_b, 13, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_BOOL | GRDC_AST_OP_FLAG_PREP },
	{ U"&&"_b, 14, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_BOOL | GRDC_AST_OP_FLAG_PREP },
	{ U"|"_b, 15, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
	{ U"^"_b, 16, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
	{ U"&"_b, 17, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
	{ U"=="_b, 18, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PRIMITIVE | GRDC_AST_OP_FLAG_PREP },
	{ U"!="_b, 18, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PRIMITIVE | GRDC_AST_OP_FLAG_PREP },
	{ U"<"_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U">"_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U"<="_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PREP },
	{ U">="_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PREP },
	{ U"<<"_b, 20, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
	{ U">>"_b, 20, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
	{ U"+"_b, 21, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U"-"_b, 21, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U"*"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U"/"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC | GRDC_AST_OP_FLAG_PREP },
	{ U"%"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT | GRDC_AST_OP_FLAG_PREP },
};

GRD_DEDUP GrdcAstOperator GRDC_AST_PREFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"!"_b, 30, GRDC_AST_OP_FLAG_PREFIX | GRDC_AST_OP_FLAG_PREP },
	{ U"~"_b, 30, GRDC_AST_OP_FLAG_PREFIX | GRDC_AST_OP_FLAG_PREP },
	{ U"+"_b, 30, GRDC_AST_OP_FLAG_PREFIX | GRDC_AST_OP_FLAG_PREP },
	{ U"-"_b, 30, GRDC_AST_OP_FLAG_PREFIX | GRDC_AST_OP_FLAG_PREP },
	{ U"++"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"--"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"*"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"&"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
};

GRD_DEDUP GrdcAstOperator GRDC_AST_POSTFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"++"_b, 40, GRDC_AST_OP_FLAG_POSTFIX },
	{ U"--"_b, 40, GRDC_AST_OP_FLAG_POSTFIX },
};

GRD_DEDUP GrdcAstOperator* grdc_find_binary_operator(GrdUnicodeString op, u64 req_flags) {
	for (auto& it: GRDC_AST_BINARY_OPERATORS_UNSORTED) {
		if (op == it.op && (req_flags & it.flags) == req_flags) {
			return &it;
		}
	}
	return NULL;
}

GRD_DEDUP GrdcAstOperator* grdc_find_prefix_unary_operator(GrdUnicodeString op, u64 req_flags) {
	for (auto& it: GRDC_AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op && (req_flags & it.flags) == req_flags) {
			return &it;
		}
	}
	return NULL;
}

GRD_DEDUP GrdcAstOperator* grdc_find_postfix_unary_operator(GrdUnicodeString op, u64 req_flags) {
	for (auto& it: GRDC_AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op && (req_flags & it.flags) == req_flags) {
			return &it;
		}
	}
	return NULL;
}

enum GrdcPrepTokenKind {
	GRDC_PREP_TOKEN_KIND_NONE,
	GRDC_PREP_TOKEN_KIND_EOF,
	GRDC_PREP_TOKEN_KIND_LINE_BREAK,
	GRDC_PREP_TOKEN_KIND_NUMBER,
	GRDC_PREP_TOKEN_KIND_STRING,
	GRDC_PREP_TOKEN_KIND_IDENT,
	GRDC_PREP_TOKEN_KIND_SPACE,
	GRDC_PREP_TOKEN_KIND_PUNCT,
	GRDC_PREP_TOKEN_KIND_GRD_CONCAT,
	GRDC_PREP_TOKEN_KIND_GRD_CONCATTED,
	GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT,
	GRDC_PREP_TOKEN_KIND_DOT_DOT_DOT,
	GRDC_PREP_TOKEN_KIND_OTHER,
	GRDC_PREP_TOKEN_KIND_HASH,
};
GRD_REFLECT(GrdcPrepTokenKind) {
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_NONE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_EOF);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_LINE_BREAK);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_NUMBER);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_STRING);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_IDENT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_SPACE);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_PUNCT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_GRD_CONCAT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_GRD_CONCATTED);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_DOT_DOT_DOT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_OTHER);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_KIND_HASH);
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
	GRDC_PREP_TOKEN_SOURCE_DOT_DOT_DOT = 8,
	GRDC_PREP_TOKEN_SOURCE_MACRO_DEF = 9,
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
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_DOT_DOT_DOT);
	GRD_ENUM_VALUE(GRDC_PREP_TOKEN_SOURCE_MACRO_DEF);
}

struct GrdcIncludedFile;
struct GrdcPrepFileSource;
struct GrdcTokenSet;

struct GrdcToken {
	GrdcPrepTokenKind          kind  = GRDC_PREP_TOKEN_KIND_NONE;
	s64                        flags = GRDC_PREP_TOKEN_FLAG_NONE;
	GrdcPrepTokenSourceKind    src_kind = GRDC_PREP_TOKEN_SOURCE_NONE;
	GrdcTokenSet*              set = NULL;
	GrdAllocatedUnicodeString  custom_str;
	GrdcPrepFileSource*        file_source = NULL;
	s64                        file_tok_idx = 0;
	GrdcIncludedFile*          included_file = NULL;
	s64                        file_start = 0;
	s64                        file_end = 0;
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
	GrdcToken*                 dot_dot_dot_1 = NULL;
	GrdcToken*                 dot_dot_dot_2 = NULL;
	GrdcToken*                 dot_dot_dot_3 = NULL;
	GrdcToken*                 og_macro_def_tok = NULL;

	GRD_REFLECT(GrdcToken) {
		GRD_MEMBER(kind);
		GRD_MEMBER(flags);
		GRD_MEMBER(src_kind);
		GRD_MEMBER(set);
		GRD_MEMBER(custom_str);
		GRD_MEMBER(file_source);
		GRD_MEMBER(file_tok_idx);
		GRD_MEMBER(included_file);
		GRD_MEMBER(file_start);
		GRD_MEMBER(file_end);
		GRD_MEMBER(stringize_exp);
		GRD_MEMBER(stringize_tok);
		GRD_MEMBER(og_macro_exp);
		GRD_MEMBER(og_macro_tok);
		GRD_MEMBER(concat_lhs);
		GRD_MEMBER(concat_rhs);
		GRD_MEMBER(concat_lhs_content);
		GRD_MEMBER(concat_rhs_content);
		GRD_MEMBER(concat_residue_og);
		GRD_MEMBER(concat_exp);
		GRD_MEMBER(prescan_exp_og);
		GRD_MEMBER(prescan_exp);
		GRD_MEMBER(included_file_og_tok);
		GRD_MEMBER(dot_dot_dot_1);
		GRD_MEMBER(dot_dot_dot_2);
		GRD_MEMBER(dot_dot_dot_3);
		GRD_MEMBER(og_macro_def_tok);
	}
};

// Immutable.
struct GrdcTokenSlice {
	GrdcTokenSet* set   = NULL;
	s64           start = 0;
	s64           end   = 0;

	GRD_DEDUP GrdcToken* operator[](s64 idx);

	GRD_DEF operator[](GrdTuple<GrdOptional<s64>, GrdOptional<s64>> x) {
		s64 a = x._0.has_value ? x._0.value : 0;
		s64 b = x._1.has_value ? x._1.value : (end - start);
		if (a < 0) a += (end - start);
		if (b < 0) b += (end - start);
		grd_assert(a <= b);
		GrdcTokenSlice grdc_make_token_slice(GrdcTokenSet* set, s64 start, s64 end);
		return grdc_make_token_slice(set, start + a, start + b);
	}

	// @TODO: remove, used only for debugging.
	GRD_DEF operator==(auto& rhs) -> bool {
		return set == rhs.set && start == rhs.start && end == rhs.end;
	}
};

GRD_DEDUP s64 grd_len(GrdcTokenSlice slice) {
	return slice.end - slice.start;
}

struct GrdcTokenSetBuilder {
	GrdArray<GrdcToken*> tokens = { .allocator = null_allocator };
};

GRD_DEDUP GrdcTokenSetBuilder grdc_make_token_set_builder(GrdAllocator allocator) {
	GrdcTokenSetBuilder b;
	b.tokens.allocator = allocator;
	return b;
}

GRD_DEF grdc_add_token(GrdcTokenSetBuilder* b, GrdcToken* tok) {
	if (tok->set) {
		grd_panic("tok->set is already set.");
	}
	grd_add(&b->tokens, tok);
}

struct GrdcTokenSetParentBuilder {
	GrdArray<GrdcTokenSlice> slices = { .allocator = null_allocator };
};

struct GrdcTokenSetIndexer;
GRD_DEDUP GrdcToken* grdc_index(GrdcTokenSetIndexer* indexer, s64 idx);
GRD_DEDUP GrdcTokenSetIndexer grdc_make_token_set_indexer(GrdcTokenSlice slice);
GRD_DEDUP GrdcTokenSlice grdc_make_token_slice(GrdcTokenSet* set, s64 start, s64 end);
GRD_DEDUP GrdcTokenSlice grdc_make_token_slice(GrdcTokenSet* set);
GRD_DEDUP s64 grd_len(GrdcTokenSet* set);

GRD_DEDUP GrdcTokenSetParentBuilder grdc_make_token_set_parent_builder(GrdAllocator allocator) {
	GrdcTokenSetParentBuilder b;
	b.slices.allocator = allocator;
	return b;
}

struct GrdcTokenSetIndexer {
	GrdcTokenSlice root_slice;
	s64            slice_start = -1;
	s64            slice_len = 0;
	GrdcTokenSlice slice;

	GRD_DEF operator[](s64 idx) {
		return grdc_index(this, idx);
	}

	GRD_DEF operator[](GrdTuple<GrdOptional<s64>, GrdOptional<s64>> x) {
		auto slice = root_slice[x];
		return grdc_make_token_set_indexer(slice);
	}
};

GRD_DEF grd_len(GrdcTokenSetIndexer* indexer) {
	return grd_len(indexer->root_slice);
}

GRD_DEDUP GrdcTokenSetIndexer grdc_make_token_set_indexer(GrdcTokenSlice slice) {
	return { .root_slice = slice };
}

// Immutable.
struct GrdcTokenSet {
	bool                     is_parent = false;
	GrdArray<GrdcTokenSlice> children;
	GrdArray<GrdcToken*>     tokens;
	s64                      count = 0;
	GrdcTokenSetIndexer      base_indexer;

	GRD_DEDUP GrdcToken* operator[](s64 idx) {
		return grdc_index(&base_indexer, idx);
	}
};

GRD_DEDUP GrdcTokenSlice grdc_make_token_slice(GrdcTokenSet* set, s64 start, s64 end) {
	assert(start >= 0 && start <= grd_len(set));
	assert(end >= 0   && end   <= grd_len(set));
	return { set, start, end };
}

GRD_DEDUP GrdcTokenSlice grdc_make_token_slice(GrdcTokenSet* set) {
	return grdc_make_token_slice(set, 0, grd_len(set));
}

GrdcToken* GrdcTokenSlice::operator[](s64 idx) {
	s64 target_idx = idx < 0 ? end + idx : start + idx;
	if (target_idx >= start && target_idx < end) {
		return (*set)[target_idx];
	}
	return (*set)[-1];
}

GRD_DEDUP GrdTuple<bool, GrdcTokenSlice, s64, s64> grdc_find_containing_token_slice(GrdcTokenSlice slice, s64 slice_start, s64 idx, s64 end_border) {
	if (!slice.set->is_parent) {
		return { true, slice, slice_start, end_border };
	}
	s64 cursor = slice_start - slice.start;
	for (auto it: slice.set->children) {
		if (idx >= cursor && idx < cursor + grd_len(it)) {
			if (idx >= slice_start && idx < slice_start + grd_len(slice)) {
				return grdc_find_containing_token_slice(it, cursor, idx, grd_min_s64(end_border, cursor + grd_len(it)));
			}
		}
		cursor += grd_len(it);
	}
	return { false, NULL, 0, 0 };
}

GRD_DEDUP GrdcToken* grdc_index(GrdcTokenSetIndexer* indexer, s64 idx) {
	if (idx < 0) {
		idx += grd_len(indexer->root_slice);
	}
	if (indexer->slice_start == -1 ||
	    idx < indexer->slice_start ||
	    idx >= indexer->slice_start + indexer->slice_len
	) {
		// GrdLogTrace("reindex");
		indexer->slice_start = -1;
		auto [did_find, slice, slice_start, end_border] = grdc_find_containing_token_slice(indexer->root_slice, 0, idx, grd_len(indexer->root_slice));
		if (!did_find) {
			return NULL;
		}
		indexer->slice = slice;
		indexer->slice_start = slice_start;
		indexer->slice_len = end_border - slice_start;
	}
	assert(!indexer->slice.set->is_parent);
	return indexer->slice.set->tokens[idx - indexer->slice_start + indexer->slice.start];
}

GRD_DEDUP GrdGenerator<GrdcToken*> grd_iterate(GrdcTokenSlice slice) {
	auto indexer = grdc_make_token_set_indexer(slice);
	for (auto idx: grd_range(grd_len(slice))) {
		co_yield grdc_index(&indexer, idx);
	}
}

GRD_DEDUP GrdGenerator<GrdcToken*> grd_iterate(GrdcTokenSet* set) {
	auto slice = grdc_make_token_slice(set, 0, grd_len(set));
	auto indexer = grdc_make_token_set_indexer(slice);
	for (auto idx: grd_range(grd_len(slice))) {
		co_yield grdc_index(&indexer, idx);
	}
}

GRD_DEDUP s64 grd_len(GrdcTokenSet* set) {
	return set->count;
}

GRD_DEDUP GrdUnicodeString grdc_tok_str(GrdcToken* tok);

GRD_DEF grd_type_format(GrdFormatter* f, GrdcTokenSlice* slice, GrdString spec) {
	grd_format(f, "[");
	auto s = *slice;
	if (grd_len(s) > 32) {
		s64 idx = 0;
		for (auto tok: s[{0, 5}]) {
			grd_format(f, idx < 4 ? "%, " : "%", grdc_tok_str(tok));
			idx += 1;
		}
		grd_format(f, "...");
		idx = 0;
		for (auto tok: s[{-5, {}}]) {
			grd_format(f, idx < (grd_len(s) - 1) ? "%, " : "%", grdc_tok_str(tok));
			idx += 1;
		}
	} else {
		s64 idx = 0;
		for (auto tok: s) {
			grd_format(f, idx < (grd_len(s) - 1) ? "%, " : "%", grdc_tok_str(tok));
			idx += 1;
		}
	}
	grd_format(f, "]");
}

GRD_DEDUP GrdcTokenSlice grdc_make_parent_token_set(GrdcTokenSetParentBuilder* b) {
	auto set = grd_make<GrdcTokenSet>();
	set->is_parent = true;
	set->children = b->slices;
	set->count = 0;
	for (auto it: b->slices) {
		set->count += grd_len(it);
	}
	b->slices = { .allocator = null_allocator }; // Make sure we crash if we try to write into |children| after.
	auto slice = grdc_make_token_slice(set);
	set->base_indexer = grdc_make_token_set_indexer(slice);
	GrdLogTrace("again");
	for (auto s: set->children) {
		GrdLogTrace("  child: %, %, %", (void*) s.set, s.start, s.end);
		for (auto tok: s) {
			GrdLogTrace("    tok: %", grdc_tok_str(tok));
		}
	}
	GrdLogTrace("slice: ");
	for (auto tok: slice) {
		GrdLogTrace("  tok: %", grdc_tok_str(tok));
	}
	return slice;
}

GRD_DEDUP GrdcTokenSlice grdc_make_token_set(GrdcTokenSetBuilder* b) {
	auto set = grd_make<GrdcTokenSet>();
	for (auto tok: b->tokens) {
		if (tok->set) {
			grd_panic("tok->set is already set.");
		}
		tok->set = set;
	}
	set->tokens = b->tokens;
	set->count = grd_len(b->tokens);
	b->tokens = { .allocator = null_allocator }; // Make sure we crash if we try to write into |tokens| after.
	set->base_indexer = grdc_make_token_set_indexer(grdc_make_token_slice(set, 0, grd_len(set)));
	return grdc_make_token_slice(set);
}

GRD_DEF grdc_add_tokens(GrdcTokenSetParentBuilder* b, GrdcTokenSlice slice) {
	GrdLogTrace("before: ");
	for (auto s: b->slices) {
		for (auto tok: s) {
			GrdLogTrace("  tok: %", grdc_tok_str(tok));
		}
	}
	GrdLogTrace("adding: ");
	for (auto tok: slice) {
		GrdLogTrace("  tok: %", grdc_tok_str(tok));
	}
	grd_defer { 
		GrdLogTrace("after: ");
		for (auto s: b->slices) {
			for (auto tok: s) {
				GrdLogTrace("  tok: %", grdc_tok_str(tok));
			}
		}
	};
	if (grd_len(b->slices) > 0) {
		auto last = &b->slices[-1];
		if (last->set == slice.set && slice.start >= last->start && slice.start <= last->end) {
			if (slice.end >= last->start) {
				GrdLogTrace("TRIGGER!");
		// 		GrdUnicodeString grdc_tok_str(GrdcToken* x);
		// 		auto cp = b->slices.copy(b->slices.allocator);
		// 		GrdLogTrace("Toks before: %", grd_to_array(grd_map(grdc_make_parent_token_set(b), grd_lambda(x, grdc_tok_str(x)))));
		// 		b->slices = cp;
		// 		grd_assert(b->slices == cp);
		// 		last = &b->slices[-1];
		// 		GrdLogTrace("DIFF: %, %", grd_len(b->slices), grd_len(cp));
		// 		GrdLogTrace("Extending last slice: %, %", last->start, last->end);
		// 		GrdLogTrace("  set %, %", last->set, slice.set);
		// 		GrdLogTrace("  with: %, %", slice.start, slice.end);
				last->end = grd_max_s64(last->end, slice.end);
		// 		GrdLogTrace("  to: %, %", last->start, last->end);
		// 		cp = b->slices.copy();
		// 		GrdLogTrace("Toks after: %", grd_to_array(grd_map(grdc_make_parent_token_set(b), grd_lambda(x, grdc_tok_str(x)))));
		// 		b->slices = cp;
				return;
			}
		}
	}
	grd_add(&b->slices, slice);
}

struct GrdcIncludedFile {
	GrdcIncludedFile*      parent = NULL;
	GrdcTokenSlice         tokens;
	// GrdArray<GrdcToken*>   site_toks;
	GrdcPrepFileSource*    file = NULL;
};

// @TODO: rename back to GrdcPrepFile.
struct GrdcPrepFileSource {
	GrdcTokenSetBuilder       tokens_builder;
	GrdcTokenSlice            tokens;
	GrdUnicodeString          fullpath;
	GrdUnicodeString          og_src;
	GrdUnicodeString          src;
	GrdArray<PrepFileMapping> splice_mappings;
	GrdArray<PrepFileMapping> comment_mappings;
	bool                      is_tokenized = false;
};

struct GrdcPrepMacro {
	GrdcTokenSlice       tokens;
	GrdcIncludedFile*    def_site = NULL;
	s64                  start_tok_idx = 0;
	s64                  end_tok_idx = 0;
	GrdcToken*           name;
	GrdArray<GrdcToken*> arg_defs;
	bool                 is_object = true;
};

struct GrdcPrepMacroArg {
	s64 def_tok_idx;
	s64 start;
	s64 end;
};

struct GrdcMacroExp {
	GrdcMacroExp*              parent = NULL;
	GrdcPrepMacro*             macro = NULL;
	GrdArray<GrdcPrepMacroArg> args;
	GrdcTokenSlice             replaced;
	GrdcTokenSlice             before_stringize;
	GrdcTokenSlice             after_stringize;
	GrdcTokenSlice             after_concat;
	GrdcTokenSlice             after_prescan;
	GrdcTokenSlice             after_rescan;
};

struct GrdcPrepIf {
	s64 current_block = 0;
	s64 taken_block = -1;
};

struct GrdcPrep {
	GrdAllocator                   allocator;
	GrdAllocator                   arena;
	GrdcTokenSetParentBuilder      tokens_builder;
	GrdArray<GrdcPrepFileSource*>  files;
	GrdArray<GrdcPrepMacro*>       macros;
	GrdcMacroExp*                  macro_exp = NULL;
	GrdcIncludedFile*              include_site = NULL;
	void*                          aux_data = NULL;
	GrdUnicodeString             (*resolve_fullpath_hook) (GrdcPrep* p, GrdUnicodeString path, bool global) = NULL;
	GrdcPrepFileSource*          (*load_file_hook)        (GrdcPrep* p, GrdUnicodeString fullpath) = NULL;
	GrdArray<GrdcPrepIf>           if_stack;
};

struct GrdcPrepTokenError;
struct GrdcPrepDetailedError;

GRD_DEDUP void grd_type_format(GrdFormatter* f, GrdcPrepTokenError* e, GrdString spec);
GRD_DEDUP void grd_type_format(GrdFormatter* f, GrdcPrepDetailedError* e, GrdString spec);

struct GrdcPrepTokenError: GrdError {
	GrdcPrep*  prep = NULL;
	GrdcToken* tok = NULL;

	GRD_REFLECT(GrdcPrepTokenError) {
		GRD_BASE_TYPE(GrdError);
		GRD_MEMBER(prep);
		GRD_MEMBER(tok);
	}
};

GRD_DEDUP GrdcToken* grdc_file_tok(GrdcToken* tok);

GRD_DEDUP GrdcPrepTokenError* grdc_make_prep_file_error(GrdcPrep* p, GrdcToken* tok, auto... args) {
	auto msg = grd_sprint(p->allocator, args...);
	auto e = grd_make_error<GrdcPrepTokenError>(msg);
	e->prep = p;
	e->tok = tok;
	return e;
}

GRD_DEDUP s64 grdc_map_index(GrdSpan<PrepFileMapping> mappings, s64 index) {
	for (auto it: mappings) {
		if (index >= it.start && index < it.start + it.length) {
			return index - it.start + it.real_start;
		}
	}
	return mappings[-1].real_start + mappings[-1].length;
}

GRD_DEDUP s64 grdc_og_file_index(GrdcPrepFileSource* source, s64 index) {
	index = grdc_map_index(source->comment_mappings, index);
	index = grdc_map_index(source->splice_mappings, index);
	return index;
}

GRD_DEDUP s64 grdc_get_residual_lines_above(GrdUnicodeString src, s64 anchor, s64 lines_count) {
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

GRD_DEDUP s64 grdc_get_residual_lines_below(GrdUnicodeString src, s64 anchor, s64 lines_count) {
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

GRD_DEDUP void grdc_print_file_range_highlighted(GrdFormatter* f, GrdcPrepFileSource* file, s64 h_start, s64 h_end, s64 color) {
	s64 mapped_start = grdc_og_file_index(file, h_start);
	s64 mapped_end = grdc_og_file_index(file, h_end);
	s64 start = grdc_get_residual_lines_above(file->og_src, mapped_start, 3);
	s64 end = grdc_get_residual_lines_below(file->og_src, mapped_end, 3);
	grd_formatln(f);
	grd_format(f, file->og_src[{start, mapped_start}]);
	grd_format(f, U"\x1b[0;%m", 30 + color);
	// Length is 0, but we have to print something to see where the error is.
	if (mapped_start == mapped_end) {
		grd_format(f, U"\x1b[%m", 40 + color);
		grd_format(f, " ");
		grd_format(f, U"\x1b[49m");
	}
	for (auto c: file->og_src[{mapped_start, mapped_end}]) {
		if (grd_is_whitespace(c)) {
			if (grd_is_line_break(c)) {
				grd_format(f, U"\x1b[%m", 40 + color);
				grd_format(f, " ");
				grd_format(f, U"\x1b[49m");
				grd_format(f, "\n");
			} else {
				grd_format(f, U"\x1b[%m", 40 + color);
				grd_format(f, c);
				grd_format(f, U"\x1b[49m");
			}
		} else {
			grd_format(f, c);
		}
	}
	grd_format(f, U"\x1b[0m");
	grd_format(f, file->og_src[{mapped_end, end}]);
	grd_formatln(f);
}

GRD_DEDUP void grdc_print_prep_token_error(GrdFormatter* f, GrdcToken* tok, s64 color) {
	assert(tok->src_kind == GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	auto file = tok->file_source;
	grdc_print_file_range_highlighted(f, file, tok->file_start, tok->file_end, color);
}

GRD_DEDUP void grdc_print_single_error_token(GrdFormatter* f, GrdcToken* tok) {
	switch (tok->src_kind) {
		case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE: {
			grd_formatln(f, "  at file: %", tok->file_source->fullpath);
			grdc_print_prep_token_error(f, tok, 1);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			grdc_print_single_error_token(f, tok->og_macro_tok);
			grd_formatln(f, "  expanded from macro: %", grdc_tok_str(tok->og_macro_exp->macro->name));
			grd_formatln(f, "  which is defined in file: %", tok->og_macro_exp->macro->def_site->file->fullpath);
			grdc_print_prep_token_error(f, tok->og_macro_exp->macro->tokens[0], 1);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED: {
			grd_formatln(f, "  at concatenated token: %", grdc_tok_str(tok));
			grd_formatln(f, "lhs token: ");
			grdc_print_single_error_token(f, tok->concat_lhs);
			grd_formatln(f, "rhs token: ");
			grdc_print_single_error_token(f, tok->concat_rhs);
			break;
		}
		case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			grd_formatln(f, "  at included file: %", tok->included_file->file->fullpath);
			grdc_print_prep_token_error(f, tok->included_file_og_tok, 1);
			break;
		}
		default: {
			grd_formatln(f, "Unknown token source kind: %", tok->src_kind);
			grd_assert(false);
		}
	}
}

GRD_DEDUP void grdc_print_debug_tok(GrdcToken* tok) {
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

GRD_DEDUP void grdc_print_token_dumper(GrdFormatter* f, GrdcTokenDumper* dumper) {
	for (auto& it: dumper->files) {
		grd_formatln(f, "File: %", it.file->file->fullpath);
		auto parent = it.file->parent;
		if (parent) {
			grd_formatln(f, "  Included from: ");
		}
		while (parent) {
			grd_formatln(f, "    %", parent->file->fullpath);
			parent = parent->parent;
		}
		for (auto entry: it.color_ranges) {
			for (auto range: entry->value.ranges) {
				grdc_print_file_range_highlighted(f, it.file->file, range._0, range._1, entry->key);
			}
		}
	}
}

GRD_DEDUP void grdc_dump_file_range(GrdcTokenDumper* dumper, GrdcIncludedFile* file, s64 start, s64 end, s64 color) {
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

GRD_DEDUP GrdcMacroExp* grdc_get_root_macro_exp(GrdcMacroExp* exp) {
	while (exp->parent) {
		exp = exp->parent;
	}
	return exp;
}

GRD_DEDUP void grdc_dump_token(GrdcTokenDumper* dumper, GrdcToken* tok, s64 color) {
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
		case GRDC_PREP_TOKEN_SOURCE_DOT_DOT_DOT: {
			grdc_dump_token(dumper, tok->dot_dot_dot_1, color);
			grdc_dump_token(dumper, tok->dot_dot_dot_2, color);
			grdc_dump_token(dumper, tok->dot_dot_dot_3, color);
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
	GrdcTokenSlice       tokens;
	GrdArray<GrdcDpSpan> spans;
	bool                 expand_site = false;
};

GRD_DEDUP bool grdc_do_token_sources_match(GrdcToken* a, GrdcToken* b) {
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

GRD_DEDUP GrdcDetailedPrinter* grdc_make_detailed_printer(GrdcTokenSlice tokens, bool expand_site) {
	auto dp = grd_make<GrdcDetailedPrinter>();
	dp->tokens = tokens;
	dp->expand_site = expand_site;
	return dp;
}

GRD_DEDUP s64 grdc_get_residual_tokens(GrdcTokenSlice tokens, s64 cursor, s64 dir) {
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

GRD_DEDUP void grdc_do_detailed_print(GrdFormatter* f, GrdcDetailedPrinter* dp) {
	if (grd_len(dp->spans) == 0) {
		grd_formatln(f, "Empty DP");
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

	// Fill gaps.
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

	s64 first_line = 1;
	for (auto i: grd_range(grd_len(regs))) {
		for (auto tok: dp->tokens[{0, regs[i]._0}]) {
			if (tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
				first_line += 1;
			}
		}
	}

	s64 line = first_line;
	bool did_print_line_number = false;

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
					grd_format(f, "\x1b[0;%m", 30 + span.color);
				}
			}
			auto it = dp->tokens[idx];
			if (!did_print_line_number && !dp->expand_site) {
				GrdSmallString str = grd_to_string(line);
				grd_format(f, "% ", str);
				for (auto i: grd_range(grd_len(str) + 1)) grd_add(&line_double, ' ');
				did_print_line_number = true;
			}
			if (it->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
				grd_formatln(f);
				if (dp->expand_site) {
					grd_format(f, line_double);
					grd_clear(&line_double);
					grd_formatln(f);
				}
				line += 1;
				did_print_line_number = false;
				continue;
			}
			if (need_exp) {
				grd_format(f, grdc_tok_str(it));
				for (auto i: grd_range(grd_len(grdc_tok_str(it)))) grd_add(&line_double, get_zone_char());
			} else {
				grd_format(f, grdc_tok_str(it));
				for (auto i: grd_range(grd_len(grdc_tok_str(it)))) grd_add(&line_double, ' ');
			}
			grd_format(f, "\x1b[0m");
		}
	}
	if (dp->expand_site) {
		grd_formatln(f);
		grd_format(f, line_double);
		grd_formatln(f);
	}
	grd_formatln(f);

	if (dp->expand_site) {
		for (auto [start, end, need_exp, zone]: regs) {
			if (end <= start) {
				continue;
			}
			GrdcToken* tok = dp->tokens[start];
			grd_formatln(f, "Site %:", zone);
			void grdc_print_dp_site(GrdFormatter* f, GrdcToken* tok);
			grdc_print_dp_site(f, tok);
		}
	}	
}

GRD_DEDUP void grdc_print_include_site(GrdFormatter* f, GrdcIncludedFile* inc) {
	while (inc) {
		grd_formatln(f, "%", inc->file->fullpath);
		inc = inc->parent;
	}
}

GRD_DEDUP void grdc_print_dp_site(GrdFormatter* f, GrdcToken* tok) {
	switch (tok->src_kind) {
		case GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE: {
			auto dp = grdc_make_detailed_printer(tok->file_source->tokens, false);
			grd_add(&dp->spans, { tok->file_tok_idx, tok->file_tok_idx + 1, 1 });
			grdc_do_detailed_print(f, dp);
		}
		break;
		// case GRDC_PREP_TOKEN_SOURCE_STRINGIZE
		case GRDC_PREP_TOKEN_SOURCE_MACRO: {
			auto macro = tok->og_macro_exp->macro;
			grd_formatln(f, "Expanded from macro: %", grdc_tok_str(macro->name));
			grd_formatln(f, "Defined in:");
			grdc_print_include_site(f, macro->def_site);
			grd_formatln(f);
			auto dp = grdc_make_detailed_printer(macro->def_site->tokens, false);
			grd_add(&dp->spans, { macro->start_tok_idx, macro->end_tok_idx, 1 });
			grdc_do_detailed_print(f, dp);
		}
		break;
		case GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE: {
			auto file = tok->included_file;
			grd_formatln(f, "Included from file: ");
			grdc_print_include_site(f, file);
		}
		break;

		// case GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED
		// case GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE
		// case GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP

		default: {
			grd_panic("Unknown token source kind: %", tok->src_kind);
		}
	}
}

struct GrdcPrepDpErrorNode {
	GrdcDetailedPrinter* dp = NULL;
	GrdUnicodeString     str;
};

struct GrdcPrepDetailedError: GrdError {
	GrdArray<GrdcPrepDpErrorNode> nodes;

	GRD_REFLECT(GrdcPrepDetailedError) {
		GRD_BASE_TYPE(GrdError);
		GRD_MEMBER(nodes);
	}
};

GRD_DEDUP GrdcPrepDetailedError* grdc_make_prep_dp_error(GrdcPrep* p, GrdCodeLoc loc, auto... args) {
	auto msg = grd_sprint(args...);
	auto e = grd_make_error<GrdcPrepDetailedError>(msg, loc);
	return e;
}

GRD_DEDUP void grdc_add_dp(GrdcPrepDetailedError* e, GrdcDetailedPrinter* dp) {
	grd_add(&e->nodes, { dp });
}

GRD_DEDUP void grdc_add_message(GrdcPrepDetailedError* e, auto... args) {
	auto msg = grd_sprint(args...);
	grd_add(&e->nodes, { msg });
}

GRD_DEDUP void grdc_print_token_spans(GrdcTokenDumper* dumper, GrdSpan<GrdcTokenSpan> spans) {
	for (auto it: spans) {
		for (auto tok: it.tokens) {
			grdc_dump_token(dumper, tok, it.color);
		}
	}
}



GRD_DEDUP void grdc_print_prep_error(GrdFormatter* f, GrdError* e) {
	grd_formatln(f);
	grd_formatln(f, "Error: %", e->text);
	if (auto x = grd_reflect_cast<GrdcPrepTokenError>(e)) {
		grd_formatln(f);
		GrdcTokenDumper dumper;
		grdc_dump_token(&dumper, x->tok, 1);
		grdc_print_token_dumper(f, &dumper);
		// grdc_print_single_error_token(x->tok);
	} else if (auto x = grd_reflect_cast<GrdcPrepDetailedError>(e)) {
		for (auto node: x->nodes) {
			if (node.dp) {
				grdc_do_detailed_print(f, node.dp);
			} else {
				grd_formatln(f, node.str);
			}
		}
	} else {
		grd_formatln(f, e);
	}
}

GRD_DEDUP void grd_type_format(GrdFormatter* f, GrdcPrepTokenError* e, GrdString spec) {
	grdc_print_prep_error(f, e);
}

GRD_DEDUP void grd_type_format(GrdFormatter* f, GrdcPrepDetailedError* e, GrdString spec) {
	grdc_print_prep_error(f, e);
}

GRD_DEDUP void grdc_print_prep_error(GrdError* e) {
	GrdAllocatedUnicodeString str;
	auto f = grd_make_formatter(&str, c_allocator);
	grdc_print_prep_error(&f, e);
	grd_print(str);
}

GRD_DEDUP GrdcToken* grdc_make_token(GrdcPrep* p, GrdcPrepTokenKind kind, GrdcPrepTokenSourceKind src_kind) {
	auto* tok = grd_make<GrdcToken>(p->arena);
	tok->kind = kind;
	tok->custom_str.allocator = null_allocator;
	tok->src_kind = src_kind;
	return tok;
}

GRD_DEDUP GrdcToken* grdc_make_file_token(GrdcPrep* p, GrdcPrepTokenKind kind, GrdcPrepFileSource* file, s64 file_start, s64 file_end) {
	auto tok = grdc_make_token(p, kind, GRDC_PREP_TOKEN_SOURCE_FILE_SOURCE);
	tok->file_source = file;
	tok->file_start = file_start;
	tok->file_end = file_end;
	return tok;
}

GRD_DEDUP void grdc_prep_use_custom_str(GrdcPrep* p, GrdcToken* tok) {
	tok->flags |= GRDC_PREP_TOKEN_FLAG_CUSTOM_STR;
	tok->custom_str.allocator = p->allocator;
}

GRD_DEDUP void grdc_prep_push_mapping(GrdArray<PrepFileMapping>* mappings, GrdcPrep* p, GrdcPrepFileSource* file, s64* cursor, s64 end, s64 rm_len, s64* removed_len, s64* i) {
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

GRD_DEDUP void grdc_prep_remove_file_splices(GrdcPrep* p, GrdcPrepFileSource* file) {
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

GRD_DEDUP void grdc_prep_remove_comments(GrdcPrep* p, GrdcPrepFileSource* file) {
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
			s64 level = 1;
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

GRD_DEDUP s64 grdc_prep_maybe_number_tok(GrdUnicodeString src, s64 start) {
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

GRD_DEDUP s64 grdc_prep_maybe_string_tok(GrdUnicodeString src, s64 start) {
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

GRD_DEDUP s64 grdc_prep_is_valid_ident_symbol(char32_t c) {
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c >= '0' && c <= '9' || c == '$';
}

GRD_DEDUP s64 grdc_prep_is_valid_first_ident_symbol(char32_t c) {
	return grdc_prep_is_valid_ident_symbol(c) && !(c >= '0' && c <= '9');
}

GRD_DEDUP bool grdc_prep_is_punct(char32_t c) {
	for (auto x: {',',';','/','.','-','=','(',')','?', '!', ':','+','*','-','<','>', '[', ']', '&', '#'}) {
		if (c == x) {
			return true;
		}
	}
	return false;
}

GRD_DEDUP s64 grdc_prep_maybe_ident(GrdUnicodeString src, s64 start) {
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

GRD_DEDUP GrdTuple<s64, GrdcPrepTokenKind> grdc_get_lang_token_end_at(GrdUnicodeString src, s64 cursor) {
	if (cursor >= grd_len(src)) {
		return { 0 };
	}
	if (grdc_prep_is_punct(src[cursor])) {
		if (grd_starts_with(src[{cursor, {}}], "==")) {
			return { cursor + 2, GRDC_PREP_TOKEN_KIND_PUNCT };
		}
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

GRD_DEDUP GrdTuple<s64, GrdcPrepTokenKind> grdc_get_token_end_at(GrdUnicodeString src, s64 cursor) {
	assert(cursor < grd_len(src));
	if (src[cursor] == '#') {
		if (grd_starts_with(src[{cursor, {}}], "##")) {
			return { cursor + 2, GRDC_PREP_TOKEN_KIND_GRD_CONCAT };
		}
		return { cursor + 1, GRDC_PREP_TOKEN_KIND_HASH };
	}
	auto [lang_end, lang_kind] = grdc_get_lang_token_end_at(src, cursor);
	if (lang_end > 0) {
		return { lang_end, lang_kind };
	}
	auto lb_len = grd_get_line_break_len(src, cursor);
	if (lb_len > 0) {
		return { cursor + lb_len, GRDC_PREP_TOKEN_KIND_LINE_BREAK };
	}
	if (grd_is_whitespace(src[cursor])) {
		return { cursor + 1, GRDC_PREP_TOKEN_KIND_SPACE };
	}
	return { cursor + 1, GRDC_PREP_TOKEN_KIND_OTHER };
}

GRD_DEDUP GrdUnicodeString grdc_tok_str(GrdcToken* tok) {
	if (tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
		return U"\n"_b;
	}
	if (tok->kind == GRDC_PREP_TOKEN_KIND_DOT_DOT_DOT) {
		return U"..."_b;
	}
	if (tok->flags & GRDC_PREP_TOKEN_FLAG_CUSTOM_STR) {
		return tok->custom_str;
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_MACRO) {
		return grdc_tok_str(tok->og_macro_tok);
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_MACRO_DEF) {
		return grdc_tok_str(tok->og_macro_def_tok);
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

GRD_DEDUP GrdUnicodeString grdc_tok_str_content(GrdcToken* tok) {
	assert(tok->kind == GRDC_PREP_TOKEN_KIND_STRING);
	return grdc_tok_str(tok)[{1, -1}];
}

GRD_DEDUP void grdc_push_source_file_token(GrdcPrepFileSource* file, GrdcToken* tok) {
	grdc_add_token(&file->tokens_builder, tok);
}

GRD_DEDUP GrdError* grdc_tokenize(GrdcPrep* p, GrdcPrepFileSource* file) {
	if (file->is_tokenized) {
		return NULL;
	}
	file->tokens_builder = grdc_make_token_set_builder(p->allocator);
	file->is_tokenized = true;
	file->src = grd_copy_string(p->allocator, file->og_src);
	grdc_prep_remove_file_splices(p, file);
	grdc_prep_remove_comments(p, file);
	s64 cursor = 0;
	while (cursor < grd_len(file->src)) {
		auto [tok_end, tok_kind] = grdc_get_token_end_at(file->src, cursor);
		if (tok_end <= 0) {
			break;
		}
		auto tok = grdc_make_file_token(p, tok_kind, file, cursor, tok_end);
		grdc_push_source_file_token(file, tok);
		cursor = tok_end;
	}
	// Accessing tokens_builder this way seems like a @Hack.
	if (grd_len(file->tokens_builder.tokens) == 0 || file->tokens_builder.tokens[-1]->kind != GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
		auto tok = grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_LINE_BREAK, file, grd_len(file->src), grd_len(file->src));
		tok->flags |= GRDC_PREP_TOKEN_FLAG_FILE_MISSING_TRAILING_LINE_BREAK;
		grdc_push_source_file_token(file, tok);
	}
	auto eof = grdc_make_file_token(p, GRDC_PREP_TOKEN_KIND_EOF, file, grd_len(file->src), grd_len(file->src));
	grdc_push_source_file_token(file, eof);
	file->tokens = grdc_make_token_set(&file->tokens_builder);
	s64 idx = 0;
	for (auto it: file->tokens) {
		it->file_tok_idx = idx;
		idx += 1;
	}
	for (auto it: file->tokens) {
		GrdLogTrace("FToken: %, %, %, %", grdc_tok_str(it), it->file_start, it->file_end, it->kind);
	}
	return NULL;
}

GRD_DEDUP GrdUnicodeString grdc_prep_default_resolve_fullpath_hook(GrdcPrep* p, GrdUnicodeString path, bool global) {
	return path;
}

GRD_DEDUP GrdcPrepFileSource* grdc_make_mem_prep_file(GrdcPrep* p, GrdUnicodeString src, GrdUnicodeString fullpath) {
	auto* file = grd_make<GrdcPrepFileSource>(p->allocator);
	file->og_src = src;
	file->fullpath = fullpath;
	file->comment_mappings.allocator = p->allocator;
	file->splice_mappings.allocator = p->allocator;
	return file;
}

GRD_DEDUP GrdcPrepFileSource* grdc_prep_default_load_file_hook(GrdcPrep* p, GrdUnicodeString fullpath) {
	auto [text, e] = grd_read_text_at_path(p->allocator, fullpath);
	if (e) {
		return NULL;
	}
	auto utf32 = grd_decode_utf8(p->allocator, text);
	text.free();
	return grdc_make_mem_prep_file(p, utf32, fullpath);
}

GRD_DEDUP GrdcPrep* grdc_make_prep(GrdAllocator allocator = c_allocator) {
	allocator = grd_make_sub_allocator(allocator);
	auto p = grd_make<GrdcPrep>(allocator);
	p->allocator = allocator;
	p->arena = grd_make_arena_allocator(allocator);
	p->files.allocator = allocator;
	p->macros.allocator = allocator;
	p->load_file_hook = grdc_prep_default_load_file_hook;
	p->resolve_fullpath_hook = grdc_prep_default_resolve_fullpath_hook;
	p->tokens_builder = grdc_make_token_set_parent_builder(p->allocator);
	return p;
}

GRD_DEDUP GrdAllocatedUnicodeString grdc_tok_arr_str(GrdSpan<GrdcToken*> tokens) {
	GrdAllocatedUnicodeString str;
	for (auto it: tokens) {
		grd_append(&str, grdc_tok_str(it));
	}
	return str;
}

GRD_DEDUP GrdAllocatedUnicodeString grdc_tok_arr_str(GrdcTokenSlice tokens) {
	GrdAllocatedUnicodeString str;
	for (auto it: tokens) {
		grd_append(&str, grdc_tok_str(it));
	}
	return str;
}

GRD_DEDUP GrdcTokenSlice grdc_get_tokens(GrdcPrep* p) {
	auto b = p->tokens_builder;
	// This is a @Hack.
	p->tokens_builder.slices = b.slices.copy(p->allocator);
	auto tokens = grdc_make_parent_token_set(&b);
	return tokens;
}

GRD_DEDUP GrdAllocatedUnicodeString grdc_prep_str(GrdcPrep* p) {
	auto toks = grdc_get_tokens(p);
	// @TODO: free toks.
	return grdc_tok_arr_str(toks);
}

// String is allocated separately from GrdcPrep.
// GRD_DEDUP GrdAllocatedUnicodeString grdc_prep_str(GrdcPrep* p) {
// 	return grdc_tok_arr_str(p->tokens);
// }

GRD_DEDUP s64 grdc_skip_spaces(GrdcTokenSlice tokens, s64 cursor) {
	while (cursor < grd_len(tokens)) {
		if (tokens[cursor]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
			return cursor;
		}
		cursor += 1;
	}
	return grd_len(tokens);
}

GRD_DEDUP s64 grdc_skip_spaces_and_line_breaks(GrdcTokenSlice tokens, s64 cursor) {
	while (cursor < grd_len(tokens)) {
		if (tokens[cursor]->kind != GRDC_PREP_TOKEN_KIND_SPACE && tokens[cursor]->kind != GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
			return cursor;
		}
		cursor += 1;
	}
	return grd_len(tokens);
}

GRD_DEDUP bool grdc_is_prep_macro_variadic(GrdcPrepMacro* macro) {
	return grd_len(macro->arg_defs) > 0 && grdc_tok_str(macro->arg_defs[-1]) == "...";
}

GRD_DEDUP s64 grdc_find_macro_arg_def_tok_idx(GrdcPrepMacro* macro, GrdUnicodeString name) {
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

GRD_DEDUP GrdError* grdc_preprocess_file(GrdcPrep* p, GrdcPrepFileSource* file);

GRD_DEDUP bool grdc_does_include_stack_contain_file(GrdcPrep* p, GrdcPrepFileSource* file) {
	auto entry = p->include_site;
	while (entry) {
		if (entry->file == file) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

GRD_DEDUP void grdc_grdc_prep_stringize_tok(GrdcToken* dst, GrdcToken* arg) {
	// @TODO: escape.
	grd_add(&dst->custom_str, grdc_tok_str(arg));
}

GRD_DEDUP GrdTuple<GrdError*, GrdArray<GrdcPrepMacroArg>> grdc_parse_macro_args(GrdcPrep* p, GrdcPrepMacro* macro, GrdcTokenSlice tokens, s64* cursor, GrdcToken* paren_tok) {
	GrdArray<GrdcPrepMacroArg> args = { .capacity = grd_len(macro->arg_defs), .allocator = p->allocator };
	s64 paren_level = 0;
	s64 arg_start = -1;
	*cursor = grdc_skip_spaces_and_line_breaks(tokens, *cursor);
	if (grdc_tok_str(tokens[*cursor]) != "(") {
		// Practically untriggerable, because if we don't detect opening brace before, this doesn't get called.
		return { grdc_make_prep_file_error(p, tokens[*cursor], "Expected '(' at the start of macro arguments") };
	}
	arg_start = *cursor + 1;
	while (true) {
		*cursor = grdc_skip_spaces_and_line_breaks(tokens, *cursor + 1);
		auto arg_tok = tokens[*cursor];
		if (arg_tok->kind == GRDC_PREP_TOKEN_KIND_EOF) {
			return { grdc_make_prep_file_error(p, arg_tok, "Unexpected EOF while parsing macro arguments") };
		}
		if (grdc_tok_str(arg_tok) == ")") {
			if (paren_level > 0) {
				paren_level -= 1;
				continue;
			} else {
				// Special handling of the macro with zero args.
				if (grd_len(macro->arg_defs) == 0) {
					bool is_all_space = true;
					for (auto i: grd_range_from_to(arg_start, *cursor)) {
						if (tokens[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE &&
						    tokens[i]->kind != GRDC_PREP_TOKEN_KIND_LINE_BREAK)
						{
							is_all_space = false;
							break;
						}
					}
					if (is_all_space) {
						*cursor += 1;
						return { NULL, args };
					}
				}
			}
		} else if (grdc_tok_str(arg_tok) == ",") {
			if (paren_level > 0) {
				continue;
			}
		} else if (grdc_tok_str(arg_tok) == "(") {
			paren_level += 1;
			continue;
		} else {
			continue;
		}
		if (grd_len(args) >= grd_len(macro->arg_defs) && !grdc_is_prep_macro_variadic(macro)) {
			return { grdc_make_prep_file_error(p, arg_tok, "Expected % argument(s) at most for a macro '%'", grd_len(macro->arg_defs), grdc_tok_str(macro->name)) };
		}
		auto def_tok_idx = grd_len(args) >= grd_len(macro->arg_defs) ? -1 : grd_len(args);
		s64 arg_end = *cursor;
		while (arg_start < arg_end) {
			if (tokens[arg_start]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				break;
			}
			arg_start += 1;
		}
		while (arg_end > arg_start && arg_end <= grd_len(tokens)) {
			if (tokens[arg_end - 1]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				break;
			}
			arg_end -= 1;
		}
		grd_add(&args, { def_tok_idx, arg_start, arg_end });
		GrdLogTrace("add arg: %, %, %", def_tok_idx, arg_start, arg_end);
		arg_start = *cursor + 1;
		if (grdc_tok_str(arg_tok) == ")") {
			*cursor += 1;
			break;
		}
	}
	if (grdc_is_prep_macro_variadic(macro)) {
		if (grd_len(args) < grd_len(macro->arg_defs) - 1) {
			return { grdc_make_prep_file_error(p, paren_tok, "Expected at least % argument(s) for a macro", grd_len(macro->arg_defs)) };
		}
	} else {
		if (grd_len(args) != grd_len(macro->arg_defs)) {
			return { grdc_make_prep_file_error(p, paren_tok, "Expected % argument(s) for a macro", grd_len(macro->arg_defs)) };
		}
	}
	return { NULL, args };
}

GRD_DEDUP bool grdc_does_macro_stack_contain_macro(GrdcPrep* p, GrdcPrepMacro* macro) {
	auto entry = p->macro_exp;
	while (entry) {
		if (entry->macro == macro) {
			return true;
		}
		entry = entry->parent;
	}
	return false;
}

GRD_DEDUP GrdcPrepMacro* grdc_find_macro(GrdcPrep* p, GrdUnicodeString name) {
	for (auto it: p->macros) {
		if (grdc_tok_str(it->name) == name) {
			return it;
		}
	}
	return NULL;
}

GRD_DEDUP GrdOptional<GrdcTokenSlice> grdc_get_arg_tokens(GrdcPrepMacro* macro, GrdcTokenSlice tokens, GrdSpan<GrdcPrepMacroArg> args, GrdUnicodeString name) {
	auto def_idx = grdc_find_macro_arg_def_tok_idx(macro, name);
	if (def_idx == -1) {
		return {};
	}
	if (grdc_tok_str(macro->arg_defs[def_idx]) == "...") {
		if (def_idx < grd_len(args)) {
			return tokens[{args[def_idx].start, args[-1].end}];
		} else {
			return tokens[{grd_len(tokens), grd_len(tokens)}];
		}
	} else {
		auto arg = args[def_idx];
		return tokens[{arg.start, arg.end}];
	}
}

GRD_DEDUP GrdcTokenSlice grdc_get_macro_exp_body(GrdcMacroExp* exp) {
	return exp->after_rescan;
}

GRD_DEDUP GrdcMacroExp* grdc_get_tok_macro_exp(GrdcPrep* p, GrdcToken* tok) {
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_MACRO) {
		return tok->og_macro_exp;
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED) {
		return tok->concat_exp;
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE) {
		return tok->concat_exp;
	}
	if (tok->src_kind == GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP) {
		return tok->prescan_exp;
	}
	return NULL;
}

GRD_DEDUP GrdTuple<GrdError*, GrdcMacroExp*> grdc_maybe_expand_macro(GrdcPrep* p, GrdcTokenSlice tokens, s64* out_cursor) {
	s64 cursor = *out_cursor;
	if (cursor >= grd_len(tokens) || 
		(tokens[cursor]->kind != GRDC_PREP_TOKEN_KIND_IDENT &&
		 tokens[cursor]->kind != GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT))
	{
		return { };
	}
	auto exp_start = cursor;
	auto name_tok = tokens[cursor];
	GrdLogTrace("Maybe expanding macro: %", grdc_tok_str(name_tok));
	auto macro = grdc_find_macro(p, grdc_tok_str(name_tok));
	if (!macro) {
		return { };
	}

	auto cur_stack = p->macro_exp;
	while (cur_stack) {
		GrdLogTrace("cur_stack: %, %", grdc_tok_str(cur_stack->macro->name), cur_stack->macro->start_tok_idx);
		if (cur_stack->macro == macro) {
			GrdLogTrace("Recursive macro expansion prevented: %", grdc_tok_str(name_tok));
			return { };
		}
		// @TODO: poison the token.
		cur_stack = cur_stack->parent;
	}

	GrdcToken* paren_tok = NULL;
	GrdLogTrace("Expanding macro: %", grdc_tok_str(name_tok));
	GrdLogTrace("is_object: %", macro->is_object);
	if (!macro->is_object) {
		cursor = grdc_skip_spaces(tokens, cursor + 1);
		paren_tok = tokens[cursor];
		if (grdc_tok_str(paren_tok) != "(" || paren_tok->file_start != name_tok->file_end) {
			return { };
		}
		// *cursor += 1; // skip (
	} else {
		cursor += 1;
	}

	// if (grdc_does_macro_stack_contain_macro(p, macro)) {
	// 	return { };
	// }

	auto exp = grd_make<GrdcMacroExp>(p->arena);
	exp->macro = macro;
	exp->parent = p->macro_exp;
	// exp->after_rescan.allocator = p->allocator;
	p->macro_exp = exp;
	grd_defer { 
		GrdLogTrace("remove exp: %, %", grdc_tok_str(exp->macro->name), exp->macro->start_tok_idx);
		p->macro_exp = exp->parent; };

	cur_stack = p->macro_exp;
	while (cur_stack) {
		GrdLogTrace("cur_stack2: %, %", grdc_tok_str(cur_stack->macro->name), cur_stack->macro->start_tok_idx);
		if (cur_stack->macro == macro) {
			// GrdLogTrace("Recursive macro expansion prevented: %", grdc_tok_str(name_tok));
			// return { };
		}
		// @TODO: poison the token.
		cur_stack = cur_stack->parent;
	}

	GrdcTokenSetBuilder before_stringize_builder = grdc_make_token_set_builder(p->allocator);

	for (auto tok: macro->tokens) {
		auto nt = grdc_make_token(p, tok->kind, GRDC_PREP_TOKEN_SOURCE_MACRO);
		nt->og_macro_tok = tok;
		nt->og_macro_exp = exp;
		grdc_add_token(&before_stringize_builder, nt);
	}
	exp->before_stringize = grdc_make_token_set(&before_stringize_builder);
	GrdLogTrace("head - before_stringize: %", grd_to_array(grd_map(exp->before_stringize, grd_lambda(x, grdc_tok_str(x)))));

	if (!macro->is_object) {
		auto [e, args] = grdc_parse_macro_args(p, macro, tokens, &cursor, paren_tok);
		if (e) {
			return { e };
		}
		exp->args = args;
		for (auto& it: args) {
			GrdLogTrace("  macro arg: %, %, %", it.def_tok_idx, it.start, it.end);
			GrdLogTrace("    str: %", grdc_tok_arr_str(tokens[{it.start, it.end}]));
		}
	}
	exp->replaced = tokens[{exp_start, cursor}];
	GrdLogTrace("  replaced: %", grdc_tok_arr_str(exp->replaced));
	if (!macro->is_object) {
		// Stringize.
		auto after_stringize_builder = grdc_make_token_set_parent_builder(p->allocator);
		for (s64 i = 0; i < grd_len(exp->before_stringize); i++) {
			if (exp->before_stringize[i]->kind == GRDC_PREP_TOKEN_KIND_HASH) {
				if (i + 1 >= grd_len(exp->before_stringize) || 
					(exp->before_stringize[i + 1]->kind != GRDC_PREP_TOKEN_KIND_IDENT &&
					 exp->before_stringize[i + 1]->kind != GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT
				)) {
					return { grdc_make_prep_file_error(p, exp->before_stringize[i + 1], "Expected macro argument after # in a macro") };
				}
				auto str_tok = grdc_make_token(p, GRDC_PREP_TOKEN_KIND_STRING, GRDC_PREP_TOKEN_SOURCE_STRINGIZE);
				str_tok->stringize_exp = exp;
				str_tok->stringize_tok = exp->before_stringize[i + 1];
				grdc_prep_use_custom_str(p, str_tok);
				grd_append(&str_tok->custom_str, U"\"");
				auto [arg_tokens, found] = grdc_get_arg_tokens(macro, tokens, exp->args, grdc_tok_str(exp->before_stringize[i + 1]));
				if (!found) {
					return { grdc_make_prep_file_error(p, exp->before_stringize[i + 1], "Macro argument is not found for stringizing") };
				}
				for (auto tok: arg_tokens) {
					grdc_grdc_prep_stringize_tok(str_tok, tok);
				}
				grd_append(&str_tok->custom_str, U"\"");
				auto b = grdc_make_token_set_builder(p->allocator);
				grdc_add_token(&b, str_tok);
				auto set = grdc_make_token_set(&b);
				grdc_add_tokens(&after_stringize_builder, set);
				i += 1;
			} else {
				grdc_add_tokens(&after_stringize_builder, exp->before_stringize[{i, i + 1}]);
			}
		}
		exp->after_stringize = grdc_make_parent_token_set(&after_stringize_builder);
		GrdLogTrace("head - after_stringize: %", grd_to_array(grd_map(exp->after_stringize, grd_lambda(x, grdc_tok_str(x)))));
		// Concat.
		auto after_concat_builder = grdc_make_token_set_parent_builder(p->allocator);
		for (s64 idx = 0; idx < grd_len(exp->after_stringize); idx++) {
			if (exp->after_stringize[idx]->kind == GRDC_PREP_TOKEN_KIND_GRD_CONCAT || (
			    idx + 1 < grd_len(exp->after_stringize) &&
			    exp->after_stringize[idx + 1]->kind == GRDC_PREP_TOKEN_KIND_GRD_CONCAT
			)) {
				s64 concat_idx = exp->after_stringize[idx]->kind == GRDC_PREP_TOKEN_KIND_GRD_CONCAT ? idx : idx + 1;
				// @TODO: use detailed printer to print the error.
				// @TODO: use detailed printer for every error reporting.
				if (concat_idx - 1 < 0 || grd_contains({GRDC_PREP_TOKEN_KIND_SPACE, GRDC_PREP_TOKEN_KIND_EOF}, exp->after_stringize[concat_idx - 1]->kind)) {
					return { grdc_make_prep_file_error(p, exp->after_stringize[concat_idx], "Missing left argument for ## operator") };
				}
				if (concat_idx + 1 >= grd_len(exp->after_stringize) || grd_contains({GRDC_PREP_TOKEN_KIND_SPACE, GRDC_PREP_TOKEN_KIND_EOF}, exp->after_stringize[concat_idx + 1]->kind)) {
					return { grdc_make_prep_file_error(p, exp->after_stringize[concat_idx], "Missing right argument for ## operator") };
				}
				GrdcToken* tok = grdc_make_token(p, GRDC_PREP_TOKEN_KIND_NONE, GRDC_PREP_TOKEN_SOURCE_GRD_CONCATTED);
				tok->concat_exp = exp;
				grdc_prep_use_custom_str(p, tok);
				auto lhs = exp->after_stringize[{concat_idx - 1, concat_idx}];
				auto rhs = exp->after_stringize[{concat_idx + 1, concat_idx + 2}];
				tok->concat_lhs = lhs[0];
				tok->concat_rhs = rhs[0];
				auto [lhs_toks, lhs_found] = grdc_get_arg_tokens(macro, tokens, exp->args, grdc_tok_str(lhs[0]));
				grd_println("lhs_found: %, lhs_toks: %", lhs_found, lhs_toks);
				if (lhs_found) {
					lhs = lhs_toks;
				}
				auto [rhs_toks, rhs_found] = grdc_get_arg_tokens(macro, tokens, exp->args, grdc_tok_str(rhs[0]));
				grd_println("rhs_found: %, rhs_toks: %", rhs_found, rhs_toks);
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
				auto [end, kind] = grdc_get_lang_token_end_at(tok->custom_str, 0);
				if (end != grd_len(tok->custom_str)) {
					auto e = grdc_make_prep_dp_error(p, grd_current_loc(), "Concatenated string '%' doesn't form a valid token", grdc_tok_str(tok));
					auto dp = grdc_make_detailed_printer(exp->after_stringize, true);
					grd_add(&dp->spans, { concat_idx, concat_idx + 1, 1 });
					grdc_add_dp(e, dp);
					return { e };
				}
				tok->kind = kind;
				auto concat_builder = grdc_make_token_set_builder(p->allocator);
				for (auto it: lhs_residue) {
					auto t = grdc_make_token(p, it->kind, GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					grdc_add_token(&concat_builder, t);
				}
				grdc_add_token(&concat_builder, tok);
				for (auto it: rhs_residue) {
					auto t = grdc_make_token(p, it->kind, GRDC_PREP_TOKEN_SOURCE_GRD_CONCAT_RESIDUE);
					t->concat_exp = exp;
					t->concat_residue_og = it;
					grdc_add_token(&concat_builder, t);
				}
				auto set = grdc_make_token_set(&concat_builder);
				grdc_add_tokens(&after_concat_builder, set);
				idx = concat_idx + 1;
				grd_println("lhs_residue: %", grd_to_array(grd_map(lhs_residue, grd_lambda(x, grdc_tok_str(x)))));
				grd_println("lhs: %", grd_to_array(grd_map(lhs, grd_lambda(x, grdc_tok_str(x)))));
				grd_println("rhs_residue: %", grd_to_array(grd_map(rhs_residue, grd_lambda(x, grdc_tok_str(x)))));
				grd_println("rhs: %", grd_to_array(grd_map(rhs, grd_lambda(x, grdc_tok_str(x)))));
			} else {
				grdc_add_tokens(&after_concat_builder, exp->after_stringize[{ idx, idx + 1 }]);
			}
		}
		exp->after_concat = grdc_make_parent_token_set(&after_concat_builder);
		GrdLogTrace("head - after_concat: %", grd_to_array(grd_map(exp->after_concat, grd_lambda(x, grdc_tok_str(x)))));
		// Prescan.
		auto prescan_builder = grdc_make_token_set_parent_builder(p->allocator);
		for (s64 i = 0; i < grd_len(exp->after_concat); i++) {
			GrdLogTrace(" -- prescan token: %, %", grdc_tok_str(exp->after_concat[i]), exp->after_concat[i]->kind);
			if (exp->after_concat[i]->kind == GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT) {
				auto [arg_tokens, found] = grdc_get_arg_tokens(macro, tokens, exp->args, grdc_tok_str(exp->after_concat[i]));
				if (found) {
					auto arg_set_builder = grdc_make_token_set_builder(p->allocator);
					for (auto it: arg_tokens) {
						auto t = grdc_make_token(p, it->kind, GRDC_PREP_TOKEN_SOURCE_PRESCAN_EXP);
						t->prescan_exp = exp;
						t->prescan_exp_og = it;
						grdc_add_token(&arg_set_builder, t);
					}
					auto toks = grdc_make_token_set(&arg_set_builder);
					// @TODO: free toks
					GrdLogTrace(" -- arg tokens: %", grdc_tok_arr_str(toks));
					s64 arg_cursor = 0;
					while (arg_cursor < grd_len(toks)) {
						if (toks[arg_cursor]->kind == GRDC_PREP_TOKEN_KIND_IDENT ||
						    toks[arg_cursor]->kind == GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT)
						{
							auto [e, sub_exp] = grdc_maybe_expand_macro(p, toks, &arg_cursor);
							if (e) {
								return { e };
							}
							if (sub_exp) {
								grdc_add_tokens(&prescan_builder, grdc_get_macro_exp_body(sub_exp));
								arg_cursor += 1;
								continue;
							}
						}
						grdc_add_tokens(&prescan_builder, toks[{arg_cursor, arg_cursor + 1}]);
						GrdLogTrace(" ---- add arg token: %", grdc_tok_str(toks[arg_cursor]));
						arg_cursor += 1;
					}
					continue;
				}
			}
			grdc_add_tokens(&prescan_builder, exp->after_concat[{i, i + 1}]);
		}
		exp->after_prescan = grdc_make_parent_token_set(&prescan_builder);
		GrdLogTrace("head - after_prescan: %", grd_to_array(grd_map(exp->after_prescan, grd_lambda(x, grdc_tok_str(x)))));
	}

	if (grdc_tok_str(macro->name) == "CHECK") {
		// GrdDebugBreak();
	}

	auto body = macro->is_object ? exp->before_stringize : exp->after_prescan;
	auto join_builder = grdc_make_token_set_parent_builder(p->allocator);
	grdc_add_tokens(&join_builder, body);
	grdc_add_tokens(&join_builder, tokens[{cursor, {}}]);
	auto joined = grdc_make_parent_token_set(&join_builder);
	s64  joined_mandatory_tokens = grd_len(body);
	// @TODO: free joined.

	GrdLogTrace("joined tokens: %", grdc_tok_arr_str(joined));

	auto rescan_builder = grdc_make_token_set_parent_builder(p->allocator);
	s64 rescan_cursor = 0;
	while (rescan_cursor < joined_mandatory_tokens && rescan_cursor < grd_len(joined)) {
		auto tok = joined[rescan_cursor];
		GrdLogTrace(" -- rescan token: %, cursor: %", grdc_tok_str(tok), rescan_cursor);
		if (tok->kind == GRDC_PREP_TOKEN_KIND_IDENT || tok->kind == GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT) {
			auto [e, sub_exp] = grdc_maybe_expand_macro(p, joined, &rescan_cursor);
			if (e) {
				return { e };
			}
			if (sub_exp) {
				grdc_add_tokens(&rescan_builder, grdc_get_macro_exp_body(sub_exp));
				continue;
			}
		}
		grdc_add_tokens(&rescan_builder, joined[{rescan_cursor, rescan_cursor + 1}]);
		rescan_cursor += 1;
	}
	exp->after_rescan = grdc_make_parent_token_set(&rescan_builder);
	GrdLogTrace("head - after_rescan: %", grd_to_array(grd_map(exp->after_rescan, grd_lambda(x, grdc_tok_str(x)))));

	if (grd_can_log(GrdLogLevel::Trace)) {
		// print steps.
		GrdLogTrace("finish - macro: %", grdc_tok_str(macro->name));
		GrdLogTrace("finish - before_stringize: %", grd_to_array(grd_map(exp->before_stringize, grd_lambda(x, grdc_tok_str(x)))));
		GrdLogTrace("finish - after_stringize: %", grd_to_array(grd_map(exp->after_stringize, grd_lambda(x, grdc_tok_str(x)))));
		GrdLogTrace("finish - after_concat: %", grd_to_array(grd_map(exp->after_concat, grd_lambda(x, grdc_tok_str(x)))));
		GrdLogTrace("finish - after_prescan: %", grd_to_array(grd_map(exp->after_prescan, grd_lambda(x, grdc_tok_str(x)))));
		GrdLogTrace("finish - after_rescan: %", grd_to_array(grd_map(exp->after_rescan, grd_lambda(x, grdc_tok_str(x)))));
	}

	// exp->exp_start = exp_start;
	// exp->exp_end = cursor;
	*out_cursor = cursor + (rescan_cursor - joined_mandatory_tokens);
	GrdLogTrace("Finished expanding macro: %", grdc_tok_str(macro->name));
	return { NULL, exp };
}

GRD_DEDUP GrdTuple<GrdError*, s64> grdc_prep_eval_expr(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor, s64 min_prec);
GRD_DEDUP GrdTuple<GrdError*, s64> grdc_prep_eval_leaf(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor) {
	*cursor = grdc_skip_spaces(tokens, *cursor + 1);
	auto tok = tokens[*cursor];
	auto tok_str = grdc_tok_str(tok);
	auto unary_op = grdc_find_prefix_unary_operator(tok_str, GRDC_AST_OP_FLAG_PREP);
	if (unary_op) {
		*cursor = grdc_skip_spaces(tokens, *cursor + 1);
		auto [e, rhs] = grdc_prep_eval_expr(p, tokens, cursor, unary_op->prec);
		if (e) {
			return { e };
		}
		if (tok_str == "-") {
			return { NULL, -rhs };
		} else if (tok_str == "+") {
			return { NULL, rhs };
		} else {
			return { grdc_make_prep_file_error(p, tok, "Unexpected unary operator '%*'", tok_str) };
		}
	}
	if (tok->kind == GRDC_PREP_TOKEN_KIND_NUMBER) {
		*cursor = grdc_skip_spaces(tokens, *cursor + 1);
		auto str = tok_str;
		s64 num;
		if (!grd_parse_integer(str, &num)) {
			return { grdc_make_prep_file_error(p, tok, "Failed to parse integer for preprocessor expression") };
		}
		return { NULL, num };
	}
	if (tok_str == "(") {
		auto [e, v] = grdc_prep_eval_expr(p, tokens, cursor, 0);
		if (e) {
			return { e };
		}
		if (grdc_tok_str(tokens[*cursor]) != ")") {
			return { grdc_make_prep_file_error(p, tokens[*cursor], "Expected ')' in preprocessor expression") };
		}
		return { NULL, v };
	}
	if (tok->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
		return { NULL, 0 }; // Ident didn't get expanded, so it's value is 0.
	}
	return { grdc_make_prep_file_error(p, tok, "Expected preprocessor expression") };
}

GRD_DEDUP GrdTuple<GrdError*, s64> grdc_prep_eval_expr(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor, s64 min_prec) {
	auto [e, v] = grdc_prep_eval_leaf(p, tokens, cursor);
	if (e) {
		return { e };
	}
	while (true) {
		auto tok = tokens[*cursor];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_EOF ||
			tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK)
		{
			break;
		}
		auto tok_str = grdc_tok_str(tok);
		auto op = grdc_find_binary_operator(tok_str, GRDC_AST_OP_FLAG_PREP);
		if (!op) {
			break;
		}
		if (op->prec < min_prec) {
			break;
		}
		if (op->op == "&&" && v == 0) {
			return { NULL, 0 };
		}
		if (op->op == "||" && v != 0) {
			return { NULL, 1 };
		}
		auto [e, rhs] = grdc_prep_eval_expr(p, tokens, cursor, (op->flags & GRDC_AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
		if (e) {
			return { e };
		}
		if (op->op == "-") {
			v -= rhs;
		} else if (op->op == "+") {
			v += rhs;
		} else if (op->op == "*") {
			v *= rhs;
		} else if (op->op == "/") {
			v /= rhs;
		} else if (op->op == "%") {
			v %= rhs;
		} else if (op->op == "&") {
			v &= rhs;
		} else if (op->op == "|") {
			v |= rhs;
		} else if (op->op == "^") {
			v ^= rhs;
		} else if (op->op == "<<") {
			v <<= rhs;
		} else if (op->op == ">>") {
			v >>= rhs;
		} else if (op->op == "==") {
			v = v == rhs;
		} else if (op->op == "!=") {
			v = v != rhs;
		} else if (op->op == "<") {
			v = v < rhs;
		} else if (op->op == ">") {
			v = v > rhs;
		} else if (op->op == "<=") {
			v = v <= rhs;
		} else if (op->op == ">=") {
			v = v >= rhs;
		} else if (op->op == "&&") {
			v = rhs;
		} else if (op->op == "||") {
			v = rhs;
		} else {
			return { grdc_make_prep_file_error(p, tok, "Unexpected operator '%' in preprocessor expression", op->op) };
		}
	}
	return { NULL, v };
}

GRD_DEDUP GrdTuple<GrdError*, s64> grdc_prep_eval_condition(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor) {
	s64 start = *cursor;
	while (*cursor < grd_len(tokens)) {
		auto tok = tokens[*cursor];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_EOF ||
			tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK)
		{
			break;
		}
		*cursor += 1;
	}
	GrdcTokenSetParentBuilder res_tokens_builder = grdc_make_token_set_parent_builder(p->allocator);
	auto expr_tokens = tokens[{start, *cursor + 1}];
	s64 expr_cursor = 0;
	while (expr_cursor < grd_len(expr_tokens)) {
		if (expr_tokens[expr_cursor]->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
			auto [e, exp] = grdc_maybe_expand_macro(p, expr_tokens, &expr_cursor);
			if (e) {
				return { e };
			}
			if (exp) {
				grdc_add_tokens(&res_tokens_builder, grdc_get_macro_exp_body(exp));
				continue;
			}
		}
		grdc_add_tokens(&res_tokens_builder, expr_tokens[{expr_cursor, expr_cursor + 1}]);
		expr_cursor += 1;
	}
	auto res_tokens = grdc_make_parent_token_set(&res_tokens_builder);
	expr_cursor = 0; // We don't care about expr_cursor anymore.
	// @TODO: free res_tokens.
	return grdc_prep_eval_expr(p, res_tokens, &expr_cursor, 0);
}

GRD_DEDUP bool grdc_prep_is_conditioned_out(GrdcPrep* p) {
	for (auto it: p->if_stack) {
		if (it.current_block != it.taken_block) {
			return true;
		}
	}
	return false;
}

GRD_DEDUP GrdError* grdc_handle_prep_directive(GrdcPrep* p, GrdcTokenSlice tokens, s64* cursor) {
	// Make sure only spaces preceed '#'.
	for (auto i: grd_reverse(grd_range_from_to(0, *cursor))) {
		auto tok = tokens[i];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_SPACE) {
			continue;
		}
		if (tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
			break;
		}
		return grdc_make_prep_file_error(p, tok, "Only spaces are allowed on the same line before preprocessor directive");
	}

	bool did_find_ident = false;
	s64 start_cursor = *cursor;
	while (++(*cursor) < grd_len(tokens)) {
		auto tok = tokens[*cursor];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
			did_find_ident = true;
			break;
		} else if (tok->kind == GRDC_PREP_TOKEN_KIND_SPACE) {
			continue;
		} else {
			return grdc_make_prep_file_error(p, tok, "Unexpected token after #");
		}
	}
	if (!did_find_ident) {
		if (grdc_prep_is_conditioned_out(p)) {
			return NULL;
		}
		return grdc_make_prep_file_error(p, tokens[start_cursor], "Expected an identifier after #");
	}
	auto directive_tok = tokens[*cursor];
	*cursor += 1;

	if (grdc_tok_str(directive_tok) == "if") {
		auto [e, v] = grdc_prep_eval_condition(p, tokens, cursor);
		if (e) {
			return e;
		}
		grd_add(&p->if_stack, { .current_block = 0, .taken_block = v != 0 ? 0 : -1 });
		return NULL;
	} else if (grdc_tok_str(directive_tok) == "else") {
		if (grd_len(p->if_stack) == 0) {
			return grdc_make_prep_file_error(p, directive_tok, "#else without #if");
		}
		p->if_stack[-1].current_block += 1;
		if (p->if_stack[-1].taken_block == -1) {
			p->if_stack[-1].taken_block = p->if_stack[-1].current_block;
		}
		return NULL;
	} else if (grdc_tok_str(directive_tok) == "elif") {
		if (grd_len(p->if_stack) == 0) {
			return grdc_make_prep_file_error(p, directive_tok, "#elif without #if");
		}
		p->if_stack[-1].current_block += 1;
		if (p->if_stack[-1].taken_block == -1) {
			auto [e, v] = grdc_prep_eval_condition(p, tokens, cursor);
			if (e) {
				return e;
			}
			if (v > 0) {
				p->if_stack[-1].taken_block = p->if_stack[-1].current_block;
			}
		}
		return NULL;
	} else if (grdc_tok_str(directive_tok) == "endif") {
		if (grd_len(p->if_stack) == 0) {
			return grdc_make_prep_file_error(p, directive_tok, "#endif without #if");
		}
		grd_pop(&p->if_stack);
		return NULL;
	} 
	if (grdc_prep_is_conditioned_out(p)) {
		return NULL;
	}
	if (grdc_tok_str(directive_tok) == "include") {
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

		GrdcTokenSetParentBuilder path_toks_builder = grdc_make_token_set_parent_builder(p->allocator);

		s64 exp_cursor = start;
		while (exp_cursor < end) {
			if (tokens[exp_cursor]->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
				auto [e, exp] = grdc_maybe_expand_macro(p, tokens, &exp_cursor);
				if (e) {
					return e;
				}
				if (exp) {
					grdc_add_tokens(&path_toks_builder, grdc_get_macro_exp_body(exp));
				} else {
					continue;
				}
			}
			grdc_add_tokens(&path_toks_builder, tokens[{exp_cursor, exp_cursor + 1}]);
			exp_cursor += 1;
		}

		auto path_toks = grdc_make_parent_token_set(&path_toks_builder);
		GrdLogTrace("path_toks: %", grdc_tok_arr_str(path_toks));
		// @TODO: free path_toks?

		// Remove preceeding and trailing spaces.
		s64 path_start = 0;
		s64 path_end = grd_len(path_toks);
		GrdLogTrace("start: % - %", path_start, path_end);
		for (s64 i = 0; i < grd_len(path_toks); i++) {
			if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				path_start = i;
				break;
			}
			GrdLogTrace(" skip start tok: %", grdc_tok_str(path_toks[i]));
		}
		for (s64 i = grd_len(path_toks) - 1; i >= 0; i--) {
			if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				path_end = i + 1;
				break;
			}
			GrdLogTrace(" skip end tok: %", grdc_tok_str(path_toks[i]));
		}
		GrdLogTrace("end: % - %", path_start, path_end);
		if (path_start >= path_end) {
			return grdc_make_prep_file_error(p, directive_tok, "Empty #include path");
		}
		GrdUnicodeString path;
		bool          is_global = false;
		if (path_toks[path_start]->kind == GRDC_PREP_TOKEN_KIND_STRING) {
			for (auto i: grd_range_from_to(path_start + 1, path_end - 1)) {
				if (path_toks[i]->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
					return grdc_make_prep_file_error(p, path_toks[i], "Unexpected token after #include path");
				}
			}
			path = grdc_tok_str_content(path_toks[path_start]);
		} else {
			// Check for starting < and trailing >
			if (grdc_tok_str(path_toks[path_start]) != "<") {
				return grdc_make_prep_file_error(p, path_toks[0], "Expected '<' at the start of #include path");
			}
			if (grdc_tok_str(path_toks[path_end - 1]) != ">") {
				return grdc_make_prep_file_error(p, path_toks[path_end - 1], "Expected '>' at the end of #include path");
			}
			auto toks = path_toks[{1, -1}];
			GrdAllocatedUnicodeString composed_include_path;
			composed_include_path.allocator = p->allocator;
			for (auto tok: toks) {
				grd_append(&composed_include_path, grdc_tok_str(tok));
			}
			path = composed_include_path;
			is_global = true;
		}

		auto fullpath = p->resolve_fullpath_hook(p, path, is_global);
		if (fullpath == "") {
			if (!is_global) {
				return grdc_make_prep_file_error(p, directive_tok, "Failed to resolve fullpath for #include \"%\"", path);
			} else {
				return grdc_make_prep_file_error(p, directive_tok, "Failed to resolve fullpath for #include <%>", path);
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
				return grdc_make_prep_file_error(p, directive_tok, "Failed to find #include file with fullpath '%'", fullpath);
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
		*cursor = end;
		// Eat line break if present, but don't eat EOF.
		// *cursor = include_trailing_newline_idx == -1 ? end - 1 : end;
		return NULL;
	} else if (grdc_tok_str(directive_tok) == "define") {
		s64 start_tok_idx = *cursor - 1;
		*cursor = grdc_skip_spaces_and_line_breaks(tokens, *cursor + 1);
		auto ident_tok = tokens[*cursor];
		if (ident_tok->kind != GRDC_PREP_TOKEN_KIND_IDENT) {
			return grdc_make_prep_file_error(p, ident_tok, "Expected an identifier after #define");
		}
		auto macro = grd_make<GrdcPrepMacro>(p->allocator);
		macro->def_site = p->include_site;
		macro->start_tok_idx = start_tok_idx;
		macro->name = ident_tok;
		grd_add(&p->macros, macro);
		*cursor += 1;
		if (*cursor < grd_len(tokens) &&
			grdc_tok_str(tokens[*cursor]) == "(" &&
			tokens[*cursor]->file_start == ident_tok->file_end)
		{
			macro->is_object = false;
			while (true) {
				*cursor = grdc_skip_spaces_and_line_breaks(tokens, *cursor + 1);
				auto arg_tok = tokens[*cursor];
				if (grdc_tok_str(arg_tok) == ")") {
					*cursor += 1;
					break;
				}
				if (grd_len(macro->arg_defs) >= 1) {
					if (grdc_tok_str(arg_tok) != ",") {
						return grdc_make_prep_file_error(p, arg_tok, "Expected ',' after macro argument");
					}
					*cursor = grdc_skip_spaces_and_line_breaks(tokens, *cursor + 1);
					arg_tok = tokens[*cursor];
				}
				// Parse ...
				if (grdc_tok_str(arg_tok) == ".") {
					if (*cursor + 1 < grd_len(tokens) && grdc_tok_str(tokens[*cursor + 1]) == ".") {
						if (*cursor + 2 < grd_len(tokens) && grdc_tok_str(tokens[*cursor + 2]) == ".") {
							arg_tok = grdc_make_token(p, GRDC_PREP_TOKEN_KIND_DOT_DOT_DOT, GRDC_PREP_TOKEN_SOURCE_DOT_DOT_DOT);
							arg_tok->dot_dot_dot_1 = tokens[*cursor];
							arg_tok->dot_dot_dot_2 = tokens[*cursor + 1];
							arg_tok->dot_dot_dot_3 = tokens[*cursor + 2];
							*cursor += 2;
						}
					}
				}
				if (arg_tok->kind != GRDC_PREP_TOKEN_KIND_DOT_DOT_DOT && arg_tok->kind != GRDC_PREP_TOKEN_KIND_IDENT) {
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
		}
		while (*cursor < grd_len(tokens)) {
			auto macro_tok = tokens[*cursor];
			if (macro_tok->kind != GRDC_PREP_TOKEN_KIND_SPACE) {
				break;
			}
			*cursor += 1;
		}
		s64 first_macro_body_tok = *cursor;
		while (*cursor < grd_len(tokens)) {
			auto macro_tok = tokens[*cursor];
			if (macro_tok->kind == GRDC_PREP_TOKEN_KIND_EOF || macro_tok->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK) {
				break;
			} else if (macro_tok->kind == GRDC_PREP_TOKEN_KIND_HASH) {
				*cursor += 1;
				if (*cursor >= grd_len(tokens) || tokens[*cursor]->kind != GRDC_PREP_TOKEN_KIND_IDENT) {
					return grdc_make_prep_file_error(p, macro_tok, "Expected macro argument after # in a macro");
				}
				auto def_tok_idx = grdc_find_macro_arg_def_tok_idx(macro, grdc_tok_str(tokens[*cursor]));
				if (def_tok_idx == -1) {
					return grdc_make_prep_file_error(p, macro_tok, "Macro argument is not found for stringizing");
				}
			}
			assert(macro_tok->kind != GRDC_PREP_TOKEN_KIND_NONE);
			*cursor += 1;
		}
		macro->end_tok_idx = *cursor;
		auto toks_builder = grdc_make_token_set_builder(p->allocator);
		for (auto tok: tokens[{first_macro_body_tok, *cursor}]) {
			auto nt = grdc_make_token(p, tok->kind, GRDC_PREP_TOKEN_SOURCE_MACRO_DEF);
			if (nt->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
				nt->kind = GRDC_PREP_TOKEN_KIND_MACRO_PRESCAN_IDENT;
			}
			nt->og_macro_def_tok = tok;
			grdc_add_token(&toks_builder, nt);
		}
		macro->tokens = grdc_make_token_set(&toks_builder);
		// GrdLogTrace("Parsing macro {%}, tokens:", grdc_tok_str(macro->name));
		// for (auto it: macro->tokens) {
		// 	GrdLogTrace("  %, %: %, %", grdc_tok_str(it), it->file_start, it->file_end, it->kind);
		// }
		return NULL;
	} else if (grdc_tok_str(directive_tok) == "error") {
		s64 start = *cursor;
		while (*cursor < grd_len(tokens)) {
			if (tokens[*cursor]->kind == GRDC_PREP_TOKEN_KIND_EOF ||
				tokens[*cursor]->kind == GRDC_PREP_TOKEN_KIND_LINE_BREAK)
			{
				break;
			}
			*cursor += 1;
		}
		auto msg = tokens[start]->included_file_og_tok->file_source->src[{tokens[start]->included_file_og_tok->file_start, tokens[*cursor]->included_file_og_tok->file_start}];
		auto e = grdc_make_prep_dp_error(p, grd_current_loc(), msg); 
		auto dp = grdc_make_detailed_printer(tokens, true);
		grd_add(&dp->spans, { start, *cursor, 1 });
		grdc_add_dp(e, dp);
		return e;
	}
	return grdc_make_prep_file_error(p, directive_tok, "Unknown preprocessor directive '%'", grdc_tok_str(directive_tok));
}

GRD_DEDUP GrdError* grdc_preprocess_file(GrdcPrep* p, GrdcPrepFileSource* source_file) {
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
	GrdcTokenSetBuilder inc_tokens = grdc_make_token_set_builder(p->allocator);
	for (auto tok: source_file->tokens) {
		auto nt = grdc_make_token(p, tok->kind, GRDC_PREP_TOKEN_SOURCE_INCLUDED_FILE);
		nt->included_file = included;
		nt->included_file_og_tok = tok;
		grdc_add_token(&inc_tokens, nt);
	}
	included->tokens = grdc_make_token_set(&inc_tokens);
	s64 cursor = 0;
	GrdcTokenSetParentBuilder b = grdc_make_token_set_parent_builder(p->allocator);
	while (cursor < grd_len(included->tokens)) {
		s64  tok_cursor = cursor;
		auto tok = included->tokens[cursor];
		if (tok->kind == GRDC_PREP_TOKEN_KIND_HASH) {
			auto e = grdc_handle_prep_directive(p, included->tokens, &cursor);
			if (e) {
				return e;
			}
			continue;
		}
		if (tok->kind == GRDC_PREP_TOKEN_KIND_EOF) {
			if (grd_len(p->if_stack) > 0) {
				// @TODO: add #if site.
				return grdc_make_prep_file_error(p, tok, "#if is not closed");
			}
			break;
		}
		if (grdc_prep_is_conditioned_out(p)) {
			cursor += 1;
			continue;
		}
		if (tok->kind == GRDC_PREP_TOKEN_KIND_IDENT) {
			auto [e, exp] = grdc_maybe_expand_macro(p, included->tokens, &cursor);
			if (e) {
				return e;
			}
			if (exp) {
				grdc_add_tokens(&p->tokens_builder, grdc_get_macro_exp_body(exp));
				continue;
			}
		}
		grdc_add_tokens(&p->tokens_builder, included->tokens[{tok_cursor, tok_cursor + 1}]);
		cursor += 1;
	}
	return NULL;
}

GRD_DEDUP GrdTuple<GrdError*, GrdcPrep*> grdc_preprocess(GrdUnicodeString str) {
	auto p = grdc_make_prep();
	auto file = grdc_make_mem_prep_file(p, str, U"<root_file>"_b);
	auto e = grdc_preprocess_file(p, file);
	return { e, p };
}

GRD_DEDUP GrdTuple<GrdError*, GrdcPrep*, GrdUnicodeString> grdc_preprocess_to_string(GrdUnicodeString str) {
	auto [e, p] = grdc_preprocess(str);
	if (e) {
		return { e };
	}
	return { NULL, p, grdc_prep_str(p) };
}
