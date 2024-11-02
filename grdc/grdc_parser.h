#pragma once

#include "../grd_error.h"
#include "../math/grd_vector.h"
#include "../grd_arena_allocator.h"
#include "../grd_reflect.h"
#include "../grd_array.h"
#include "../grd_tuple.h"
#include "../grd_string.h"
#include "../grd_panic.h"


enum GrdcAstOperatorFlags {
	GRDC_AST_OP_FLAG_LEFT_ASSOC = 1 << 0,
	GRDC_AST_OP_FLAG_MOD_ASSIGN = 1 << 1,
	GRDC_AST_OP_FLAG_BOOL       = 1 << 3,
	GRDC_AST_OP_FLAG_INT        = 1 << 4,
	GRDC_AST_OP_FLAG_PRIMITIVE  = 1 << 5,
	GRDC_AST_OP_FLAG_NUMERIC    = 1 << 6,
	GRDC_AST_OP_FLAG_POSTFIX    = 1 << 7,
	GRDC_AST_OP_FLAG_PREFIX     = 1 << 8,
};

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

GrdcAstOperator GRDC_AST_BINARY_OPERATORS_UNSORTED[] = { 
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
	{ U"||"_b, 13, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_BOOL },
	{ U"&&"_b, 14, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_BOOL },
	{ U"|"_b, 15, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT },
	{ U"^"_b, 16, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT},
	{ U"&"_b, 17, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT},
	{ U"=="_b, 18, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PRIMITIVE },
	{ U"!="_b, 18, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_PRIMITIVE},
	{ U"<"_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC },
	{ U">"_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC},
	{ U"<="_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC },
	{ U">="_b, 19, GRDC_AST_OP_FLAG_LEFT_ASSOC },
	{ U"<<"_b, 20, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT },
	{ U">>"_b, 20, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT },
	{ U"+"_b, 21, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC },
	{ U"-"_b, 21, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC },
	{ U"*"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC },
	{ U"/"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_NUMERIC },
	{ U"%"_b, 22, GRDC_AST_OP_FLAG_LEFT_ASSOC | GRDC_AST_OP_FLAG_INT },
};

GrdcAstOperator GRDC_AST_PREFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"!"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"~"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"+"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"-"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"++"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"--"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"*"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
	{ U"&"_b, 30, GRDC_AST_OP_FLAG_PREFIX },
};

GrdcAstOperator GRDC_AST_POSTFIX_UNARY_OPERATORS_UNSORTED[] = {
	{ U"++"_b, 40, GRDC_AST_OP_FLAG_POSTFIX },
	{ U"--"_b, 40, GRDC_AST_OP_FLAG_POSTFIX },
};

enum GrdcTokenFlags {
	CTOKEN_FLAG_FLOATING_POINT = 1 << 0,
	CTOKEN_FLAG_INTEGER = 1 << 1,
};

struct GrdcProgramTextRegion {
	s64 start  = 0;
	s64 end    = 0;

	GRD_REFLECT(GrdcProgramTextRegion) {
		GRD_MEMBER(start);
		GRD_MEMBER(end);
	}
};

struct GrdcToken {
	GrdUnicodeString     str;
	GrdcProgramTextRegion reg;
	u32               flags = 0;
};

struct GrdcProgram;
struct GrdcAstNode;
struct GrdcAstPrimitiveType;
struct GrdcAstStructType;
struct GrdcAstType;

struct GrdcParser {
	GrdAllocator                           allocator;
	GrdcProgram*                           program;
	GrdArray<GrdcAstNode*>                 scope;
	GrdUnicodeString                       str;
	s64                                    cursor = 0;
	GrdcToken                              current_token;
	GrdArray<GrdUnicodeString>             op_tokens_sorted;
	GrdHashMap<GrdcAstType*, GrdcAstType*> ptr_types;

	GrdcAstPrimitiveType*    void_tp = NULL;
	GrdcAstPrimitiveType*    bool_tp = NULL;
	GrdcAstPrimitiveType*    s8_tp = NULL;
	GrdcAstPrimitiveType*    u8_tp = NULL;
	GrdcAstPrimitiveType*    s16_tp = NULL;
	GrdcAstPrimitiveType*    u16_tp = NULL;
	GrdcAstPrimitiveType*    s32_tp = NULL;
	GrdcAstPrimitiveType*    u32_tp = NULL;
	GrdcAstPrimitiveType*    s64_tp = NULL;
	GrdcAstPrimitiveType*    u64_tp = NULL;
	GrdcAstPrimitiveType*    f32_tp = NULL;
	GrdcAstPrimitiveType*    f64_tp = NULL;

	GrdcAstStructType*       float2_tp = NULL;
	GrdcAstStructType*       float3_tp = NULL;
	GrdcAstStructType*       float4_tp = NULL;
};

struct GrdcAstNode {
	GrdType*                           type;
	GrdcParser*                        p = NULL;
	GrdOptional<GrdcProgramTextRegion> text_region;

	GRD_REFLECT(GrdcAstNode) {
		GRD_MEMBER(p);
		GRD_MEMBER(type);
			GRD_TAG(GrdRealTypeMember{});
		GRD_MEMBER(text_region);
	}
};

template <typename T>
T* grdc_make_ast_node(GrdcParser* p, GrdOptional<GrdcProgramTextRegion> text_region) {
	auto x = grd_make<T>(p->allocator);
	x->type = grd_reflect_type_of<T>();
	x->p = p;
	x->text_region = text_region;
	return x;
}

struct GrdcAstSymbol: GrdcAstNode {
	GrdUnicodeString name;
	bool             is_global = false;

	GRD_REFLECT(GrdcAstSymbol) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(name);
		GRD_MEMBER(is_global);
	}
};

struct GrdcAstType: GrdcAstSymbol {
	u64 size = 0;
	u64 alignment = 0;

	GRD_REFLECT(GrdcAstType) {
		GRD_BASE_TYPE(GrdcAstSymbol);
		GRD_MEMBER(size);
		GRD_MEMBER(alignment);
	}
};

enum class GrdcMetalAddrSpace {
	Unspecified = 0,
	Device = 1,
	Constant = 2,
	Thread = 2,
	Threadgroup = 3,
	// ThreadgroupImageblock = 4,
	// RayData = 5,
	// ObjectData = 6,
};
GRD_REFLECT(GrdcMetalAddrSpace) {
	GRD_ENUM_VALUE(Unspecified);
	GRD_ENUM_VALUE(Device);
	GRD_ENUM_VALUE(Constant);
	GRD_ENUM_VALUE(Thread);
	GRD_ENUM_VALUE(Threadgroup);
	// GRD_ENUM_VALUE(ThreadgroupImageblock);
	// GRD_ENUM_VALUE(RayData);
	// GRD_ENUM_VALUE(ObjectData);
}

enum class GrdcVulkanStorageClass {
	Unspecified = 0,
	Private = 1,
	Uniform = 2,
	Input = 3,
	PushConstant = 4,
};
GRD_REFLECT(GrdcVulkanStorageClass) {
	GRD_ENUM_VALUE(Unspecified);
	GRD_ENUM_VALUE(Private);
	GRD_ENUM_VALUE(Uniform);
	GRD_ENUM_VALUE(Input);
	GRD_ENUM_VALUE(PushConstant);
}

struct GrdcAstExpr: GrdcAstNode {
	GrdcAstType* expr_type = NULL;
	bool         is_lvalue = false;

	GRD_REFLECT(GrdcAstExpr) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(expr_type);
		GRD_MEMBER(is_lvalue);
	}
};

constexpr bool GRDC_AST_EXPR_LVALUE = true;
constexpr bool GRDC_AST_EXPR_RVALUE = false;

template <typename T>
T* grdc_make_ast_expr(GrdcParser* p, GrdcAstType* expr_type, bool is_lvalue, GrdOptional<GrdcProgramTextRegion> text_region) {
	auto x = grdc_make_ast_node<T>(p, text_region);
	if (!expr_type) {
		grd_panic("expr_type must be non-null");
	}
	x->expr_type = expr_type;
	x->is_lvalue = is_lvalue;
	return x;
}

struct GrdcAstBlock: GrdcAstNode {
	GrdArray<GrdcAstNode*> statements;

	GRD_REFLECT(GrdcAstBlock) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(statements);
	}
};


struct GrdcProgram: GrdcAstNode {
	GrdArray<GrdcAstNode*>   globals; // Main diff between globals and global_syms is var decl groups vs var decls.
	GrdArray<GrdcAstSymbol*> global_syms;

	GRD_REFLECT(GrdcProgram) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(globals);
		GRD_MEMBER(global_syms);
	}
};

struct GrdcAstPrimitiveType: GrdcAstType {
	GrdPrimitiveType* c_tp = NULL;
	bool              is_signed = false;

	GRD_REFLECT(GrdcAstPrimitiveType) {
		GRD_BASE_TYPE(GrdcAstType);
		GRD_MEMBER(c_tp);
		GRD_MEMBER(is_signed);
	}
};


struct GrdcAstTypeSite;

struct GrdcAstVar: GrdcAstSymbol {
	GrdcAstTypeSite* var_ts = NULL;
	
	GRD_REFLECT(GrdcAstVar) {
		GRD_BASE_TYPE(GrdcAstSymbol);
		GRD_MEMBER(var_ts);
	}
};

struct GrdcShaderIntrinVar: GrdcAstVar {

	GRD_REFLECT(GrdcShaderIntrinVar) {
		GRD_BASE_TYPE(GrdcAstVar);
	}
};

struct GrdcAstAttr;

struct GrdcAstFunctionArg: GrdcAstVar {
	GrdArray<GrdcAstAttr*> attrs;

	GRD_REFLECT(GrdcAstFunctionArg) {
		GRD_BASE_TYPE(GrdcAstVar);
		GRD_MEMBER(attrs);
	}
};

enum class GrdcAstFunctionKind {
	Plain = 1,
	Vertex = 2,
	Fragment = 3,
};
GRD_REFLECT(GrdcAstFunctionKind) {
	GRD_ENUM_VALUE(Plain);
	GRD_ENUM_VALUE(Vertex);
	GRD_ENUM_VALUE(Fragment);
}

struct GrdcMtlBufferIdxAttr;
struct GrdcVkSetBindingAttr;

struct GrdcAstFunction: GrdcAstSymbol {
	GrdcAstTypeSite*                return_ts = NULL; 
	GrdArray<GrdcAstFunctionArg*>   args;
	GrdArray<GrdcAstAttr*>          attrs;
	GrdcAstFunctionKind             kind = GrdcAstFunctionKind::Plain;
	GrdcAstBlock*                   block = NULL;
	GrdcAstFunctionArg*             stage_in_arg = NULL;
	GrdArray<GrdcMtlBufferIdxAttr*> mtl_buffers;
	GrdArray<GrdcVkSetBindingAttr*> vk_set_bindings;

	GRD_REFLECT(GrdcAstFunction) {
		GRD_BASE_TYPE(GrdcAstSymbol);
		GRD_MEMBER(return_ts);
		GRD_MEMBER(args);
		GRD_MEMBER(attrs);
		GRD_MEMBER(kind);
		GRD_MEMBER(block);
		GRD_MEMBER(stage_in_arg);
		GRD_MEMBER(mtl_buffers);
		GRD_MEMBER(vk_set_bindings);
	}
};

template <typename T>
T* grdc_make_ast_symbol(GrdcParser* p, GrdUnicodeString name, GrdOptional<GrdcProgramTextRegion> text_region) {
	auto node = grdc_make_ast_node<T>(p, text_region);
	node->name = name;
	return node;
}

struct GrdcAstUnaryExpr: GrdcAstExpr {
	GrdcAstExpr*      expr;
	GrdcAstOperator*  op;

	GRD_REFLECT(GrdcAstUnaryExpr) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(expr);
		GRD_MEMBER(op);
	}
};

struct GrdcAstDerefExpr: GrdcAstExpr {
	GrdcAstExpr* lhs;

	GRD_REFLECT(GrdcAstDerefExpr) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(lhs);
	}
};

struct GrdcAstBinaryExpr: GrdcAstExpr {
	GrdcAstExpr*      lhs = NULL;
	GrdcAstExpr*      rhs = NULL;
	GrdcAstOperator*  op = NULL;
	GrdcAstOperator*  pure_op = NULL;

	GRD_REFLECT(GrdcAstBinaryExpr) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(rhs);
		GRD_MEMBER(op);
		GRD_MEMBER(pure_op);
	}
};

struct GrdcAstVarDecl: GrdcAstVar { 
	GrdcAstExpr*      init = NULL;

	GRD_REFLECT(GrdcAstVarDecl) {
		GRD_BASE_TYPE(GrdcAstVar);
		GRD_MEMBER(init);
	}
};


struct GrdcAstVarDeclGroup: GrdcAstNode {
	GrdArray<GrdcAstVarDecl*> var_decls;

	GRD_REFLECT(GrdcAstVarDeclGroup) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(var_decls);
	}
};

struct GrdcAstStructType;
struct GrdcAstAttr;

