#pragma once

#include "../error.h"
#include "../math/vector.h"
#include "../arena_allocator.h"
#include "../reflect.h"
#include "../array.h"
#include "../tuple.h"
#include "../string.h"
#include "../panic.h"

struct ProgramTextRegion {
	s64 start  = 0;
	s64 end    = 0;
};

struct AstNode {
	Type*                       type;
	Optional<ProgramTextRegion> text_region;

	REFLECT(AstNode) {
		MEMBER(type);
			TAG(RealTypeMember{});
		MEMBER(text_region);
	}
};

template <typename T>
T* make_ast_node(Allocator allocator, Optional<ProgramTextRegion> text_region) {
	auto x = make<T>(allocator);
	x->type = reflect_type_of<T>();
	x->text_region = text_region;
	return x;
}

struct AstSymbol: AstNode {
	UnicodeString name;

	REFLECT(AstSymbol) {
		BASE_TYPE(AstNode);
		MEMBER(name);
	}
};

struct AstType: AstSymbol {
	REFLECT(AstType) {
		BASE_TYPE(AstSymbol);
	}
};

struct CTypeAlias: AstType {
	Type* c_type;
	
	REFLECT(CTypeAlias) {
		BASE_TYPE(AstType);
		MEMBER(c_type);
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
T* make_ast_expr(Allocator allocator, AstType* expr_type, bool is_lvalue, Optional<ProgramTextRegion> text_region) {
	auto x = make_ast_node<T>(allocator, text_region);
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
	Array<AstNode*> globals;

	REFLECT(CLikeProgram) {
		BASE_TYPE(AstNode);
		MEMBER(globals);
	}
};


enum AstOperatorFlags {
	AST_OP_FLAG_LEFT_ASSOC = 1 << 0,
	AST_OP_FLAG_MOD_ASSIGN = 1 << 1,
	AST_OP_FLAG_BOOL       = 1 << 3,
	AST_OP_FLAG_INT        = 1 << 4,
	AST_OP_FLAG_PRIMITIVE  = 1 << 5,
	AST_OP_FLAG_NUMERIC    = 1 << 6,
};

struct AstOperator {
	UnicodeString op;
	s32           prec;
	s32           flags = 0;
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
	{ U"!"_b, 30 },
	{ U"~"_b, 30 },
	{ U"+"_b, 30 },
	{ U"-"_b, 30 },
	{ U"++"_b, 30 },
	{ U"--"_b, 30 },
	{ U"*"_b, 30 },
	{ U"&"_b, 30 },
};

AstOperator AST_POSTFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"++"_b, 40 },
	{ U"--"_b, 40 },
};

enum CTokenFlags {
	CTOKEN_FLAG_FLOATING_POINT = 1 << 0,
	CTOKEN_FLAG_INTEGER = 1 << 1,
};

struct Token {
	UnicodeString     str;
	ProgramTextRegion reg;
	u32               flags = 0;
};

struct CLikeParser {
	Allocator            allocator;
	CLikeProgram*        program;
	Array<AstNode*>      scope;
	UnicodeString        str;
	s64                  cursor = 0;
	Token                current_token;
	Array<UnicodeString> op_tokens_sorted;
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

struct AstFunctionArg: AstNode {
	UnicodeString name;
	AstType*      arg_type = NULL;

	REFLECT(AstFunctionArg) {
		BASE_TYPE(AstNode);
		MEMBER(name);
		MEMBER(arg_type);
	}
};

struct AstFunction: AstSymbol {
	AstType*               return_type = NULL;
	Array<AstFunctionArg*> args;
	AstBlock*              block = NULL;

	REFLECT(AstFunction) {
		BASE_TYPE(AstSymbol);
		MEMBER(args);
		MEMBER(block);
	}
};

struct ShaderIntrinFunc: AstFunction {

