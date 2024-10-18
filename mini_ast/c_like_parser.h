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

	GRD_REFLECT(AstOperator) {
		GRD_MEMBER(op);
		GRD_MEMBER(prec);
		GRD_MEMBER(flags);
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

	GRD_REFLECT(ProgramTextRegion) {
		GRD_MEMBER(start);
		GRD_MEMBER(end);
	}
};

struct Token {
	UnicodeString     str;
	ProgramTextRegion reg;
	u32               flags = 0;
};

struct CLikeProgram;
struct AstNode;
struct AstGrdPrimitiveType;
struct AstStructType;
struct AstType;

struct CLikeParser {
	GrdAllocator            allocator;
	CLikeProgram*        program;
	Array<AstNode*>      scope;
	UnicodeString        str;
	s64                  cursor = 0;
	Token                current_token;
	Array<UnicodeString> op_tokens_sorted;
	HashMap<AstType*, AstType*> ptr_types;

	AstGrdPrimitiveType*    void_tp = NULL;
	AstGrdPrimitiveType*    bool_tp = NULL;
	AstGrdPrimitiveType*    s8_tp = NULL;
	AstGrdPrimitiveType*    u8_tp = NULL;
	AstGrdPrimitiveType*    s16_tp = NULL;
	AstGrdPrimitiveType*    u16_tp = NULL;
	AstGrdPrimitiveType*    s32_tp = NULL;
	AstGrdPrimitiveType*    u32_tp = NULL;
	AstGrdPrimitiveType*    s64_tp = NULL;
	AstGrdPrimitiveType*    u64_tp = NULL;
	AstGrdPrimitiveType*    f32_tp = NULL;
	AstGrdPrimitiveType*    f64_tp = NULL;

	AstStructType*       float2_tp = NULL;
	AstStructType*       float3_tp = NULL;
	AstStructType*       float4_tp = NULL;
};

struct AstNode {
	Type*                       type;
	CLikeParser*                p = NULL;
	Optional<ProgramTextRegion> text_region;

	GRD_REFLECT(AstNode) {
		GRD_MEMBER(p);
		GRD_MEMBER(type);
			GRD_TAG(GrdRealTypeMember{});
		GRD_MEMBER(text_region);
	}
};

template <typename T>
T* grd_make_ast_node(CLikeParser* p, Optional<ProgramTextRegion> text_region) {
	auto x = grd_make<T>(p->allocator);
	x->type = grd_reflect_type_of<T>();
	x->p = p;
	x->text_region = text_region;
	return x;
}

struct AstSymbol: AstNode {
	UnicodeString name;
	bool          is_global = false;

	GRD_REFLECT(AstSymbol) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(name);
		GRD_MEMBER(is_global);
	}
};

struct AstType: AstSymbol {
	u64 size = 0;
	u64 alignment = 0;

	GRD_REFLECT(AstType) {
		GRD_BASE_TYPE(AstSymbol);
		GRD_MEMBER(size);
		GRD_MEMBER(alignment);
	}
};

enum class MetalAddrSpace {
	Unspecified = 0,
	Device = 1,
	Constant = 2,
	Thread = 2,
	Threadgroup = 3,
	// ThreadgroupImageblock = 4,
	// RayData = 5,
	// ObjectData = 6,
};
GRD_REFLECT(MetalAddrSpace) {
	GRD_ENUM_VALUE(Unspecified);
	GRD_ENUM_VALUE(Device);
	GRD_ENUM_VALUE(Constant);
	GRD_ENUM_VALUE(Thread);
	GRD_ENUM_VALUE(Threadgroup);
	// GRD_ENUM_VALUE(ThreadgroupImageblock);
	// GRD_ENUM_VALUE(RayData);
	// GRD_ENUM_VALUE(ObjectData);
}

enum class VulkanStorageClass {
	Unspecified = 0,
	Private = 1,
	Uniform = 2,
	Input = 3,
	PushConstant = 4,
};
GRD_REFLECT(VulkanStorageClass) {
	GRD_ENUM_VALUE(Unspecified);
	GRD_ENUM_VALUE(Private);
	GRD_ENUM_VALUE(Uniform);
	GRD_ENUM_VALUE(Input);
	GRD_ENUM_VALUE(PushConstant);
}

struct AstExpr: AstNode {
	AstType* expr_type = NULL;
	bool     is_lvalue = false;

	GRD_REFLECT(AstExpr) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(expr_type);
		GRD_MEMBER(is_lvalue);
	}
};

constexpr bool AST_EXPR_LVALUE = true;
constexpr bool AST_EXPR_RVALUE = false;

template <typename T>
T* grd_make_ast_expr(CLikeParser* p, AstType* expr_type, bool is_lvalue, Optional<ProgramTextRegion> text_region) {
	auto x = grd_make_ast_node<T>(p, text_region);
	if (!expr_type) {
		panic("expr_type must be non-null");
	}
	x->expr_type = expr_type;
	x->is_lvalue = is_lvalue;
	return x;
}

struct AstBlock: AstNode {
	Array<AstNode*> statements;

	GRD_REFLECT(AstBlock) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(statements);
	}
};


struct CLikeProgram: AstNode {
	Array<AstNode*>   globals; // Main diff between globals and global_syms is var decl groups vs var decls.
	Array<AstSymbol*> global_syms;

	GRD_REFLECT(CLikeProgram) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(globals);
		GRD_MEMBER(global_syms);
	}
};

struct AstGrdPrimitiveType: AstType {
	GrdPrimitiveType* c_tp = NULL;
	bool           is_signed = false;

	GRD_REFLECT(AstGrdPrimitiveType) {
		GRD_BASE_TYPE(AstType);
		GRD_MEMBER(c_tp);
		GRD_MEMBER(is_signed);
	}
};


struct AstTypeSite;

struct AstVar: AstSymbol {
	AstTypeSite* var_ts = NULL;
	
	GRD_REFLECT(AstVar) {
		GRD_BASE_TYPE(AstSymbol);
		GRD_MEMBER(var_ts);
	}
};

struct ShaderIntrinVar: AstVar {

	GRD_REFLECT(ShaderIntrinVar) {
		GRD_BASE_TYPE(AstVar);
	}
};

struct AstAttr;

struct AstFunctionArg: AstVar {
	Array<AstAttr*> attrs;

	GRD_REFLECT(AstFunctionArg) {
		GRD_BASE_TYPE(AstVar);
		GRD_MEMBER(attrs);
	}
};

enum class AstFunctionKind {
	Plain = 1,
	Vertex = 2,
	Fragment = 3,
};
GRD_REFLECT(AstFunctionKind) {
	GRD_ENUM_VALUE(Plain);
	GRD_ENUM_VALUE(Vertex);
	GRD_ENUM_VALUE(Fragment);
}

struct MtlBufferIdxAttr;
struct VkSetBindingAttr;

struct AstFunction: AstSymbol {
	AstTypeSite*           return_ts = NULL; 
	Array<AstFunctionArg*> args;
	Array<AstAttr*>        attrs;
	AstFunctionKind        kind = AstFunctionKind::Plain;
	AstBlock*              block = NULL;
	AstFunctionArg*        stage_in_arg = NULL;
	Array<MtlBufferIdxAttr*> mtl_buffers;
	Array<VkSetBindingAttr*> vk_set_bindings;

	GRD_REFLECT(AstFunction) {
		GRD_BASE_TYPE(AstSymbol);
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
T* grd_make_ast_symbol(CLikeParser* p, UnicodeString name, Optional<ProgramTextRegion> text_region) {
	auto node = grd_make_ast_node<T>(p, text_region);
	node->name = name;
	return node;
}

struct AstUnaryExpr: AstExpr {
	AstExpr*      expr;
	AstOperator*  op;

	GRD_REFLECT(AstUnaryExpr) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(expr);
		GRD_MEMBER(op);
	}
};

struct AstDerefExpr: AstExpr {
	AstExpr* lhs;

	GRD_REFLECT(AstDerefExpr) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(lhs);
	}
};

struct AstBinaryExpr: AstExpr {
	AstExpr*      lhs = NULL;
	AstExpr*      rhs = NULL;
	AstOperator*  op = NULL;
	AstOperator*  pure_op = NULL;

	GRD_REFLECT(AstBinaryExpr) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(rhs);
		GRD_MEMBER(op);
		GRD_MEMBER(pure_op);
	}
};

struct AstVarDecl: AstVar { 
	AstExpr*      init = NULL;

	GRD_REFLECT(AstVarDecl) {
		GRD_BASE_TYPE(AstVar);
		GRD_MEMBER(init);
	}
};


