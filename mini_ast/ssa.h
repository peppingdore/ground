#pragma once

#include "c_like_parser.h"

struct AstRunner {
	CLikeProgram* program;
};

AstRunner* make_ast_runner(CLikeProgram* program) {
	auto runner = make<AstRunner>();
	runner->program = program;
	return runner;
}

struct SsaValue {
	AstType* type = NULL;
	
		
};

struct Ssa;
struct SsaBasicBlock;

struct SsaReg {
	s64 v = 0;

	bool operator==(SsaReg rhs) {
		return v == rhs.v;
	}

	REFLECT(SsaReg) {
		MEMBER(v);
	}
};

void type_format(Formatter* fmt, SsaReg* reg, String spec) {
	format(fmt, "$%", reg->v);
}

constexpr SsaReg SSA_NO_REG = { .v = 0 };

struct SsaInst {
	Type*          type = NULL;
	SsaBasicBlock* block = NULL;
	SsaReg         dst = SSA_NO_REG;

	REFLECT(SsaInst) {
		MEMBER(type); TAG(RealTypeMember{});
		MEMBER(block);
		MEMBER(dst);
	}
};

struct SsaPhi;

struct SsaBasicBlock {
	String                name;
	Ssa*                  ssa;
	bool                  is_sealed = false;
	Array<SsaInst*>       insts;
	Array<SsaPhi*>       incomplete_phis;
	Array<SsaBasicBlock*> pred;
	Array<SsaBasicBlock*> successors;
	HashMap<AstVar*, SsaReg> var_map;
};

void add_pred(SsaBasicBlock* block, SsaBasicBlock* pred) {
	if (block->is_sealed) {
		panic("Cannot add pred to sealed block");
	}
	add(&block->pred, pred);
}

struct Ssa {
	Allocator             allocator = c_allocator;
	SsaBasicBlock*        entry = NULL;
	s64                   reg_counter = 0;

	void free() {
		free_allocator(allocator);
	}
};

SsaReg alloc_reg(Ssa* ssa) {
	return SsaReg { ++ssa->reg_counter };
}

SsaBasicBlock* make_ssa_basic_block(Ssa* ssa, String name = ""_b) {
	SsaBasicBlock* block = make<SsaBasicBlock>(ssa->allocator);
	block->name = name;
	block->ssa = ssa;
	block->insts = { .allocator = ssa->allocator };
	block->incomplete_phis = { .allocator = ssa->allocator };
	block->pred = { .allocator = ssa->allocator };
	block->successors = { .allocator = ssa->allocator };
	block->var_map = { .allocator = ssa->allocator };
	return block;
}

void write_var(SsaBasicBlock* block, AstVar* var, SsaReg reg) {
	put(&block->var_map, var, reg);
}

SsaReg read_var(SsaBasicBlock* block, AstVar* var);

struct SsaPhi: SsaInst {
	Array<SsaReg>         args;
	Array<SsaBasicBlock*> pred;
	AstVar*               var = NULL;
};

template <typename T>
T* make_ssa_inst(SsaBasicBlock* block) {
	T* inst = make<T>(block->ssa->allocator);
	inst->type = reflect_type_of<T>();
	inst->block = block;
	add(&block->insts, inst);
	return inst;
}

void add_phi_args(SsaPhi* phi) {
	for (auto pred: phi->block->pred) {
		SsaReg reg = read_var(pred, phi->var);
		if (reg == SSA_NO_REG) {
			panic("Unknown variable: %", phi->var->name);
		}
		add(&phi->args, reg);
		add(&phi->pred, pred);
	}
}

SsaReg read_var_recursive(SsaBasicBlock* block, AstVar* var) {
	SsaReg reg = SSA_NO_REG;
	if (!block->is_sealed) {
		auto phi = make_ssa_inst<SsaPhi>(block);
		phi->dst = alloc_reg(block->ssa);
		phi->var = var;
		add(&block->incomplete_phis, phi);
		reg = phi->dst;
	} else if (len(block->pred) == 1) {
		reg = read_var(block->pred[0], var);
	} else {
		auto phi = make_ssa_inst<SsaPhi>(block);
		phi->dst = alloc_reg(block->ssa);
		phi->var = var;
		write_var(block, var, phi->dst);
		add_phi_args(phi);
		reg = phi->dst;
	}
	assert(reg != SSA_NO_REG);
	write_var(block, var, reg);
	return reg;
}

void seal_block(SsaBasicBlock* block) {
	for (auto phi: block->incomplete_phis) {
		add_phi_args(phi);
		remove(&block->incomplete_phis, phi);
	}
	block->is_sealed = true;
}