struct GrdcAstStructMember: GrdcAstNode {
	GrdcAstStructType*     struct_type = NULL;
	GrdcAstTypeSite*       member_ts = NULL;
	GrdUnicodeString       name;
	s64                    offset = 0;
	GrdArray<GrdcAstAttr*> attrs;

	GRD_REFLECT(GrdcAstStructMember) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(struct_type);
		GRD_MEMBER(member_ts);
		GRD_MEMBER(name);
		GRD_MEMBER(offset);
		GRD_MEMBER(attrs);
	}
};

struct GrdcAstStructType: GrdcAstType {
	GrdArray<GrdcAstStructMember*> members;

	GRD_REFLECT(GrdcAstStructType) {
		GRD_BASE_TYPE(GrdcAstType);
		GRD_MEMBER(members);
	}
};

struct GrdcAstVarMemberAccess: GrdcAstExpr {
	GrdcAstExpr*         lhs;
	GrdcAstStructMember* member;

	GRD_REFLECT(GrdcAstVarMemberAccess) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(member);
	}
};

struct GrdcAstGrdArrayAccess: GrdcAstExpr {
	GrdcAstExpr*      lhs;
	GrdcAstExpr*      index;

	GRD_REFLECT(GrdcAstGrdArrayAccess) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(index);
	}
};

struct GrdcAstLiteralExpr: GrdcAstExpr {
	GrdPrimitiveType* lit_type = NULL;
	GrdPrimitiveValue  lit_value = {};
};

struct GrdcAstAttr: GrdcAstNode {
	GrdUnicodeString   name;
	GrdArray<GrdcAstExpr*> args;
	bool            is_used = false;

	GRD_REFLECT(GrdcAstAttr) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(name);
		GRD_MEMBER(args);
		GRD_MEMBER(is_used);
	}
};

GrdcToken grdc_set_current_token(GrdcParser* p, s64 end, u32 flags = 0) {
	grd_defer { p->cursor = end; };
	GrdcToken tok = {
		.str = p->str[p->cursor, end],
		.reg = { .start = p->cursor, .end = end },
		.flags = flags,
	};
	p->current_token = tok;
	return tok;
}

