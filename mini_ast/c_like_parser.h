#pragma once

#include "../error.h"
#include "../math/vector.h"
#include "../arena_allocator.h"
#include "../reflect.h"
#include "../array.h"
#include "../tuple.h"
#include "../string.h"
#include "../panic.h"


enum AstOperatorFlags {
	AST_OP_FLAG_LEFT_ASSOC = 1 << 0,
	AST_OP_FLAG_MOD_ASSIGN = 1 << 1,
	AST_OP_FLAG_BOOL       = 1 << 3,
	AST_OP_FLAG_INT        = 1 << 4,
	AST_OP_FLAG_PRIMITIVE  = 1 << 5,
	AST_OP_FLAG_NUMERIC    = 1 << 6,
	AST_OP_FLAG_POSTFIX    = 1 << 7,
	AST_OP_FLAG_PREFIX     = 1 << 8,
};

struct AstOperator {
	UnicodeString op;
	s32           prec;
	s32           flags = 0;

	REFLECT(AstOperator) {
		MEMBER(op);
		MEMBER(prec);
		MEMBER(flags);
	}
};

AstOperator AST_BINARY_OPERATORS_UNSORTED[] = { 
	{ U","_b, 10, AST_OP_FLAG_LEFT_ASSOC },
	{ U"="_b, 11, 0 },
	{ U"|="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"^="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"&="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"<<="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U">>="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"+="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"-="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"*="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"/="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"%="_b, 11, AST_OP_FLAG_MOD_ASSIGN },
	{ U"?"_b, 12, 0 },
	{ U"||"_b, 13, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_BOOL },
	{ U"&&"_b, 14, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_BOOL },
	{ U"|"_b, 15, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT },
	{ U"^"_b, 16, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT},
	{ U"&"_b, 17, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT},
	{ U"=="_b, 18, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_PRIMITIVE },
	{ U"!="_b, 18, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_PRIMITIVE},
	{ U"<"_b, 19, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC },
	{ U">"_b, 19, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC},
	{ U"<="_b, 19, AST_OP_FLAG_LEFT_ASSOC },
	{ U">="_b, 19, AST_OP_FLAG_LEFT_ASSOC },
	{ U"<<"_b, 20, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT },
	{ U">>"_b, 20, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT },
	{ U"+"_b, 21, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC },
	{ U"-"_b, 21, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC },
	{ U"*"_b, 22, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC },
	{ U"/"_b, 22, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_NUMERIC },
	{ U"%"_b, 22, AST_OP_FLAG_LEFT_ASSOC | AST_OP_FLAG_INT },
};

AstOperator AST_PREFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"!"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"~"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"+"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"-"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"++"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"--"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"*"_b, 30, AST_OP_FLAG_PREFIX },
	{ U"&"_b, 30, AST_OP_FLAG_PREFIX },
};

AstOperator AST_POSTFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"++"_b, 40, AST_OP_FLAG_POSTFIX },
	{ U"--"_b, 40, AST_OP_FLAG_POSTFIX },
};

enum CTokenFlags {
	CTOKEN_FLAG_FLOATING_POINT = 1 << 0,
	CTOKEN_FLAG_INTEGER = 1 << 1,
};

struct ProgramTextRegion {
	s64 start  = 0;
	s64 end    = 0;

	REFLECT(ProgramTextRegion) {
		MEMBER(start);
		MEMBER(end);
	}
};

struct Token {
	UnicodeString     str;
	ProgramTextRegion reg;
	u32               flags = 0;
};

struct CLikeProgram;
struct AstNode;
struct AstPrimitiveType;
struct AstStructType;
struct AstType;

struct CLikeParser {
	Allocator            allocator;
	CLikeProgram*        program;
	Array<AstNode*>      scope;
	UnicodeString        str;
	s64                  cursor = 0;
	Token                current_token;
	Array<UnicodeString> op_tokens_sorted;
	HashMap<AstType*, AstType*> ptr_types;

	AstPrimitiveType*    void_tp = NULL;
	AstPrimitiveType*    bool_tp = NULL;
	AstPrimitiveType*    s8_tp = NULL;
	AstPrimitiveType*    u8_tp = NULL;
	AstPrimitiveType*    s16_tp = NULL;
	AstPrimitiveType*    u16_tp = NULL;
	AstPrimitiveType*    s32_tp = NULL;
	AstPrimitiveType*    u32_tp = NULL;
	AstPrimitiveType*    s64_tp = NULL;
	AstPrimitiveType*    u64_tp = NULL;
	AstPrimitiveType*    f32_tp = NULL;
	AstPrimitiveType*    f64_tp = NULL;

	AstStructType*       float2_tp = NULL;
	AstStructType*       float3_tp = NULL;
	AstStructType*       float4_tp = NULL;
};

struct AstNode {
	Type*                       type;
	CLikeParser*                p = NULL;
	Optional<ProgramTextRegion> text_region;

	REFLECT(AstNode) {
		MEMBER(p);
		MEMBER(type);
			TAG(RealTypeMember{});
		MEMBER(text_region);
	}
};

template <typename T>
T* make_ast_node(CLikeParser* p, Optional<ProgramTextRegion> text_region) {
	auto x = make<T>(p->allocator);
	x->type = reflect_type_of<T>();
	x->p = p;
	x->text_region = text_region;
	return x;
}

struct AstSymbol: AstNode {
	UnicodeString name;
	bool          is_global = false;

	REFLECT(AstSymbol) {
		BASE_TYPE(AstNode);
		MEMBER(name);
		MEMBER(is_global);
	}
};

struct AstType: AstSymbol {
	u64 size = 0;
	u64 alignment = 0;

	REFLECT(AstType) {
		BASE_TYPE(AstSymbol);
		MEMBER(size);
		MEMBER(alignment);
	}
};

struct AstExpr: AstNode {
	AstType* expr_type = NULL;
	bool     is_lvalue = false;

	REFLECT(AstExpr) {
		BASE_TYPE(AstNode);
		MEMBER(expr_type);
		MEMBER(is_lvalue);
	}
};

constexpr bool AST_EXPR_LVALUE = true;
constexpr bool AST_EXPR_RVALUE = false;

template <typename T>
T* make_ast_expr(CLikeParser* p, AstType* expr_type, bool is_lvalue, Optional<ProgramTextRegion> text_region) {
	auto x = make_ast_node<T>(p, text_region);
	if (!expr_type) {
		panic("expr_type must be non-null");
	}
	x->expr_type = expr_type;
	x->is_lvalue = is_lvalue;
	return x;
}

struct AstBlock: AstNode {
	Array<AstNode*> statements;

	REFLECT(AstBlock) {
		BASE_TYPE(AstNode);
		MEMBER(statements);
	}
};


struct CLikeProgram: AstNode {
	Array<AstNode*>   globals; // Main diff between globals and global_syms is var decl groups vs var decls.
	Array<AstSymbol*> global_syms;

	REFLECT(CLikeProgram) {
		BASE_TYPE(AstNode);
		MEMBER(globals);
		MEMBER(global_syms);
	}
};

struct AstPrimitiveType: AstType {
	PrimitiveType* c_tp = NULL;
	bool           is_signed = false;

	REFLECT(AstPrimitiveType) {
		BASE_TYPE(AstType);
		MEMBER(c_tp);
		MEMBER(is_signed);
	}
};


struct AstVar: AstSymbol {
	AstType* var_type;
	
	REFLECT(AstVar) {
		BASE_TYPE(AstSymbol);
		MEMBER(var_type);
	}
};

struct ShaderIntrinVar: AstVar {

	REFLECT(ShaderIntrinVar) {
		BASE_TYPE(AstVar);
	}
};

struct AstFunctionArg: AstVar {

	REFLECT(AstFunctionArg) {
		BASE_TYPE(AstVar);
	}
};

enum class AstFunctionKind {
	Plain = 1,
	Vertex = 2,
	Fragment = 3,
};

struct AstFunction: AstSymbol {
	AstType*               return_type = NULL;
	Array<AstFunctionArg*> args;
	AstBlock*              block = NULL;
	AstFunctionKind        kind = AstFunctionKind::Plain;

	REFLECT(AstFunction) {
		BASE_TYPE(AstSymbol);
		MEMBER(return_type);
		MEMBER(args);
		MEMBER(block);
		MEMBER(kind);
	}
};

template <typename T>
T* make_ast_symbol(CLikeParser* p, UnicodeString name, Optional<ProgramTextRegion> text_region) {
	auto node = make_ast_node<T>(p, text_region);
	node->name = name;
	return node;
}

struct AstUnaryExpr: AstExpr {
	AstExpr*      expr;
	AstOperator*  op;

	REFLECT(AstUnaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(expr);
		MEMBER(op);
	}
};