struct AstVarDeclGroup: AstNode {
	Array<AstVarDecl*> var_decls;

	GRD_REFLECT(AstVarDeclGroup) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(var_decls);
	}
};

struct AstStructType;
struct AstAttr;

struct AstStructMember: AstNode {
	AstStructType*  struct_type = NULL;
	AstTypeSite*    member_ts = NULL;
	UnicodeString   name;
	s64             offset = 0;
	Array<AstAttr*> attrs;

	GRD_REFLECT(AstStructMember) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(struct_type);
		GRD_MEMBER(member_ts);
		GRD_MEMBER(name);
		GRD_MEMBER(offset);
		GRD_MEMBER(attrs);
	}
};

struct AstStructType: AstType {
	Array<AstStructMember*> members;

	GRD_REFLECT(AstStructType) {
		GRD_BASE_TYPE(AstType);
		GRD_MEMBER(members);
	}
};

struct AstVarMemberAccess: AstExpr {
	AstExpr*         lhs;
	AstStructMember* member;

	GRD_REFLECT(AstVarMemberAccess) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(member);
	}
};

struct AstArrayAccess: AstExpr {
	AstExpr*      lhs;
	AstExpr*      index;

	GRD_REFLECT(AstArrayAccess) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(index);
	}
};

struct AstLiteralExpr: AstExpr {
	GrdPrimitiveType* lit_type = NULL;
	GrdPrimitiveValue lit_value = {};
};

struct AstAttr: AstNode {
	UnicodeString   name;
	Array<AstExpr*> args;
	bool            is_used = false;

	GRD_REFLECT(AstAttr) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(name);
		GRD_MEMBER(args);
		GRD_MEMBER(is_used);
	}
};