	REFLECT(ShaderIntrinFunc) {
		BASE_TYPE(AstFunction);
	}
};

void add_c_type_alias(CLikeParser* p, Type* type, UnicodeString name) {
	// @TODO: check for duplicates
	auto node = make_ast_node<CTypeAlias>(p->allocator, {});
	node->c_type = type;
	node->name = name;
	add(&p->program->globals, node);
}

void add_shader_intrinsic_var(CLikeParser* p, UnicodeString name, AstType* type) {
	// @TODO: check for duplicates
	if (!type) {
		panic("No type for shader intrinsic var");
	}
	auto node = make_ast_node<ShaderIntrinVar>(p->allocator, {});
	node->var_type = type;
	node->name = name;
	add(&p->program->globals, node);
}

void add_shader_intrinsic_func(CLikeParser* p, UnicodeString name, AstType* return_type, std::initializer_list<AstType*> args) {
	// @TODO: check for duplicates
	auto node = make_ast_node<ShaderIntrinFunc>(p->allocator, {});
	node->args.allocator = p->allocator;
	node->name = name;
	node->return_type = return_type;
	s64 i = 0;
	for (auto arg: args) {
		auto arg_node = make_ast_node<AstFunctionArg>(p->allocator, {});
		arg_node->arg_type = arg;
		arg_node->name = sprint_unicode(p->allocator, U"arg_%"_b, i);
		add(&node->args, arg_node);
		i += 1;
	}
	add(&p->program->globals, node);
}

struct CPostfixExpr: AstExpr {
	AstNode* lhs;
	UnicodeString op;

	REFLECT(CPostfixExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(op);
	}
};

struct CUnaryExpr: AstExpr {
	AstNode*      expr;
	UnicodeString op;

	REFLECT(CUnaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(expr);
		MEMBER(op);
	}
};

struct CBinaryExpr: AstExpr {
	AstNode*      lhs = NULL;
	AstNode*      rhs = NULL;
	UnicodeString op;

	REFLECT(CBinaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(rhs);
		MEMBER(op);
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

struct AstVarMemberAccess: AstExpr {
	AstNode*      lhs;
	UnicodeString member;

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

struct LiteralExpr: AstExpr {
	Token tok;
	f32   f32_value = 0;
	f64   f64_value = 0;
	u64   u64_value = 0;
	s64   s64_value = 0;

	REFLECT(LiteralExpr) {
		BASE_TYPE(AstNode);
		MEMBER(tok);
		MEMBER(f32_value);
		MEMBER(f64_value);
		MEMBER(u64_value);
		MEMBER(s64_value);
	}
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
	CParserErrorPiece piece = {
		.message = sprint_unicode(args...),
	};
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

		for (auto c: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>'}) {
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

AstNode* resolve_symbol(AstNode* node, UnicodeString name) {
	if (auto sym = reflect_cast<AstSymbol>(node)) {
		if (sym->name == name) {
			return sym;
		}
	}
	if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(node)) {
		for (auto it: var_decl_group->var_decls) {
			if (auto symbol = resolve_symbol(it, name)) {
				return symbol;
			}
		}
	}
	return NULL;
}

AstNode* lookup_symbol(CLikeParser* p, UnicodeString name) {
	for (auto i: reverse(range(len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto program = reflect_cast<CLikeProgram>(scope)) {
			for (auto child: program->globals) {
				if (auto symbol = resolve_symbol(child, name)) {
					return symbol;
				}
			}
		}
		if (auto block = reflect_cast<AstBlock>(scope)) {
			for (auto child: block->statements) {
				if (auto symbol = resolve_symbol(child, name)) {
					return symbol;
				}
			}
		}
	}
	return NULL;
}

AstType* find_type(CLikeParser* p, UnicodeString name) {
	auto symbol = lookup_symbol(p, name);
	if (symbol) {
		if (auto tp = reflect_cast<AstType>(symbol)) {
			return tp;
		}
	}
	return NULL;
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
	return { type, NULL };
}

Tuple<Token, Error*> parse_ident(CLikeParser* p) {
	auto tok = peek(p);
	if (len(tok.str) > 0) {
		next(p);
		return { tok, NULL };
	}
	return { {}, simple_parser_error(p, current_loc(), tok.reg, U"Expected an identifier."_b) };
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p);
Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec);

Tuple<AstNode*, Error*> parse_var_decl(CLikeParser* p, AstType* type, Token type_tok, Token ident_tok) {
	Array<AstVarDecl*> var_decls = { .allocator = p->allocator };
	defer { var_decls.free(); };

	auto current_ident = ident_tok;
	while (true) {
		auto node = make_ast_node<AstVarDecl>(p->allocator, {});
		node->var_type = type;
		node->name = current_ident.str;
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

	if (len(var_decls) > 1) {
		auto node = make_ast_node<AstVarDeclGroup>(p->allocator, text_region);
		node->var_decls = var_decls;
		var_decls = {};
		return { node, NULL };
	} else {
		return { var_decls[0], NULL };
	}
}

Tuple<AstNode*, Error*> parse_unary_expr(CLikeParser* p, Token op_token, UnicodeString op, s32 min_prec) {
	auto [expr, e] = parse_expr(p, min_prec);
	if (e) {
		return { NULL, e };
	}
	auto reg = ProgramTextRegion { op_token.reg.start, peek(p).reg.start };
	auto unary = make_ast_node<CUnaryExpr>(p->allocator, reg);
	unary->expr = (AstExpr*) expr;
	unary->op = op;
	return { unary, NULL };
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
	auto call = make_ast_expr<AstFunctionCall>(p->allocator, func->return_type, AST_EXPR_RVALUE, {});
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
	AstNode* cond;
	AstNode* then;
	AstNode* else_;

	REFLECT(AstTernary) {
		BASE_TYPE(AstExpr);
		MEMBER(cond);
		MEMBER(then);
		MEMBER(else_);
	}
};

AstType* resolve_ast_type_alias(AstType* type) {
	// @TODO: implement.
	return type;
}

bool is_floating_point(AstType* type) {
	type = resolve_ast_type_alias(type);
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		if (c_alias->c_type == reflect_type_of<float>() ||
			c_alias->c_type == reflect_type_of<double>()) {
			return true;
		}
	}
	return false;
}

bool is_integer(AstType* type) {
	type = resolve_ast_type_alias(type);
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		if (c_alias->c_type == reflect_type_of<s32>() ||
			c_alias->c_type == reflect_type_of<s64>())
		{			
			return true;
		}
	}
	return false;
}

bool is_numeric(AstType* type) {
	return is_floating_point(type) || is_integer(type);
}

struct AstPointerType: AstType {
	AstType* pointee = NULL;

	REFLECT(AstPointerType) {
		BASE_TYPE(AstType);
		MEMBER(pointee);
	}
};

bool is_pointer(AstType* type) {
	type = resolve_ast_type_alias(type);
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
	type = resolve_ast_type_alias(type);
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (c_type->as<StructType>()) {
			return true;
		}
	}
	return false;
}

bool is_bool(AstType* type) {
	type = resolve_ast_type_alias(type);
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (auto primitive_type = c_type->as<PrimitiveType>()) {
			return primitive_type->primitive_kind == PrimitiveKind::P_bool;
		}
	}
	return false;
}

struct AstStructMember {
	AstType*      type = NULL;
	UnicodeString name;
	s64           offset = 0;