s64 grdc_maybe_eat_floating_point_literal(GrdcParser* p) {
	s64 start = p->cursor;
	s64 c = p->cursor;
	
	for (; c < grd_len(p->str); c++) {
		if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
			break;
		}
	}
	if (c >= grd_len(p->str)) {
		return 0;
	}
	bool have_stuff_before_dot = c > start;
	bool have_dot = false;
	bool have_stuff_after_dot = false;
	if (p->str[c] == '.') {
		have_dot = true;
		c += 1;
		s64 decimal_start = c;
		for (; c < grd_len(p->str); c++) {
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
	if (c < grd_len(p->str)) {
		if (p->str[c] == 'e' || p->str[c] == 'E') {
			have_exp = true;
			c += 1;
			if (c >= grd_len(p->str)) {
				return 0;
			}
			if (p->str[c] == '+' || p->str[c] == '-') {
				c += 1;
				if (c >= grd_len(p->str)) {
					return 0;
				}
			}
			for (; c < grd_len(p->str); c++) {
				if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
					break;
				}
			}
		}
	}
	bool have_suffix = false;
	if (c < grd_len(p->str)) {
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

s64 grdc_maybe_eat_integer_literal(GrdcParser* p) {
	auto start = p->str[p->cursor, {}];

	s32 prefix_len = 0;
	if (grd_len(start) > 0 && (start[0] >= '0' && start[0] <= '9')) {
		prefix_len = 1;
	} else if (
		grd_starts_with(start, U"0x"_b) ||
		grd_starts_with(start, U"0X"_b) ||
		grd_starts_with(start, U"0b"_b) ||
		grd_starts_with(start, U"0B"_b) ||
		grd_starts_with(start, U"0o"_b) ||
		grd_starts_with(start, U"0O"_b))
	{
		prefix_len = 2;
	}

	for (auto i: grd_range_from_to(p->cursor + prefix_len, grd_len(p->str))) {
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
	return prefix_len > 0 ? grd_len(p->str) : 0;
}

GrdcToken grdc_next(GrdcParser* p) {
	for (auto i: grd_range_from_to(p->cursor, grd_len(p->str))) {
		if (grd_is_whitespace(p->str[i])) {
			if (p->cursor < i) {
				return grdc_set_current_token(p, i);
			} else {
				p->cursor = i + 1;
				continue;
			}
		}

		s64 fp_end = grdc_maybe_eat_floating_point_literal(p);
		if (fp_end != 0) {
			return grdc_set_current_token(p, fp_end, CTOKEN_FLAG_FLOATING_POINT);
		}

		s64 int_end = grdc_maybe_eat_integer_literal(p);
		if (int_end != 0) {
			return grdc_set_current_token(p, int_end, CTOKEN_FLAG_INTEGER);
		}

		for (auto c: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>', '[', ']', '&'}) {
			if (p->str[i] == c) {
				if (p->cursor < i) {
					return grdc_set_current_token(p, i);
				} else {
					// parse multichar operators.
					auto txt = p->str[i, {}];
					for (auto it: p->op_tokens_sorted) {
						if (grd_starts_with(txt, it)) {
							return grdc_set_current_token(p, i + grd_len(it));
						}
					}
					return grdc_set_current_token(p, i + 1);
				}
			}
		}
	}
	return grdc_set_current_token(p, grd_len(p->str));
}

GrdcToken grdc_peek(GrdcParser* p) {
	if (grd_len(p->current_token.str) == 0) {
		return grdc_next(p);
	}
	return p->current_token;
}

struct GrdcPrepComment {
	s64 start = 0;
	s64 end   = 0;

	GRD_REFLECT(GrdcPrepComment) {
		GRD_MEMBER(start);
		GRD_MEMBER(end);
	}
};

struct GrdcPreprocessor;

struct GrdcPrepNode {
	GrdType*             type = NULL;
	GrdcPreprocessor* pp = NULL;
	GrdcPrepNode*     parent = NULL;

	GRD_REFLECT(GrdcPrepNode) {
		GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
		GRD_MEMBER(pp);
		GRD_MEMBER(parent);
	}
};

template <typename T>
T* grdc_make_prep_node(GrdcPreprocessor* pp) {
	auto node = grd_make<T>(pp->allocator);
	node->type = grd_reflect_type_of<T>();
	node->pp = pp;
	return node;
}

struct GrdcPrepFile {
	GrdUnicodeString path;
	GrdUnicodeString str;
};

struct GrdcPrepFileNode: GrdcPrepNode {
	GrdcPrepFile* file = NULL;
	s64           file_offset = 0;

	GRD_REFLECT(GrdcPrepFileNode) {
		GRD_BASE_TYPE(GrdcPrepNode);
		GRD_MEMBER(file);
		GRD_MEMBER(file_offset);
	}
};

struct GrdcPrepNodeReg {
	GrdcPrepNode* node = NULL;
	s64       start = 0;
	s64       end = -1;
};

struct GrdcPreprocessor {
	GrdAllocator              allocator;
	GrdAllocatedUnicodeString pr;
	GrdArray<GrdcPrepNode*>       scope;
	GrdArray<GrdcPrepFile*>       files;
	GrdArray<GrdcPrepNodeReg*>    regs;
};

void grdc_push_reg(GrdcPreprocessor* pp) {
	if (grd_len(pp->scope) == 0) {
		return;
	}
	auto reg = grd_make<GrdcPrepNodeReg>(pp->allocator);
	reg->node = pp->scope[-1];
	reg->start = grd_len(pp->pr);
	while (grd_len(pp->regs) > 0) {
		if (pp->regs[-1]->start < reg->start) {
			break;
		}
		grd_remove(&pp->regs, -1);
	}
	if (grd_len(pp->regs) > 0) {
		pp->regs[-1]->end = reg->start;
	}
	grd_add(&pp->regs, reg);
}

void grdc_push_scope(GrdcPreprocessor* pp, GrdcPrepNode* node) {
	if (grd_len(pp->scope) > 0) {
		assert(node->parent == NULL);
		node->parent = pp->scope[-1];
	}
	grd_add(&pp->scope, node);
	grdc_push_reg(pp);
}

void grdc_pop_scope(GrdcPreprocessor* pp) {
	grd_pop(&pp->scope);
	grdc_push_reg(pp);
}

GrdError* grdc_prep_file(GrdcPreprocessor* pp, GrdUnicodeString str, GrdUnicodeString path) {
	auto file = grd_make<GrdcPrepFile>(pp->allocator);
	file->path = path;
	file->str = str;
	grd_add(&pp->files, file);

	auto node = grdc_make_prep_node<GrdcPrepFileNode>(pp);
	node->file = file;
	node->file_offset = 0;
	grdc_push_scope(pp, node);

	s64 cursor = 0;
	while (cursor < grd_len(str)) {
		GrdUnicodeString remaining = str[cursor, {}];
		if (grd_starts_with(remaining, "//")) {
			s64 start = cursor;
			s64 end = grd_len(str);
			for (auto i: grd_range_from_to(cursor, grd_len(str))) {
				if (grd_is_line_break(str[i])) {
					end = i;
					break;
				}
			}
			cursor = end;
			grdc_pop_scope(pp);
			auto comment_node = grdc_make_prep_node<GrdcPrepFileNode>(pp);
			comment_node->file = file;
			comment_node->file_offset = start;
			grdc_push_scope(pp, comment_node);
			grd_add(&pp->pr, ' ');
			grdc_pop_scope(pp);
			auto new_node = grdc_make_prep_node<GrdcPrepFileNode>(pp);
			new_node->file = file;
			new_node->file_offset = end;
			grdc_push_scope(pp, new_node);
			continue;
		}
		if (grd_starts_with(remaining, "/*")) {
			s64 start = cursor;
			cursor += 2;
			s64 level = 1;
			while (cursor < grd_len(str)) {
				if (grd_starts_with(str[cursor, {}], "/*")) {
					cursor += 2;
					level += 1;
				} else if (grd_starts_with(str[cursor, {}], "*/")) {
					cursor += 2;
					level -= 1;
					if (level == 0) {
						break;
					}
				} else {
					cursor += 1;
				}
			}
			if (level > 0) {
				// @TODO: continue.
				// Add preprocessor text region.
			}
		}
		// if (grd_starts_with(remaining, "#")) {
			
		// }
		grd_add(&pp->pr, str[cursor]);
		cursor += 1;
	}
	return NULL;
}

GrdcPreprocessor* grdc_make_preprocessor(GrdAllocator allocator) {
	auto pp = grd_make<GrdcPreprocessor>(allocator);
	pp->allocator = allocator;
	pp->pr.allocator = allocator;
	pp->files.allocator = allocator;
	pp->scope.allocator = allocator;
	pp->regs.allocator = allocator;
	return pp;
}

GrdGenerator<GrdcAstSymbol*> grdc_resolve_symbols(GrdcAstNode* node) {
	if (auto sym = grd_reflect_cast<GrdcAstSymbol>(node)) {
		co_yield sym;
		co_return;
	}
	if (auto var_decl_group = grd_reflect_cast<GrdcAstVarDeclGroup>(node)) {
		for (auto it: var_decl_group->var_decls) {
			auto gen = grdc_resolve_symbols(it);
			for (auto sym: gen) {
				co_yield sym;
			}
		}
	}
}

GrdcAstNode* grdc_lookup_symbol(GrdcParser* p, GrdUnicodeString name) {
	for (auto i: grd_reverse(grd_range(grd_len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto program = grd_reflect_cast<GrdcProgram>(scope)) {
			for (auto child: program->globals) {
				for (auto sym: grdc_resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto block = grd_reflect_cast<GrdcAstBlock>(scope)) {
			for (auto child: block->statements) {
				for (auto sym: grdc_resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto f = grd_reflect_cast<GrdcAstFunction>(scope)) {
			for (auto arg: f->args) {
				if (arg->name == name) {
					return arg;
				}
			}
		}
	}
	return NULL;
}

GrdError* grdc_add_global(GrdcParser* p, GrdcAstNode* s) {
	for (auto sym: grdc_resolve_symbols(s)) {
		auto found = grdc_lookup_symbol(p, sym->name);
		if (found) {
			auto e = grdc_make_parser_error(p, grd_current_loc(), "Duplicate global variable '%'"_b, sym->name);
			add_site(p, e, CParserErrorToken{ .reg = sym->text_region.value, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED });
			if (found->text_region.has_value) {
				add_text(p, e, U"Previous definition was here:"_b);
				add_site(p, e, CParserErrorToken{ .reg = found->text_region.value, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN });
			}
			return e;
		}
		sym->is_global = true;
	}
	grd_add(&p->program->globals, s);
	return NULL;
}

GrdcAstType* grdc_find_type(GrdcParser* p, GrdUnicodeString name) {
	if (name == "float") {
		name = U"f32"_b;
	}
	if (name == "double") {
		name = U"f64"_b;
	}
	if (name == "int") {
		name = U"s32"_b;
	}

	auto symbol = grdc_lookup_symbol(p, name);
	if (symbol) {
		if (auto tp = grd_reflect_cast<GrdcAstType>(symbol)) {
			return tp;
		}
	}
	return NULL;
}

struct GrdcAstPointerType: GrdcAstType {
	GrdcAstType*           pointee = NULL;
	GrdcVulkanStorageClass ptr_vk = GrdcVulkanStorageClass::Unspecified;
	GrdcMetalAddrSpace     ptr_mtl = GrdcMetalAddrSpace::Unspecified;

	GRD_REFLECT(GrdcAstPointerType) {
		GRD_BASE_TYPE(GrdcAstType);
		GRD_MEMBER(pointee);
		GRD_MEMBER(ptr_mtl);
		GRD_MEMBER(ptr_vk);
	}
};

struct GrdcAstTypeSite: GrdcAstNode {
	GrdcAstType* tp = NULL;

	GRD_REFLECT(GrdcAstTypeSite) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(tp);
	}
};

struct GrdcAstPointerTypeSite: GrdcAstTypeSite {
	GrdcAstPointerTypeSite* pointee = NULL;

	GRD_REFLECT(GrdcAstPointerTypeSite) {
		GRD_BASE_TYPE(GrdcAstTypeSite);
		GRD_MEMBER(pointee);
	}
};

// GrdAstType* grdc_get_pointer_type(GrdcParser* p, GrdAstType* type) {
// 	auto found = get(&p->ptr_types, type);
// 	if (found) {
// 		return *found;
// 	}
// 	auto name = sprint_unicode(p->allocator, U"%(*)*"_b, type->name);
// 	auto ptr = grdc_make_ast_symbol<GrdcAstPointerType>(p, name, {});
// 	ptr->pointee = type;
// 	put(&p->ptr_types, type, ptr);
// 	return ptr;
// }

struct GrdcPreType {
	GrdcAstType*          t = NULL;
	s64                   pointer_indir_level = 0;
	GrdcProgramTextRegion reg;
};

GrdTuple<GrdcPreType, GrdError*> grdc_parse_pre_type(GrdcParser* p) {
	auto tok = grdc_peek(p);
	auto c_str = grd_encode_utf8(tok.str);
	grd_defer { c_str.free(); };
	GrdcAstType* type = grdc_find_type(p, tok.str);
	if (!type) {
		return { {}, grdc_simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not find type."_b) };
	}
	GrdcPreType pt = { .t = type };
	pt.reg = { tok.reg.start, tok.reg.end };
	grdc_next(p);
	while (true) {
		tok = grdc_peek(p);
		if (tok.str == "*") {
			pt.pointer_indir_level += 1;
			pt.reg.end = tok.reg.end;
			grdc_next(p);
		} else {
			break;
		}
	}
	return { pt, NULL };
}

GrdcAstPointerType* grdc_get_pointer_type(GrdcParser* p, GrdcAstType* tp, GrdcVulkanStorageClass st_class, GrdcMetalAddrSpace mtl_space) {
	auto ptr = grdc_make_ast_node<GrdcAstPointerType>(p, tp->text_region);
	ptr->name = grd_sprint_unicode(p->allocator, U"%(*)*"_b, tp->name);
	ptr->pointee = tp;
	ptr->size = 8;
	ptr->alignment = 8;
	ptr->ptr_vk = st_class;
	ptr->ptr_mtl = mtl_space;
	return ptr;
}

GrdTuple<GrdcAstTypeSite*, GrdError*> grdc_finalize_type(GrdcParser* p, GrdcPreType pt, GrdSpan<GrdcAstAttr*> attrs) {
	if (pt.pointer_indir_level > 0) {
		auto st_class = GrdcVulkanStorageClass::Unspecified;
		for (auto attr: attrs) {
			if (attr->name == "vk_uniform") {
				if (st_class != GrdcVulkanStorageClass::Unspecified) {
					auto e = grd_make_parser_error(p, grd_current_loc(), "Vulkan storage class must be specified for pointer type only once"_b);
					add_site(p, e,
						CParserErrorToken{
							.reg = pt.reg,
							.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED
						}
					);
					return { {}, e };
				}
				st_class = GrdcVulkanStorageClass::Uniform;
			}
		}
		if (st_class == GrdcVulkanStorageClass::Unspecified) {
			return { {}, grd_simple_parser_error(p, grd_current_loc(), pt.reg, U"Vulkan storage class must be specified for pointer type."_b) };
		}
		// @TODO: implement GrdcMetalAddrSpace
		auto tp = grdc_get_pointer_type(p, pt.t, st_class, GrdcMetalAddrSpace::Unspecified);
		auto qt = grdc_make_ast_node<GrdcAstPointerTypeSite>(p, pt.reg);
		qt->tp = tp;
		for (auto i: grd_range(pt.pointer_indir_level - 1)) {
			auto sub = grdc_make_ast_node<GrdcAstPointerTypeSite>(p, pt.reg);
			auto sub_tp = grdc_get_pointer_type(p, qt->tp, st_class, GrdcMetalAddrSpace::Unspecified);
			sub->tp = sub_tp;
			qt = sub;
		}
		return { qt, NULL };
	} else {
		auto qt = grdc_make_ast_node<GrdcAstPointerTypeSite>(p, pt.reg);
		qt->tp = pt.t;
		return { qt, NULL };
	}
}

GrdTuple<GrdcToken, GrdError*> grdc_parse_ident(GrdcParser* p) {
	auto tok = grdc_peek(p);
	if (grd_len(tok.str) == 0) {
		return { {}, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected an identifier."_b) };
	}
	if ((tok.str[0] < 'a' || tok.str[0] > 'z') && (tok.str[0] < 'A' || tok.str[0] > 'Z')) {
		return { {}, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"First letter of an identifier must be a letter."_b) };
	}
	for (auto c: tok.str) {
		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_') {
			return { {}, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Only letters, numbers, and underscores are allowed in identifiers."_b) };
		}
	}
	grdc_next(p);
	return { tok, NULL };
}

GrdTuple<GrdcAstExpr*, GrdError*> grdc_parse_primary_expr(GrdcParser* p);
GrdTuple<GrdcAstExpr*, GrdError*> grdc_parse_expr(GrdcParser* p, s32 min_prec);
GrdTuple<GrdArray<GrdcAstAttr*>, GrdError*> grdc_parse_attrs(GrdcParser* p);

GrdError* grdc_check_if_attrs_used(GrdcParser* p, GrdArray<GrdcAstAttr*> attrs, GrdCodeLoc loc = grd_caller_loc()) {
	for (auto attr: attrs) {
		if (!attr->is_used) {
			auto e = grd_simple_parser_error(p, loc, attr->text_region, "Attribute '%' is not used."_b, attr->name);
			return e;
		}
	}
	return NULL;
}

GrdTuple<GrdcAstVarDeclGroup*, GrdError*> grdc_parse_var_decl(GrdcParser* p, GrdcPreType pt, GrdcToken ident_tok, bool is_global) {
	GrdArray<GrdcAstVarDecl*> var_decls = { .allocator = p->allocator };
	grd_defer { var_decls.free(); };

	auto val_decl_attrs = [&](auto* p, GrdArray<GrdcAstAttr*> attrs) -> GrdError* {
		for (auto attr: attrs) {
			if (is_global) {
				return grd_simple_parser_error(p, grd_current_loc(), attr->text_region, "Global variables cannot have attributes"_b);
			} else {
				return grd_simple_parser_error(p, grd_current_loc(), attr->text_region, "Local variables cannot have attributes"_b);
			}
		}
		return NULL;
	};
	auto [attrs, e] = grdc_parse_attrs(p);
	if (e) {
		return { NULL, e };
	}
	auto [ts, e2] = grdc_finalize_type(p, pt, attrs);
	if (e2) {
		return { NULL, e2 };
	}
	auto e3 = grdc_check_if_attrs_used(p, attrs);
	if (e3) {
		return { NULL, e3 };
	}
	auto current_ident = ident_tok;
	while (true) {
		auto node = grdc_make_ast_node<GrdcAstVarDecl>(p, {});
		node->var_ts = ts;
		node->name = current_ident.str;
		node->is_global = is_global;
		grd_add(&var_decls, node);

		auto tok = grdc_peek(p);
		if (tok.str != U","_b) {
			break;
		}
		grdc_next(p);
		auto [ident, e] = grdc_parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		current_ident = ident;
	}

	auto tok = grdc_peek(p);
	if (tok.str == U"="_b) {
		grdc_next(p);
		auto [expr, e] = grdc_parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		if (expr->expr_type != ts->tp) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", ts->tp->name, expr->expr_type->name) };
		}
		for (auto it: var_decls) {
			it->init = (GrdcAstExpr*) expr;
		}
	}

	tok = grdc_peek(p);
	if (tok.str != U";"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected , or ;"_b) };
	}

	GrdcProgramTextRegion text_region = { pt.reg.start, tok.reg.end };
	for (auto it: var_decls) {
		it->text_region = text_region;
	}

	auto node = grdc_make_ast_node<GrdcAstVarDeclGroup>(p, text_region);
	node->var_decls = var_decls;
	var_decls = {};
	return { node, NULL };
}

struct GrdcAstFunctionCall: GrdcAstExpr {
	GrdcAstNode*           f;
	GrdArray<GrdcAstExpr*> args;

	GRD_REFLECT(GrdcAstFunctionCall) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(args);
	}
};

GrdcAstOperator* grdc_find_binary_operator(GrdcParser* p, GrdUnicodeString op) {
	for (auto& it: GRDC_AST_BINARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}

GrdcAstOperator* grdc_find_prefix_unary_operator(GrdcParser* p, GrdUnicodeString op) {
	for (auto& it: GRDC_AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}

GrdcAstOperator* grdc_find_postfix_unary_operator(GrdcParser* p, GrdUnicodeString op) {
	for (auto& it: GRDC_AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		if (op == it.op) {
			return &it;
		}
	}
	return NULL;
}


GrdTuple<GrdcAstExpr*, GrdError*> grdc_parse_function_call(GrdcParser* p, GrdcToken func_token, GrdcAstFunction* func) {
	auto tok = grdc_peek(p);
	if (tok.str != "("_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ("_b) };
	}
	auto call = grdc_make_ast_expr<GrdcAstFunctionCall>(p, func->return_ts->tp, GRDC_AST_EXPR_RVALUE, {});
	call->args.allocator = p->allocator;
	call->f = func;
	grdc_next(p);
	while (true) {
		auto tok = grdc_peek(p);
		if (tok.str == ")"_b) {
			call->text_region = GrdcProgramTextRegion { func_token.reg.start, tok.reg.end };
			grdc_next(p);
			break;
		}
		auto op = grdc_find_binary_operator(p, U","_b);
		auto [expr, e] = grdc_parse_expr(p, op->prec + 1);
		if (e) {
			return { NULL, e };
		}
		grd_add(&call->args, expr);
		if (grdc_peek(p).str == ","_b) {
			grdc_next(p);
		}
	}
	return { call, NULL };
}

struct GrdcAstTernary: GrdcAstExpr {
	GrdcAstExpr* cond;
	GrdcAstExpr* then;
	GrdcAstExpr* else_;

	GRD_REFLECT(GrdcAstTernary) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(cond);
		GRD_MEMBER(then);
		GRD_MEMBER(else_);
	}
};

struct GrdcAstIf: GrdcAstNode {
	GrdcAstExpr*  cond = NULL;
	GrdcAstBlock* then = NULL;
	GrdcAstBlock* else_block = NULL;
	GrdcAstIf*    else_if = NULL;

	GRD_REFLECT(GrdcAstIf) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(cond);
		GRD_MEMBER(then);
		GRD_MEMBER(else_block);
		GRD_MEMBER(else_if);
	}
};

GrdcAstType* grdc_resolve_type_alias(GrdcAstType* type) {
	// @TODO: implement.
	return type;
}

bool grdc_is_floating_point(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	if (type == type->p->f32_tp || type == type->p->f64_tp) {
		return true;
	}
	return false;
}

bool grdc_is_integer(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
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

bool grdc_is_numeric(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	return grdc_is_floating_point(type) || grdc_is_integer(type);
}

bool grdc_is_pointer(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	if (auto ptr = grd_reflect_cast<GrdcAstPointerType>(type)) {
		return true;
	}
	return false;
}

bool grdc_is_array(GrdcAstType* type) {
	// @TODO: implement.
	return false;
}

GrdcAstType* grdc_get_element_type(GrdcAstType* type) {
	if (auto ptr = grd_reflect_cast<GrdcAstPointerType>(type)) {
		return ptr->pointee;
	}
	return NULL;
}

struct GrdcAstVariableAccess: GrdcAstExpr {
	GrdcAstVar* var = NULL;

	GRD_REFLECT(GrdcAstVariableAccess) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(var);
	}
};

bool grdc_is_struct(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	return grd_reflect_cast<GrdcAstStructType>(type) != NULL;
}

bool grdc_is_bool(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	return type == type->p->bool_tp;
}

GrdcAstStructMember* grdc_find_struct_member(GrdcParser* p, GrdcAstStructType* type, GrdUnicodeString name) {
	for (auto it: type->members) {
		if (it->name == name) {
			return it;
		}
	}
	return NULL;
}

struct GrdcAstSwizzleExpr: GrdcAstExpr {
	GrdcAstExpr* lhs;
	s32          swizzle[4];
	s32          swizzle_len = 0;

	GRD_REFLECT(GrdcAstSwizzleExpr) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(swizzle);
		GRD_MEMBER(swizzle_len);
	}
};

bool grdc_parse_swizzle_ident(GrdcParser* p, GrdUnicodeString ident, s32* swizzle, s32 src_len) {
	s64 ident_len = grd_len(ident);
	if (ident_len > 4) {
		return false;
	}
	bool did_fail = false;
	for (auto i: grd_range(ident_len)) {
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
	for (auto i: grd_range(ident_len)) {
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

GrdTuple<GrdcAstExpr*, GrdError*> grdc_try_parse_swizzle_expr(GrdcParser* p, GrdcAstExpr* lhs, GrdcAstStructType* struct_tp, GrdcToken ident) {
	int swizzle_idx[4] = { -1, -1, -1, -1 };
	if (struct_tp == p->float2_tp) {
		bool ok = grdc_parse_swizzle_ident(p, ident.str, swizzle_idx, 2);
		if (!ok) {
			return { NULL, NULL };
		}
	} else if (struct_tp == p->float3_tp) {
		bool ok = grdc_parse_swizzle_ident(p, ident.str, swizzle_idx, 3);
		if (!ok) {
			return { NULL, NULL };
		}
	} else if (struct_tp == p->float4_tp) {
		bool ok = grdc_parse_swizzle_ident(p, ident.str, swizzle_idx, 4);
		if (!ok) {
			return { NULL, NULL };
		}
	} else {
		return { NULL, NULL };
	}

	int swizzle_len = 0;
	for (auto i: grd_range(4)) {
		if (swizzle_idx[i] == -1) {
			break;
		}
		swizzle_len += 1;
	}

	GrdcAstType* swizzle_type = NULL;
	if (swizzle_len == 1) {
		swizzle_type = p->f32_tp;
	} else if (swizzle_len == 2) {
		swizzle_type = p->float2_tp;
	} else if (swizzle_len == 3) {
		swizzle_type = p->float3_tp;
	} else if (swizzle_len == 4) {
		swizzle_type = p->float4_tp;
	} else {
		grd_panic(p, U"Unexpected swizzle_len = %"_b, swizzle_len);
	}

	GrdcProgramTextRegion text_region = { ident.reg.start, grdc_peek(p).reg.start };
	if (lhs->text_region.has_value) {
		text_region.start = lhs->text_region.value.start;
	}

	if (swizzle_len == 1) {
		auto expr = grdc_make_ast_expr<GrdcAstVarMemberAccess>(p, swizzle_type, lhs->is_lvalue, text_region);
		expr->lhs = lhs;
		auto member = grdc_find_struct_member(p, struct_tp, ident.str);
		if (!member) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), ident.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
		}
		expr->member = member;
		return { expr, NULL };
	}

	bool is_unique = true;
	int unique_check[grd_static_array_count(swizzle_idx)] = {};
	for (auto i: grd_range(swizzle_len)) {
		auto idx = swizzle_idx[i];
		unique_check[idx]++;
		if (unique_check[idx] > 1) {
			is_unique = false;
		}
	}

	auto expr = grdc_make_ast_expr<GrdcAstSwizzleExpr>(p, swizzle_type, lhs->is_lvalue && is_unique, text_region);
	expr->lhs = lhs;
	expr->swizzle[0] = swizzle_idx[0];
	expr->swizzle[1] = swizzle_idx[1];
	expr->swizzle[2] = swizzle_idx[2];
	expr->swizzle[3] = swizzle_idx[3];
	expr->swizzle_len = swizzle_len;
	return { expr, NULL };
}

struct GrdcAstStructInitMember {
	GrdcAstExpr*         expr = NULL;
	GrdcAstStructMember* member;
};

struct GrdcAstStructInitializer: GrdcAstExpr {
	GrdcAstType*                      struct_type = NULL;
	GrdArray<GrdcAstStructInitMember> members;

	GRD_REFLECT(GrdcAstStructInitializer) {
		GRD_BASE_TYPE(GrdcAstExpr);
		GRD_MEMBER(struct_type);
		GRD_MEMBER(members);
	}
};

bool grdc_is_vector_type(GrdcAstType* type) {
	type = grdc_resolve_type_alias(type);
	return type == type->p->float2_tp || type == type->p->float3_tp || type == type->p->float4_tp;
}

bool grdc_is_integral(GrdcAstType* type) {
	return grdc_is_integer(type) || grdc_is_bool(type);
}

bool grdc_is_primitive(GrdcAstType* type) {
	return grdc_is_bool(type) || grdc_is_integer(type) || grdc_is_floating_point(type);
}

GrdTuple<GrdcAstExpr*, GrdError*> grdc_typecheck_binary_expr(GrdcParser* p, GrdcAstExpr* lhs, GrdcAstExpr* rhs, GrdcToken op_tok, GrdcAstOperator* op) {

	GrdcProgramTextRegion text_region = { op_tok.reg.start, grdc_peek(p).reg.end };
	if (lhs->text_region.has_value) {
		text_region.start = lhs->text_region.value.start;
	}
	if (rhs->text_region.has_value) {
		text_region.end = rhs->text_region.value.end;
	}

	if (op->op == ",") {
		auto node = grdc_make_ast_expr<GrdcAstBinaryExpr>(p, rhs->expr_type, GRDC_AST_EXPR_RVALUE, text_region);
		node->lhs = lhs;
		node->rhs = rhs;
		node->op = op;
		node->pure_op = op;
		return { node, NULL };
	}

	auto pure_op = op;
	GrdcAstType* expr_result_type = NULL;
	if ((op->flags & GRDC_AST_OP_FLAG_MOD_ASSIGN) || op->op == "=") {
		if (op->flags & GRDC_AST_OP_FLAG_MOD_ASSIGN) {
			pure_op = grdc_find_binary_operator(p, op->op[0, grd_len(op->op) - 1]);
		}
		if (!lhs->is_lvalue) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Left side of '%' must be an lvalue.", op->op) };
		}
	}
	if (lhs->expr_type == grdc_find_type(p, U"void"_b)) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Cannot apply operator '%' to void", op->op) };
	}
	if (grdc_is_vector_type(lhs->expr_type)) {
		if (pure_op->op == "*" ||
			pure_op->op == "/" ||
			pure_op->op == "+" ||
			pure_op->op == "-" ||
			pure_op->op == "="
		) {
			if (rhs->expr_type != grdc_find_type(p, U"float"_b) && 
				rhs->expr_type != grdc_find_type(p, U"double"_b) &&
				lhs->expr_type != rhs->expr_type) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' for vector requires rhs to be float or double or matching vector type, got '%'. lhs type = '%'", op->op, rhs->expr_type->name, lhs->expr_type->name) };
			}
			expr_result_type = lhs->expr_type;
		} else {
			auto e = grd_make_parser_error(p, grd_current_loc(), "Operator '%' cannot be applied to '%'.", op->op, lhs->expr_type->name);

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
		if (pure_op->flags & GRDC_AST_OP_FLAG_BOOL) {
			if (!grdc_is_bool(lhs->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to bool types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & GRDC_AST_OP_FLAG_INT) {
			if (!grdc_is_integer(lhs->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to integer types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & GRDC_AST_OP_FLAG_PRIMITIVE) {
			if (!grdc_is_primitive(lhs->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to primitive types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & GRDC_AST_OP_FLAG_NUMERIC) {
			if (!grdc_is_numeric(lhs->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (lhs->expr_type != rhs->expr_type) {
			auto e = grd_make_parser_error(p, grd_current_loc(), "Binary expression type mismatch, expected '%' but got '%'", lhs->expr_type->name, rhs->expr_type->name);

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
			expr_result_type = grdc_find_type(p, U"bool"_b);
			assert(expr_result_type);
		} else {
			expr_result_type = lhs->expr_type;
		}
	}
	if (!expr_result_type) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Unexpected error: could not determine result type of binary expression '%'.", op->op) };
	}
	auto node = grdc_make_ast_expr<GrdcAstBinaryExpr>(p, expr_result_type, GRDC_AST_EXPR_RVALUE, text_region);
	node->lhs = lhs;
	node->rhs = rhs;
	node->op = op;
	node->pure_op = pure_op;
	return { node, NULL };
}

GrdTuple<GrdcAstExpr*, GrdError*> grdc_parse_primary_expr(GrdcParser* p) {
	auto tok = grdc_peek(p);

	if (tok.flags & CTOKEN_FLAG_FLOATING_POINT) {
		auto lit = tok.str;
		bool parse_as_float = false;
		if (grd_len(lit) > 0) {
			if (lit[grd_len(lit) - 1] == 'f' || lit[grd_len(lit) - 1] == 'F') {
				lit.count -= 1;
				parse_as_float = true;
			}
			if (lit[grd_len(lit) - 1] == 'd' || lit[grd_len(lit) - 1] == 'D') {
				lit.count -= 1;
			}
		}

		GrdPrimitiveValue lit_value;
		GrdPrimitiveType* lit_type = NULL;
		f64 float_val;
		f64 double_val;

		bool ok = false;
		if (parse_as_float) {
			ok = grd_parse_float(lit, &lit_value.f32_value);
			lit_type = grd_reflect_type_of<f32>()->as<GrdPrimitiveType>();
		} else {
			ok = grd_parse_float(lit, &lit_value.f64_value);
			lit_type = grd_reflect_type_of<f64>()->as<GrdPrimitiveType>();
		}
		if (!ok) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not parse floating point literal."_b) };
		}
		auto literal = grdc_make_ast_expr<GrdcAstLiteralExpr>(p, grdc_find_type(p, parse_as_float ? U"float"_b : U"double"_b), GRDC_AST_EXPR_RVALUE, tok.reg);
		literal->lit_type = lit_type;
		literal->lit_value = lit_value;
		grdc_next(p);
		return { literal, NULL };
	}

	if (tok.flags & CTOKEN_FLAG_INTEGER) {
		s64 u;
		bool ok = grd_parse_integer(tok.str, &u);
		if (!ok) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not parse integer literal."_b) };
		}
		auto literal = grdc_make_ast_expr<GrdcAstLiteralExpr>(p, grdc_find_type(p, U"int"_b), GRDC_AST_EXPR_RVALUE, tok.reg);
		literal->lit_value.s64_value = u;
		literal->lit_type = grd_reflect_type_of(u)->as<GrdPrimitiveType>();
		grdc_next(p);
		return { literal, NULL };
	}

	auto unary_op = grdc_find_prefix_unary_operator(p, tok.str);
	if (unary_op) {
		auto op = tok;
		grdc_next(p);
		auto [expr, e] = grdc_parse_expr(p, unary_op->prec);
		if (e) {
			return { NULL, e };
		}
		if (tok.str == "*") {
			auto ptr_tp = grd_reflect_cast<GrdcAstPointerType>(expr->expr_type);
			if (ptr_tp == NULL) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '*' can only be applied to pointer types, got '%'"_b, expr->expr_type->name) };
			}
			auto deref = grdc_make_ast_expr<GrdcAstDerefExpr>(p, ptr_tp->pointee, GRDC_AST_EXPR_LVALUE, tok.reg);
			deref->lhs = expr;
			return { deref, NULL };
		}
		if (tok.str == "&") {
			if (!expr->is_lvalue) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '&' can only be applied to lvalues.", op.str) };
			}
			if (grd_reflect_cast<GrdcAstSwizzleExpr>(expr)) {
				auto e = grd_make_parser_error(p, grd_current_loc(), U"Cannot take an address of a swizzle"_b, expr->expr_type->name);
				add_site(p, e,
					CParserErrorToken{ .reg = tok.reg, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED },
					CParserErrorToken{ .reg = expr->text_region, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN }
				);
				return { NULL, e };
			}
		}
		if (tok.str == "++" || tok.str == "--") {
			if (!expr->is_lvalue) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", op.str) };
			}
		}
		if (tok.str == "!" || tok.str == "~") {
			if (!grdc_is_integer(expr->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to integer types.", op.str) };
			}
		}
		if (!grdc_is_numeric(expr->expr_type)) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op.str, expr->expr_type->name) };
		}
		GrdcProgramTextRegion text_region = { op.reg.start, grdc_peek(p).reg.start };
		auto unary = grdc_make_ast_expr<GrdcAstUnaryExpr>(p, expr->expr_type, GRDC_AST_EXPR_RVALUE, text_region);
		unary->expr = expr;
		unary->op = unary_op;
		return { unary, NULL };
	}

	if (tok.str == "("_b) {
		grdc_next(p);
		auto [expr, e] = grdc_parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		if (grdc_peek(p).str != ")"_b) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ')'.", tok) };
		}
		grdc_next(p);
		return { expr, NULL };
	}

	GrdcAstNode* sym = grdc_lookup_symbol(p, tok.str);
	if (!sym) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Unknown identifier.") };
	}
	GrdcAstExpr* lhs = NULL;
	if (auto type = grd_reflect_cast<GrdcAstType>(sym)) {
		auto initializer_reg_start = tok.reg.start;
		grdc_next(p);
		if (grdc_peek(p).str == "("_b) {
			grdc_next(p);
			auto struct_tp = grd_reflect_cast<GrdcAstStructType>(type);
			if (!struct_tp) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct type but got '%'"_b, type->name) };
			}
			s64 member_index = 0;
			GrdArray<GrdcAstExpr*> args = { .allocator = p->allocator };
			grd_defer { args.free(); };
			while (true) {
				auto tok = grdc_peek(p);
				if (tok.str == ")"_b) {
					if (grd_len(args) != grd_len(struct_tp->members)) {
						return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected % arguments in initializer but got '%'.", grd_len(struct_tp->members), grd_len(args)) };
					}
					GrdcProgramTextRegion text_region = { initializer_reg_start, tok.reg.end };
					auto node = grdc_make_ast_expr<GrdcAstStructInitializer>(p, type, GRDC_AST_EXPR_RVALUE, text_region);
					node->struct_type = type;
					node->members.allocator = p->allocator;
					s64 idx = 0;
					for (auto it: args) {
						auto member = struct_tp->members[idx];
						auto m = GrdcAstStructInitMember {
							.expr = it,
							.member = member,
						};
						grd_add(&node->members, m);
						idx += 1;
					}
					args = {};
					grdc_next(p);
					lhs = node;
					break;
				}
				auto op = grdc_find_binary_operator(p, U","_b);
				auto [expr, e] = grdc_parse_expr(p, op->prec + 1);
				if (e) {
					return { NULL, e };
				}
				if (member_index >= grd_len(struct_tp->members)) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Exceeded arguments count in initializer, struct '%' has % members.", type->name, grd_len(struct_tp->members)) };
				}
				auto member = struct_tp->members[member_index];
				if (member->member_ts->tp != expr->expr_type) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", member->member_ts->tp->name, expr->expr_type->name) };
				}
				grd_add(&args, expr);
				member_index += 1;
				if (grdc_peek(p).str == ","_b) {
					grdc_next(p);
				}
			}
		} else {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct initializer, got '%'.", tok.str) };
		}
	} else if (auto var = grd_reflect_cast<GrdcAstVar>(sym)) {
		auto reg = tok.reg;
		grdc_next(p);
		auto expr = grdc_make_ast_expr<GrdcAstVariableAccess>(p, var->var_ts->tp, GRDC_AST_EXPR_LVALUE, reg);
		expr->var = var;
		lhs = expr;
	} else if (auto func = grd_reflect_cast<GrdcAstFunction>(sym)) {
		grdc_next(p);
		auto [call, e] = grdc_parse_function_call(p, tok, func);
		if (e) {
			return { NULL, e };
		}
		lhs = call;
	} else {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected type or variable"_b) };
	}

	while (true) {
		auto tok = grdc_peek(p);

		auto postfix_op = grdc_find_postfix_unary_operator(p, tok.str);
		if (postfix_op) {
			if (tok.str == "++"_b || tok.str == "--"_b) {
				if (!grdc_is_numeric(lhs->expr_type)) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types. Got '%'.", tok.str, lhs->expr_type->name) };
				}
				if (!lhs->is_lvalue) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", tok.str) };
				}
			}
			grdc_next(p);
			GrdcProgramTextRegion text_region = { tok.reg.start, tok.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = grdc_make_ast_expr<GrdcAstUnaryExpr>(p, lhs->expr_type, GRDC_AST_EXPR_RVALUE, text_region);
			node->expr = lhs;
			node->op = postfix_op;
			lhs = node;
			continue;
		}
		if (tok.str == "("_b) {
			// @TODO: support calling on function pointers.
			auto func = grd_reflect_cast<GrdcAstFunction>(lhs);
			if (!func) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a function, got '%'"_b, lhs->type->name) };
			}
			auto [call, e] = grdc_parse_function_call(p, tok, func);
			if (e) {
				return { NULL, e };
			}
			lhs = call;
			continue;
		}
		if (tok.str == "."_b) {
			GrdcAstStructType* access_tp = NULL;
			if (auto struct_tp = grd_reflect_cast<GrdcAstStructType>(lhs->expr_type)) {
				access_tp = struct_tp;
			} else if (auto ptr_tp = grd_reflect_cast<GrdcAstPointerType>(lhs->expr_type)) {
				if (auto struct_tp = grd_reflect_cast<GrdcAstStructType>(ptr_tp->pointee)) {
					access_tp = struct_tp;
				} else {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct type but got '%'"_b, ptr_tp->pointee->name) };
				}
			} else {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a struct, got '%'"_b, lhs->expr_type->name) };
			}
			grdc_next(p);
			auto [ident, e] = grdc_parse_ident(p);
			if (e) {
				return { NULL, e };
			}
			
			auto [swizzle, ee] = grdc_try_parse_swizzle_expr(p, lhs, access_tp, ident);
			if (ee) {
				return { NULL, ee };
			}
			if (swizzle) {
				lhs = swizzle;
				continue;
			}

			auto member = grdc_find_struct_member(p, access_tp, ident.str);
			if (!member) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
			}
			GrdcProgramTextRegion text_region = { ident.reg.start, ident.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = grdc_make_ast_expr<GrdcAstVarMemberAccess>(p, member->member_ts->tp, lhs->is_lvalue, text_region);
			node->lhs = lhs;
			node->member = member;
			lhs = node;
			continue;
		}
		if (tok.str == "["_b) {
			bool ok = grdc_is_pointer(lhs->expr_type) || grdc_is_array(lhs->expr_type);
			if (!ok) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected an array or pointer, got '%'"_b, lhs->expr_type->name) };
			}
			grdc_next(p);
			auto [expr, e] = grdc_parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			if (!grdc_is_integer(expr->expr_type)) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"GrdArray index must be an integer, got '%'"_b, expr->expr_type->name) };
			}
			if (grdc_peek(p).str != "]"_b) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a ]"_b) };
			}
			grdc_next(p);
			auto element_type = grdc_get_element_type(lhs->expr_type);
			if (!element_type) {
				panic(p, U"Unexpected error. expected an array or pointer, got %"_b, lhs->expr_type->name);
			}
			GrdcProgramTextRegion text_region = { tok.reg.start, grdc_peek(p).reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = grdc_make_ast_expr<GrdcAstGrdArrayAccess>(p, element_type, GRDC_AST_EXPR_LVALUE, text_region);
			node->lhs = lhs;
			node->index = expr;
			lhs = node;
			continue;
		}
		break;
	}

	return { lhs, NULL };
}