Token set_current_token(CLikeParser* p, s64 end, u32 flags = 0) {
	grd_defer { p->cursor = end; };
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

		for (auto c: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>', '[', ']', '&'}) {
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

struct PrepComment {
	s64 start = 0;
	s64 end   = 0;

	GRD_REFLECT(PrepComment) {
		GRD_MEMBER(start);
		GRD_MEMBER(end);
	}
};

struct Preprocessor;

struct PrepNode {
	Type*         type = NULL;
	Preprocessor* pp = NULL;
	PrepNode*     parent = NULL;

	GRD_REFLECT(PrepNode) {
		GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
		GRD_MEMBER(pp);
		GRD_MEMBER(parent);
	}
};

template <typename T>
T* grd_make_prep_node(Preprocessor* pp) {
	auto node = grd_make<T>(pp->allocator);
	node->type = grd_reflect_type_of<T>();
	node->pp = pp;
	return node;
}

struct PrepFile {
	UnicodeString path;
	UnicodeString str;
};

struct PrepFileNode: PrepNode {
	PrepFile* file = NULL;
	s64       file_offset = 0;

	GRD_REFLECT(PrepFileNode) {
		GRD_BASE_TYPE(PrepNode);
		GRD_MEMBER(file);
		GRD_MEMBER(file_offset);
	}
};

struct PrepNodeReg {
	PrepNode* node = NULL;
	s64       start = 0;
	s64       end = -1;
};

struct Preprocessor {
	GrdAllocator              allocator;
	AllocatedUnicodeString pr;
	Array<PrepNode*>       scope;
	Array<PrepFile*>       files;
	Array<PrepNodeReg*>    regs;
};

void push_reg(Preprocessor* pp) {
	if (len(pp->scope) == 0) {
		return;
	}
	auto reg = grd_make<PrepNodeReg>(pp->allocator);
	reg->node = pp->scope[-1];
	reg->start = len(pp->pr);
	while (len(pp->regs) > 0) {
		if (pp->regs[-1]->start < reg->start) {
			break;
		}
		remove_at_index(&pp->regs, -1);
	}
	if (len(pp->regs) > 0) {
		pp->regs[-1]->end = reg->start;
	}
	add(&pp->regs, reg);
}

void push_scope(Preprocessor* pp, PrepNode* node) {
	if (len(pp->scope) > 0) {
		assert(node->parent == NULL);
		node->parent = pp->scope[-1];
	}
	add(&pp->scope, node);
	push_reg(pp);
}

void pop_scope(Preprocessor* pp) {
	pop(&pp->scope);
	push_reg(pp);
}

Error* prep_file(Preprocessor* pp, UnicodeString str, UnicodeString path) {
	auto file = grd_make<PrepFile>(pp->allocator);
	file->path = path;
	file->str = str;
	add(&pp->files, file);

	auto node = grd_make_prep_node<PrepFileNode>(pp);
	node->file = file;
	node->file_offset = 0;
	push_scope(pp, node);

	s64 cursor = 0;
	while (cursor < len(str)) {
		UnicodeString remaining = str[cursor, {}];
		if (starts_with(remaining, "//")) {
			s64 start = cursor;
			s64 end = len(str);
			for (auto i: range_from_to(cursor, len(str))) {
				if (is_line_break(str[i])) {
					end = i;
					break;
				}
			}
			cursor = end;
			pop_scope(pp);
			auto comment_node = grd_make_prep_node<PrepFileNode>(pp);
			comment_node->file = file;
			comment_node->file_offset = start;
			push_scope(pp, comment_node);
			add(&pp->pr, ' ');
			pop_scope(pp);
			auto new_node = grd_make_prep_node<PrepFileNode>(pp);
			new_node->file = file;
			new_node->file_offset = end;
			push_scope(pp, new_node);
			continue;
		}
		if (starts_with(remaining, "/*")) {
			s64 start = cursor;
			cursor += 2;
			s64 level = 1;
			while (cursor < len(str)) {
				if (starts_with(str[cursor, {}], "/*")) {
					cursor += 2;
					level += 1;
				} else if (starts_with(str[cursor, {}], "*/")) {
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
		// if (starts_with(remaining, "#")) {
			
		// }
		add(&pp->pr, str[cursor]);
		cursor += 1;
	}
	return NULL;
}

Preprocessor* grd_make_preprocessor(GrdAllocator allocator) {
	auto pp = grd_make<Preprocessor>(allocator);
	pp->allocator = allocator;
	pp->pr.allocator = allocator;
	pp->files.allocator = allocator;
	pp->scope.allocator = allocator;
	pp->regs.allocator = allocator;
	return pp;
}

GrdGenerator<AstSymbol*> resolve_symbols(AstNode* node) {
	if (auto sym = grd_reflect_cast<AstSymbol>(node)) {
		co_yield sym;
		co_return;
	}
	if (auto var_decl_group = grd_reflect_cast<AstVarDeclGroup>(node)) {
		for (auto it: var_decl_group->var_decls) {
			auto gen = resolve_symbols(it);
			for (auto sym: gen) {
				co_yield sym;
			}
		}
	}
}

AstNode* lookup_symbol(CLikeParser* p, UnicodeString name) {
	for (auto i: reverse(grd_range(len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto program = grd_reflect_cast<CLikeProgram>(scope)) {
			for (auto child: program->globals) {
				for (auto sym: resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto block = grd_reflect_cast<AstBlock>(scope)) {
			for (auto child: block->statements) {
				for (auto sym: resolve_symbols(child)) {
					if (sym->name == name) {
						return sym;
					}
				}
			}
		}
		if (auto f = grd_reflect_cast<AstFunction>(scope)) {
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
			auto e = grd_make_parser_error(p, grd_current_loc(), "Duplicate global variable '%'"_b, sym->name);
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
		if (auto tp = grd_reflect_cast<AstType>(symbol)) {
			return tp;
		}
	}
	return NULL;
}

struct AstGrdPointerType: AstType {
	AstType*           pointee = NULL;
	VulkanStorageClass ptr_vk = VulkanStorageClass::Unspecified;
	MetalAddrSpace     ptr_mtl = MetalAddrSpace::Unspecified;

	GRD_REFLECT(AstGrdPointerType) {
		GRD_BASE_TYPE(AstType);
		GRD_MEMBER(pointee);
		GRD_MEMBER(ptr_mtl);
		GRD_MEMBER(ptr_vk);
	}
};

struct AstTypeSite: AstNode {
	AstType* tp = NULL;

	GRD_REFLECT(AstTypeSite) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(tp);
	}
};

struct AstGrdPointerTypeSite: AstTypeSite {
	AstGrdPointerTypeSite* pointee = NULL;

	GRD_REFLECT(AstGrdPointerTypeSite) {
		GRD_BASE_TYPE(AstTypeSite);
		GRD_MEMBER(pointee);
	}
};

// AstType* get_pointer_type(CLikeParser* p, AstType* type) {
// 	auto found = get(&p->ptr_types, type);
// 	if (found) {
// 		return *found;
// 	}
// 	auto name = sprint_unicode(p->allocator, U"%(*)*"_b, type->name);
// 	auto ptr = grd_make_ast_symbol<AstGrdPointerType>(p, name, {});
// 	ptr->pointee = type;
// 	put(&p->ptr_types, type, ptr);
// 	return ptr;
// }

struct PreType {
	AstType*          t = NULL;
	s64               pointer_indir_level = 0;
	ProgramTextRegion reg;
};

Tuple<PreType, Error*> parse_pre_type(CLikeParser* p) {
	auto tok = peek(p);
	auto c_str = encode_utf8(tok.str);
	grd_defer { c_str.free(); };
	AstType* type = find_type(p, tok.str);
	if (!type) {
		return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not find type."_b) };
	}
	PreType pt = { .t = type };
	pt.reg = { tok.reg.start, tok.reg.end };
	next(p);
	while (true) {
		tok = peek(p);
		if (tok.str == "*") {
			pt.pointer_indir_level += 1;
			pt.reg.end = tok.reg.end;
			next(p);
		} else {
			break;
		}
	}
	return { pt, NULL };
}

AstGrdPointerType* get_pointer_type(CLikeParser* p, AstType* tp, VulkanStorageClass st_class, MetalAddrSpace mtl_space) {
	auto ptr = grd_make_ast_node<AstGrdPointerType>(p, tp->text_region);
	ptr->name = sprint_unicode(p->allocator, U"%(*)*"_b, tp->name);
	ptr->pointee = tp;
	ptr->size = 8;
	ptr->alignment = 8;
	ptr->ptr_vk = st_class;
	ptr->ptr_mtl = mtl_space;
	return ptr;
}

Tuple<AstTypeSite*, Error*> finalize_type(CLikeParser* p, PreType pt, Span<AstAttr*> attrs) {
	if (pt.pointer_indir_level > 0) {
		auto st_class = VulkanStorageClass::Unspecified;
		for (auto attr: attrs) {
			if (attr->name == "vk_uniform") {
				if (st_class != VulkanStorageClass::Unspecified) {
					auto e = grd_make_parser_error(p, grd_current_loc(), "Vulkan storage class must be specified for pointer type only once"_b);
					add_site(p, e,
						CParserErrorToken{
							.reg = pt.reg,
							.color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED
						}
					);
					return { {}, e };
				}
				st_class = VulkanStorageClass::Uniform;
			}
		}
		if (st_class == VulkanStorageClass::Unspecified) {
			return { {}, simple_parser_error(p, grd_current_loc(), pt.reg, U"Vulkan storage class must be specified for pointer type."_b) };
		}
		// @TODO: implement MetalAddrSpace
		auto tp = get_pointer_type(p, pt.t, st_class, MetalAddrSpace::Unspecified);
		auto qt = grd_make_ast_node<AstGrdPointerTypeSite>(p, pt.reg);
		qt->tp = tp;
		for (auto i: grd_range(pt.pointer_indir_level - 1)) {
			auto sub = grd_make_ast_node<AstGrdPointerTypeSite>(p, pt.reg);
			auto sub_tp = get_pointer_type(p, qt->tp, st_class, MetalAddrSpace::Unspecified);
			sub->tp = sub_tp;
			qt = sub;
		}
		return { qt, NULL };
	} else {
		auto qt = grd_make_ast_node<AstGrdPointerTypeSite>(p, pt.reg);
		qt->tp = pt.t;
		return { qt, NULL };
	}
}

Tuple<Token, Error*> parse_ident(CLikeParser* p) {
	auto tok = peek(p);
	if (len(tok.str) == 0) {
		return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected an identifier."_b) };
	}
	if ((tok.str[0] < 'a' || tok.str[0] > 'z') && (tok.str[0] < 'A' || tok.str[0] > 'Z')) {
		return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"First letter of an identifier must be a letter."_b) };
	}
	for (auto c: tok.str) {
		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_') {
			return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"Only letters, numbers, and underscores are allowed in identifiers."_b) };
		}
	}
	next(p);
	return { tok, NULL };
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p);
Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec);
Tuple<Array<AstAttr*>, Error*> parse_attrs(CLikeParser* p);

Error* check_if_attrs_used(CLikeParser* p, Array<AstAttr*> attrs, GrdCodeLoc loc = grd_caller_loc()) {
	for (auto attr: attrs) {
		if (!attr->is_used) {
			auto e = simple_parser_error(p, loc, attr->text_region, "Attribute '%' is not used."_b, attr->name);
			return e;
		}
	}
	return NULL;
}

Tuple<AstVarDeclGroup*, Error*> parse_var_decl(CLikeParser* p, PreType pt, Token ident_tok, bool is_global) {
	Array<AstVarDecl*> var_decls = { .allocator = p->allocator };
	grd_defer { var_decls.free(); };

	auto val_decl_attrs = [&](auto* p, Array<AstAttr*> attrs) -> Error* {
		for (auto attr: attrs) {
			if (is_global) {
				return simple_parser_error(p, grd_current_loc(), attr->text_region, "Global variables cannot have attributes"_b);
			} else {
				return simple_parser_error(p, grd_current_loc(), attr->text_region, "Local variables cannot have attributes"_b);
			}
		}
		return NULL;
	};
	auto [attrs, e] = parse_attrs(p);
	if (e) {
		return { NULL, e };
	}
	auto [ts, e2] = finalize_type(p, pt, attrs);
	if (e2) {
		return { NULL, e2 };
	}
	auto e3 = check_if_attrs_used(p, attrs);
	if (e3) {
		return { NULL, e3 };
	}
	auto current_ident = ident_tok;
	while (true) {
		auto node = grd_make_ast_node<AstVarDecl>(p, {});
		node->var_ts = ts;
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
		if (expr->expr_type != ts->tp) {
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", ts->tp->name, expr->expr_type->name) };
		}
		for (auto it: var_decls) {
			it->init = (AstExpr*) expr;
		}
	}

	tok = peek(p);
	if (tok.str != U";"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected , or ;"_b) };
	}

	ProgramTextRegion text_region = { pt.reg.start, tok.reg.end };
	for (auto it: var_decls) {
		it->text_region = text_region;
	}

	auto node = grd_make_ast_node<AstVarDeclGroup>(p, text_region);
	node->var_decls = var_decls;
	var_decls = {};
	return { node, NULL };
}

struct AstFunctionCall: AstExpr {
	AstNode*        f;
	Array<AstExpr*> args;

	GRD_REFLECT(AstFunctionCall) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(args);
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
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ("_b) };
	}
	auto call = grd_make_ast_expr<AstFunctionCall>(p, func->return_ts->tp, AST_EXPR_RVALUE, {});
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

	GRD_REFLECT(AstTernary) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(cond);
		GRD_MEMBER(then);
		GRD_MEMBER(else_);
	}
};

struct AstIf: AstNode {
	AstExpr*  cond = NULL;
	AstBlock* then = NULL;
	AstBlock* else_block = NULL;
	AstIf*    else_if = NULL;

	GRD_REFLECT(AstIf) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(cond);
		GRD_MEMBER(then);
		GRD_MEMBER(else_block);
		GRD_MEMBER(else_if);
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
	if (auto ptr = grd_reflect_cast<AstGrdPointerType>(type)) {
		return true;
	}
	return false;
}

bool is_array(AstType* type) {
	// @TODO: implement.
	return false;
}

AstType* get_element_type(AstType* type) {
	if (auto ptr = grd_reflect_cast<AstGrdPointerType>(type)) {
		return ptr->pointee;
	}
	return NULL;
}

struct AstVariableAccess: AstExpr {
	AstVar* var = NULL;

	GRD_REFLECT(AstVariableAccess) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(var);
	}
};

bool is_struct(AstType* type) {
	type = resolve_type_alias(type);
	return grd_reflect_cast<AstStructType>(type) != NULL;
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

	GRD_REFLECT(AstSwizzleExpr) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(lhs);
		GRD_MEMBER(swizzle);
		GRD_MEMBER(swizzle_len);
	}
};