	REFLECT(AstStructMember) {
		MEMBER(type);
		MEMBER(name);
		MEMBER(offset);
	}
};

AstStructMember make_ast_struct_member(AstType* type, UnicodeString name, s64 offset) {
	AstStructMember member;
	member.type = type;
	member.name = name;
	member.offset = offset;
	return member;
}

s64 get_members_count(AstType* type) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (auto casted = c_type->as<StructType>()) {
			if (casted == reflect_type_of<Vector2_f32>()) {
				return 2;
			} else if (casted == reflect_type_of<Vector3_f32>()) {
				return 3;
			} else if (casted == reflect_type_of<Vector4_f32>()) {
				return 4;
			}
			return casted->members.count;
		}
	}
	return -1;
}

Optional<AstStructMember> get_struct_member(CLikeParser* p, AstType* type, s64 index) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (auto casted = c_type->as<StructType>()) {
			if (casted == reflect_type_of<Vector2_f32>()) {
				if (index == 0)      return make_ast_struct_member(find_type(p, U"float"_b), U"x"_b, 0);
				else if (index == 1) return make_ast_struct_member(find_type(p, U"float"_b), U"y"_b, 4);
			} else if (casted == reflect_type_of<Vector3_f32>()) {
				if (index == 0)      return make_ast_struct_member(find_type(p, U"float"_b), U"x"_b, 0);
				else if (index == 1) return make_ast_struct_member(find_type(p, U"float"_b), U"y"_b, 4);
				else if (index == 2) return make_ast_struct_member(find_type(p, U"float"_b), U"z"_b, 8);
			} else if (casted == reflect_type_of<Vector4_f32>()) {
				if (index == 0)      return make_ast_struct_member(find_type(p, U"float"_b), U"x"_b, 0);
				else if (index == 1) return make_ast_struct_member(find_type(p, U"float"_b), U"y"_b, 4);
				else if (index == 2) return make_ast_struct_member(find_type(p, U"float"_b), U"z"_b, 8);
				else if (index == 3) return make_ast_struct_member(find_type(p, U"float"_b), U"w"_b, 12);
			}

			if (index < casted->members.count) {
				auto unicode_str = copy_unicode_string(make_string(casted->members[index]->name));

				auto found = find_type(p, unicode_str);
				if (!found) {
					panic("Type '%' not found", unicode_str);
				}
				return make_ast_struct_member(found, unicode_str, casted->members[index]->offset);
			}
		}
	}
	return {};
}