GrdTuple<GrdcAstExpr*, GrdError*> grdc_parse_expr(GrdcParser* p, s32 min_prec) {
	auto [lhs, e] = grdc_parse_primary_expr(p);
	if (e) {
		return { NULL, e };
	}

	while (true) {
		auto tok = grdc_peek(p);
		auto op = grdc_find_binary_operator(p, tok.str);
		if (!op) {
			break;
		}
		if (op->prec < min_prec) {
			break;
		}
		if (tok.str == "?"_b) {
			grdc_next(p);
			auto [then_expr, e2] = grdc_parse_expr(p, 0);
			if (e2) {
				return { NULL, e2 };
			}
			auto tok = grdc_peek(p);
			if (tok.str != ":"_b) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected :"_b) };
			}
			grdc_next(p);
			auto [else_expr, e3] = grdc_parse_expr(p, (op->flags & GRDC_AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
			if (e3) {
				return { NULL, e3 };
			}
			if (lhs->expr_type != else_expr->expr_type) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Ternary expression type mismatch, expected '%' but got '%'.", lhs->expr_type->name, else_expr->expr_type->name) };
			}
			GrdcProgramTextRegion text_region = { tok.reg.start, grdc_peek(p).reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			if (then_expr->text_region.has_value) {
				text_region.end = then_expr->text_region.value.end;
			}
			if (else_expr->text_region.has_value) {
				text_region.end = else_expr->text_region.value.end;
			}
			auto node = grdc_make_ast_expr<GrdcAstTernary>(p, lhs->expr_type, GRDC_AST_EXPR_RVALUE, text_region);
			node->cond = lhs;
			node->then = then_expr;
			node->else_ = else_expr;
			lhs = node;
			continue;
		}
		grdc_next(p);
		auto [rhs, e] = grdc_parse_expr(p, (op->flags & GRDC_AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
		if (e) {
			return { NULL, e };
		}

		auto [expr, e2] = grdc_typecheck_binary_expr(p, lhs, rhs, tok, op);
		if (e2) {
			return { NULL, e2 };
		}
		lhs = expr;
	}
	return { lhs, NULL };
}

GrdTuple<GrdcAstBlock*, GrdError*> grdc_parse_block(GrdcParser* p);
GrdTuple<GrdcAstNode*, GrdError*, bool> grdc_parse_stmt(GrdcParser* p);
GrdTuple<GrdcAstBlock*, GrdError*> grdc_parse_block_or_one_stmt(GrdcParser* p);

GrdTuple<s64, GrdError*> grdc_eval_const_int_inner(GrdcAstExpr* expr) {
	auto p = expr->p;
	if (auto i = grd_reflect_cast<GrdcAstLiteralExpr>(expr)) {
		if (i->lit_type == grd_reflect_type_of<s64>()) {
			return { i->lit_value.s64_value, NULL };
		} else if (i->lit_type == grd_reflect_type_of<s32>()) {
			return { i->lit_value.s32_value, NULL };
		} else {
			return { 0, grd_simple_parser_error(p, grd_current_loc(), expr->text_region, "Expected s32 or s64 literal, got '%*'", i->lit_type->name) };
		}
	} else if (auto i = grd_reflect_cast<GrdcAstUnaryExpr>(expr)) {
		if (i->op->op == "-"_b) {
			auto [v, e] = grdc_eval_const_int_inner(i->expr);
			if (e) {
				return { 0, e };
			}
			return { -v, NULL };
		} else {
			return { 0, grd_simple_parser_error(p, grd_current_loc(), expr->text_region, "Unexpected unary operator '%*'", i->op->op) };
		}
	} else {
		return { 0, grd_simple_parser_error(p, grd_current_loc(), expr->text_region, "Expected literal, got '%*'", expr->type->name) };
	}
}

GrdTuple<s64, GrdError*> grdc_eval_const_int(GrdcAstExpr* expr, s64 min, s64 max_inclusive) {
	auto [v, e] = grdc_eval_const_int_inner(expr);
	if (e) {
		return { 0, e };
	}
	if (v < min) {
		return { 0, grd_simple_parser_error(expr->p, grd_current_loc(), expr->text_region, "Expected >= %, got %.", min, v) };
	}
	if (v > max_inclusive) {
		return { 0, grd_simple_parser_error(expr->p, grd_current_loc(), expr->text_region, "Expected <= %, got %.", max_inclusive, v) };
	}
	return { v, NULL };
}

struct GrdcMtlBufferIdxAttr: GrdcAstAttr {
	s64 index = 0;

	GRD_REFLECT(GrdcMtlBufferIdxAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
		GRD_MEMBER(index);
	}
};

struct GrdcMtlBufferAttr: GrdcMtlBufferIdxAttr {
	GRD_REFLECT(GrdcMtlBufferAttr) {
		GRD_BASE_TYPE(GrdcMtlBufferIdxAttr);
	}
};

struct GrdcMtlConstantAttr: GrdcMtlBufferIdxAttr {
	GRD_REFLECT(GrdcMtlConstantAttr) {
		GRD_BASE_TYPE(GrdcMtlBufferIdxAttr);
	}
};

struct GrdcVkSetBindingAttr: GrdcAstAttr {
	s64 set = 0;
	s64 binding = 0;

	GRD_REFLECT(GrdcVkSetBindingAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
		GRD_MEMBER(set);
		GRD_MEMBER(binding);
	}
};

struct GrdcVkUniformAttr: GrdcVkSetBindingAttr {
	GRD_REFLECT(GrdcVkUniformAttr) {
		GRD_BASE_TYPE(GrdcVkSetBindingAttr);
	}
};


struct GrdcPositionAttr: GrdcAstAttr {
	GRD_REFLECT(GrdcPositionAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
	}
};

struct GrdcFragmentAttr: GrdcAstAttr {
	GRD_REFLECT(GrdcFragmentAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
	}
};

struct GrdcVertexAttr: GrdcAstAttr {
	GRD_REFLECT(GrdcVertexAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
	}
};

struct GrdcStageInAttr: GrdcAstAttr {
	GRD_REFLECT(GrdcStageInAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
	}
};

struct GrdcColorAttr: GrdcAstAttr {
	s64 idx = 0;

	GRD_REFLECT(GrdcColorAttr) {
		GRD_BASE_TYPE(GrdcAstAttr);
		GRD_MEMBER(idx);
	}
};

template <typename T>
T* grdc_make_ast_attr(GrdcParser* p, GrdcToken name, GrdArray<GrdcAstExpr*> args, GrdcProgramTextRegion reg) {
	auto node = grdc_make_ast_node<T>(p, reg);
	node->name = name.str;
	node->args = args;
	return node;
}

GrdTuple<GrdcAstAttr*, GrdError*> grdc_parse_attr(GrdcParser* p, GrdcToken name, GrdArray<GrdcAstExpr*> args, GrdcProgramTextRegion reg) {
	if (name.str == "vk_uniform") {
		auto attr = grdc_make_ast_attr<GrdcVkUniformAttr>(p, name, args, reg);
		if (grd_len(args) == 0) {

		} else if (grd_len(args) == 1) {
			auto [v, e] = grdc_eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->binding = v;
		} else if (grd_len(args) == 2) {
			auto [v, e] = grdc_eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->set = v;
			auto [v2, e2] = grdc_eval_const_int(args[1], 0, s64_max);
			if (e2) {
				return { NULL, e2 };
			}
			attr->binding = v2;
		} else {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), reg, "Expected (set, binding) or (set).") };
		}
		return { attr, NULL };
	} else if (name.str == "mtl_buffer") {
		auto attr = grdc_make_ast_attr<GrdcMtlBufferAttr>(p, name, args, reg);
		if (grd_len(args) == 0) {

		} else if (grd_len(args) == 1) {
			auto [v, e] = grdc_eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->index = v;
		} else {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), reg, "Expected (index).") };
		}
		return { attr, NULL };
	} else if (name.str == "mtl_constant") {
		auto attr = grdc_make_ast_attr<GrdcMtlConstantAttr>(p, name, args, reg);
		if (grd_len(args) == 0) {

		} else if (grd_len(args) == 1) {
			auto [v, e] = grdc_eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->index = v;
		} else {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), reg, "Expected (index).") };
		}
		return { attr, NULL };
	} else if (name.str == "position") {
		if (grd_len(args) != 0) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for position attribute, got %"_b, grd_len(args)) };
		}
		auto attr = grdc_make_ast_attr<GrdcPositionAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "fragment") {
		if (grd_len(args) != 0) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for fragment attribute, got %"_b, grd_len(args)) };
		}
		auto attr = grdc_make_ast_attr<GrdcFragmentAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "stage_in") {
		if (grd_len(args) != 0) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for stage_in attribute, got %"_b, grd_len(args)) };
		}
		auto attr = grdc_make_ast_attr<GrdcStageInAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "color") {
		if (grd_len(args) != 1) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), name.reg, "Expected 1 argument for color attribute, got %"_b, grd_len(args)) };
		}
		auto [v, e] = grdc_eval_const_int(args[0], 0, s64_max);
		if (e) {
			return { NULL, e };
		}
		auto attr = grdc_make_ast_attr<GrdcColorAttr>(p, name, args, reg);
		attr->idx = v;
		return { attr, NULL };
	} else {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), name.reg, "Unknown attribute") };
	}
}