struct AstDerefExpr: AstExpr {
	AstExpr* lhs;

	REFLECT(AstDerefExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
	}
};

struct AstBinaryExpr: AstExpr {
	AstExpr*      lhs = NULL;
	AstExpr*      rhs = NULL;
	AstOperator*  op = NULL;
	AstOperator*  pure_op = NULL;

	REFLECT(AstBinaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(rhs);
		MEMBER(op);
		MEMBER(pure_op);
	}
};

struct AstVarDecl: AstVar { 
	AstExpr*      init = NULL;

	REFLECT(AstVarDecl) {
		BASE_TYPE(AstVar);
		MEMBER(init);
	}
};


struct AstVarDeclGroup: AstNode {
	Array<AstVarDecl*> var_decls;

	REFLECT(AstVarDeclGroup) {
		BASE_TYPE(AstNode);
		MEMBER(var_decls);
	}
};

struct AstStructType;
struct AstAttr;

struct AstStructMember: AstNode {
	AstStructType*  struct_type = NULL;
	AstType*        member_type = NULL;
	UnicodeString   name;
	s64             offset = 0;
	Array<AstAttr*> attrs;

	REFLECT(AstStructMember) {
		BASE_TYPE(AstNode);
		MEMBER(struct_type);
		MEMBER(member_type);
		MEMBER(name);
		MEMBER(offset);
		MEMBER(attrs);
	}
};

struct AstStructType: AstType {
	Array<AstStructMember*> members;

	REFLECT(AstStructType) {
		BASE_TYPE(AstType);
		MEMBER(members);
	}
};

struct AstVarMemberAccess: AstExpr {
	AstExpr*         lhs;
	AstStructMember* member;

	REFLECT(AstVarMemberAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(member);
	}
};

struct AstArrayAccess: AstExpr {
	AstExpr*      lhs;
	AstExpr*      index;

	REFLECT(AstArrayAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(index);
	}
};

struct AstLiteralExpr: AstExpr {
	PrimitiveType* lit_type = NULL;
	PrimitiveValue lit_value = {};
};

enum CParserErrorTokenColor: u64 {
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_BLACK = 1,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED = 2,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN = 3,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_YELLOW = 4,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_BLUE = 5,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_MAGENTA = 6,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_CYAN = 7,
	CPARSER_ERROR_TOKEN_COLOR_REGULAR_WHITE = 8,
};

struct CParserErrorHighlight {
	s64 start;
	s64 length;
	u64 color = 0;
};

struct CParserErrorPiece {
	AllocatedUnicodeString       message;
	Array<CParserErrorHighlight> highlight_regions;

	void free() {
		message.free();
		highlight_regions.free();
	}
};

struct CLikeParserError: Error {
	AllocatedUnicodeString   program_text;
	Array<CParserErrorPiece> pieces;

	REFLECT(CLikeParserError) {
		BASE_TYPE(Error);
		MEMBER(program_text);
		MEMBER(pieces);
	}
};

CLikeParserError* make_parser_error(CLikeParser* p, CodeLocation loc, auto... args) {
	auto str = sprint_unicode(p->allocator, args...);
	defer { Free(p->allocator, str.data); };
	auto error = format_error<CLikeParserError>(loc, str);
	error->program_text = copy_string(p->str);
	error->on_free = [](Error* uncasted) {
		auto e = (CLikeParserError*) uncasted;
		e->program_text.free();
		for (auto it: e->pieces) {
			it.free();
		}
		e->pieces.free();
	};
	return error;
}

struct CParserErrorToken {
	Optional<ProgramTextRegion> reg;
	u64                         color = 0;
};