bool parse_swizzle_ident(CLikeParser* p, UnicodeString ident, s32* swizzle, s32 src_len) {
	s64 ident_len = len(ident);
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
	for (auto i: grd_range(4)) {
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
		auto expr = grd_make_ast_expr<AstVarMemberAccess>(p, swizzle_type, lhs->is_lvalue, text_region);
		expr->lhs = lhs;
		auto member = find_struct_member(p, struct_tp, ident.str);
		if (!member) {
			return { NULL, simple_parser_error(p, grd_current_loc(), ident.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
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

	auto expr = grd_make_ast_expr<AstSwizzleExpr>(p, swizzle_type, lhs->is_lvalue && is_unique, text_region);
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

	GRD_REFLECT(AstStructInitializer) {
		GRD_BASE_TYPE(AstExpr);
		GRD_MEMBER(struct_type);
		GRD_MEMBER(members);
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
		auto node = grd_make_ast_expr<AstBinaryExpr>(p, rhs->expr_type, AST_EXPR_RVALUE, text_region);
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
			return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Left side of '%' must be an lvalue.", op->op) };
		}
	}
	if (lhs->expr_type == find_type(p, U"void"_b)) {
		return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Cannot apply operator '%' to void", op->op) };
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
				return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' for vector requires rhs to be float or double or matching vector type, got '%'. lhs type = '%'", op->op, rhs->expr_type->name, lhs->expr_type->name) };
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
		if (pure_op->flags & AST_OP_FLAG_BOOL) {
			if (!is_bool(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to bool types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_INT) {
			if (!is_integer(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to integer types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_PRIMITIVE) {
			if (!is_primitive(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to primitive types, got '%'.", op->op, lhs->expr_type->name) };
			}
		}
		if (pure_op->flags & AST_OP_FLAG_NUMERIC) {
			if (!is_numeric(lhs->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op->op, lhs->expr_type->name) };
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
			expr_result_type = find_type(p, U"bool"_b);
			assert(expr_result_type);
		} else {
			expr_result_type = lhs->expr_type;
		}
	}
	if (!expr_result_type) {
		return { NULL, simple_parser_error(p, grd_current_loc(), op_tok.reg, U"Unexpected error: could not determine result type of binary expression '%'.", op->op) };
	}
	auto node = grd_make_ast_expr<AstBinaryExpr>(p, expr_result_type, AST_EXPR_RVALUE, text_region);
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

		GrdPrimitiveValue lit_value;
		GrdPrimitiveType* lit_type = NULL;
		f64 float_val;
		f64 double_val;

		bool ok = false;
		if (parse_as_float) {
			ok = parse_float(lit, &lit_value.f32_value);
			lit_type = grd_reflect_type_of<f32>()->as<GrdPrimitiveType>();
		} else {
			ok = parse_float(lit, &lit_value.f64_value);
			lit_type = grd_reflect_type_of<f64>()->as<GrdPrimitiveType>();
		}
		if (!ok) {
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not parse floating point literal."_b) };
		}
		auto literal = grd_make_ast_expr<AstLiteralExpr>(p, find_type(p, parse_as_float ? U"float"_b : U"double"_b), AST_EXPR_RVALUE, tok.reg);
		literal->lit_type = lit_type;
		literal->lit_value = lit_value;
		next(p);
		return { literal, NULL };
	}

	if (tok.flags & CTOKEN_FLAG_INTEGER) {
		s64 u;
		bool ok = parse_integer(tok.str, &u);
		if (!ok) {
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Could not parse integer literal."_b) };
		}
		auto literal = grd_make_ast_expr<AstLiteralExpr>(p, find_type(p, U"int"_b), AST_EXPR_RVALUE, tok.reg);
		literal->lit_value.s64_value = u;
		literal->lit_type = grd_reflect_type_of(u)->as<GrdPrimitiveType>();
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
			auto ptr_tp = grd_reflect_cast<AstGrdPointerType>(expr->expr_type);
			if (ptr_tp == NULL) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '*' can only be applied to pointer types, got '%'"_b, expr->expr_type->name) };
			}
			auto deref = grd_make_ast_expr<AstDerefExpr>(p, ptr_tp->pointee, AST_EXPR_LVALUE, tok.reg);
			deref->lhs = expr;
			return { deref, NULL };
		}
		if (tok.str == "&") {
			if (!expr->is_lvalue) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '&' can only be applied to lvalues.", op.str) };
			}
			if (grd_reflect_cast<AstSwizzleExpr>(expr)) {
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
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", op.str) };
			}
		}
		if (tok.str == "!" || tok.str == "~") {
			if (!is_integer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to integer types.", op.str) };
			}
		}
		if (!is_numeric(expr->expr_type)) {
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types, got '%'.", op.str, expr->expr_type->name) };
		}
		ProgramTextRegion text_region = { op.reg.start, peek(p).reg.start };
		auto unary = grd_make_ast_expr<AstUnaryExpr>(p, expr->expr_type, AST_EXPR_RVALUE, text_region);
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
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ')'.", tok) };
		}
		next(p);
		return { expr, NULL };
	}

	AstNode* sym = lookup_symbol(p, tok.str);
	if (!sym) {
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Unknown identifier.") };
	}
	AstExpr* lhs = NULL;
	if (auto type = grd_reflect_cast<AstType>(sym)) {
		auto initializer_reg_start = tok.reg.start;
		next(p);
		if (peek(p).str == "("_b) {
			next(p);
			auto struct_tp = grd_reflect_cast<AstStructType>(type);
			if (!struct_tp) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct type but got '%'"_b, type->name) };
			}
			s64 member_index = 0;
			Array<AstExpr*> args = { .allocator = p->allocator };
			grd_defer { args.free(); };
			while (true) {
				auto tok = peek(p);
				if (tok.str == ")"_b) {
					if (len(args) != len(struct_tp->members)) {
						return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected % arguments in initializer but got '%'.", len(struct_tp->members), len(args)) };
					}
					ProgramTextRegion text_region = { initializer_reg_start, tok.reg.end };
					auto node = grd_make_ast_expr<AstStructInitializer>(p, type, AST_EXPR_RVALUE, text_region);
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
					return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Exceeded arguments count in initializer, struct '%' has % members.", type->name, len(struct_tp->members)) };
				}
				auto member = struct_tp->members[member_index];
				if (member->member_ts->tp != expr->expr_type) {
					return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Initializer type mismatch, expected '%' but got '%'.", member->member_ts->tp->name, expr->expr_type->name) };
				}
				add(&args, expr);
				member_index += 1;
				if (peek(p).str == ","_b) {
					next(p);
				}
			}
		} else {
			return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct initializer, got '%'.", tok.str) };
		}
	} else if (auto var = grd_reflect_cast<AstVar>(sym)) {
		auto reg = tok.reg;
		next(p);
		auto expr = grd_make_ast_expr<AstVariableAccess>(p, var->var_ts->tp, AST_EXPR_LVALUE, reg);
		expr->var = var;
		lhs = expr;
	} else if (auto func = grd_reflect_cast<AstFunction>(sym)) {
		next(p);
		auto [call, e] = parse_function_call(p, tok, func);
		if (e) {
			return { NULL, e };
		}
		lhs = call;
	} else {
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected type or variable"_b) };
	}

	while (true) {
		auto tok = peek(p);

		auto postfix_op = find_postfix_unary_operator(p, tok.str);
		if (postfix_op) {
			if (tok.str == "++"_b || tok.str == "--"_b) {
				if (!is_numeric(lhs->expr_type)) {
					return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to numeric types. Got '%'.", tok.str, lhs->expr_type->name) };
				}
				if (!lhs->is_lvalue) {
					return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Operator '%' can only be applied to lvalues.", tok.str) };
				}
			}
			next(p);
			ProgramTextRegion text_region = { tok.reg.start, tok.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = grd_make_ast_expr<AstUnaryExpr>(p, lhs->expr_type, AST_EXPR_RVALUE, text_region);
			node->expr = lhs;
			node->op = postfix_op;
			lhs = node;
			continue;
		}
		if (tok.str == "("_b) {
			// @TODO: support calling on function pointers.
			auto func = grd_reflect_cast<AstFunction>(lhs);
			if (!func) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a function, got '%'"_b, lhs->type->name) };
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
			if (auto struct_tp = grd_reflect_cast<AstStructType>(lhs->expr_type)) {
				access_tp = struct_tp;
			} else if (auto ptr_tp = grd_reflect_cast<AstGrdPointerType>(lhs->expr_type)) {
				if (auto struct_tp = grd_reflect_cast<AstStructType>(ptr_tp->pointee)) {
					access_tp = struct_tp;
				} else {
					return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected struct type but got '%'"_b, ptr_tp->pointee->name) };
				}
			} else {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a struct, got '%'"_b, lhs->expr_type->name) };
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
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Member '%' not found in struct '%'.", ident.str, lhs->expr_type->name) };
			}
			ProgramTextRegion text_region = { ident.reg.start, ident.reg.end };
			if (lhs->text_region.has_value) {
				text_region.start = lhs->text_region.value.start;
			}
			auto node = grd_make_ast_expr<AstVarMemberAccess>(p, member->member_ts->tp, lhs->is_lvalue, text_region);
			node->lhs = lhs;
			node->member = member;
			lhs = node;
			continue;
		}
		if (tok.str == "["_b) {
			bool ok = is_pointer(lhs->expr_type) || is_array(lhs->expr_type);
			if (!ok) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected an array or pointer, got '%'"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			if (!is_integer(expr->expr_type)) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Array index must be an integer, got '%'"_b, expr->expr_type->name) };
			}
			if (peek(p).str != "]"_b) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected a ]"_b) };
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
			auto node = grd_make_ast_expr<AstArrayAccess>(p, element_type, AST_EXPR_LVALUE, text_region);
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
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected :"_b) };
			}
			next(p);
			auto [else_expr, e3] = parse_expr(p, (op->flags & AST_OP_FLAG_LEFT_ASSOC) ? op->prec + 1 : op->prec);
			if (e3) {
				return { NULL, e3 };
			}
			if (lhs->expr_type != else_expr->expr_type) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Ternary expression type mismatch, expected '%' but got '%'.", lhs->expr_type->name, else_expr->expr_type->name) };
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
			auto node = grd_make_ast_expr<AstTernary>(p, lhs->expr_type, AST_EXPR_RVALUE, text_region);
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

Tuple<s64, Error*> eval_const_int_inner(AstExpr* expr) {
	auto p = expr->p;
	if (auto i = grd_reflect_cast<AstLiteralExpr>(expr)) {
		if (i->lit_type == grd_reflect_type_of<s64>()) {
			return { i->lit_value.s64_value, NULL };
		} else if (i->lit_type == grd_reflect_type_of<s32>()) {
			return { i->lit_value.s32_value, NULL };
		} else {
			return { 0, simple_parser_error(p, grd_current_loc(), expr->text_region, "Expected s32 or s64 literal, got '%*'", i->lit_type->name) };
		}
	} else if (auto i = grd_reflect_cast<AstUnaryExpr>(expr)) {
		if (i->op->op == "-"_b) {
			auto [v, e] = eval_const_int_inner(i->expr);
			if (e) {
				return { 0, e };
			}
			return { -v, NULL };
		} else {
			return { 0, simple_parser_error(p, grd_current_loc(), expr->text_region, "Unexpected unary operator '%*'", i->op->op) };
		}
	} else {
		return { 0, simple_parser_error(p, grd_current_loc(), expr->text_region, "Expected literal, got '%*'", expr->type->name) };
	}
}

Tuple<s64, Error*> eval_const_int(AstExpr* expr, s64 min, s64 max_inclusive) {
	auto [v, e] = eval_const_int_inner(expr);
	if (e) {
		return { 0, e };
	}
	if (v < min) {
		return { 0, simple_parser_error(expr->p, grd_current_loc(), expr->text_region, "Expected >= %, got %.", min, v) };
	}
	if (v > max_inclusive) {
		return { 0, simple_parser_error(expr->p, grd_current_loc(), expr->text_region, "Expected <= %, got %.", max_inclusive, v) };
	}
	return { v, NULL };
}

struct MtlBufferIdxAttr: AstAttr {
	s64 index = 0;

	GRD_REFLECT(MtlBufferIdxAttr) {
		GRD_BASE_TYPE(AstAttr);
		GRD_MEMBER(index);
	}
};

struct MtlBufferAttr: MtlBufferIdxAttr {
	GRD_REFLECT(MtlBufferAttr) {
		GRD_BASE_TYPE(MtlBufferIdxAttr);
	}
};

struct MtlConstantAttr: MtlBufferIdxAttr {
	GRD_REFLECT(MtlConstantAttr) {
		GRD_BASE_TYPE(MtlBufferIdxAttr);
	}
};

struct VkSetBindingAttr: AstAttr {
	s64 set = 0;
	s64 binding = 0;

	GRD_REFLECT(VkSetBindingAttr) {
		GRD_BASE_TYPE(AstAttr);
		GRD_MEMBER(set);
		GRD_MEMBER(binding);
	}
};

struct VkUniformAttr: VkSetBindingAttr {
	GRD_REFLECT(VkUniformAttr) {
		GRD_BASE_TYPE(VkSetBindingAttr);
	}
};


struct PositionAttr: AstAttr {
	GRD_REFLECT(PositionAttr) {
		GRD_BASE_TYPE(AstAttr);
	}
};

struct FragmentAttr: AstAttr {
	GRD_REFLECT(FragmentAttr) {
		GRD_BASE_TYPE(AstAttr);
	}
};

struct VertexAttr: AstAttr {
	GRD_REFLECT(VertexAttr) {
		GRD_BASE_TYPE(AstAttr);
	}
};

struct StageInAttr: AstAttr {
	GRD_REFLECT(StageInAttr) {
		GRD_BASE_TYPE(AstAttr);
	}
};

struct ColorAttr: AstAttr {
	s64 idx = 0;

	GRD_REFLECT(ColorAttr) {
		GRD_BASE_TYPE(AstAttr);
		GRD_MEMBER(idx);
	}
};

template <typename T>
T* grd_make_ast_attr(CLikeParser* p, Token name, Array<AstExpr*> args, ProgramTextRegion reg) {
	auto node = grd_make_ast_node<T>(p, reg);
	node->name = name.str;
	node->args = args;
	return node;
}

Tuple<AstAttr*, Error*> parse_attr(CLikeParser* p, Token name, Array<AstExpr*> args, ProgramTextRegion reg) {
	if (name.str == "vk_uniform") {
		auto attr = grd_make_ast_attr<VkUniformAttr>(p, name, args, reg);
		if (len(args) == 0) {

		} else if (len(args) == 1) {
			auto [v, e] = eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->binding = v;
		} else if (len(args) == 2) {
			auto [v, e] = eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->set = v;
			auto [v2, e2] = eval_const_int(args[1], 0, s64_max);
			if (e2) {
				return { NULL, e2 };
			}
			attr->binding = v2;
		} else {
			return { NULL, simple_parser_error(p, grd_current_loc(), reg, "Expected (set, binding) or (set).") };
		}
		return { attr, NULL };
	} else if (name.str == "mtl_buffer") {
		auto attr = grd_make_ast_attr<MtlBufferAttr>(p, name, args, reg);
		if (len(args) == 0) {

		} else if (len(args) == 1) {
			auto [v, e] = eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->index = v;
		} else {
			return { NULL, simple_parser_error(p, grd_current_loc(), reg, "Expected (index).") };
		}
		return { attr, NULL };
	} else if (name.str == "mtl_constant") {
		auto attr = grd_make_ast_attr<MtlConstantAttr>(p, name, args, reg);
		if (len(args) == 0) {

		} else if (len(args) == 1) {
			auto [v, e] = eval_const_int(args[0], 0, s64_max);
			if (e) {
				return { NULL, e };
			}
			attr->index = v;
		} else {
			return { NULL, simple_parser_error(p, grd_current_loc(), reg, "Expected (index).") };
		}
		return { attr, NULL };
	} else if (name.str == "position") {
		if (len(args) != 0) {
			return { NULL, simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for position attribute, got %"_b, len(args)) };
		}
		auto attr = grd_make_ast_attr<PositionAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "fragment") {
		if (len(args) != 0) {
			return { NULL, simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for fragment attribute, got %"_b, len(args)) };
		}
		auto attr = grd_make_ast_attr<FragmentAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "stage_in") {
		if (len(args) != 0) {
			return { NULL, simple_parser_error(p, grd_current_loc(), name.reg, "Expected 0 arguments for stage_in attribute, got %"_b, len(args)) };
		}
		auto attr = grd_make_ast_attr<StageInAttr>(p, name, args, reg);
		return { attr, NULL };
	} else if (name.str == "color") {
		if (len(args) != 1) {
			return { NULL, simple_parser_error(p, grd_current_loc(), name.reg, "Expected 1 argument for color attribute, got %"_b, len(args)) };
		}
		auto [v, e] = eval_const_int(args[0], 0, s64_max);
		if (e) {
			return { NULL, e };
		}
		auto attr = grd_make_ast_attr<ColorAttr>(p, name, args, reg);
		attr->idx = v;
		return { attr, NULL };
	} else {
		return { NULL, simple_parser_error(p, grd_current_loc(), name.reg, "Unknown attribute") };
	}
}

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
				if (len(args) > 0) {
					if (tok.str != ",") {
						return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ','"_b) };
					}
					next(p);
				}
				auto op = find_binary_operator(p, U","_b);
				auto [expr, e] = parse_expr(p, op->prec + 1);
				if (e) {
					return { {}, e };
				}
				add(&args, expr);
			}
		}
		auto end_tok = peek(p);
		if (end_tok.str != "]]") {
			return { {}, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ]]"_b) };
		}
		next(p);
		ProgramTextRegion reg = { start_tok.reg.start, end_tok.reg.end };
		auto [attr, e3] = parse_attr(p, name, args, reg);
		if (e3) {
			return { {}, e3 };
		}
		add(&attrs, attr);
	}
	return { attrs, NULL };
}