GrdTuple<GrdArray<GrdcAstAttr*>, GrdError*> grdc_parse_attrs(GrdcParser* p) {
	GrdArray<GrdcAstAttr*> attrs;
	attrs.allocator = p->allocator;
	while (true) {
		auto start_tok = grdc_peek(p);
		if (start_tok.str != "[[") {
			break;
		}
		grdc_next(p);
		auto [name, e] = grdc_parse_ident(p);
		if (e) {
			return { {}, e };
		}
		auto tok = grdc_peek(p);
		GrdArray<GrdcAstExpr*> args = { .allocator = p->allocator };
		if (tok.str == "(") {
			grdc_next(p);
			while (true) {
				auto tok = grdc_peek(p);
				if (tok.str == ")") {
					grdc_next(p);
					break;
				}
				if (grd_len(args) > 0) {
					if (tok.str != ",") {
						return { {}, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ','"_b) };
					}
					grdc_next(p);
				}
				auto op = grdc_find_binary_operator(p, U","_b);
				auto [expr, e] = grdc_parse_expr(p, op->prec + 1);
				if (e) {
					return { {}, e };
				}
				grd_add(&args, expr);
			}
		}
		auto end_tok = grdc_peek(p);
		if (end_tok.str != "]]") {
			return { {}, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ]]"_b) };
		}
		grdc_next(p);
		GrdcProgramTextRegion reg = { start_tok.reg.start, end_tok.reg.end };
		auto [attr, e3] = grdc_parse_attr(p, name, args, reg);
		if (e3) {
			return { {}, e3 };
		}
		grd_add(&attrs, attr);
	}
	return { attrs, NULL };
}