bool sort_parser_error_toks(Span<CParserErrorToken> toks, s64 a, s64 b) {
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

#include "../one_dim_intersect.h"

void add_site(CLikeParser* p, CLikeParserError* error, auto... tok_arglist) {
	CParserErrorToken toks_list[] = { tok_arglist... }; 

	Array<CParserErrorToken> toks = { .allocator = p->allocator };
	defer { toks.free(); };
	for (auto it: toks_list) {
		if (it.reg.has_value) {
			add(&toks, it);
		}
	}

	if (len(toks) == 0) {
		return;
	}

	sort(toks, sort_parser_error_toks);

	s64 toks_start = toks[0].reg.value.start;
	s64 toks_end = toks[-1].reg.value.end;

	assert(toks_start >= 0);
	assert(toks_end <= len(p->str));

	constexpr s64 RESIDUAL_LINES_COUNT = 3;

	s64 bottom_lines_count = 0;
	s64 top_lines_count = 0;
	s64 region_start = 0;
	s64 region_end = len(p->str);
	for (auto i: reverse(range(toks_start))) {
		if (is_line_break(p->str[i])) {
			top_lines_count += 1;
			if (top_lines_count >= 3) {
				region_start = i;
				break;
			}
		}
	}
	for (auto i: range_from_to(toks_end, len(p->str))) {
		if (is_line_break(p->str[i])) {
			bottom_lines_count += 1;
			if (bottom_lines_count >= RESIDUAL_LINES_COUNT) {
				region_end = i + 1;
				break;
			}
		}
	}

	Array<CParserErrorHighlight> result_highlights;
	add(&result_highlights, CParserErrorHighlight{
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
			auto highlight = CParserErrorHighlight{
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

	CParserErrorPiece piece = {
		.highlight_regions = result_highlights,
	};
	add(&error->pieces, piece);
}

void add_text(CLikeParser* p, CLikeParserError* error, auto... args) {
	CParserErrorPiece piece;
	piece.message = sprint_unicode(args...);
	add(&error->pieces, piece);
}

void print_parser_error(CLikeParserError* e) {
	print(e->text);
	for (auto it: e->pieces) {
		if (len(it.message) > 0) {
			println(it.message);
		}
		for (auto reg: it.highlight_regions) {
			auto text = e->program_text[reg.start, reg.start + reg.length];
			u64 regular_color = reg.color & 0xf;
			if (regular_color) {
				print("\x1b[0;%m", 30 + regular_color - 1);
			}
			print(text);
			print("\x1b[0m");
		}
	}
}

CLikeParserError* simple_parser_error(CLikeParser* p, CodeLocation loc, ProgramTextRegion reg, auto... args) {
	auto error = make_parser_error(p, loc, args...);
	if (reg.end > reg.start) {
		add_site(p, error, CParserErrorToken {
			.reg = reg,
			.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED,
		}); 
	}
	return error;
}

Token set_current_token(CLikeParser* p, s64 end, u32 flags = 0) {
	defer { p->cursor = end; };
	Token tok = {
		.str = p->str[p->cursor, end],
		.reg = { .start = p->cursor, .end = end },
		.flags = flags,
	};
	p->current_token = tok;
	return tok;
}

s64 maybe_eat_floating_point_literal(CLikeParser* p) {
	s64 start = p->cursor;
	s64 c = p->cursor;
	
	for (; c < len(p->str); c++) {
		if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
			break;
		}
	}
	if (c >= len(p->str)) {
		return 0;
	}
	bool have_stuff_before_dot = c > start;
	bool have_dot = false;
	bool have_stuff_after_dot = false;
	if (p->str[c] == '.') {
		have_dot = true;
		c += 1;
		s64 decimal_start = c;
		for (; c < len(p->str); c++) {
			if (!(p->str[c] >= '0' && p->str[c] <= '9')) { 
				break;
			}
		}
		have_stuff_after_dot = c > decimal_start;
		if (!have_stuff_before_dot && !have_stuff_after_dot) {
			return 0;
		}
	}
	if (c == start) {
		return 0;
	}
	bool have_exp = false;
	if (c < len(p->str)) {
		if (p->str[c] == 'e' || p->str[c] == 'E') {
			have_exp = true;
			c += 1;
			if (c >= len(p->str)) {
				return 0;
			}
			if (p->str[c] == '+' || p->str[c] == '-') {
				c += 1;
				if (c >= len(p->str)) {
					return 0;
				}
			}
			for (; c < len(p->str); c++) {
				if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
					break;
				}
			}
		}
	}
	bool have_suffix = false;
	if (c < len(p->str)) {
		if (p->str[c] == 'f' || p->str[c] == 'F' || p->str[c] == 'd' || p->str[c] == 'D') {
			have_suffix = true;
			c += 1;
		}
	}
	if ((!have_dot && !have_exp) && !have_suffix) {
		return 0;
	}
	return c;
}

s64 maybe_eat_integer_literal(CLikeParser* p) {
	auto start = p->str[p->cursor, {}];

	s32 prefix_len = 0;
	if (len(start) > 0 && (start[0] >= '0' && start[0] <= '9')) {
		prefix_len = 1;
	} else if (
		starts_with(start, U"0x"_b) ||
		starts_with(start, U"0X"_b) ||
		starts_with(start, U"0b"_b) ||
		starts_with(start, U"0B"_b) ||
		starts_with(start, U"0o"_b) ||
		starts_with(start, U"0O"_b))
	{
		prefix_len = 2;
	}

	for (auto i: range_from_to(p->cursor + prefix_len, len(p->str))) {
		if (p->str[i] >= '0' && p->str[i] <= '9') {
			continue;
		}
		if (prefix_len > 0) {
			if (p->str[i] >= 'a' && p->str[i] <= 'z') {
				continue;
			}
			if (p->str[i] >= 'A' && p->str[i] <= 'Z') {
				continue;
			}
		}
		return i != p->cursor ? i : 0;
	}
	return prefix_len > 0 ? len(p->str) : 0;
}

Token next(CLikeParser* p) {
	for (auto i: range_from_to(p->cursor, len(p->str))) {
		if (is_whitespace(p->str[i])) {
			if (p->cursor < i) {
				return set_current_token(p, i);
			} else {
				p->cursor = i + 1;
				continue;
			}
		}

		s64 fp_end = maybe_eat_floating_point_literal(p);
		if (fp_end != 0) {
			return set_current_token(p, fp_end, CTOKEN_FLAG_FLOATING_POINT);
		}

		s64 int_end = maybe_eat_integer_literal(p);
		if (int_end != 0) {
			return set_current_token(p, int_end, CTOKEN_FLAG_INTEGER);
		}

		for (auto c: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>', '[', ']'}) {
			if (p->str[i] == c) {
				if (p->cursor < i) {
					return set_current_token(p, i);
				} else {
					// parse multichar operators.
					auto txt = p->str[i, {}];
					for (auto it: p->op_tokens_sorted) {
						if (starts_with(txt, it)) {
							return set_current_token(p, i + len(it));
						}
					}
					return set_current_token(p, i + 1);
				}
			}
		}
	}
	return set_current_token(p, len(p->str));
}

Token peek(CLikeParser* p) {
	if (len(p->current_token.str) == 0) {
		return next(p);
	}
	return p->current_token;
}

Generator<AstSymbol*> resolve_symbols(AstNode* node) {
	if (auto sym = reflect_cast<AstSymbol>(node)) {
		co_yield sym;
		co_return;
	}
	if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(node)) {
		for (auto it: var_decl_group->var_decls) {
			auto gen = resolve_symbols(it);
			for (auto sym: gen) {
				co_yield sym;
			}
		}
	}
}

AstNode* lookup_symbol(CLikeParser* p, UnicodeString name) {
	for (auto i: reverse(range(len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto program = reflect_cast<CLikeProgram>(scope)) {
			for (auto child: program->globals) {
				for (auto sym: resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto block = reflect_cast<AstBlock>(scope)) {
			for (auto child: block->statements) {
				for (auto sym: resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto f = reflect_cast<AstFunction>(scope)) {
			for (auto arg: f->args) {
				if (arg->name == name) {
					return arg;
				}
			}
		}
	}
	return NULL;
}

Error* add_global(CLikeParser* p, AstNode* s) {
	for (auto sym: resolve_symbols(s)) {
		auto found = lookup_symbol(p, sym->name);
		if (found) {
			auto e = make_parser_error(p, current_loc(), "Duplicate global variable '%'"_b, sym->name);
			add_site(p, e, CParserErrorToken{ .reg = sym->text_region.value, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED });
			if (found->text_region.has_value) {
				add_text(p, e, U"Previous definition was here:"_b);
				add_site(p, e, CParserErrorToken{ .reg = found->text_region.value, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN });
			}
			return e;
		}
		sym->is_global = true;
	}
	add(&p->program->globals, s);
	return NULL;
}

AstType* find_type(CLikeParser* p, UnicodeString name) {
	if (name == "float") {
		name = U"f32"_b;
	}
	if (name == "double") {
		name = U"f64"_b;
	}
	if (name == "int") {
		name = U"s32"_b;
	}

	auto symbol = lookup_symbol(p, name);
	if (symbol) {
		if (auto tp = reflect_cast<AstType>(symbol)) {
			return tp;
		}
	}
	return NULL;
}

struct AstPointerType: AstType {
	AstType* pointee = NULL;

	REFLECT(AstPointerType) {
		BASE_TYPE(AstType);
		MEMBER(pointee);
	}
};

AstType* get_pointer_type(CLikeParser* p, AstType* type) {
	auto found = get(&p->ptr_types, type);
	if (found) {
		return *found;
	}
	auto name = sprint_unicode(p->allocator, U"%(*)*"_b, type->name);
	auto ptr = make_ast_symbol<AstPointerType>(p, name, {});
	ptr->pointee = type;
	put(&p->ptr_types, type, ptr);
	return ptr;
}

Tuple<AstType*, Error*> parse_type(CLikeParser* p) {
	auto tok = peek(p);
	auto c_str = encode_utf8(tok.str);
	defer { c_str.free(); };
	AstType* type = find_type(p, tok.str);
	if (!type) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Could not find type."_b) };
	}
	next(p);
	while (true) {
		tok = peek(p);
		if (tok.str == "*") {
			type = get_pointer_type(p, type);
			next(p);
		} else {
			break;
		}
	}
	return { type, NULL };
}

Tuple<Token, Error*> parse_ident(CLikeParser* p) {
	auto tok = peek(p);
	if (len(tok.str) == 0) {
		return { {}, simple_parser_error(p, current_loc(), tok.reg, U"Expected an identifier."_b) };
	}
	if ((tok.str[0] < 'a' || tok.str[0] > 'z') && (tok.str[0] < 'A' || tok.str[0] > 'Z')) {
		return { {}, simple_parser_error(p, current_loc(), tok.reg, U"First letter of an identifier must be a letter."_b) };
	}
	for (auto c: tok.str) {
		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_') {
			return { {}, simple_parser_error(p, current_loc(), tok.reg, U"Only letters, numbers, and underscores are allowed in identifiers."_b) };
		}
	}
	next(p);
	return { tok, NULL };
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p);
Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec);

Tuple<AstVarDeclGroup*, Error*> parse_var_decl(CLikeParser* p, AstType* type, Token type_tok, Token ident_tok, bool is_global) {
	Array<AstVarDecl*> var_decls = { .allocator = p->allocator };
	defer { var_decls.free(); };

	auto current_ident = ident_tok;
	while (true) {
		auto node = make_ast_node<AstVarDecl>(p, {});
		node->var_type = type;
		node->name = current_ident.str;
		node->is_global = is_global;
		add(&var_decls, node);

		auto tok = peek(p);
		if (tok.str != U","_b) {
			break;
		}
		next(p);
		auto [ident, e] = parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		current_ident = ident;
	}

	auto tok = peek(p);
	if (tok.str == U"="_b) {
		next(p);
		auto [expr, e] = parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		for (auto it: var_decls) {
			it->init = (AstExpr*) expr;
		}
	}

	tok = peek(p);
	if (tok.str != U";"_b) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected , or ;"_b) };
	}

	ProgramTextRegion text_region = { type_tok.reg.start, tok.reg.end };
	for (auto it: var_decls) {
		it->text_region = text_region;
	}

	auto node = make_ast_node<AstVarDeclGroup>(p, text_region);
	node->var_decls = var_decls;
	var_decls = {};
	return { node, NULL };
}

struct AstFunctionCall: AstExpr {
	AstNode*        f;
	Array<AstExpr*> args;

	REFLECT(AstFunctionCall) {
		BASE_TYPE(AstExpr);
		MEMBER(args);
	}
};

AstOperator* find_binary_operator(CLikeParser* p, UnicodeString op) {
	for (auto& it: AST_BINARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}

AstOperator* find_prefix_unary_operator(CLikeParser* p, UnicodeString op) {
	for (auto& it: AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}

AstOperator* find_postfix_unary_operator(CLikeParser* p, UnicodeString op) {
	for (auto& it: AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}


Tuple<AstExpr*, Error*> parse_function_call(CLikeParser* p, Token func_token, AstFunction* func) {
	auto tok = peek(p);
	if (tok.str != "("_b) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ("_b) };
	}
	auto call = make_ast_expr<AstFunctionCall>(p, func->return_type, AST_EXPR_RVALUE, {});
	call->args.allocator = p->allocator;
	call->f = func;
	next(p);
	while (true) {
		auto tok = peek(p);
		if (tok.str == ")"_b) {
			call->text_region = ProgramTextRegion { func_token.reg.start, tok.reg.end };
			next(p);
			break;
		}
		auto op = find_binary_operator(p, U","_b);
		auto [expr, e] = parse_expr(p, op->prec + 1);
		if (e) {
			return { NULL, e };
		}
		add(&call->args, expr);
		if (peek(p).str == ","_b) {
			next(p);
		}
	}
	return { call, NULL };
}

struct AstTernary: AstExpr {
	AstExpr* cond;
	AstExpr* then;
	AstExpr* else_;

	REFLECT(AstTernary) {
		BASE_TYPE(AstExpr);
		MEMBER(cond);
		MEMBER(then);
		MEMBER(else_);
	}
};

struct AstIf: AstNode {
	AstExpr*  cond = NULL;
	AstBlock* then = NULL;
	AstBlock* else_block = NULL;
	AstIf*    else_if = NULL;

	REFLECT(AstIf) {
		BASE_TYPE(AstNode);
		MEMBER(cond);
		MEMBER(then);
		MEMBER(else_block);
		MEMBER(else_if);
	}
};

AstType* resolve_type_alias(AstType* type) {
	// @TODO: implement.
	return type;
}

bool is_floating_point(AstType* type) {
	type = resolve_type_alias(type);
	if (type == type->p->f32_tp || type == type->p->f64_tp) {
		return true;
	}
	return false;
}

bool is_integer(AstType* type) {
	type = resolve_type_alias(type);
	if (
		type == type->p->s8_tp ||
		type == type->p->u8_tp ||
		type == type->p->s16_tp ||
		type == type->p->u16_tp ||
		type == type->p->s32_tp ||
		type == type->p->u32_tp ||
		type == type->p->s64_tp ||
		type == type->p->u64_tp
	) {
		return true;
	}
	return false;
}

bool is_numeric(AstType* type) {
	type = resolve_type_alias(type);
	return is_floating_point(type) || is_integer(type);
}

bool is_pointer(AstType* type) {
	type = resolve_type_alias(type);
	if (auto ptr = reflect_cast<AstPointerType>(type)) {
		return true;
	}
	return false;
}

bool is_array(AstType* type) {
	// @TODO: implement.
	return false;
}

AstType* get_element_type(AstType* type) {
	if (auto ptr = reflect_cast<AstPointerType>(type)) {
		return ptr->pointee;
	}
	return NULL;
}

struct AstVariableAccess: AstExpr {
	AstVar* var = NULL;

	REFLECT(AstVariableAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(var);
	}
};

bool is_struct(AstType* type) {
	type = resolve_type_alias(type);
	return reflect_cast<AstStructType>(type) != NULL;
}

bool is_bool(AstType* type) {
	type = resolve_type_alias(type);
	return type == type->p->bool_tp;
}

AstStructMember* find_struct_member(CLikeParser* p, AstStructType* type, UnicodeString name) {
	for (auto it: type->members) {
		if (it->name == name) {
			return it;
		}
	}
	return NULL;
}

struct AstSwizzleExpr: AstExpr {
	AstExpr* lhs;
	s32      swizzle[4];
	s32      swizzle_len = 0;

	REFLECT(AstSwizzleExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(swizzle);
		MEMBER(swizzle_len);
	}
};

bool parse_swizzle_ident(CLikeParser* p, UnicodeString ident, s32* swizzle, s32 src_len) {
	s64 ident_len = len(ident);
	if (ident_len > 4) {
		return false;
	}
	bool did_fail = false;
	for (auto i: range(ident_len)) {
		if (ident[i] == 'x') {
			swizzle[i] = 0;
		} else if (src_len >= 2 && ident[i] == 'y') {
			swizzle[i] = 1;
		} else if (src_len >= 3 && ident[i] == 'z') {
			swizzle[i] = 2;
		} else if (src_len >= 4 && ident[i] == 'w') {
			swizzle[i] = 3;
		} else {
			did_fail = true;
			break;
		}
	}
	if (!did_fail) {
		return true;
	}
	for (auto i: range(ident_len)) {
		if (ident[i] == 'r') {
			swizzle[i] = 0;
		} else if (src_len >= 2 && ident[i] == 'g') {
			swizzle[i] = 1;
		} else if (src_len >= 3 && ident[i] == 'b') {
			swizzle[i] = 2;
		} else if (src_len >= 4 && ident[i] == 'a') {
			swizzle[i] = 3;
		} else {
			return false;
		}
	}
	return true;
}

Tuple<AstExpr*, Error*> try_parse_swizzle_expr(CLikeParser* p, AstExpr* lhs, AstStructType* struct_tp, Token ident) {
	int swizzle_idx[4] = { -1, -1, -1, -1 };
	if (struct_tp == p->float2_tp) {
		bool ok = parse_swizzle_ident(p, ident.str, swizzle_idx, 2);
		if (!ok) {
			return { NULL, NULL };
		}
	} else if (struct_tp == p->float3_tp) {
		bool ok = parse_swizzle_ident(p, ident.str, swizzle_idx, 3);
		if (!ok) {
			return { NULL, NULL };
		}
	} else if (struct_tp == p->float4_tp) {
		bool ok = parse_swizzle_ident(p, ident.str, swizzle_idx, 4);
		if (!ok) {
			return { NULL, NULL };
		}
	} else {
		return { NULL, NULL };
	}

	int swizzle_len = 0;
	for (auto i: range(4)) {
		if (swizzle_idx[i] == -1) {
			break;
		}
		swizzle_len += 1;
	}

	AstType* swizzle_type = NULL;
	if (swizzle_len == 1) {
		swizzle_type = p->f32_tp;
	} else if (swizzle_len == 2) {
		swizzle_type = p->float2_tp;
	} else if (swizzle_len == 3) {
		swizzle_type = p->float3_tp;
	} else if (swizzle_len == 4) {
		swizzle_type = p->float4_tp;
	} else {
		panic(p, U"Unexpected swizzle_len = %"_b, swizzle_len);
	}

	ProgramTextRegion text_region = { ident.reg.start, peek(p).reg.start };
	if (lhs->text_region.has_value) {
		text_region.start = lhs->text_region.value.start;
	}

	if (swizzle_len == 1) {
		auto expr = make_ast_expr<AstVarMemberAccess>(p, swizzle_type, lhs->is_lvalue, text_region);
		expr->lhs = lhs;
		auto member = find_struct_member(p, struct_tp, ident.str);
		if (!member) {
			return { NULL, simple_parser_error(p, current_loc(), ident.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
		}
		expr->member = member;
		return { expr, NULL };
	}

	auto expr = make_ast_expr<AstSwizzleExpr>(p, swizzle_type, lhs->is_lvalue, text_region);
	expr->lhs = lhs;
	expr->swizzle[0] = swizzle_idx[0];
	expr->swizzle[1] = swizzle_idx[1];
	expr->swizzle[2] = swizzle_idx[2];
	expr->swizzle[3] = swizzle_idx[3];
	expr->swizzle_len = swizzle_len;
	return { expr, NULL };
}

struct AstStructInitMember {
	AstExpr*         expr = NULL;
	AstStructMember* member;
};

struct AstStructInitializer: AstExpr {
	AstType*                   struct_type = NULL;
	Array<AstStructInitMember> members;

	REFLECT(AstStructInitializer) {
		BASE_TYPE(AstExpr);
		MEMBER(struct_type);
		MEMBER(members);
	}
};

bool is_vector_type(AstType* type) {
	type = resolve_type_alias(type);
	return type == type->p->float2_tp || type == type->p->float3_tp || type == type->p->float4_tp;
}

bool is_integral(AstType* type) {
	return is_integer(type) || is_bool(type);
}

bool is_primitive(AstType* type) {
	return is_bool(type) || is_integer(type) || is_floating_point(type);
}

Tuple<AstExpr*, Error*> typecheck_binary_expr(CLikeParser* p, AstExpr* lhs, AstExpr* rhs, Token op_tok, AstOperator* op) {

	ProgramTextRegion text_region = { op_tok.reg.start, peek(p).reg.end };
	if (lhs->text_region.has_value) {
		text_region.start = lhs->text_region.value.start;
	}
	if (rhs->text_region.has_value) {
		text_region.end = rhs->text_region.value.end;
	}

	if (op->op == ",") {
		auto node = make_ast_expr<AstBinaryExpr>(p, rhs->expr_type, AST_EXPR_RVALUE, text_region);
		node->lhs = lhs;
		node->rhs = rhs;
		node->op = op;
		node->pure_op = op;
		return { node, NULL };
	}

	auto pure_op = op;
	AstType* expr_result_type = NULL;
	if ((op->flags & AST_OP_FLAG_MOD_ASSIGN) || op->op == "=") {
		if (op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			pure_op = find_binary_operator(p, op->op[0, len(op->op) - 1]);
		}
		if (!lhs->is_lvalue) {
			return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' can only be applied to lvalues.", op->op) };
		}
	}
	if (lhs->expr_type == find_type(p, U"void"_b)) {
		return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Cannot apply operator '%' to void", op->op) };
	}
	if (is_vector_type(lhs->expr_type)) {
		if (pure_op->op == "*" ||
			pure_op->op == "/" ||
			pure_op->op == "+" ||
			pure_op->op == "-" ||
			pure_op->op == "="
		) {
			if (rhs->expr_type != find_type(p, U"float"_b) && 
				rhs->expr_type != find_type(p, U"double"_b) &&
				lhs->expr_type != rhs->expr_type) {
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' for vector requires rhs to be float or double or matching vector type, got '%'. lhs type = '%'", op->op, rhs->expr_type->name, lhs->expr_type->name) };
			}
			expr_result_type = lhs->expr_type;
		} else {
			auto e = make_parser_error(p, current_loc(), "Operator '%' cannot be applied to '%'.", op->op, lhs->expr_type->name);

			add_site(p, e,
				CParserErrorToken{
					.reg = lhs->text_region,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN,
				},
				CParserErrorToken{
					.reg = op_tok.reg,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED,
				},
				CParserErrorToken{
					.reg = rhs->text_region,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_CYAN,
				}
			);

			return { NULL, e };
		}
	} else {
		if (pure_op->flags & AST_OP_FLAG_BOOL) {
			if (!is_bool(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' can only be applied to bool types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_INT) {
			if (!is_integer(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' can only be applied to integer types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_PRIMITIVE) {
			if (!is_primitive(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' can only be applied to primitive types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_NUMERIC) {
			if (!is_numeric(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (lhs->expr_type != rhs->expr_type) {
			auto e = make_parser_error(p, current_loc(), "Binary expression type mismatch, expected '%' but got '%'", lhs->expr_type->name, rhs->expr_type->name);

			add_site(p, e,
				CParserErrorToken{
					.reg = lhs->text_region,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN,
				},
				CParserErrorToken{
					.reg = op_tok.reg,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED,
				},
				CParserErrorToken{
					.reg = rhs->text_region,
					.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_CYAN,
				}
			);
			return { NULL, e };
		}
		if (op->op == "==" ||
			op->op == "!=" ||
			op->op == "<=" ||
			op->op == ">="
		) {
			expr_result_type = find_type(p, U"bool"_b);
			assert(expr_result_type);
		} else {
			expr_result_type = lhs->expr_type;
		}
	}
	if (!expr_result_type) {
		return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Unexpected error: could not determine result type of binary expression '%'.", op->op) };
	}
	auto node = make_ast_expr<AstBinaryExpr>(p, expr_result_type, AST_EXPR_RVALUE, text_region);
	node->lhs = lhs;
	node->rhs = rhs;
	node->op = op;
	node->pure_op = pure_op;
	return { node, NULL };
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p) {
	auto tok = peek(p);

	if (tok.flags & CTOKEN_FLAG_FLOATING_POINT) {
		auto lit = tok.str;
		bool parse_as_float = false;
		if (len(lit) > 0) {
			if (lit[len(lit) - 1] == 'f' || lit[len(lit) - 1] == 'F') {
				lit.count -= 1;
				parse_as_float = true;
			}
			if (lit[len(lit) - 1] == 'd' || lit[len(lit) - 1] == 'D') {
				lit.count -= 1;
			}
		}

		PrimitiveValue lit_value;
		PrimitiveType* lit_type = NULL;
		f64 float_val;
		f64 double_val;

		bool ok = false;
		if (parse_as_float) {
			ok = parse_float(lit, &lit_value.f32_value);
			lit_type = reflect_type_of<f32>()->as<PrimitiveType>();
		} else {
			ok = parse_float(lit, &lit_value.f64_value);
			lit_type = reflect_type_of<f64>()->as<PrimitiveType>();
		}
		if (!ok) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Could not parse floating point literal."_b) };
		}
		auto literal = make_ast_expr<AstLiteralExpr>(p, find_type(p, parse_as_float ? U"float"_b : U"double"_b), AST_EXPR_RVALUE, tok.reg);
		literal->lit_type = lit_type;
		literal->lit_value = lit_value;
		next(p);
		return { literal, NULL };
	}

	if (tok.flags & CTOKEN_FLAG_INTEGER) {
		s64 u;
		bool ok = parse_integer(tok.str, &u);
		if (!ok) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Could not parse integer literal."_b) };
		}
		auto literal = make_ast_expr<AstLiteralExpr>(p, find_type(p, U"int"_b), AST_EXPR_RVALUE, tok.reg);
		literal->lit_value.s64_value = u;
		literal->lit_type = reflect_type_of(u)->as<PrimitiveType>();
		next(p);
		return { literal, NULL };
	}

	auto unary_op = find_prefix_unary_operator(p, tok.str);
	if (unary_op) {
		auto op = tok;
		next(p);
		auto [expr, e] = parse_expr(p, unary_op->prec);
		if (e) {
			return { NULL, e };
		}
		if (tok.str == "*") {
			auto ptr_tp = reflect_cast<AstPointerType>(expr->expr_type);
			if (ptr_tp == NULL) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '*' can only be applied to pointer types, got '%'"_b, expr->expr_type->name) };
			}
			auto deref = make_ast_expr<AstDerefExpr>(p, ptr_tp->pointee, AST_EXPR_LVALUE, tok.reg);
			deref->lhs = expr;
			return { deref, NULL };
		}
		if (tok.str == "&") {
			if (!expr->is_lvalue) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '&' can only be applied to lvalues.", op.str) };
			}
		}
		if (tok.str == "++" || tok.str == "--") {
			if (!expr->is_lvalue) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", op.str) };
			}
		}
		if (tok.str == "!" || tok.str == "~") {
			if (!is_integer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to integer types.", op.str) };
			}
		}
		if (!is_numeric(expr->expr_type)) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op.str, expr->expr_type->name) };
		}
		ProgramTextRegion text_region = { op.reg.start, peek(p).reg.start };
		auto unary = make_ast_expr<AstUnaryExpr>(p, expr->expr_type, AST_EXPR_RVALUE, text_region);
		unary->expr = expr;
		unary->op = unary_op;
		return { unary, NULL };
	}

	if (tok.str == "("_b) {
		next(p);
		auto [expr, e] = parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		if (peek(p).str != ")"_b) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ')'.", tok) };
		}
		next(p);
		return { expr, NULL };
	}

	AstNode* sym = lookup_symbol(p, tok.str);
	if (!sym) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Unknown identifier.") };
	}
	AstExpr* lhs = NULL;
	if (auto type = reflect_cast<AstType>(sym)) {
		auto initializer_reg_start = tok.reg.start;
		next(p);
		if (peek(p).str == "("_b) {
			next(p);
			auto struct_tp = reflect_cast<AstStructType>(type);
			if (!struct_tp) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected struct type but got '%'"_b, type->name) };
			}
			s64 member_index = 0;
			Array<AstExpr*> args = { .allocator = p->allocator };
			defer { args.free(); };
			while (true) {
				auto tok = peek(p);
				if (tok.str == ")"_b) {
					if (len(args) != len(struct_tp->members)) {
						return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected % arguments in initializer but got '%'.", len(struct_tp->members), len(args)) };
					}
					ProgramTextRegion text_region = { initializer_reg_start, tok.reg.end };
					auto node = make_ast_expr<AstStructInitializer>(p, type, AST_EXPR_RVALUE, text_region);
					node->struct_type = type;
					node->members.allocator = p->allocator;
					s64 idx = 0;
					for (auto it: args) {
						auto member = struct_tp->members[idx];
						auto m = AstStructInitMember {
							.expr = it,
							.member = member,
						};
						add(&node->members, m);
						idx += 1;
					}
					args = {};
					next(p);
					lhs = node;
					break;
				}
				auto op = find_binary_operator(p, U","_b);
				auto [expr, e] = parse_expr(p, op->prec + 1);
				if (e) {
					return { NULL, e };
				}
				if (member_index >= len(struct_tp->members)) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Exceeded arguments count in initializer, struct '%' has % members.", type->name, len(struct_tp->members)) };
				}
				auto member = struct_tp->members[member_index];
				if (member->member_type != expr->expr_type) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", member->member_type->name, expr->expr_type->name) };
				}
				add(&args, expr);
				member_index += 1;
				if (peek(p).str == ","_b) {
					next(p);
				}
			}
		} else {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected struct initializer, got '%'.", tok.str) };
		}
	} else if (auto var = reflect_cast<AstVar>(sym)) {
		auto reg = tok.reg;
		next(p);
		auto expr = make_ast_expr<AstVariableAccess>(p, var->var_type, AST_EXPR_LVALUE, reg);
		expr->var = var;
		lhs = expr;
	} else if (auto func = reflect_cast<AstFunction>(sym)) {
		next(p);
		auto [call, e] = parse_function_call(p, tok, func);
		if (e) {
			return { NULL, e };
		}
		lhs = call;
	} else {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected type or variable"_b) };
	}

	while (true) {
		auto tok = peek(p);

		auto postfix_op = find_postfix_unary_operator(p, tok.str);
		if (postfix_op) {
			if (tok.str == "++"_b || tok.str == "--"_b) {
				if (!is_numeric(lhs->expr_type)) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types. Got '%'.", tok.str, lhs->expr_type->name) };
				}
				if (!lhs->is_lvalue) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", tok.str) };
				}
			}
			next(p);
			ProgramTextRegion text_region = { tok.reg.start, tok.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = make_ast_expr<AstUnaryExpr>(p, lhs->expr_type, AST_EXPR_RVALUE, text_region);
			node->expr = lhs;
			node->op = postfix_op;
			lhs = node;
			continue;
		}
		if (tok.str == "("_b) {
			// @TODO: support calling on function pointers.
			auto func = reflect_cast<AstFunction>(lhs);
			if (!func) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a function, got '%'"_b, lhs->type->name) };
			}
			auto [call, e] = parse_function_call(p, tok, func);
			if (e) {
				return { NULL, e };
			}
			lhs = call;
			continue;
		}
		if (tok.str == "."_b) {
			AstStructType* access_tp = NULL;
			if (auto struct_tp = reflect_cast<AstStructType>(lhs->expr_type)) {
				access_tp = struct_tp;
			} else if (auto ptr_tp = reflect_cast<AstPointerType>(lhs->expr_type)) {
				if (auto struct_tp = reflect_cast<AstStructType>(ptr_tp->pointee)) {
					access_tp = struct_tp;
				} else {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected struct type but got '%'"_b, ptr_tp->pointee->name) };
				}
			} else {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a struct, got '%'"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [ident, e] = parse_ident(p);
			if (e) {
				return { NULL, e };
			}
			
			auto [swizzle, ee] = try_parse_swizzle_expr(p, lhs, access_tp, ident);
			if (ee) {
				return { NULL, ee };
			}
			if (swizzle) {
				lhs = swizzle;
				continue;
			}

			auto member = find_struct_member(p, access_tp, ident.str);
			if (!member) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
			}
			ProgramTextRegion text_region = { ident.reg.start, ident.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = make_ast_expr<AstVarMemberAccess>(p, member->member_type, lhs->is_lvalue, text_region);
			node->lhs = lhs;
			node->member = member;
			lhs = node;
			continue;
		}
		if (tok.str == "["_b) {
			bool ok = is_pointer(lhs->expr_type) || is_array(lhs->expr_type);
			if (!ok) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected an array or pointer, got '%'"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			if (!is_integer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Array index must be an integer, got '%'"_b, expr->expr_type->name) };
			}
			if (peek(p).str != "]"_b) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a ]"_b) };
			}
			next(p);
			auto element_type = get_element_type(lhs->expr_type);
			if (!element_type) {
				panic(p, U"Unexpected error. expected an array or pointer, got %"_b, lhs->expr_type->name);
			}
			ProgramTextRegion text_region = { tok.reg.start, peek(p).reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = make_ast_expr<AstArrayAccess>(p, element_type, AST_EXPR_LVALUE, text_region);
			node->lhs = lhs;
			node->index = expr;
			lhs = node;
			continue;
		}
		break;
	}

	return { lhs, NULL };
}

Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec) {
	auto [lhs, e] = parse_primary_expr(p);
	if (e) {
		return { NULL, e };
	}

	while (true) {
		auto tok = peek(p);
		auto op = find_binary_operator(p, tok.str);
		if (!op) {
			break;
		}
		if (op->prec < min_prec) {
			break;
		}
		if (tok.str == "?"_b) {
			next(p);
			auto [then_expr, e2] = parse_expr(p, 0);
			if (e2) {
				return { NULL, e2 };
			}
			auto tok = peek(p);
			if (tok.str != ":"_b) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected :"_b) };
			}
			next(p);
			auto [else_expr, e3] = parse_expr(p, (op->flags & AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
			if (e3) {
				return { NULL, e3 };
			}
			if (lhs->expr_type != else_expr->expr_type) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Ternary expression type mismatch, expected '%' but got '%'.", lhs->expr_type->name, else_expr->expr_type->name) };
			}
			ProgramTextRegion text_region = { tok.reg.start, peek(p).reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			if (then_expr->text_region.has_value) {
				text_region.end = then_expr->text_region.value.end;
			}
			if (else_expr->text_region.has_value) {
				text_region.end = else_expr->text_region.value.end;
			}
			auto node = make_ast_expr<AstTernary>(p, lhs->expr_type, AST_EXPR_RVALUE, text_region);
			node->cond = lhs;
			node->then = then_expr;
			node->else_ = else_expr;
			lhs = node;
			continue;
		}
		next(p);
		auto [rhs, e] = parse_expr(p, (op->flags & AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
		if (e) {
			return { NULL, e };
		}

		auto [expr, e2] = typecheck_binary_expr(p, lhs, rhs, tok, op);
		if (e2) {
			return { NULL, e2 };
		}
		lhs = expr;
	}
	return { lhs, NULL };
}

Tuple<AstBlock*, Error*> parse_block(CLikeParser* p);
Tuple<AstNode*, Error*, bool> parse_stmt(CLikeParser* p);
Tuple<AstBlock*, Error*> parse_block_or_one_stmt(CLikeParser* p);

struct AstAttr: AstNode {
	UnicodeString   name;
	Array<AstExpr*> args;

	REFLECT(AstAttr) {
		BASE_TYPE(AstNode);
		MEMBER(name);
		MEMBER(args);
	}
};

Tuple<Array<AstAttr*>, Error*> parse_attrs(CLikeParser* p) {
	Array<AstAttr*> attrs;
	attrs.allocator = p->allocator;
	while (true) {
		auto start_tok = peek(p);
		if (start_tok.str != "[[") {
			break;
		}
		next(p);
		auto [name, e] = parse_ident(p);
		if (e) {
			return { {}, e };
		}
		auto tok = peek(p);
		Array<AstExpr*> args = { .allocator = p->allocator };
		if (tok.str == "(") {
			next(p);
			while (true) {
				auto tok = peek(p);
				if (tok.str == ")") {
					next(p);
					break;
				}
				auto [expr, e] = parse_expr(p, 0);
				if (e) {
					return { {}, e };
				}
				add(&args, expr);
				if (peek(p).str != ",") {
					next(p);
				}
			}
		}
		auto end_tok = peek(p);
		if (end_tok.str != "]]") {
			return { {}, simple_parser_error(p, current_loc(), tok.reg, U"Expected ]]"_b) };
		}
		next(p);
		ProgramTextRegion reg = { start_tok.reg.start, end_tok.reg.end };
		auto attr = make_ast_node<AstAttr>(p, reg);
		attr->args = args;
		attr->name = name.str;
		add(&attrs, attr);
	}
	return { attrs, NULL };
}

Tuple<AstIf*, Error*> parse_if(CLikeParser* p, Token if_tok) {
	if (peek(p).str != "("_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ("_b) };
	}
	next(p);
	auto [cond, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (cond->expr_type != find_type(p, U"bool"_b)) {
		auto error_reg = if_tok.reg;
		if (cond->text_region.has_value) {
			error_reg = cond->text_region.value;
		}
		return { NULL, simple_parser_error(p, current_loc(), error_reg, U"Condition must be bool, but it's '%'"_b, cond->expr_type->name) };
	}
	if (peek(p).str != ")"_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected )"_b) };
	}
	next(p);
	auto [then, e2] = parse_block_or_one_stmt(p);
	if (e2) {
		return { NULL, e2 };
	}
	ProgramTextRegion reg = if_tok.reg;
	if (then->text_region.has_value) {
		reg.end = then->text_region.value.end;
	}
	auto node = make_ast_node<AstIf>(p, reg);
	node->cond = cond;
	node->then = then;
	auto tok = peek(p);
	if (tok.str == U"else"_b) {
		next(p);
		tok = peek(p);
		if (tok.str == U"if"_b) {
			next(p);
			auto [else_if, e3] = parse_if(p, tok);
			if (e3) {
				return { NULL, e3 };
			}
			node->else_if = else_if;
			return { node, NULL };
		}
		auto [else_block, e3] = parse_block_or_one_stmt(p);
		if (e3) {
			return { NULL, e3 };
		}
		node->else_block = else_block;
		return { node, NULL };
	} else {
		return { node, NULL };
	}
}

Tuple<AstBlock*, Error*> parse_block_or_one_stmt(CLikeParser* p) {
	auto tok = peek(p);
	if (tok.str == "{"_b) {
		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		return { block, NULL };
	} else {
		auto [stmt, e, semicolon_opt] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		if (!semicolon_opt) {
			if (peek(p).str != U";"_b) {
				return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ;"_b) };
			}
			next(p);
		}
		auto block = make_ast_node<AstBlock>(p, stmt->text_region);
		add(&block->statements, stmt);
		return { block, NULL };
	}
}

struct AstFor: AstNode {
	AstExpr*  init_expr = NULL;
	AstExpr*  cond_expr = NULL;
	AstExpr*  incr_expr = NULL;
	AstBlock* body = NULL;

	REFLECT(AstFor) {
		BASE_TYPE(AstNode);
		MEMBER(init_expr);
		MEMBER(cond_expr);
		MEMBER(incr_expr);
		MEMBER(body);
	}
};

Tuple<AstNode*, Error*> parse_for(CLikeParser* p, Token for_tok) {
	if (peek(p).str != "("_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ("_b) };
	}
	next(p);
	auto [init_expr, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (peek(p).str != ";"_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ;"_b) };
	}
	next(p);
	auto [cond_expr, e2] = parse_expr(p, 0);
	if (e2) {
		return { NULL, e2 };
	}
	if (peek(p).str != ";"_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ;"_b) };
	}
	next(p);
	auto [incr_expr, e3] = parse_expr(p, 0);
	if (e3) {
		return { NULL, e3 };
	}
	if (peek(p).str != ")"_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected )"_b) };
	}
	next(p);
	auto [body, e4] = parse_block_or_one_stmt(p);
	if (e4) {
		return { NULL, e4 };
	}
	ProgramTextRegion text_region = { for_tok.reg.start, peek(p).reg.end };
	if (body->text_region.has_value) {
		text_region.end = body->text_region.value.end;
	}
	auto node = make_ast_node<AstFor>(p, text_region);
	node->init_expr = init_expr;
	node->cond_expr = cond_expr;
	node->incr_expr = incr_expr;
	node->body = body;
	return { node, NULL };
}

AstFunction* get_current_function(CLikeParser* p) {
	for (auto i: reverse(range(len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto f = reflect_cast<AstFunction>(scope)) {
			return f;
		}
	}
	return NULL;
}

struct AstReturn: AstNode {
	AstExpr* rhs = NULL;

	REFLECT(AstReturn) {
		BASE_TYPE(AstNode);
		MEMBER(rhs);
	}
};

Tuple<AstNode*, Error*, bool> parse_stmt(CLikeParser* p) {
	auto type_tok = peek(p);
	if (type_tok.str == "if"_b) {
		next(p);
		auto [if_, e] = parse_if(p, type_tok);
		if (e) {
			return { NULL, e };
		}
		return { if_, NULL, true };
	} else if (type_tok.str == "for"_b) {
		next(p);
		auto [for_, e] = parse_for(p, type_tok);
		if (e) {
			return { NULL, e };
		}
		return { for_, NULL, true };
	} else if (type_tok.str == "return"_b) {
		auto f = get_current_function(p);
		if (!f) {
			return { NULL, simple_parser_error(p, current_loc(), type_tok.reg, U"return must be inside a function"_b) };
		}
		next(p);

		AstExpr* rhs = NULL;
		if (f->return_type != p->void_tp) {
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			rhs = expr;
		}
		ProgramTextRegion reg = type_tok.reg;
		if (rhs->text_region.has_value) {
			reg.end = rhs->text_region.value.end;
		}
		auto node = make_ast_node<AstReturn>(p, reg);
		node->rhs = rhs;
		return { node, NULL };
	}

	auto type = find_type(p, type_tok.str);
	if (type) {
		next(p);
		auto [ident, e] = parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		auto [decl, ee] = parse_var_decl(p, type, type_tok, ident, false);
		if (ee) {
			return { NULL, ee };
		}
		return { decl, NULL, false };
	}
	auto [expr, e] = parse_expr(p, 0);
	return { expr, e, false };
}

Tuple<AstBlock*, Error*> parse_block(CLikeParser* p) {
	auto tok = peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected {"_b) };
	}
	next(p);
	auto block = make_ast_node<AstBlock>(p, {});
	block->statements.allocator = p->allocator;
	add(&p->scope, block);
	defer { pop(&p->scope); };

	s64 region_end = -1;
	
	while (true) {
		auto tok = peek(p);
		if (tok.str == U"}"_b) {
			region_end = tok.reg.end;
			next(p);
			break;
		}
		auto [stmt, e, semicolon_opt] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		if (!semicolon_opt) {
			if (peek(p).str != U";"_b) {
				return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ;"_b) };
			}
			next(p);
		}
		add(&block->statements, stmt);
	}
	assert(region_end != -1);
	ProgramTextRegion text_region = { tok.reg.start, region_end };
	block->text_region = text_region;
	return { block, NULL };
}

Tuple<AstSymbol*, Error*> parse_function(CLikeParser* p, AstType* return_type, AstFunctionKind kind, Token start_tok, Token ident) {
	Array<AstFunctionArg*> args = { .allocator = p->allocator };
	defer { args.free(); };

	next(p);

	while (true) {
		auto tok = peek(p);
		if (tok.str == U")"_b) {
			next(p);
			break;
		}
		if (len(args) > 0) {
			if (tok.str != U","_b) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ','"_b) };
			}
			next(p);
		}
		auto start = peek(p).reg.start;
		auto [type, e] = parse_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [arg_name, e2] = parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		auto [attrs, e3] = parse_attrs(p);
		if (e3) {
			return { NULL, e3 };
		}
		ProgramTextRegion text_region = { start, peek(p).reg.end };
		if (len(attrs) > 0) {
			auto last = attrs[-1];
			if (last->text_region.has_value) {
				text_region.end = last->text_region.value.end;
			}
		}
		auto arg_node = make_ast_symbol<AstFunctionArg>(p, arg_name.str, text_region);
		arg_node->var_type = type;
		add(&args, arg_node);
	}

	ProgramTextRegion text_region = { start_tok.reg.start, peek(p).reg.end };
	auto f = make_ast_node<AstFunction>(p, text_region);
	f->args.allocator = p->allocator;
	f->return_type = return_type;
	f->name = ident.str;
	f->args = args;
	f->kind = kind;

	auto tok = peek(p);
	if (tok.str == ";") {
		next(p);
		f->text_region.value.end = tok.reg.end;
		return { f, NULL };
	}
	if (tok.str == "{") {
		add(&p->scope, f);
		defer { pop(&p->scope); };

		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		f->block = (AstBlock*) block;
		if (f->block->text_region.has_value) {
			f->text_region.value.end = f->block->text_region.value.end;
		}
		return { f, NULL };
	}

	return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ; or { after function header"_b) };
}

Tuple<AstStructType*, Error*> parse_struct(CLikeParser* p, Token start_tok) {
	auto [ident, e] = parse_ident(p);
	if (e) {
		return { NULL, e };
	}
	auto tok = peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected {"_b) };
	}
	next(p);

	Array<AstStructMember*> members = { .allocator = p->allocator };
	defer { members.free(); };

	ProgramTextRegion st_reg;
	st_reg.start = start_tok.reg.start;
	st_reg.end = start_tok.reg.end;

	while (true) {
		auto tok = peek(p);
		if (tok.str == U"}"_b) {
			st_reg.end = tok.reg.end;
			next(p);
			break;
		}
		auto [tp, e] = parse_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [name, e2] = parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		tok = peek(p);
		AstExpr* init_expr = NULL;
		if (tok.str == "=") {
			next(p);
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			init_expr = expr;
		}
		auto [attrs, e3] = parse_attrs(p);
		if (e3) {
			return { NULL, e3 };
		}

		if (peek(p).str != ";") {
			return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ;"_b) };
		}
		next(p);

		ProgramTextRegion reg;
		reg.start = start_tok.reg.start;
		reg.end = name.reg.end;
		if (init_expr && init_expr->text_region.has_value) {
			reg.end = init_expr->text_region.value.end;
		}
		if (len(attrs) > 0) {
			auto last = attrs[-1];
			if (last->text_region.has_value) {
				reg.end = last->text_region.value.end;
			}
		}
		auto member = make_ast_node<AstStructMember>(p, reg);
		member->member_type = tp;
		member->name = name.str;
		member->attrs = attrs;
		add(&members, member);
	}

	auto st = make_ast_node<AstStructType>(p, st_reg);
	st->members = members;
	st->name = ident.str;
	members = {};
	return { st, NULL };
}

Error* parse_top_level(CLikeParser* p) {
	auto [attrs, e1] = parse_attrs(p);
	if (e1) {
		return e1;
	}

	auto start_tok = peek(p);

	if (start_tok.str == "struct") {
		next(p);
		auto [type, e] = parse_struct(p, start_tok);
		if (e) {
			return e;
		}
		e = add_global(p, type);
		if (e) {
			return e;
		}
		return NULL;
	}
	
	auto type_tok = peek(p);
	auto [type, error] = parse_type(p);
	if (error) {
		return error;
	}
	auto [ident, e] = parse_ident(p);
	if (e) {
		return e;
	}
	auto tok = peek(p);
	if (tok.str == U"("_b) {
		auto [f, e] = parse_function(p, type, AstFunctionKind::Plain, start_tok, ident);
		if (e) {
			return e;
		}
		e = add_global(p, f);
		if (e) {
			return e;
		}
		return NULL;
	} else if (tok.str == U","_b || tok.str == U";"_b || tok.str == U"="_b) {
		auto [group, e] = parse_var_decl(p, type, type_tok, ident, true);
		if (e) {
			return e;
		}
		e = add_global(p, group);
		if (e) {
			return e;
		}
		return NULL;
	} else {
		return simple_parser_error(p, current_loc(), tok.reg, U"Expected ( or , or ; or ="_b);
	}
}

template <typename T>
AstPrimitiveType* push_prim_type(CLikeParser* p, UnicodeString name, u64 size, u64 alignment, bool is_signed) {
	auto tp = make_ast_symbol<AstPrimitiveType>(p, name, {});
	tp->size = size;
	tp->alignment = alignment;
	tp->is_signed = is_signed;
	tp->c_tp = reflect_type_of<T>()->template as<PrimitiveType>();
	assert(tp->c_tp);
	auto e = add_global(p, tp);
	if (e) {
		panic(e);
	}
	return tp;
}

u64 calc_struct_size(AstStructType* tp) {
	u64 end = 0;
	u64 max_member_align = 1;
	for (auto it: tp->members) {
		u64 m_end = it->member_type->size + it->offset;
		if (m_end > end) {
			end = m_end;
		}
		if (it->member_type->alignment > max_member_align) {
			max_member_align = it->member_type->alignment;
		}
	}
	u64 size = align(end, max_member_align);
	return size;
}

void push_member(AstStructType* tp, UnicodeString name, AstType* type) {
	auto m = make_ast_node<AstStructMember>(tp->p, {});
	m->struct_type = tp;
	m->member_type = type;
	m->name = name;
	auto sz = calc_struct_size(tp);
	m->offset = align(sz, type->alignment);
	add(&tp->members, m);
}

void push_base_types(CLikeParser* p) {
	p->void_tp = push_prim_type<void>(p, U"void"_b, 0, 1, false);
	p->bool_tp = push_prim_type<bool>(p, U"bool"_b, 1, 1, false);
	p->s8_tp  = push_prim_type<s8>(p, U"s8"_b, 1, 1, true);
	p->s16_tp = push_prim_type<s16>(p, U"s16"_b, 2, 2, true);
	p->s32_tp = push_prim_type<s32>(p, U"s32"_b, 4, 4, true);
	p->s64_tp = push_prim_type<s64>(p, U"s64"_b, 8, 8, true);
	p->u8_tp  = push_prim_type<u8>(p, U"u8"_b, 1, 1, false);
	p->u16_tp = push_prim_type<u16>(p, U"u16"_b, 2, 2, false);
	p->u32_tp = push_prim_type<u32>(p, U"u32"_b, 4, 4, false);
	p->u64_tp = push_prim_type<u64>(p, U"u64"_b, 8, 8, false);
	p->f32_tp = push_prim_type<f32>(p, U"f32"_b, 4, 4, true);
	p->f64_tp = push_prim_type<f64>(p, U"f64"_b, 8, 8, true);

	p->float2_tp = make_ast_symbol<AstStructType>(p, U"float2"_b, {});
	push_member(p->float2_tp, U"x"_b, p->f32_tp);
	push_member(p->float2_tp, U"y"_b, p->f32_tp);
	p->float2_tp->alignment = 8;
	p->float2_tp->size = 8;
	auto e = add_global(p, p->float2_tp);
	if (e) {
		panic(e);
	}

	p->float3_tp = make_ast_symbol<AstStructType>(p, U"float3"_b, {});
	push_member(p->float3_tp, U"x"_b, p->f32_tp);
	push_member(p->float3_tp, U"y"_b, p->f32_tp);
	push_member(p->float3_tp, U"z"_b, p->f32_tp);
	p->float3_tp->alignment = 16;
	p->float3_tp->size = 16;
	e = add_global(p, p->float3_tp);
	if (e) {
		panic(e);
	}

	p->float4_tp = make_ast_symbol<AstStructType>(p, U"float4"_b, {});
	push_member(p->float4_tp, U"x"_b, p->f32_tp);
	push_member(p->float4_tp, U"y"_b, p->f32_tp);
	push_member(p->float4_tp, U"z"_b, p->f32_tp);
	push_member(p->float4_tp, U"w"_b, p->f32_tp);
	p->float4_tp->alignment = 16;
	p->float4_tp->size = 16;
	e = add_global(p, p->float4_tp);
	if (e) {
		panic(e);
	}
}


struct ShaderIntrinFunc: AstFunction {

	REFLECT(ShaderIntrinFunc) {
		BASE_TYPE(AstFunction);
	}
};

void add_shader_intrinsic_var(CLikeParser* p, UnicodeString name, AstType* type) {
	// @TODO: check for duplicates
	if (!type) {
		panic("No type for shader intrinsic var");
	}
	auto node = make_ast_symbol<ShaderIntrinVar>(p, name, {});
	node->var_type = type;
	auto e = add_global(p, node);
	if (e) {
		panic(e);
	}
}

void add_shader_intrinsic_func(CLikeParser* p, UnicodeString name, AstType* return_type, std::initializer_list<AstType*> args) {
	// @TODO: check for duplicates
	auto node = make_ast_symbol<ShaderIntrinFunc>(p, name, {});
	node->args.allocator = p->allocator;
	node->return_type = return_type;
	s64 i = 0;
	for (auto arg: args) {
		auto name = sprint_unicode(p->allocator, U"arg_%"_b, i);
		auto arg_node = make_ast_symbol<AstFunctionArg>(p, name, {});
		arg_node->var_type = arg;
		add(&node->args, arg_node);
		i += 1;
	}
	auto e = add_global(p, node);
	if (e) {
		panic(e);
	}
}

Tuple<CLikeProgram*, Error*> parse_c_like(UnicodeString str) {
	auto allocator = make_arena_allocator();
	auto p = make<CLikeParser>(allocator);
	p->allocator = allocator;
	p->program = make_ast_node<CLikeProgram>(p, {});
	p->program->globals.allocator = p->allocator;
	p->str = str;
	add(&p->scope, p->program);
	for (auto it: AST_BINARY_OPERATORS_UNSORTED) {
		add(&p->op_tokens_sorted, it.op);
	}
	for (auto it: AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		add(&p->op_tokens_sorted, it.op);
	}
	for (auto it: AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		add(&p->op_tokens_sorted, it.op);
	}
	add(&p->op_tokens_sorted, U"[["_b);
	add(&p->op_tokens_sorted, U"]]"_b);
	sort(p->op_tokens_sorted, lambda(len($0[$1]) > len($0[$2]))); 

	push_base_types(p);

	add_shader_intrinsic_func(p, U"hsv"_b, p->float3_tp, { p->f32_tp, p->f32_tp, p->f32_tp });
	add_shader_intrinsic_func(p, U"log"_b, p->f32_tp, { p->f32_tp });
	add_shader_intrinsic_func(p, U"sin"_b, p->f32_tp, { p->f32_tp });
	add_shader_intrinsic_func(p, U"cos"_b, p->f32_tp, { p->f32_tp });
	add_shader_intrinsic_func(p, U"dot_vec3"_b, p->f32_tp, { p->float3_tp, p->float3_tp });
	add_shader_intrinsic_func(p, U"atan"_b, p->f32_tp, { p->f32_tp });
	add_shader_intrinsic_func(p, U"length"_b, p->f32_tp, { p->float3_tp });
	add_shader_intrinsic_func(p, U"abs"_b, p->f32_tp, { p->f32_tp });

	while (peek(p).str != ""_b) {
		if (peek(p).str == U";"_b) {
			next(p);
			continue;
		}
		auto e = parse_top_level(p);
		if (e) {
			return { NULL, e };
		}
	}
	return { p->program, NULL };
}