Tuple<AstIf*, Error*> parse_if(CLikeParser* p, Token if_tok) {
	if (peek(p).str != "("_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ("_b) };
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
		return { NULL, simple_parser_error(p, grd_current_loc(), error_reg, U"Condition must be bool, but it's '%'"_b, cond->expr_type->name) };
	}
	if (peek(p).str != ")"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected )"_b) };
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
	auto node = grd_make_ast_node<AstIf>(p, reg);
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
				return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ;"_b) };
			}
			next(p);
		}
		auto block = grd_make_ast_node<AstBlock>(p, stmt->text_region);
		add(&block->statements, stmt);
		return { block, NULL };
	}
}

struct AstFor: AstNode {
	AstExpr*  init_expr = NULL;
	AstExpr*  cond_expr = NULL;
	AstExpr*  incr_expr = NULL;
	AstBlock* body = NULL;

	GRD_REFLECT(AstFor) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(init_expr);
		GRD_MEMBER(cond_expr);
		GRD_MEMBER(incr_expr);
		GRD_MEMBER(body);
	}
};

Tuple<AstNode*, Error*> parse_for(CLikeParser* p, Token for_tok) {
	if (peek(p).str != "("_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ("_b) };
	}
	next(p);
	auto [init_expr, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (peek(p).str != ";"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ;"_b) };
	}
	next(p);
	auto [cond_expr, e2] = parse_expr(p, 0);
	if (e2) {
		return { NULL, e2 };
	}
	if (peek(p).str != ";"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ;"_b) };
	}
	next(p);
	auto [incr_expr, e3] = parse_expr(p, 0);
	if (e3) {
		return { NULL, e3 };
	}
	if (peek(p).str != ")"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected )"_b) };
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
	auto node = grd_make_ast_node<AstFor>(p, text_region);
	node->init_expr = init_expr;
	node->cond_expr = cond_expr;
	node->incr_expr = incr_expr;
	node->body = body;
	return { node, NULL };
}

AstFunction* get_current_function(CLikeParser* p) {
	for (auto i: reverse(grd_range(len(p->scope)))) { 
		auto scope = p->scope[i];
		if (auto f = grd_reflect_cast<AstFunction>(scope)) {
			return f;
		}
	}
	return NULL;
}

struct AstReturn: AstNode {
	AstExpr* rhs = NULL;

	GRD_REFLECT(AstReturn) {
		GRD_BASE_TYPE(AstNode);
		GRD_MEMBER(rhs);
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
			return { NULL, simple_parser_error(p, grd_current_loc(), type_tok.reg, U"return must be inside a function"_b) };
		}
		next(p);

		AstExpr* rhs = NULL;
		if (f->return_ts->tp != p->void_tp) {
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			rhs = expr;
			if (f->return_ts->tp != rhs->expr_type) {
				return { NULL, simple_parser_error(p, grd_current_loc(), rhs->text_region, U"Expected return type '%' but got '%'"_b, f->return_ts->tp->name, rhs->expr_type->name) };
			}
		}
		ProgramTextRegion reg = type_tok.reg;
		if (rhs->text_region.has_value) {
			reg.end = rhs->text_region.value.end;
		}
		auto node = grd_make_ast_node<AstReturn>(p, reg);
		node->rhs = rhs;
		return { node, NULL };
	}

	auto lookup_tp = find_type(p, type_tok.str);
	if (lookup_tp) {
		auto [pre_type, e1] = parse_pre_type(p);
		if (e1) {
			return { NULL, e1 };
		}
		auto [ident, e2] = parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		auto [decl, ee] = parse_var_decl(p, pre_type, ident, false);
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
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected {"_b) };
	}
	next(p);
	auto block = grd_make_ast_node<AstBlock>(p, {});
	block->statements.allocator = p->allocator;
	add(&p->scope, block);
	grd_defer { pop(&p->scope); };

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
				return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ;"_b) };
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

Error* grd_make_double_site_error(CLikeParser* p, GrdCodeLoc loc, Optional<ProgramTextRegion> reg, Optional<ProgramTextRegion> reg2, auto... args) {
	auto error = grd_make_parser_error(p, loc, args...);
	add_site(p, error,
		CParserErrorToken{ .reg = reg, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_GREEN },
		CParserErrorToken{ .reg = reg2, .color = CPARSER_ERROR_TOKEN_COLOR_REGULAR_RED }
	);
	return error;
}

Tuple<AstFunctionKind, Error*> resolve_function_kind(CLikeParser* p, AstFunction* f) {
	AstFunctionKind kind = AstFunctionKind::Plain;
	AstAttr* kind_attr = NULL;
	for (auto attr: f->attrs) {
		AstFunctionKind new_kind = AstFunctionKind::Plain;
		if (auto m = grd_reflect_cast<FragmentAttr>(attr)) {
			new_kind = AstFunctionKind::Fragment;
		}
		if (auto m = grd_reflect_cast<VertexAttr>(attr)) {
			new_kind = AstFunctionKind::Vertex;
		}
		if (new_kind != AstFunctionKind::Plain) {
			if (kind != AstFunctionKind::Plain) {
				return { {}, grd_make_double_site_error(p, grd_current_loc(), kind_attr->text_region, attr->text_region, "Function kind already defined."_b) };
			}
			kind = new_kind;
		}
	}
	return { kind, NULL };
}

Error* handle_entry_point_arg(CLikeParser* p, AstFunction* f, s64 idx) {
	auto arg = f->args[idx];

	enum class EntryPointArgKind {
		Undefined,
		StageIn,
		Buffer,
	};
	EntryPointArgKind kind = EntryPointArgKind::Undefined;

	for (auto attr: arg->attrs) {
		if (auto m = grd_reflect_cast<StageInAttr>(attr)) {
			m->is_used = true;
			kind = EntryPointArgKind::StageIn;
			break;
		}
		if (auto m = grd_reflect_cast<MtlBufferAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
		if (auto m = grd_reflect_cast<MtlConstantAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
		if (auto m = grd_reflect_cast<VkUniformAttr>(attr)) {
			kind = EntryPointArgKind::Buffer;
			break;
		}
	}
	if (kind == EntryPointArgKind::Undefined) {
		return simple_parser_error(p, grd_current_loc(), arg->text_region, "Unexpected entry point argument."_b);
	}
	if (kind == EntryPointArgKind::Buffer) {
		if (!is_pointer(arg->var_ts->tp)) {
			return simple_parser_error(p, grd_current_loc(), arg->text_region, "Expected pointer type for a buffer."_b);
		}
		VkUniformAttr* vk_attr = NULL;
		MtlBufferIdxAttr* mtl_attr = NULL;
		for (auto attr: arg->attrs) {
			if (auto m = grd_reflect_cast<VkUniformAttr>(attr)) {
				if (vk_attr) {
					return grd_make_double_site_error(p, grd_current_loc(), vk_attr->text_region, attr->text_region, "Vk uniform attribute is already specified."_b);
				}
				vk_attr = m;
			} else if (auto m = grd_reflect_cast<MtlBufferAttr>(attr)) {
				if (mtl_attr) {
					return grd_make_double_site_error(p, grd_current_loc(), mtl_attr->text_region, attr->text_region, "Metal buffer attribute is already specified."_b);
				}
				mtl_attr = m;
			} else if (auto m = grd_reflect_cast<MtlConstantAttr>(attr)) {
				if (mtl_attr) {
					return grd_make_double_site_error(p, grd_current_loc(), mtl_attr->text_region, attr->text_region, "Metal constant attribute is already specified."_b);
				}
				mtl_attr = m;
			} else {
				return simple_parser_error(p, grd_current_loc(), attr->text_region, "Unexpected attribute."_b);
			}
		}
		if (!vk_attr) {
			return simple_parser_error(p, grd_current_loc(), arg->text_region, "Vk uniform attribute must be specified."_b);
		}
		if (!mtl_attr) {
			return simple_parser_error(p, grd_current_loc(), arg->text_region, "Metal buffer attribute must be specified."_b);
		}
		for (auto it: f->mtl_buffers) {
			if (it->index == mtl_attr->index) {
				return grd_make_double_site_error(p, grd_current_loc(), it->text_region, mtl_attr->text_region, "Metal buffer index (%) is already used."_b, mtl_attr->index);
			}
		}
		for (auto it: f->vk_set_bindings) {
			if (it->set == vk_attr->set && it->binding == vk_attr->binding) {
				return grd_make_double_site_error(p, grd_current_loc(), it->text_region, vk_attr->text_region, "Vulkan set/binding (%, %) is already used."_b, vk_attr->set, vk_attr->binding);
			}
		}
		mtl_attr->is_used = true;
		vk_attr->is_used = true;
		add(&f->mtl_buffers, mtl_attr);
		add(&f->vk_set_bindings, vk_attr);
	}
	if (kind == EntryPointArgKind::StageIn) {
		if (!is_struct(arg->var_ts->tp)) {
			return simple_parser_error(p, grd_current_loc(), arg->text_region, "Expected struct type for a stage_in argument."_b);
		}
		if (f->stage_in_arg) {
			return grd_make_double_site_error(p, grd_current_loc(), f->stage_in_arg->text_region, arg->text_region, "Stage_in argument is already specified."_b);
		}
		f->stage_in_arg = arg;
	}
	return NULL;
}

Error* handle_entry_point_args(CLikeParser* p, AstFunction* f) {
	for (auto i: grd_range(len(f->args))) {
		if (f->kind != AstFunctionKind::Plain) {
			auto e = handle_entry_point_arg(p, f, i);
			if (e) {
				return e;
			}
		}
		auto e = check_if_attrs_used(p, f->args[i]->attrs);
		if (e) {
			return e;
		}
	}
	return NULL;
}

Error* ensure_function_has_return(CLikeParser* p, AstFunction* f) {
	if (f->return_ts->tp == p->void_tp) {
		return NULL;
	}
	if (!f->block) {
		return simple_parser_error(p, grd_current_loc(), f->text_region, "Function % has no body", f->name);
	}
	if (len(f->block->statements) == 0) {
		// Expected return statement.
		return simple_parser_error(p, grd_current_loc(), f->block->text_region, "Expected return statement");
	}
	auto last = f->block->statements[-1];
	auto ret = grd_reflect_cast<AstReturn>(last);
	if (!ret) {
		return simple_parser_error(p, grd_current_loc(), f->block->text_region, "Expected return statement");
	}
	return NULL;
}

Error* validate_fragment_output_struct(CLikeParser* p, AstStructType* tp) {
	s64 idx = 0;
	for (auto it: tp->members) {
		ColorAttr* c_attr = NULL;
		for (auto attr: it->attrs) {
			if (auto m = grd_reflect_cast<ColorAttr>(attr)) {
				if (c_attr) {
					return grd_make_double_site_error(p, grd_current_loc(), c_attr->text_region, attr->text_region, "Color attribute is already specified."_b);
				}
				c_attr = m;
			}
		}
		if (c_attr == NULL) {
			return simple_parser_error(p, grd_current_loc(), it->text_region, "Color attribute must be specified."_b);
		}
		for (auto i: grd_range(idx)) {
			auto prev_m = tp->members[i];
			for (auto attr: prev_m->attrs) {
				if (auto m = grd_reflect_cast<ColorAttr>(attr)) {
					if (m->idx == c_attr->idx) {
						return grd_make_double_site_error(p, grd_current_loc(), m->text_region, c_attr->text_region, "Color index (%) is already used."_b, c_attr->idx);
					}
				}
			}
		}
		idx += 1;
	}
	return NULL;
}

Error* typecheck_function_return_value(CLikeParser* p, AstFunction* f) {
	if (f->kind == AstFunctionKind::Fragment) {
		if (f->return_ts->tp == p->void_tp) {

		} else if (f->return_ts->tp == p->float4_tp) {

		} else if (is_struct(f->return_ts->tp)) {
			auto stp = grd_reflect_cast<AstStructType>(f->return_ts->tp);
			if (!stp) {
				return simple_parser_error(p, grd_current_loc(), f->text_region, "Internal error: expected struct type.");
			}
			auto e = validate_fragment_output_struct(p, stp);
			if (e) {
				return e;
			}
		} else {
			return simple_parser_error(p, grd_current_loc(), f->text_region, "Fragment function must return void, float4, or struct."_b);
		}
	} else if (f->kind == AstFunctionKind::Vertex) {
		if (f->return_ts->tp != p->void_tp) {
			return simple_parser_error(p, grd_current_loc(), f->text_region, "Vertex function must return void."_b);
		}
	}
	return NULL;
}

Tuple<AstSymbol*, Error*> parse_function(CLikeParser* p, PreType pre_type,  Array<AstAttr*> attrs, Token start_tok, Token ident) {
	Array<AstFunctionArg*> args = { .allocator = p->allocator };
	grd_defer { args.free(); };

	next(p);

	AstTypeSite* return_ts = NULL;

	while (true) {
		auto tok = peek(p);
		if (tok.str == U")"_b) {
			next(p);
			auto [attrs, e] = parse_attrs(p);
			if (e) {
				return { NULL, e };
			}
			auto [ts, e2] = finalize_type(p, pre_type, attrs);
			if (e2) {
				return { NULL, e2 };
			}
			return_ts = ts;
			auto e4 = check_if_attrs_used(p, attrs);
			if (e4) {
				return { NULL, e4 };
			}
			break;
		}
		if (len(args) > 0) {
			if (tok.str != U","_b) {
				return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ','"_b) };
			}
			next(p);
		}
		auto start = peek(p).reg.start;
		auto type_tok = peek(p);
		auto [pre_type, e] = parse_pre_type(p);
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
		auto [ts, e4] = finalize_type(p, pre_type, attrs);
		if (e4) {
			return { NULL, e4 };
		}
		ProgramTextRegion text_region = { start, peek(p).reg.end };
		if (len(attrs) > 0) {
			auto last = attrs[-1];
			if (last->text_region.has_value) {
				text_region.end = last->text_region.value.end;
			}
		}
		auto arg_node = grd_make_ast_symbol<AstFunctionArg>(p, arg_name.str, text_region);
		arg_node->var_ts = ts;
		arg_node->attrs = attrs;
		add(&args, arg_node);
	}

	ProgramTextRegion text_region = { start_tok.reg.start, peek(p).reg.end };
	auto f = grd_make_ast_node<AstFunction>(p, text_region);
	f->args.allocator = p->allocator;
	f->return_ts = return_ts;
	f->name = ident.str;
	f->args = args;
	f->attrs = attrs;
	f->mtl_buffers.allocator = p->allocator;
	f->vk_set_bindings.allocator = p->allocator;

	for (auto attr: attrs) {
		if (grd_reflect_cast<FragmentAttr>(attr)) {
			f->kind = AstFunctionKind::Fragment;
			attr->is_used = true;
			break;
		}
	}

	auto e = typecheck_function_return_value(p, f);
	if (e) {
		return { NULL, e };
	}
	e = handle_entry_point_args(p, f);
	if (e) {
		return { NULL, e };
	}
	e = check_if_attrs_used(p, attrs);
	if (e) {
		return { NULL, e };
	}

	auto tok = peek(p);
	if (tok.str == ";") {
		next(p);
		f->text_region.value.end = tok.reg.end;
		return { f, NULL };
	}
	if (tok.str == "{") {
		add(&p->scope, f);
		grd_defer { pop(&p->scope); };

		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		f->block = (AstBlock*) block;
		if (f->block->text_region.has_value) {
			f->text_region.value.end = f->block->text_region.value.end;
		}
		auto e2 = ensure_function_has_return(p, f);
		if (e2) {
			return { NULL, e2 };
		}
		return { f, NULL };
	}

	return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ; or { after function header"_b) };
}

Tuple<AstStructType*, Error*> parse_struct(CLikeParser* p, Token start_tok) {
	auto [ident, e] = parse_ident(p);
	if (e) {
		return { NULL, e };
	}
	auto tok = peek(p);
	if (tok.str != U"{"_b) {
		return { NULL, simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected {"_b) };
	}
	next(p);

	Array<AstStructMember*> members = { .allocator = p->allocator };
	grd_defer { members.free(); };

	ProgramTextRegion st_reg;
	st_reg.start = start_tok.reg.start;
	st_reg.end = start_tok.reg.end;

	AstStructMember* pos_member = NULL;

	while (true) {
		auto tok = peek(p);
		if (tok.str == U"}"_b) {
			st_reg.end = tok.reg.end;
			next(p);
			break;
		}
		auto [pre_type, e] = parse_pre_type(p);
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
		auto [ts, e4] = finalize_type(p, pre_type, attrs);
		if (e4) {
			return { NULL, e4 };
		}
		if (peek(p).str != ";") {
			return { NULL, simple_parser_error(p, grd_current_loc(), peek(p).reg, U"Expected ;"_b) };
		}
		next(p);

		ProgramTextRegion reg;
		reg.start = pre_type.reg.start;
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
		auto member = grd_make_ast_node<AstStructMember>(p, reg);
		member->member_ts = ts;
		member->name = name.str;
		member->attrs = attrs;
		add(&members, member);

		for (auto attr: attrs) {
			if (auto x = grd_reflect_cast<PositionAttr>(attr)) {
				if (pos_member) {
					return { NULL, simple_parser_error(p, grd_current_loc(), attr->text_region, "Position attribute can only be specified once."_b) };
				}
				pos_member = member;
				if (member->member_ts->tp != p->float4_tp) {
					return { NULL, simple_parser_error(p, grd_current_loc(), attr->text_region, "Position attribute can only be specified for float4 member."_b) };
				}
				x->is_used = true;
			}
		}
		auto e5 = check_if_attrs_used(p, attrs);
		if (e5) {
			return { NULL, e5 };
		}
	}

	auto st = grd_make_ast_node<AstStructType>(p, st_reg);
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
	auto [pre_type, e0] = parse_pre_type(p);
	if (e0) {
		return e0;
	}
	auto [ident, e] = parse_ident(p);
	if (e) {
		return e;
	}
	auto tok = peek(p);
	if (tok.str == U"("_b) {
		auto [f, e] = parse_function(p, pre_type, attrs, start_tok, ident);
		if (e) {
			return e;
		}
		e = add_global(p, f);
		if (e) {
			return e;
		}
		auto e2 = check_if_attrs_used(p, attrs);
		if (e2) {
			return e2;
		}
		return NULL;
	} else if (tok.str == U","_b || tok.str == U";"_b || tok.str == U"="_b) {
		auto [group, e] = parse_var_decl(p, pre_type, ident, true);
		if (e) {
			return e;
		}
		e = add_global(p, group);
		if (e) {
			return e;
		}
		auto e2 = check_if_attrs_used(p, attrs);
		if (e2) {
			return e2;
		}
		return NULL;
	} else {
		return simple_parser_error(p, grd_current_loc(), tok.reg, U"Expected ( or , or ; or ="_b);
	}
}

template <typename T>
AstGrdPrimitiveType* push_prim_type(CLikeParser* p, UnicodeString name, u64 size, u64 alignment, bool is_signed) {
	auto tp = grd_make_ast_symbol<AstGrdPrimitiveType>(p, name, {});
	tp->size = size;
	tp->alignment = alignment;
	tp->is_signed = is_signed;
	tp->c_tp = grd_reflect_type_of<T>()->template as<GrdPrimitiveType>();
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
		u64 m_end = it->member_ts->tp->size + it->offset;
		if (m_end > end) {
			end = m_end;
		}
		if (it->member_ts->tp->alignment > max_member_align) {
			max_member_align = it->member_ts->tp->alignment;
		}
	}
	u64 size = align(end, max_member_align);
	return size;
}

void push_member(AstStructType* tp, UnicodeString name, AstTypeSite* ts) {
	auto m = grd_make_ast_node<AstStructMember>(tp->p, {});
	m->struct_type = tp;
	m->member_ts = ts;
	m->name = name;
	auto sz = calc_struct_size(tp);
	m->offset = align(sz, ts->tp->alignment);
	add(&tp->members, m);
}

AstTypeSite* grd_make_siteless_type_site(CLikeParser* p, AstType* tp) {
	auto ts = grd_make_ast_node<AstTypeSite>(p, {});
	ts->tp = tp;
	return ts;
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

	auto f32_ts = grd_make_siteless_type_site(p, p->f32_tp);

	p->float2_tp = grd_make_ast_symbol<AstStructType>(p, U"float2"_b, {});
	push_member(p->float2_tp, U"x"_b, f32_ts);
	push_member(p->float2_tp, U"y"_b, f32_ts);
	p->float2_tp->alignment = 8;
	p->float2_tp->size = 8;
	auto e = add_global(p, p->float2_tp);
	if (e) {
		panic(e);
	}

	p->float3_tp = grd_make_ast_symbol<AstStructType>(p, U"float3"_b, {});
	push_member(p->float3_tp, U"x"_b, f32_ts);
	push_member(p->float3_tp, U"y"_b, f32_ts);
	push_member(p->float3_tp, U"z"_b, f32_ts);
	p->float3_tp->alignment = 16;
	p->float3_tp->size = 16;
	e = add_global(p, p->float3_tp);
	if (e) {
		panic(e);
	}

	p->float4_tp = grd_make_ast_symbol<AstStructType>(p, U"float4"_b, {});
	push_member(p->float4_tp, U"x"_b, f32_ts);
	push_member(p->float4_tp, U"y"_b, f32_ts);
	push_member(p->float4_tp, U"z"_b, f32_ts);
	push_member(p->float4_tp, U"w"_b, f32_ts);
	p->float4_tp->alignment = 16;
	p->float4_tp->size = 16;
	e = add_global(p, p->float4_tp);
	if (e) {
		panic(e);
	}
}


struct ShaderIntrinFunc: AstFunction {

	GRD_REFLECT(ShaderIntrinFunc) {
		GRD_BASE_TYPE(AstFunction);
	}
};

void add_shader_intrinsic_var(CLikeParser* p, UnicodeString name, AstTypeSite* ts) {
	// @TODO: check for duplicates
	if (!ts) {
		panic("No type for shader intrinsic var");
	}
	auto node = grd_make_ast_symbol<ShaderIntrinVar>(p, name, {});
	node->var_ts = ts;
	auto e = add_global(p, node);
	if (e) {
		panic(e);
	}
}

void add_shader_intrinsic_func(CLikeParser* p, UnicodeString name, AstTypeSite* ret_ts, std::initializer_list<AstTypeSite*> args) {
	// @TODO: check for duplicates
	auto node = grd_make_ast_symbol<ShaderIntrinFunc>(p, name, {});
	node->args.allocator = p->allocator;
	node->return_ts = ret_ts;
	s64 i = 0;
	for (auto arg: args) {
		auto name = sprint_unicode(p->allocator, U"arg_%"_b, i);
		auto arg_node = grd_make_ast_symbol<AstFunctionArg>(p, name, {});
		arg_node->var_ts = arg;
		add(&node->args, arg_node);
		i += 1;
	}
	auto e = add_global(p, node);
	if (e) {
		panic(e);
	}
}

Tuple<CLikeProgram*, Error*> parse_c_like(UnicodeString str) {
	auto allocator = grd_make_arena_allocator();
	auto p = grd_make<CLikeParser>(allocator);
	p->allocator = allocator;
	p->program = grd_make_ast_node<CLikeProgram>(p, {});
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

	auto f32_ts = grd_make_siteless_type_site(p, p->f32_tp);
	auto float3_ts = grd_make_siteless_type_site(p, p->float3_tp);

	add_shader_intrinsic_func(p, U"hsv"_b, float3_ts, { f32_ts, f32_ts, f32_ts });
	add_shader_intrinsic_func(p, U"log"_b, f32_ts, { f32_ts });
	add_shader_intrinsic_func(p, U"sin"_b, f32_ts, { f32_ts });
	add_shader_intrinsic_func(p, U"cos"_b, f32_ts, { f32_ts });
	add_shader_intrinsic_func(p, U"dot_vec3"_b, f32_ts, { float3_ts, float3_ts });
	add_shader_intrinsic_func(p, U"atan"_b, f32_ts, { f32_ts });
	add_shader_intrinsic_func(p, U"length"_b, f32_ts, { float3_ts });
	add_shader_intrinsic_func(p, U"abs"_b, f32_ts, { f32_ts });

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