void add_prec(SsaBasicBlock* block, SsaBasicBlock* pred) {
	if (!pred->is_sealed) {
		panic("Cannot add unsealed pred");
	}	
	if (block->is_sealed) {
		panic("Cannot add pred to sealed block");
	}
	add(&block->pred, pred);
	add(&pred->successors, block);
}

SsaReg read_var(SsaBasicBlock* block, AstVar* var) {
	assert(var->is_global);
	auto found = get(&block->var_map, var);
	if (found) {
		return *found;
	}
	return SSA_NO_REG;
}

enum class SsaLValueNodeKind {
	ArrayIndex,
	MemberAccess,
	Swizzle,
};

struct SsaLValueNode {
	SsaLValueNodeKind kind;
	s32               swizzle[4];
	s32               swizzle_len = 0;
	SsaReg            idx = SSA_NO_REG;
	s64               elem_size = 0;
	s64               member_offset = 0;
};

struct SsaLvalue {
	AstVar*              var = NULL;
	SsaReg               addr_reg = SSA_NO_REG;
	Array<SsaLValueNode> nodes;
};

struct SsaLoadConst: SsaInst {
	AstType*  lit_type = NULL;
	Array<u8> data;

	REFLECT(SsaLoadConst) {
		BASE_TYPE(SsaInst);
		MEMBER(lit_type);
		MEMBER(data);
	}
};

struct SsaLoadCConst: SsaInst {
	Any data;

	REFLECT(SsaLoadCConst) {
		BASE_TYPE(SsaInst);
		MEMBER(data);
	}
};

template <typename T>
SsaReg load_const(SsaBasicBlock* block, T value) {
	Type* type = reflect_type_of<T>();
	void* data = Malloc(block->ssa->allocator, sizeof(T));
	memcpy(data, &value, sizeof(T));
	auto inst = make_ssa_inst<SsaLoadCConst>(block);
	inst->dst = alloc_reg(block->ssa);
	inst->data = make_any(type, data);
	return inst->dst;
}

s64 resovle_type_size(AstType* type) {
	// @TODO:
	return 0;
}

Tuple<SsaLvalue, Error*> emit_lvalue(SsaBasicBlock* block, AstExpr* expr) {
	if (!expr->is_lvalue) {
		println("%*+", expr);
		panic();
	}
	assert(expr->is_lvalue);

	if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		return { SsaLvalue { var_access->var }, NULL };
	} else if (auto arr_access = reflect_cast<AstArrayAccess>(expr)) {
		auto [lhs, e] = emit_lvalue(block, arr_access->lhs);
		if (e) {
			return { {}, e };
		}
		s64 elem_size = resovle_type_size(arr_access->expr_type);
		auto idx = load_const(block, make_any(&elem_size));
		auto node = SsaLValueNode { 
			.kind = SsaLValueNodeKind::ArrayIndex,
			.idx = idx,
			.elem_size = elem_size
		};
		add(&lhs.nodes, node);
		return { lhs, NULL };
	} else if (auto access = reflect_cast<AstVarMemberAccess>(expr)) {
		auto [lhs, e] = emit_lvalue(block, access->lhs);
		if (e) {
			return { {}, e };
		}
		auto node = SsaLValueNode {
			.kind = SsaLValueNodeKind::MemberAccess,
			.member_offset = access->member.offset
		};
		add(&lhs.nodes, node);
		return { lhs, NULL };
	} else if (auto swizzle = reflect_cast<AstSwizzleExpr>(expr)) {
		auto [lhs, e] = emit_lvalue(block, swizzle->lhs);
		if (e) {
			return { {}, e };
		}
		auto node = SsaLValueNode {
			.kind = SsaLValueNodeKind::Swizzle,
			.swizzle_len = swizzle->swizzle_len,
		};
		memcpy(node.swizzle, swizzle->swizzle, sizeof(s32) * swizzle->swizzle_len);
		add(&lhs.nodes, node);
		return { lhs, NULL };
	} else {
		println("%*+", expr);
		return { {}, format_error("Unsupported expression: %*", expr->type->name) };
	}
}

struct SsaBinaryOp: SsaInst {
	UnicodeString op;
	SsaReg        lhs = SSA_NO_REG;
	SsaReg        rhs = SSA_NO_REG;

	REFLECT(SsaBinaryOp) {
		BASE_TYPE(SsaInst);
		MEMBER(op);
		MEMBER(lhs);
		MEMBER(rhs);
	}
};

SsaReg emit_binary_op(SsaBasicBlock* block, UnicodeString op, SsaReg lhs, SsaReg rhs) {
	auto inst = make_ssa_inst<SsaBinaryOp>(block);
	inst->dst = alloc_reg(block->ssa);
	inst->op = op;
	inst->lhs = lhs;
	inst->rhs = rhs;
	return inst->dst;
}

struct SsaFunctionCall: SsaInst {
	AstNode* f = NULL;
	Array<SsaReg> args;