GrdTuple<GrdcAstIf*, GrdError*> parse_if(GrdcParser* p, GrdcToken if_tok) {
	if (grdc_peek(p).str != "("_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ("_b) };
	}
	grdc_next(p);
	auto [cond, e] = grdc_parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (cond->expr_type != grdc_find_type(p, U"bool"_b)) {
		auto error_reg = if_tok.reg;
		if (cond->text_region.has_value) {
			error_reg = cond->text_region.value;
		}
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), error_reg, U"Condition must be bool, but it's '%'"_b, cond->expr_type->name) };
	}
	if (grdc_peek(p).str != ")"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected )"_b) };
	}
	grdc_next(p);
	auto [then, e2] = grdc_parse_block_or_one_stmt(p);
	if (e2) {
		return { NULL, e2 };
	}
	GrdcProgramTextRegion reg = if_tok.reg;
	if (then->text_region.has_value) {
		reg.end = then->text_region.value.end;
	}
	auto node = grdc_make_ast_node<GrdcAstIf>(p, reg);
	node->cond = cond;
	node->then = then;
	auto tok = grdc_peek(p);
	if (tok.str == U"else"_b) {
		grdc_next(p);
		tok = grdc_peek(p);
		if (tok.str == U"if"_b) {
			grdc_next(p);
			auto [else_if, e3] = parse_if(p, tok);
			if (e3) {
				return { NULL, e3 };
			}
			node->else_if = else_if;
			return { node, NULL };
		}
		auto [else_block, e3] = grdc_parse_block_or_one_stmt(p);
		if (e3) {
			return { NULL, e3 };
		}
		node->else_block = else_block;
		return { node, NULL };
	} else {
		return { node, NULL };
	}
}

GrdTuple<GrdcAstBlock*, GrdError*> grdc_parse_block_or_one_stmt(GrdcParser* p) {
	auto tok = grdc_peek(p);
	if (tok.str == "{"_b) {
		auto [block, e] = grdc_parse_block(p);
		if (e) {
			return { NULL, e };
		}
		return { block, NULL };
	} else {
		auto [stmt, e, semicolon_opt] = grdc_parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		if (!semicolon_opt) {
			if (grdc_peek(p).str != U";"_b) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ;"_b) };
			}
			grdc_next(p);
		}
		auto block = grdc_make_ast_node<GrdcAstBlock>(p, stmt->text_region);
		grd_add(&block->statements, stmt);
		return { block, NULL };
	}
}

struct GrdcAstFor: GrdcAstNode {
	GrdcAstExpr*  init_expr = NULL;
	GrdcAstExpr*  cond_expr = NULL;
	GrdcAstExpr*  incr_expr = NULL;
	GrdcAstBlock* body = NULL;

	GRD_REFLECT(GrdcAstFor) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(init_expr);
		GRD_MEMBER(cond_expr);
		GRD_MEMBER(incr_expr);
		GRD_MEMBER(body);
	}
};

GrdTuple<GrdcAstNode*, GrdError*> grdc_parse_for(GrdcParser* p, GrdcToken for_tok) {
	if (grdc_peek(p).str != "("_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ("_b) };
	}
	grdc_next(p);
	auto [init_expr, e] = grdc_parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (grdc_peek(p).str != ";"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ;"_b) };
	}
	grdc_next(p);
	auto [cond_expr, e2] = grdc_parse_expr(p, 0);
	if (e2) {
		return { NULL, e2 };
	}
	if (grdc_peek(p).str != ";"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ;"_b) };
	}
	grdc_next(p);
	auto [incr_expr, e3] = grdc_parse_expr(p, 0);
	if (e3) {
		return { NULL, e3 };
	}
	if (grdc_peek(p).str != ")"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected )"_b) };
	}
	grdc_next(p);
	auto [body, e4] = grdc_parse_block_or_one_stmt(p);
	if (e4) {
		return { NULL, e4 };
	}
	GrdcProgramTextRegion text_region = { for_tok.reg.start, grdc_peek(p).reg.end };
	if (body->text_region.has_value) {
		text_region.end = body->text_region.value.end;
	}
	auto node = grdc_make_ast_node<GrdcAstFor>(p, text_region);
	node->init_expr = init_expr;
	node->cond_expr = cond_expr;
	node->incr_expr = incr_expr;
	node->body = body;
	return { node, NULL };
}

GrdcAstFunction* grdc_get_current_function(GrdcParser* p) {
	for (auto i: grd_reverse(grd_range(grd_len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto f = grd_reflect_cast<GrdcAstFunction>(scope)) {
			return f;
		}
	}
	return NULL;
}

struct GrdcAstReturn: GrdcAstNode {
	GrdcAstExpr* rhs = NULL;

	GRD_REFLECT(GrdcAstReturn) {
		GRD_BASE_TYPE(GrdcAstNode);
		GRD_MEMBER(rhs);
	}
};

GrdTuple<GrdcAstNode*, GrdError*, bool> grdc_parse_stmt(GrdcParser* p) {
	auto type_tok = grdc_peek(p);
	if (type_tok.str == "if"_b) {
		grdc_next(p);
		auto [if_, e] = parse_if(p, type_tok);
		if (e) {
			return { NULL, e };
		}
		return { if_, NULL, true };
	} else if (type_tok.str == "for"_b) {
		grdc_next(p);
		auto [for_, e] = grdc_parse_for(p, type_tok);
		if (e) {
			return { NULL, e };
		}
		return { for_, NULL, true };
	} else if (type_tok.str == "return"_b) {
		auto f = grdc_get_current_function(p);
		if (!f) {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), type_tok.reg, U"return must be inside a function"_b) };
		}
		grdc_next(p);

		GrdcAstExpr* rhs = NULL;
		if (f->return_ts->tp != p->void_tp) {
			auto [expr, e] = grdc_parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			rhs = expr;
			if (f->return_ts->tp != rhs->expr_type) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), rhs->text_region, U"Expected return type '%' but got '%'"_b, f->return_ts->tp->name, rhs->expr_type->name) };
			}
		}
		GrdcProgramTextRegion reg = type_tok.reg;
		if (rhs->text_region.has_value) {
			reg.end = rhs->text_region.value.end;
		}
		auto node = grdc_make_ast_node<GrdcAstReturn>(p, reg);
		node->rhs = rhs;
		return { node, NULL };
	}

	auto lookup_tp = grdc_find_type(p, type_tok.str);
	if (lookup_tp) {
		auto [pre_type, e1] = grdc_parse_pre_type(p);
		if (e1) {
			return { NULL, e1 };
		}
		auto [ident, e2] = grdc_parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		auto [decl, ee] = grdc_parse_var_decl(p, pre_type, ident, false);
		if (ee) {
			return { NULL, ee };
		}
		return { decl, NULL, false };
	}
	auto [expr, e] = grdc_parse_expr(p, 0);
	return { expr, e, false };
}

GrdTuple<GrdcAstBlock*, GrdError*> grdc_parse_block(GrdcParser* p) {
	auto tok = grdc_peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected {"_b) };
	}
	grdc_next(p);
	auto block = grdc_make_ast_node<GrdcAstBlock>(p, {});
	block->statements.allocator = p->allocator;
	grd_add(&p->scope, block);
	grd_defer { grd_pop(&p->scope); };

	s64 region_end = -1;
	
	while (true) {
		auto tok = grdc_peek(p);
		if (tok.str == U"}"_b) {
			region_end = tok.reg.end;
			grdc_next(p);
			break;
		}
		auto [stmt, e, semicolon_opt] = grdc_parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		if (!semicolon_opt) {
			if (grdc_peek(p).str != U";"_b) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ;"_b) };
			}
			grdc_next(p);
		}
		grd_add(&block->statements, stmt);
	}
	assert(region_end != -1);
	GrdcProgramTextRegion text_region = { tok.reg.start, region_end };
	block->text_region = text_region;
	return { block, NULL };
}

GrdError* grdc_make_double_site_error(GrdcParser* p, GrdCodeLoc loc, GrdOptional<GrdcProgramTextRegion> reg, GrdOptional<GrdcProgramTextRegion> reg2, auto... args) {
	auto error = grd_make_parser_error(p, loc, args...);
	add_site(p, error,
		CParserErrorToken{ .reg = reg, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN },
		CParserErrorToken{ .reg = reg2, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED }
	);
	return error;
}