Optional<AstStructMember> get_struct_member(CLikeParser* p, AstType* type, UnicodeString name) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (auto c_struct_type = c_type->as<StructType>()) {
			for (auto& member: c_struct_type->members) {
				if (make_string(member.name) == name) {
					auto unicode_str = copy_unicode_string(p->allocator, make_string(member.name));
					auto found = find_type(p, unicode_str);
					if (!found) {
						panic("Type '%' not found", unicode_str);
					}
					return make_ast_struct_member(found, unicode_str, member.offset);
				}
			}
		}
	}
	return {};
}

struct AstSwizzleExpr: AstExpr {
	AstExpr* lhs;
	int      swizzle[4];
	int      swizzle_len = 0;

	REFLECT(AstSwizzleExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(swizzle);
		MEMBER(swizzle_len);
	}
};

bool parse_swizzle_ident(CLikeParser* p, UnicodeString ident, int* swizzle, int src_len) {
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

Tuple<AstExpr*, Error*> try_parse_swizzle_expr(CLikeParser* p, AstExpr* lhs, Token ident) {
	if (auto c_type_alias = reflect_cast<CTypeAlias>(lhs->expr_type)) {
		int swizzle_idx[4] = { -1, -1, -1, -1 };
		if (c_type_alias->c_type == reflect_type_of<Vector2_f32>()) {
			bool ok = parse_swizzle_ident(p, ident.str, swizzle_idx, 2);
			if (!ok) {
				return { NULL, NULL };
			}
		} else if (c_type_alias->c_type == reflect_type_of<Vector3_f32>()) {
			bool ok = parse_swizzle_ident(p, ident.str, swizzle_idx, 3);
			if (!ok) {
				return { NULL, NULL };
			}
		} else if (c_type_alias->c_type == reflect_type_of<Vector4_f32>()) {
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
			swizzle_type = find_type(p, U"float"_b);
		} else if (swizzle_len == 2) {
			swizzle_type = find_type(p, U"vec2"_b);
		} else if (swizzle_len == 3) {
			swizzle_type = find_type(p, U"vec3"_b);
		} else if (swizzle_len == 4) {
			swizzle_type = find_type(p, U"vec4"_b);
		} else {
			panic(p, U"Unexpected swizzle_len = %"_b, swizzle_len);
		}

		ProgramTextRegion text_region = { ident.reg.start, peek(p).reg.start };
		if (lhs->text_region.has_value) {
			text_region.start = lhs->text_region.value.start;
		}

		if (swizzle_len == 1) {
			auto expr = make_ast_expr<AstArrayAccess>(p->allocator, swizzle_type, AST_EXPR_LVALUE, text_region);
			expr->lhs = lhs;
			auto idx = make_ast_expr<LiteralExpr>(p->allocator, find_type(p, U"int"_b), AST_EXPR_RVALUE, {});
			idx->s64_value = swizzle_idx[0];
			expr->index = idx;
			lhs = expr;
			return { lhs, NULL };
		}

		auto expr = make_ast_expr<AstSwizzleExpr>(p->allocator, lhs->expr_type, AST_EXPR_RVALUE, text_region);
		expr->lhs = lhs;
		expr->swizzle[0] = swizzle_idx[0];
		expr->swizzle[1] = swizzle_idx[1];
		expr->swizzle[2] = swizzle_idx[2];
		expr->swizzle[3] = swizzle_idx[3];
		expr->swizzle_len = swizzle_len;
		return { expr, NULL };
	}
	return { NULL, NULL };
}

struct AstStructInitializer: AstExpr {
	AstType*        struct_type = NULL;
	Array<AstExpr*> members;

	REFLECT(AstStructInitializer) {
		BASE_TYPE(AstExpr);
		MEMBER(struct_type);
		MEMBER(members);
	}
};

bool is_vector_type(AstType* type) {
	type = resolve_ast_type_alias(type);
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (c_type == reflect_type_of<Vector2_f32>() ||
			c_type == reflect_type_of<Vector3_f32>() ||
			c_type == reflect_type_of<Vector4_f32>())
		{
			return true;
		}
	}
	return false;
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
		auto node = make_ast_expr<CBinaryExpr>(p->allocator, rhs->expr_type, AST_EXPR_RVALUE, text_region);
		node->lhs = lhs;
		node->rhs = rhs;
		node->op = op_tok.str;
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
				return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Operator '%' for vector requires rhs to be float or double or vector, got '%'.", op->op, rhs->expr_type->name) };
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
		if (op->op == "==" ||
			op->op == "!=" ||
			op->op == "<=" ||
			op->op == ">="
		) {
			expr_result_type = find_type(p, U"bool"_b);
		} else {
			expr_result_type = lhs->expr_type;
		}
	}
	if (!expr_result_type) {
		return { NULL, simple_parser_error(p, current_loc(), op_tok.reg, U"Unexpected error: could not determine result type of binary expression '%'.", op->op) };
	}
	auto node = make_ast_expr<CBinaryExpr>(p->allocator, expr_result_type, AST_EXPR_RVALUE, text_region);
	node->lhs = lhs;
	node->rhs = rhs;
	node->op = op_tok.str;
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

		f64 float_val;
		f64 double_val;
		bool ok = false;
		if (parse_as_float) {
			ok = parse_float(lit, &float_val);
		} else {
			ok = parse_float(lit, &double_val);
		}
		if (!ok) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Could not parse floating point literal."_b) };
		}
		auto literal = make_ast_expr<LiteralExpr>(p->allocator, find_type(p, parse_as_float ? U"float"_b : U"double"_b), AST_EXPR_RVALUE, tok.reg);
		literal->tok = tok;
		literal->f32_value = float_val;
		literal->f64_value = double_val;
		println("Parsed fp literal value: %, % from str: %", literal->f32_value, literal->f64_value, lit);
		next(p);
		return { literal, NULL };
	}

	if (tok.flags & CTOKEN_FLAG_INTEGER) {
		s64 u;
		bool ok = parse_integer(tok.str, &u);
		if (!ok) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Could not parse integer literal."_b) };
		}
		auto literal = make_ast_expr<LiteralExpr>(p->allocator, find_type(p, U"int"_b), AST_EXPR_RVALUE, tok.reg);
		literal->tok = tok;
		literal->s64_value = u;
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
			if (!is_pointer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '*' can only be applied to pointer types.", op) };
			}
		}
		if (tok.str == "&") {
			if (!expr->is_lvalue) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '&' can only be applied to lvalues.", op) };
			}
		}
		if (tok.str == "++" || tok.str == "--") {
			if (!expr->is_lvalue) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", op) };
			}
		}
		if (tok.str == "!" || tok.str == "~") {
			if (!is_integer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to integer types.", op) };
			}
		}
		if (!is_numeric(expr->expr_type)) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op, expr->expr_type->name) };
		}
		ProgramTextRegion text_region = { op.reg.start, peek(p).reg.start };
		auto unary = make_ast_expr<CUnaryExpr>(p->allocator, expr->expr_type, AST_EXPR_RVALUE, text_region);
		unary->expr = expr;
		unary->op = tok.str;
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
			s64 member_index = 0;
			Array<AstExpr*> args = { .allocator = p->allocator };
			defer { args.free(); };
			while (true) {
				auto tok = peek(p);
				if (tok.str == ")"_b) {
					if (args.count != get_members_count(type)) {
						return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected % arguments in initializer but got '%'.", get_members_count(type), args.count) };
					}
					ProgramTextRegion text_region = { initializer_reg_start, tok.reg.end };
					auto node = make_ast_expr<AstStructInitializer>(p->allocator, type, AST_EXPR_RVALUE, text_region);
					node->struct_type = type;
					node->members = args;
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
				auto [member, found] = get_struct_member(p, type, member_index);
				if (!found) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Exceeded arguments count in initializer, struct '%' has % members.", type->name, get_members_count(type)) };
				}
				if (member.type != expr->expr_type) {
					return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", member.type->name, expr->expr_type->name) };
				}
				add(&args, expr);
				member_index += 1;
				if (peek(p).str == ","_b) {
					next(p);
				}
			}
		}
	} else if (auto var = reflect_cast<AstVar>(sym)) {
		auto reg = tok.reg;
		next(p);
		auto expr = make_ast_expr<AstVariableAccess>(p->allocator, var->var_type, AST_EXPR_LVALUE, reg);
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
			auto node = make_ast_expr<CPostfixExpr>(p->allocator, lhs->expr_type, false, text_region);
			node->lhs = lhs;
			node->op = tok.str;
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
			if (!is_struct(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a struct, got '%'"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [ident, e] = parse_ident(p);
			if (e) {
				return { NULL, e };
			}
			
			auto [swizzle, ee] = try_parse_swizzle_expr(p, lhs, ident);
			if (ee) {
				return { NULL, ee };
			}
			if (swizzle) {
				lhs = swizzle;
				continue;
			}

			auto [member, ok] = get_struct_member(p, lhs->expr_type, ident.str);
			if (!ok) {
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Member '%' not found in struct '%'.", ident, lhs->expr_type->name) };
			}
			ProgramTextRegion text_region = { ident.reg.start, ident.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = make_ast_expr<AstVarMemberAccess>(p->allocator, member.type, lhs->is_lvalue, text_region);
			node->lhs = lhs;
			node->member = ident.str;
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
			auto node = make_ast_expr<AstArrayAccess>(p->allocator, element_type, AST_EXPR_LVALUE, text_region);
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
				return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a :"_b) };
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
			auto node = make_ast_expr<AstTernary>(p->allocator, lhs->expr_type, AST_EXPR_RVALUE, text_region);
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

Tuple<AstNode*, Error*> parse_if(CLikeParser* p) {
	if (peek(p).str != "("_b) {
		return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"Expected ("_b) };
	}
	next(p);
	auto [expr, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	return { NULL, simple_parser_error(p, current_loc(), peek(p).reg, U"TODO: parse if"_b) };
}

Tuple<AstNode*, Error*> parse_block(CLikeParser* p);
Tuple<AstNode*, Error*> parse_stmt(CLikeParser* p);

Tuple<AstNode*, Error*> parse_block_or_one_stmt(CLikeParser* p) {
	auto tok = peek(p);
	if (tok.str == "{"_b) {
		next(p);
		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		return { block, NULL };
	} else {
		auto [stmt, e] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		auto block = make_ast_node<AstBlock>(p->allocator, stmt->text_region);
		add(&block->statements, stmt);
		return { block, NULL };
	}
}

struct AstFor: AstNode {
	AstNode* init_expr = NULL;
	AstNode* cond_expr = NULL;
	AstNode* incr_expr = NULL;
	AstNode* body = NULL;

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
	auto node = make_ast_node<AstFor>(p->allocator, text_region);
	node->init_expr = init_expr;
	node->cond_expr = cond_expr;
	node->incr_expr = incr_expr;
	node->body = body;
	return { node, NULL };
}

Tuple<AstNode*, Error*> parse_stmt(CLikeParser* p) {
	auto type_tok = peek(p);
	if (type_tok.str == "if"_b) {
		next(p);
		return parse_if(p);
	} else if (type_tok.str == "for"_b) {
		next(p);
		return parse_for(p, type_tok);
	}

	auto type = find_type(p, type_tok.str);
	if (type) {
		next(p);
		auto [ident, e] = parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		auto [decl, ee] = parse_var_decl(p, type, type_tok, ident);
		if (ee) {
			return { NULL, ee };
		}
		return { decl, NULL };
	}
	auto [expr, e] = parse_expr(p, 0);
	return { expr, e };
}

Tuple<AstNode*, Error*> parse_block(CLikeParser* p) {
	auto tok = peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected a {"_b) };
	}
	next(p);
	auto block = make_ast_node<AstBlock>(p->allocator, {});
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
		auto [stmt, e] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		tok = peek(p);
		if (tok.str != U";"_b) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ;"_b) };
		}
		next(p);
		add(&block->statements, stmt);
	}
	assert(region_end != -1);
	ProgramTextRegion text_region = { tok.reg.start, region_end };
	block->text_region = text_region;
	return { block, NULL };
}

Tuple<AstNode*, Error*> parse_function(CLikeParser* p, AstType* return_type, Token ident) {
	Array<AstFunctionArg*> args = { .allocator = p->allocator };
	defer { args.free(); };

	while (true) {
		auto tok = next(p);
		if (tok.str == U")"_b) {
			next(p);
			break;
		} else if (tok.str != U","_b) {
			return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ')' or ','"_b) };
		}
		next(p);
		auto start = peek(p).reg.start;
		auto [type, e] = parse_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [arg_name, e2] = parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		ProgramTextRegion text_region = { start, peek(p).reg.end };
		auto arg_node = make_ast_node<AstFunctionArg>(p->allocator, text_region);
		arg_node->name = arg_name.str;
		arg_node->arg_type = type;
		add(&args, arg_node);
	}

	ProgramTextRegion text_region = { ident.reg.start, peek(p).reg.end };
	auto f = make_ast_node<AstFunction>(p->allocator, text_region);
	f->args.allocator = p->allocator;
	f->return_type = return_type;
	f->name = ident.str;
	f->args = args;

	auto tok = peek(p);
	if (tok.str == ";") {
		next(p);
		f->text_region.value.end = tok.reg.end;
		return { f, NULL };
	}
	if (tok.str == "{") {
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

Tuple<AstNode*, Error*> parse_top_level(CLikeParser* p) {
	auto type_tok = peek(p);
	auto [type, error] = parse_type(p);
	if (error) {
		return { NULL, error };
	}
	auto [ident, e] = parse_ident(p);
	if (e) {
		return { NULL, e };
	}
	auto tok = peek(p);
	if (tok.str == U"("_b) {
		return parse_function(p, type, ident);
	} else if (tok.str == U","_b || tok.str == U";"_b || tok.str == U"="_b) {
		return parse_var_decl(p, type, type_tok, ident);
	} else {
		return { NULL, simple_parser_error(p, current_loc(), tok.reg, U"Expected ( or , or ; or ="_b) };
	}
}

Tuple<AstNode*, Error*> parse_c_like(UnicodeString str) {
	CLikeParser p;
	p.allocator = make_arena_allocator();
	p.program = make_ast_node<CLikeProgram>(p.allocator, {});
	p.program->globals.allocator = p.allocator;
	p.str = str;
	add(&p.scope, p.program);
	for (auto it: AST_BINARY_OPERATORS_UNSORTED) {
		add(&p.op_tokens_sorted, it.op);
	}
	for (auto it: AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		add(&p.op_tokens_sorted, it.op);
	}
	for (auto it: AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		add(&p.op_tokens_sorted, it.op);
	}
	sort(p.op_tokens_sorted, lambda(len($0[$1]) > len($0[$2]))); 

	add_c_type_alias(&p, reflect_type_of<s64>(), U"int"_b);
	add_c_type_alias(&p, reflect_type_of<double>(), U"double"_b);
	add_c_type_alias(&p, reflect_type_of<float>(), U"float"_b);
	add_c_type_alias(&p, reflect_type_of<void>(), U"void"_b);
	add_c_type_alias(&p, reflect_type_of<Vector2_f32>(), U"vec2"_b);
	add_c_type_alias(&p, reflect_type_of<Vector3_f32>(), U"vec3"_b);
	add_c_type_alias(&p, reflect_type_of<Vector4_f32>(), U"vec4"_b);

	auto float_type = find_type(&p, U"float"_b);

	add_shader_intrinsic_var(&p, U"FC"_b, find_type(&p, U"vec4"_b)); // frag coord
	add_shader_intrinsic_var(&p, U"r"_b, find_type(&p, U"vec2"_b)); // resolution
	add_shader_intrinsic_var(&p, U"o"_b, find_type(&p, U"vec4"_b)); // out color
	add_shader_intrinsic_var(&p, U"t"_b, find_type(&p, U"float"_b)); // time?
	add_shader_intrinsic_func(&p, U"hsv"_b, find_type(&p, U"vec3"_b), { float_type, float_type, float_type });
	add_shader_intrinsic_func(&p, U"log"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"sin"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"cos"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"dot_vec3"_b, float_type, { find_type(&p, U"vec3"_b), find_type(&p, U"vec3"_b) });
	add_shader_intrinsic_func(&p, U"atan"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"length"_b, float_type, { find_type(&p, U"vec3"_b) });
	add_shader_intrinsic_func(&p, U"abs"_b, float_type, { float_type });

	while (peek(&p).str != ""_b) {
		if (peek(&p).str == U";"_b) {
			next(&p);
			continue;
		}
		auto [node, e] = parse_top_level(&p);
		if (e) {
			return { NULL, e };
		}
		add(&p.program->globals, node);
	}
	return { p.program, NULL };
}