	REFLECT(SsaFunctionCall) {
		BASE_TYPE(SsaInst);
		MEMBER(f);
		MEMBER(args);
	}
};

struct SsaStore: SsaInst {
	SsaLvalue lhs;
	SsaReg    rhs = SSA_NO_REG;

	REFLECT(SsaStore) {
		BASE_TYPE(SsaInst);
		MEMBER(lhs);
		MEMBER(rhs);
	}
};

SsaStore* emit_store(SsaBasicBlock* block, SsaLvalue lhs, SsaReg rhs) {
	auto inst = make_ssa_inst<SsaStore>(block);
	inst->dst = alloc_reg(block->ssa);
	inst->lhs = lhs;
	inst->rhs = rhs;
	return inst;
}

struct SsaLoadLvalue: SsaInst {
	SsaLvalue lhs;

	REFLECT(SsaLoadLvalue) {
		BASE_TYPE(SsaInst);
		MEMBER(lhs);
	}
};

SsaReg emit_load_lvalue(SsaBasicBlock* block, SsaLvalue lhs) {
	auto inst = make_ssa_inst<SsaLoadLvalue>(block);
	inst->dst = alloc_reg(block->ssa);
	inst->lhs = lhs;
	return inst->dst;
}

Tuple<SsaReg, Error*> emit_expr(SsaBasicBlock* block, AstExpr* expr) {
	if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		auto inst = make_ssa_inst<SsaFunctionCall>(block);
		for (auto arg: call->args) {
			auto [var, e] = emit_expr(block, arg);
			if (e) {
				return { NULL, e };
			}
			add(&inst->args, var);
		}
		inst->f = call->f;
		inst->dst = alloc_reg(block->ssa);
		return { inst->dst, NULL };
	} else if (auto binary_expr = reflect_cast<CBinaryExpr>(expr)) {
		auto [rhs, e0] = emit_expr(block, binary_expr->rhs);
		if (e0) {
			return { NULL, e0 };
		}
		if (binary_expr->op->op == "=") {
			auto [lhs, e] = emit_lvalue(block, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			emit_store(block, lhs, rhs);
			return { rhs, NULL };
		} else if (binary_expr->op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			auto [lhs, e] = emit_lvalue(block, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto loaded = emit_load_lvalue(block, lhs);
			auto oped = emit_binary_op(block, binary_expr->pure_op->op, loaded, rhs);
			emit_store(block, lhs, oped);
			return { oped, NULL };
		} else {
			auto [lhs, e] = emit_expr(block, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto oped = emit_binary_op(block, binary_expr->op->op, lhs, rhs);
			return { oped, NULL };
		}
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		auto [lvalue, e] = emit_lvalue(block, expr);
		if (e) {
			return { NULL, e };
		}
		auto loaded = emit_load_lvalue(block, lvalue);
		return { loaded, NULL };
	} else if (auto literal_expr = reflect_cast<LiteralExpr>(expr)) {
		auto lit = make_ssa_inst<SsaLoadConst>(block);
		lit->dst = alloc_reg(block->ssa);
		lit->lit_type = literal_expr->expr_type;
		if (literal_expr->expr_type->name == U"int"_b) {
			lit->data.allocator = block->ssa->allocator;
			add(&lit->data, (u8*) &literal_expr->s64_value, sizeof(literal_expr->s64_value));
		} else if (literal_expr->expr_type->name == U"float"_b) {
			lit->data.allocator = block->ssa->allocator;
			add(&lit->data, (u8*) &literal_expr->f32_value, sizeof(literal_expr->f32_value));
		} else if (literal_expr->expr_type->name == U"double"_b) {
			lit->data.allocator = block->ssa->allocator;
			add(&lit->data, (u8*) &literal_expr->f64_value, sizeof(literal_expr->f64_value));
		} else {
			return { NULL, format_error("Unsupported literal type: %", literal_expr->expr_type->name) };
		}
		return { lit->dst, NULL };
	} else if (auto arr_access = reflect_cast<AstArrayAccess>(expr)) {
		auto [lvalue, e] = emit_lvalue(block, arr_access->lhs);
		if (e) {
			return { NULL, e };
		}
		auto loaded = emit_load_lvalue(block, lvalue);
		return { loaded, NULL };
	} else if (auto swizzle = reflect_cast<AstSwizzleExpr>(expr)) {
		auto [lvalue, e] = emit_lvalue(block, swizzle);
		if (e) {
			return { NULL, e };
		}
		auto loaded = emit_load_lvalue(block, lvalue);
		return { loaded, NULL };
	} else if (auto postfix_expr = reflect_cast<CPostfixExpr>(expr)) {
		auto [lvalue, eo] = emit_lvalue(block, postfix_expr->lhs);
		if (eo) {
			return { NULL, eo };
		}
		auto loaded = emit_load_lvalue(block, lvalue);
		UnicodeString op;
		if (postfix_expr->op == "++") {
			op = U"+"_b;
		} else if (postfix_expr->op == "--") {
			op = U"-"_b;
		} else {
			return { NULL, format_error("Unsupported postfix operator: %", postfix_expr->op) };
		}
		auto const_one = load_const(block, (s64) 1);
		auto oped = emit_binary_op(block, op, loaded, const_one);
		return { oped, NULL };
	} else if (auto access = reflect_cast<AstVarMemberAccess>(expr)) {
		auto [lvalue, e] = emit_lvalue(block, access);
		if (e) {
			return { NULL, e };
		}
		auto loaded = emit_load_lvalue(block, lvalue);
		return { loaded, NULL };
	} else {
		return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
	}
}

struct SsaLocalVar: SsaInst {
	AstVarDecl* var = NULL;

	REFLECT(SsaLocalVar) {
		BASE_TYPE(SsaInst);
		MEMBER(var);
	}
};

Error* emit_stmt(SsaBasicBlock* block, AstNode* stmt) {
	if (auto expr = reflect_cast<AstExpr>(stmt)) {
		auto [var, e] = emit_expr(block, expr);
		if (e) {
			return e;
		}
		return NULL;
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		auto first = var_decl_group->var_decls[0];
		SsaReg init_reg = SSA_NO_REG;
		if (first->init) {
			auto [var, e] = emit_expr(block, first->init);
			if (e) {
				return e;
			}
			init_reg = var;
		}
		for (auto var_decl: var_decl_group->var_decls) {
			SsaReg var_reg = SSA_NO_REG;
			if (init_reg != SSA_NO_REG) {
				var_reg = init_reg;
			} else {
				auto inst = make_ssa_inst<SsaLocalVar>(block);
				inst->dst = alloc_reg(block->ssa);
				inst->var = var_decl;
				var_reg = inst->dst;
			}
			write_var(block, var_decl, var_reg);
		}
		return NULL;
	} else if (auto var_decl = reflect_cast<AstVarDecl>(stmt)) {
		SsaReg reg = SSA_NO_REG;
		if (var_decl->init) {
			auto [var, e] = emit_expr(block, var_decl->init);
			if (e) {
				return e;
			}
			reg = var;
		} else {
			auto inst = make_ssa_inst<SsaLocalVar>(block);
			inst->dst = alloc_reg(block->ssa);
			inst->var = var_decl;
			reg = inst->dst;
		}
		write_var(block, var_decl, reg);
		return NULL;
	} else if (auto _for = reflect_cast<AstFor>(stmt)) {
		auto init_block = make_ssa_basic_block(block->ssa, "for_init"_b);
		auto [_, e0] = emit_expr(init_block, _for->init_expr);
		if (e0) {
			return e0;
		}
		seal_block(block);
		add_prec(init_block, block);
		seal_block(init_block);
		return NULL;
	} else {
		return format_error("Unsupported statement: %*", stmt->type->name);
	}
}

Error* emit_block(SsaBasicBlock* ssa_block, AstBlock* block) {
	for (auto stmt: block->statements) {
		auto e = emit_stmt(ssa_block, stmt);
		if (e) {
			return e;
		}
	}
	return NULL;
}

void print_ssa_inst(SsaInst* inst) {
	auto type = inst->type->as<StructType>();
	if (!type) {
		panic("Unsupported type: %*", inst->type->name);
	}
	if (inst->dst != SSA_NO_REG) {
		print("% = ", inst->dst);
	}
	print("%* ", inst->type->name);
	for (auto m: type->members) {
		if (strcmp(m.name, "type") == 0) {
			continue;
		}
		if (strcmp(m.name, "dst") == 0) {
			continue;
		}
		void* ptr = ptr_add(inst, m.offset);
		auto any = make_any(m.type, ptr);
		print("%", any);
	}
	println();
}

void print_ssa_block(SsaBasicBlock* block) {
	println("%", block->name);
	for (auto inst: block->insts) {
		print("  ");
		print_ssa_inst(inst);
	}
	println();
	for (auto succ: block->successors) {
		print_ssa_block(succ);
	}
}

Tuple<Ssa, Error*> emit_function_ssa(Allocator allocator, AstFunction* f) {
	Ssa ssa;
	ssa.allocator = make_arena_allocator(allocator, 1024 * 1024);
	if (!f->block) {
		return { {}, format_error("Function % has no body", f->name) };
	}
	auto entry_block = make_ssa_basic_block(&ssa);
	defer { 
		print_ssa_block(entry_block);
	};
	auto e = emit_block(entry_block, f->block);
	if (e) {
		return { {}, e };
	}
	return { ssa, NULL };
}