GrdTuple<GrdcAstFunctionKind, GrdError*> grdc_resolve_function_kind(GrdcParser* p, GrdcAstFunction* f) {
	GrdcAstFunctionKind kind = GrdcAstFunctionKind::Plain;
	GrdcAstAttr* kind_attr = NULL;
	for (auto attr: f->attrs) {
		GrdcAstFunctionKind new_kind = GrdcAstFunctionKind::Plain;
		if (auto m = grd_reflect_cast<GrdcFragmentAttr>(attr)) {
			new_kind = GrdcAstFunctionKind::Fragment;
		}
		if (auto m = grd_reflect_cast<GrdcVertexAttr>(attr)) {
			new_kind = GrdcAstFunctionKind::Vertex;
		}
		if (new_kind != GrdcAstFunctionKind::Plain) {
			if (kind != GrdcAstFunctionKind::Plain) {
				return { {}, grdc_make_double_site_error(p, grd_current_loc(), kind_attr->text_region, attr->text_region, "Function kind already defined."_b) };
			}
			kind = new_kind;
		}
	}
	return { kind, NULL };
}

GrdError* grdc_handle_entry_point_arg(GrdcParser* p, GrdcAstFunction* f, s64 idx) {
	auto arg = f->args[idx];

	enum class EntryPointArgKind {
		Undefined,
		StageIn,
		Buffer,
	};
	EntryPointArgKind kind = EntryPointArgKind::Undefined;

	for (auto attr: arg->attrs) {
		if (auto m = grd_reflect_cast<GrdcStageInAttr>(attr)) {
			m->is_used = true;
			kind = EntryPointArgKind::StageIn;
			break;
		}
		if (auto m = grd_reflect_cast<GrdcMtlBufferAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
		if (auto m = grd_reflect_cast<GrdcMtlConstantAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
		if (auto m = grd_reflect_cast<GrdcVkUniformAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
	}
	if (kind == EntryPointArgKind::Undefined) {
		return grd_simple_parser_error(p, grd_current_loc(), arg->text_region, "Unexpected entry point argument."_b);
	}
	if (kind == EntryPointArgKind::Buffer) {
		if (!grdc_is_pointer(arg->var_ts->tp)) {
			return grd_simple_parser_error(p, grd_current_loc(), arg->text_region, "Expected pointer type for a buffer."_b);
		}
		GrdcVkUniformAttr* vk_attr = NULL;
		GrdcMtlBufferIdxAttr* mtl_attr = NULL;
		for (auto attr: arg->attrs) {
			if (auto m = grd_reflect_cast<GrdcVkUniformAttr>(attr)) {
				if (vk_attr) {
					return grdc_make_double_site_error(p, grd_current_loc(), vk_attr->text_region, attr->text_region, "Vk uniform attribute is already specified."_b);
				}
				vk_attr = m;
			} else if (auto m = grd_reflect_cast<GrdcMtlBufferAttr>(attr)) {
				if (mtl_attr) {
					return grdc_make_double_site_error(p, grd_current_loc(), mtl_attr->text_region, attr->text_region, "Metal buffer attribute is already specified."_b);
				}
				mtl_attr = m;
			} else if (auto m = grd_reflect_cast<GrdcMtlConstantAttr>(attr)) {
				if (mtl_attr) {
					return grdc_make_double_site_error(p, grd_current_loc(), mtl_attr->text_region, attr->text_region, "Metal constant attribute is already specified."_b);
				}
				mtl_attr = m;
			} else {
				return grd_simple_parser_error(p, grd_current_loc(), attr->text_region, "Unexpected attribute."_b);
			}
		}
		if (!vk_attr) {
			return grd_simple_parser_error(p, grd_current_loc(), arg->text_region, "Vk uniform attribute must be specified."_b);
		}
		if (!mtl_attr) {
			return grd_simple_parser_error(p, grd_current_loc(), arg->text_region, "Metal buffer attribute must be specified."_b);
		}
		for (auto it: f->mtl_buffers) {
			if (it->index == mtl_attr->index) {
				return grdc_make_double_site_error(p, grd_current_loc(), it->text_region, mtl_attr->text_region, "Metal buffer index (%) is already used."_b, mtl_attr->index);
			}
		}
		for (auto it: f->vk_set_bindings) {
			if (it->set == vk_attr->set && it->binding == vk_attr->binding) {
				return grdc_make_double_site_error(p, grd_current_loc(), it->text_region, vk_attr->text_region, "Vulkan set/binding (%, %) is already used."_b, vk_attr->set, vk_attr->binding);
			}
		}
		mtl_attr->is_used = true;
		vk_attr->is_used = true;
		grd_add(&f->mtl_buffers, mtl_attr);
		grd_add(&f->vk_set_bindings, vk_attr);
	}
	if (kind == EntryPointArgKind::StageIn) {
		if (!grdc_is_struct(arg->var_ts->tp)) {
			return grd_simple_parser_error(p, grd_current_loc(), arg->text_region, "Expected struct type for a stage_in argument."_b);
		}
		if (f->stage_in_arg) {
			return grdc_make_double_site_error(p, grd_current_loc(), f->stage_in_arg->text_region, arg->text_region, "Stage_in argument is already specified."_b);
		}
		f->stage_in_arg = arg;
	}
	return NULL;
}

GrdError* grdc_handle_entry_point_args(GrdcParser* p, GrdcAstFunction* f) {
	for (auto i: grd_range(grd_len(f->args))) {
		if (f->kind != GrdcAstFunctionKind::Plain) {
			auto e = grdc_handle_entry_point_arg(p, f, i);
			if (e) {
				return e;
			}
		}
		auto e = grdc_check_if_attrs_used(p, f->args[i]->attrs);
		if (e) {
			return e;
		}
	}
	return NULL;
}

GrdError* grdc_ensure_function_has_return(GrdcParser* p, GrdcAstFunction* f) {
	if (f->return_ts->tp == p->void_tp) {
		return NULL;
	}
	if (!f->block) {
		return grd_simple_parser_error(p, grd_current_loc(), f->text_region, "Function % has no body", f->name);
	}
	if (grd_len(f->block->statements) == 0) {
		// Expected return statement.
		return grd_simple_parser_error(p, grd_current_loc(), f->block->text_region, "Expected return statement");
	}
	auto last = f->block->statements[-1];
	auto ret = grd_reflect_cast<GrdcAstReturn>(last);
	if (!ret) {
		return grd_simple_parser_error(p, grd_current_loc(), f->block->text_region, "Expected return statement");
	}
	return NULL;
}

GrdError* grdc_validate_fragment_output_struct(GrdcParser* p, GrdcAstStructType* tp) {
	s64 idx = 0;
	for (auto it: tp->members) {
		GrdcColorAttr* c_attr = NULL;
		for (auto attr: it->attrs) {
			if (auto m = grd_reflect_cast<GrdcColorAttr>(attr)) {
				if (c_attr) {
					return grdc_make_double_site_error(p, grd_current_loc(), c_attr->text_region, attr->text_region, "Color attribute is already specified."_b);
				}
				c_attr = m;
			}
		}
		if (c_attr == NULL) {
			return grd_simple_parser_error(p, grd_current_loc(), it->text_region, "Color attribute must be specified."_b);
		}
		for (auto i: grd_range(idx)) {
			auto prev_m = tp->members[i];
			for (auto attr: prev_m->attrs) {
				if (auto m = grd_reflect_cast<GrdcColorAttr>(attr)) {
					if (m->idx == c_attr->idx) {
						return grdc_make_double_site_error(p, grd_current_loc(), m->text_region, c_attr->text_region, "Color index (%) is already used."_b, c_attr->idx);
					}
				}
			}
		}
		idx += 1;
	}
	return NULL;
}

GrdError* grdc_typecheck_function_return_value(GrdcParser* p, GrdcAstFunction* f) {
	if (f->kind == GrdcAstFunctionKind::Fragment) {
		if (f->return_ts->tp == p->void_tp) {

		} else if (f->return_ts->tp == p->float4_tp) {

		} else if (grdc_is_struct(f->return_ts->tp)) {
			auto stp = grd_reflect_cast<GrdcAstStructType>(f->return_ts->tp);
			if (!stp) {
				return grd_simple_parser_error(p, grd_current_loc(), f->text_region, "Internal error: expected struct type.");
			}
			auto e = grdc_validate_fragment_output_struct(p, stp);
			if (e) {
				return e;
			}
		} else {
			return grd_simple_parser_error(p, grd_current_loc(), f->text_region, "Fragment function must return void, float4, or struct."_b);
		}
	} else if (f->kind == GrdcAstFunctionKind::Vertex) {
		if (f->return_ts->tp != p->void_tp) {
			return grd_simple_parser_error(p, grd_current_loc(), f->text_region, "Vertex function must return void."_b);
		}
	}
	return NULL;
}

GrdTuple<GrdcAstSymbol*, GrdError*> grdc_parse_function(GrdcParser* p, GrdcPreType pre_type,  GrdArray<GrdcAstAttr*> attrs, GrdcToken start_tok, GrdcToken ident) {
	GrdArray<GrdcAstFunctionArg*> args = { .allocator = p->allocator };
	grd_defer { args.free(); };

	grdc_next(p);

	GrdcAstTypeSite* return_ts = NULL;

	while (true) {
		auto tok = grdc_peek(p);
		if (tok.str == U")"_b) {
			grdc_next(p);
			auto [attrs, e] = grdc_parse_attrs(p);
			if (e) {
				return { NULL, e };
			}
			auto [ts, e2] = grdc_finalize_type(p, pre_type, attrs);
			if (e2) {
				return { NULL, e2 };
			}
			return_ts = ts;
			auto e4 = grdc_check_if_attrs_used(p, attrs);
			if (e4) {
				return { NULL, e4 };
			}
			break;
		}
		if (grd_len(args) > 0) {
			if (tok.str != U","_b) {
				return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ','"_b) };
			}
			grdc_next(p);
		}
		auto start = grdc_peek(p).reg.start;
		auto type_tok = grdc_peek(p);
		auto [pre_type, e] = grdc_parse_pre_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [arg_name, e2] = grdc_parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		auto [attrs, e3] = grdc_parse_attrs(p);
		if (e3) {
			return { NULL, e3 };
		}
		auto [ts, e4] = grdc_finalize_type(p, pre_type, attrs);
		if (e4) {
			return { NULL, e4 };
		}
		GrdcProgramTextRegion text_region = { start, grdc_peek(p).reg.end };
		if (grd_len(attrs) > 0) {
			auto last = attrs[-1];
			if (last->text_region.has_value) {
				text_region.end = last->text_region.value.end;
			}
		}
		auto arg_node = grdc_make_ast_symbol<GrdcAstFunctionArg>(p, arg_name.str, text_region);
		arg_node->var_ts = ts;
		arg_node->attrs = attrs;
		grd_add(&args, arg_node);
	}

	GrdcProgramTextRegion text_region = { start_tok.reg.start, grdc_peek(p).reg.end };
	auto f = grdc_make_ast_node<GrdcAstFunction>(p, text_region);
	f->args.allocator = p->allocator;
	f->return_ts = return_ts;
	f->name = ident.str;
	f->args = args;
	f->attrs = attrs;
	f->mtl_buffers.allocator = p->allocator;
	f->vk_set_bindings.allocator = p->allocator;

	for (auto attr: attrs) {
		if (grd_reflect_cast<GrdcFragmentAttr>(attr)) {
			f->kind = GrdcAstFunctionKind::Fragment;
			attr->is_used = true;
			break;
		}
	}

	auto e = grdc_typecheck_function_return_value(p, f);
	if (e) {
		return { NULL, e };
	}
	e = grdc_handle_entry_point_args(p, f);
	if (e) {
		return { NULL, e };
	}
	e = grdc_check_if_attrs_used(p, attrs);
	if (e) {
		return { NULL, e };
	}

	auto tok = grdc_peek(p);
	if (tok.str == ";") {
		grdc_next(p);
		f->text_region.value.end = tok.reg.end;
		return { f, NULL };
	}
	if (tok.str == "{") {
		grd_add(&p->scope, f);
		grd_defer { grd_pop(&p->scope); };

		auto [block, e] = grdc_parse_block(p);
		if (e) {
			return { NULL, e };
		}
		f->block = (GrdcAstBlock*) block;
		if (f->block->text_region.has_value) {
			f->text_region.value.end = f->block->text_region.value.end;
		}
		auto e2 = grdc_ensure_function_has_return(p, f);
		if (e2) {
			return { NULL, e2 };
		}
		return { f, NULL };
	}

	return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ; or { after function header"_b) };
}

GrdTuple<GrdcAstStructType*, GrdError*> grdc_parse_struct(GrdcParser* p, GrdcToken start_tok) {
	auto [ident, e] = grdc_parse_ident(p);
	if (e) {
		return { NULL, e };
	}
	auto tok = grdc_peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected {"_b) };
	}
	grdc_next(p);

	GrdArray<GrdcAstStructMember*> members = { .allocator = p->allocator };
	grd_defer { members.free(); };

	GrdcProgramTextRegion st_reg;
	st_reg.start = start_tok.reg.start;
	st_reg.end = start_tok.reg.end;

	GrdcAstStructMember* pos_member = NULL;

	while (true) {
		auto tok = grdc_peek(p);
		if (tok.str == U"}"_b) {
			st_reg.end = tok.reg.end;
			grdc_next(p);
			break;
		}
		auto [pre_type, e] = grdc_parse_pre_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [name, e2] = grdc_parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		tok = grdc_peek(p);
		GrdcAstExpr* init_expr = NULL;
		if (tok.str == "=") {
			grdc_next(p);
			auto [expr, e] = grdc_parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			init_expr = expr;
		}
		auto [attrs, e3] = grdc_parse_attrs(p);
		if (e3) {
			return { NULL, e3 };
		}
		auto [ts, e4] = grdc_finalize_type(p, pre_type, attrs);
		if (e4) {
			return { NULL, e4 };
		}
		if (grdc_peek(p).str != ";") {
			return { NULL, grd_simple_parser_error(p, grd_current_loc(), grdc_peek(p).reg, U"Expected ;"_b) };
		}
		grdc_next(p);

		GrdcProgramTextRegion reg;
		reg.start = pre_type.reg.start;
		reg.end = name.reg.end;
		if (init_expr && init_expr->text_region.has_value) {
			reg.end = init_expr->text_region.value.end;
		}
		if (grd_len(attrs) > 0) {
			auto last = attrs[-1];
			if (last->text_region.has_value) {
				reg.end = last->text_region.value.end;
			}
		}
		auto member = grdc_make_ast_node<GrdcAstStructMember>(p, reg);
		member->member_ts = ts;
		member->name = name.str;
		member->attrs = attrs;
		grd_add(&members, member);

		for (auto attr: attrs) {
			if (auto x = grd_reflect_cast<GrdcPositionAttr>(attr)) {
				if (pos_member) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), attr->text_region, "Position attribute can only be specified once."_b) };
				}
				pos_member = member;
				if (member->member_ts->tp != p->float4_tp) {
					return { NULL, grd_simple_parser_error(p, grd_current_loc(), attr->text_region, "Position attribute can only be specified for float4 member."_b) };
				}
				x->is_used = true;
			}
		}
		auto e5 = grdc_check_if_attrs_used(p, attrs);
		if (e5) {
			return { NULL, e5 };
		}
	}

	auto st = grdc_make_ast_node<GrdcAstStructType>(p, st_reg);
	st->members = members;
	st->name = ident.str;
	members = {};
	return { st, NULL };
}

GrdError* grdc_parse_top_level(GrdcParser* p) {
	auto [attrs, e1] = grdc_parse_attrs(p);
	if (e1) {
		return e1;
	}

	auto start_tok = grdc_peek(p);

	if (start_tok.str == "struct") {
		grdc_next(p);
		auto [type, e] = grdc_parse_struct(p, start_tok);
		if (e) {
			return e;
		}
		e = grdc_add_global(p, type);
		if (e) {
			return e;
		}
		return NULL;
	}
	
	auto type_tok = grdc_peek(p);
	auto [pre_type, e0] = grdc_parse_pre_type(p);
	if (e0) {
		return e0;
	}
	auto [ident, e] = grdc_parse_ident(p);
	if (e) {
		return e;
	}
	auto tok = grdc_peek(p);
	if (tok.str == U"("_b) {
		auto [f, e] = grdc_parse_function(p, pre_type, attrs, start_tok, ident);
		if (e) {
			return e;
		}
		e = grdc_add_global(p, f);
		if (e) {
			return e;
		}
		auto e2 = grdc_check_if_attrs_used(p, attrs);
		if (e2) {
			return e2;
		}
		return NULL;
	} else if (tok.str == U","_b || tok.str == U";"_b || tok.str == U"="_b) {
		auto [group, e] = grdc_parse_var_decl(p, pre_type, ident, true);
		if (e) {
			return e;
		}
		e = grdc_add_global(p, group);
		if (e) {
			return e;
		}
		auto e2 = grdc_check_if_attrs_used(p, attrs);
		if (e2) {
			return e2;
		}
		return NULL;
	} else {
		return grd_simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ( or , or ; or ="_b);
	}
}

template <typename T>
GrdcAstPrimitiveType* grdc_push_prim_type(GrdcParser* p, GrdUnicodeString name, u64 size, u64 alignment, bool is_signed) {
	auto tp = grdc_make_ast_symbol<GrdcAstPrimitiveType>(p, name, {});
	tp->size = size;
	tp->alignment = alignment;
	tp->is_signed = is_signed;
	tp->c_tp = grd_reflect_type_of<T>()->template as<GrdPrimitiveType>();
	assert(tp->c_tp);
	auto e = grdc_add_global(p, tp);
	if (e) {
		panic(e);
	}
	return tp;
}

u64 grdc_calc_struct_size(GrdcAstStructType* tp) {
	u64 end = 0;
	u64 max_member_align = 1;
	for (auto it: tp->members) {
		u64 m_end = it->member_ts->tp->size + it->offset;
		if (m_end > end) {
			end = m_end;
		}
		if (it->member_ts->tp->alignment > max_member_align) {
			max_member_align = it->member_ts->tp->alignment;
		}
	}
	u64 size = grd_align(end, max_member_align);
	return size;
}

void grdc_push_member(GrdcAstStructType* tp, GrdUnicodeString name, GrdcAstTypeSite* ts) {
	auto m = grdc_make_ast_node<GrdcAstStructMember>(tp->p, {});
	m->struct_type = tp;
	m->member_ts = ts;
	m->name = name;
	auto sz = grdc_calc_struct_size(tp);
	m->offset = grd_align(sz, ts->tp->alignment);
	grd_add(&tp->members, m);
}

GrdcAstTypeSite* grdc_make_siteless_type_site(GrdcParser* p, GrdcAstType* tp) {
	auto ts = grdc_make_ast_node<GrdcAstTypeSite>(p, {});
	ts->tp = tp;
	return ts;
}

void grdc_push_base_types(GrdcParser* p) {
	p->void_tp = grdc_push_prim_type<void>(p, U"void"_b, 0, 1, false);
	p->bool_tp = grdc_push_prim_type<bool>(p, U"bool"_b, 1, 1, false);
	p->s8_tp  = grdc_push_prim_type<s8>(p, U"s8"_b, 1, 1, true);
	p->s16_tp = grdc_push_prim_type<s16>(p, U"s16"_b, 2, 2, true);
	p->s32_tp = grdc_push_prim_type<s32>(p, U"s32"_b, 4, 4, true);
	p->s64_tp = grdc_push_prim_type<s64>(p, U"s64"_b, 8, 8, true);
	p->u8_tp  = grdc_push_prim_type<u8>(p, U"u8"_b, 1, 1, false);
	p->u16_tp = grdc_push_prim_type<u16>(p, U"u16"_b, 2, 2, false);
	p->u32_tp = grdc_push_prim_type<u32>(p, U"u32"_b, 4, 4, false);
	p->u64_tp = grdc_push_prim_type<u64>(p, U"u64"_b, 8, 8, false);
	p->f32_tp = grdc_push_prim_type<f32>(p, U"f32"_b, 4, 4, true);
	p->f64_tp = grdc_push_prim_type<f64>(p, U"f64"_b, 8, 8, true);

	auto f32_ts = grdc_make_siteless_type_site(p, p->f32_tp);

	p->float2_tp = grdc_make_ast_symbol<GrdcAstStructType>(p, U"float2"_b, {});
	grdc_push_member(p->float2_tp, U"x"_b, f32_ts);
	grdc_push_member(p->float2_tp, U"y"_b, f32_ts);
	p->float2_tp->alignment = 8;
	p->float2_tp->size = 8;
	auto e = grdc_add_global(p, p->float2_tp);
	if (e) {
		grd_panic(e);
	}

	p->float3_tp = grdc_make_ast_symbol<GrdcAstStructType>(p, U"float3"_b, {});
	grdc_push_member(p->float3_tp, U"x"_b, f32_ts);
	grdc_push_member(p->float3_tp, U"y"_b, f32_ts);
	grdc_push_member(p->float3_tp, U"z"_b, f32_ts);
	p->float3_tp->alignment = 16;
	p->float3_tp->size = 16;
	e = grdc_add_global(p, p->float3_tp);
	if (e) {
		grd_panic(e);
	}

	p->float4_tp = grdc_make_ast_symbol<GrdcAstStructType>(p, U"float4"_b, {});
	grdc_push_member(p->float4_tp, U"x"_b, f32_ts);
	grdc_push_member(p->float4_tp, U"y"_b, f32_ts);
	grdc_push_member(p->float4_tp, U"z"_b, f32_ts);
	grdc_push_member(p->float4_tp, U"w"_b, f32_ts);
	p->float4_tp->alignment = 16;
	p->float4_tp->size = 16;
	e = grdc_add_global(p, p->float4_tp);
	if (e) {
		grd_panic(e);
	}
}


struct GrdcShaderIntrinFunc: GrdcAstFunction {

	GRD_REFLECT(GrdcShaderIntrinFunc) {
		GRD_BASE_TYPE(GrdcAstFunction);
	}
};

void grdc_add_shader_intrinsic_var(GrdcParser* p, GrdUnicodeString name, GrdcAstTypeSite* ts) {
	// @TODO: check for duplicates
	if (!ts) {
		grd_panic("No type for shader intrinsic var");
	}
	auto node = grdc_make_ast_symbol<GrdcShaderIntrinVar>(p, name, {});
	node->var_ts = ts;
	auto e = grdc_add_global(p, node);
	if (e) {
		grd_panic(e);
	}
}

void grdc_add_shader_intrinsic_func(GrdcParser* p, GrdUnicodeString name, GrdcAstTypeSite* ret_ts, std::initializer_list<GrdcAstTypeSite*> args) {
	// @TODO: check for duplicates
	auto node = grdc_make_ast_symbol<GrdcShaderIntrinFunc>(p, name, {});
	node->args.allocator = p->allocator;
	node->return_ts = ret_ts;
	s64 i = 0;
	for (auto arg: args) {
		auto name = grd_sprint_unicode(p->allocator, U"arg_%"_b, i);
		auto arg_node = grdc_make_ast_symbol<GrdcAstFunctionArg>(p, name, {});
		arg_node->var_ts = arg;
		grd_add(&node->args, arg_node);
		i += 1;
	}
	auto e = grdc_add_global(p, node);
	if (e) {
		grd_panic(e);
	}
}

GrdTuple<GrdcProgram*, GrdError*> grdc_parse(GrdUnicodeString str) {
	auto allocator = grd_make_arena_allocator();
	auto p = grd_make<GrdcParser>(allocator);
	p->allocator = allocator;
	p->program = grdc_make_ast_node<GrdcProgram>(p, {});
	p->program->globals.allocator = p->allocator;
	p->str = str;
	grd_add(&p->scope, p->program);
	for (auto it: GRDC_AST_BINARY_OPERATORS_UNSORTED) {
		grd_add(&p->op_tokens_sorted, it.op);
	}
	for (auto it: GRDC_AST_PREFIX_UNARY_OPERATORS_UNSORTED) {
		grd_add(&p->op_tokens_sorted, it.op);
	}
	for (auto it: GRDC_AST_POSTFIX_UNARY_OPERATORS_UNSORTED) {
		grd_add(&p->op_tokens_sorted, it.op);
	}
	grd_add(&p->op_tokens_sorted, U"[["_b);
	grd_add(&p->op_tokens_sorted, U"]]"_b);
	grd_sort(p->op_tokens_sorted, grd_lambda(grd_len($._0[$._1]) > grd_len($._0[$._2]))); 

	grdc_push_base_types(p);

	auto f32_ts = grdc_make_siteless_type_site(p, p->f32_tp);
	auto float3_ts = grdc_make_siteless_type_site(p, p->float3_tp);

	grdc_add_shader_intrinsic_func(p, U"hsv"_b, float3_ts, { f32_ts, f32_ts, f32_ts });
	grdc_add_shader_intrinsic_func(p, U"log"_b, f32_ts, { f32_ts });
	grdc_add_shader_intrinsic_func(p, U"sin"_b, f32_ts, { f32_ts });
	grdc_add_shader_intrinsic_func(p, U"cos"_b, f32_ts, { f32_ts });
	grdc_add_shader_intrinsic_func(p, U"dot_vec3"_b, f32_ts, { float3_ts, float3_ts });
	grdc_add_shader_intrinsic_func(p, U"atan"_b, f32_ts, { f32_ts });
	grdc_add_shader_intrinsic_func(p, U"length"_b, f32_ts, { float3_ts });
	grdc_add_shader_intrinsic_func(p, U"abs"_b, f32_ts, { f32_ts });

	while (grdc_peek(p).str != ""_b) {
		if (grdc_peek(p).str == U";"_b) {
			grdc_next(p);
			continue;
		}
		auto e = grdc_parse_top_level(p);
		if (e) {
			return { NULL, e };
		}
	}
	return { p->program, NULL };
}
