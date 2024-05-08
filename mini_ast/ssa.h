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


enum class SsaOp: s64 {
	Nop        = 0,
	Call,
	LocalVar,
	Phi,
	Store,
	Modify,
	Load,
	LoadIndir,
	Loadi64Const,
	Loadf32Const,
	Loadf64Const,
	Div,
	Mul,
	Add,
	Sub,
	Subscript,
	Swizzle,
	LoadLocalSlice,
};
REFLECT(SsaOp) {
	ENUM_VALUE(Nop);
	ENUM_VALUE(Call);
	ENUM_VALUE(LocalVar);
	ENUM_VALUE(Phi);
	ENUM_VALUE(Store);
	ENUM_VALUE(Modify);
	ENUM_VALUE(Load);
	ENUM_VALUE(LoadIndir);
	ENUM_VALUE(Loadi64Const);
	ENUM_VALUE(Loadf32Const);
	ENUM_VALUE(Loadf64Const);
	ENUM_VALUE(Div);
	ENUM_VALUE(Mul);
	ENUM_VALUE(Add);
	ENUM_VALUE(Sub);
	ENUM_VALUE(Subscript);
	ENUM_VALUE(Swizzle);
	ENUM_VALUE(LoadLocalSlice);
}

struct Ssa;
struct SsaBasicBlock;

struct SsaReg {
	s64 v = 0;

	bool operator==(SsaReg rhs) {
		return v == rhs.v;
	}
};

constexpr SsaReg SSA_NO_REG = { .v = 0 };

struct SsaInst {
	SsaBasicBlock*  block;
	SsaOp           op = SsaOp::Nop;
	SsaReg          dst = SSA_NO_REG;
	Array<SsaReg>   args;
	Array<Any>      any_args;
};

struct SsaBasicBlock {
	String                name;
	Ssa*                  ssa;
	bool                  is_sealed = false;
	Array<SsaInst*>       insts;
	Array<SsaInst*>       incomplete_phis;
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

SsaInst* make_ssa_inst(SsaBasicBlock* block, SsaOp op) {
	SsaInst* inst = make<SsaInst>(block->ssa->allocator);
	inst->block = block;
	inst->op = op;
	inst->args = { .allocator = block->ssa->allocator };
	inst->any_args = { .allocator = block->ssa->allocator };
	return inst;
}

void write_var(SsaBasicBlock* block, AstVar* var, SsaReg reg) {
	put(&block->var_map, var, reg);
}

SsaReg read_var(SsaBasicBlock* block, AstVar* var);

void add_phi_args(SsaInst* inst, AstVar* var) {
	for (auto pred: inst->block->pred) {
		SsaReg reg = read_var(pred, var);
		if (reg == SSA_NO_REG) {
			panic("Unknown variable: %", var->name);
		}
		add(&inst->args, reg);
		add(&inst->any_args, make_any(pred));
	}
}

SsaReg read_var_recursive(SsaBasicBlock* block, AstVar* var) {
	SsaReg reg = SSA_NO_REG;
	if (!block->is_sealed) {
		auto phi = make_ssa_inst(block, SsaOp::Phi);
		phi->dst = alloc_reg(block->ssa);
		add(&phi->any_args, make_any(var));
		add(&block->incomplete_phis, phi);
		reg = phi->dst;
	} else if (len(block->pred) == 1) {
		reg = read_var(block->pred[0], var);
	} else {
		auto phi = make_ssa_inst(block, SsaOp::Phi);
		phi->dst = alloc_reg(block->ssa);
		write_var(block, var, phi->dst);
		add_phi_args(phi, var);
		reg = phi->dst;
	}
	assert(reg != SSA_NO_REG);
	write_var(block, var, reg);
	return reg;
}

void seal_block(SsaBasicBlock* block) {
	for (auto phi: block->incomplete_phis) {
		add_phi_args(phi, (AstVar*) phi->any_args[0].ptr);
		remove(&block->incomplete_phis, phi);
		remove_at_index(&phi->any_args, 0);
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

void commit_inst(SsaInst* inst) {
	add(&inst->block->insts, inst);
}

struct SsaExpr {
	s64      reg = 0;
	bool     is_lvalue = false;
	bool     is_addr = false;
	AstVar*  var = NULL;
	s64      var_offset = 0;

	REFLECT(SsaExpr) {
		MEMBER(reg);
		MEMBER(is_lvalue);
		MEMBER(is_addr);
		MEMBER(var);
		MEMBER(var_offset);
	}
};

SsaExpr* make_ssa_expr(SsaBasicBlock* block, s64 reg, bool is_lvalue, bool is_addr) {
	auto expr = make<SsaExpr>(block->ssa->allocator);
	expr->reg = reg;
	expr->is_lvalue = is_lvalue;
	return expr;
}

Tuple<SsaExpr*, Error*> emit_store(SsaBasicBlock* block, AstExpr* expr, SsaExpr* dst, s64 src) { 
	if (!dst->is_lvalue) {
		return { NULL, format_error("Left hand side of assignment must be an lvalue: %*", expr->type->name) };
	}
	if (dst->var) {
		if (dst->var->is_global) {
			auto inst = make_ssa_inst(block, SsaOp::Store);
			inst->dst = alloc_reg(block->ssa);
			add(&inst->node_args, dst->var);
			add(&inst->args, dst->var_offset);
			add(&inst->args, src);
			commit_inst(inst);
			return { make_ssa_expr(block, inst->dst, false, false), NULL };
		} else {
			auto inst = make_ssa_inst(block, SsaOp::Modify);
			inst->dst = alloc_reg(block->ssa);
			add(&inst->args, dst->reg);
			add(&inst->args, src);
			add(&inst->args, dst->var_offset);
			commit_inst(inst);
			write_var(block, dst->var, src);
			return { make_ssa_expr(block, inst->dst, false, false), NULL };
		}
		return { NULL, format_error("Unexpected var for assignment at %", expr->text_region) };
	}
	if (dst->is_addr) {
		auto inst = make_ssa_inst(block, SsaOp::Store);
		add(&inst->args, dst->reg);
		add(&inst->args, src);
		commit_inst(inst);
		auto ssa_expr = make_ssa_expr(block, src, false, false);
		return { ssa_expr, NULL };
	}
	println("%*+", dst);
	return { NULL, format_error("Unexpected lvalue for assignment at %", expr->text_region) };
}

struct SsaLvalue {
	AstVar* var = NULL;
	s64     var_offset_reg = 0;
	s64     reg = 0;
	s64     index = 0;
	bool    is_addr = false;
};

Tuple<SsaLvalue, Error*> emit_lvalue(SsaBasicBlock* block, AstExpr* expr) {
	assert(expr->is_lvalue);
	

	if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		return { SsaLvalue { var_access->var }, NULL };
	} else if (auto arr_access = reflect_cast<AstArrayAccess>(expr)) {
		auto [lhs, e] = emit_lvalue(block, arr_access->lhs);
		if (e) {
			return { {}, e };
		}
		return { SsaLvalue { arr_access->lhs }, NULL };
	} else {
		println("%*+", expr);
		return { {}, format_error("Unsupported expression: %*", expr->type->name) };
	}
}

Tuple<SsaExpr*, Error*> emit_load_lvalue(SsaBasicBlock* block, SsaExpr* expr, AstType* type) {
	if (!expr->is_lvalue) {
		return { NULL, format_error("Left hand side of assignment must be an lvalue") };
	}
	if (expr->var) {
		if (expr->var->is_global) {
			auto inst = make_ssa_inst(block, SsaOp::Load);
			inst->dst = alloc_reg(block->ssa);
			add(&inst->node_args, expr->var);
			add(&inst->args, expr->var_offset);
			commit_inst(inst);
			auto ssa_expr = make_ssa_expr(block, inst->dst, false, false);
			return { ssa_expr, NULL };
		} else {
			auto reg = read_var(block, expr->var);
			if (reg == -1) {
				return { NULL, format_error("Unknown variable: %", expr->var->name) };
			}
			return { make_ssa_expr(block, reg, true, false), NULL };
		}
	}
	if (expr->is_addr) {
		auto inst = make_ssa_inst(block, SsaOp::LoadIndir);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->args, expr->reg);
		commit_inst(inst);
		auto ssa_expr = make_ssa_expr(block, inst->dst, true, false); 
		return { ssa_expr, NULL };
	}
	return { NULL, format_error("Unsupported expression: %", type->name) };
}

Tuple<SsaExpr*, Error*> emit_binary_op(SsaBasicBlock* block, AstOperator* op, s64 lhs, s64 rhs) {
	SsaOp ssa_op = SsaOp::Nop;
	if (op->op == "/") {
		ssa_op = SsaOp::Div;
	} else if (op->op == "*") {
		ssa_op = SsaOp::Mul;
	} else if (op->op == "+") {
		ssa_op = SsaOp::Add;
	} else if (op->op == "-") {
		ssa_op = SsaOp::Sub;
	} else {
		return { NULL, format_error("Unsupported binary op: %", op->op) };
	}
	auto inst = make_ssa_inst(block, ssa_op);
	inst->dst = alloc_reg(block->ssa);
	add(&inst->args, lhs);
	add(&inst->args, rhs);
	commit_inst(inst);
	return { make_ssa_expr(block, inst->dst, false, false), NULL };
}

Tuple<SsaExpr*, Error*> emit_expr(SsaBasicBlock* block, AstExpr* expr) {
	if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		auto inst = make_ssa_inst(block, SsaOp::Call);
		for (auto arg: call->args) {
			auto [var, e] = emit_expr(block, arg);
			if (e) {
				return { NULL, e };
			}
			add(&inst->args, var->reg);
		}
		add(&inst->node_args, call->f);
		inst->dst = alloc_reg(block->ssa);
		commit_inst(inst);
		return { make_ssa_expr(block, inst->dst, false, false), NULL };
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
			auto [ssa_expr, e1] = emit_store(block, expr, lhs, rhs->reg);
			return { rhs, e1 };
		} else if (binary_expr->op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			auto [lhs, e] = emit_lvalue(block, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto [loaded, e2] = emit_load_lvalue(block, lhs, binary_expr->lhs->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			auto [ssa_expr, e3] = emit_binary_op(block, binary_expr->pure_op, loaded->reg, rhs->reg);
			if (e3) {
				return { NULL, e3 };
			}
			return { ssa_expr, NULL };
		} else {
			auto [lhs, e] = emit_expr(block, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto [res, e2] = emit_binary_op(block, binary_expr->op, lhs->reg, rhs->reg);
			if (e2) {
				return { NULL, e2 };
			}
			return { res, NULL };
		}
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		if (var_access->var) {
			if (var_access->var->is_global) {
				auto inst = make_ssa_inst(block, SsaOp::Load);
				inst->dst = alloc_reg(block->ssa);
				add(&inst->node_args, var_access->var);
				commit_inst(inst);
				auto ssa_expr = make_ssa_expr(block, inst->dst, false, false);
				return { ssa_expr, NULL };
			} else {
				auto reg = read_var(block, var_access->var);
				if (reg == -1) {
					return { NULL, format_error("Unknown variable: %", var_access->var->name) };
				}
				return { make_ssa_expr(block, reg, false, false), NULL };
			}
		}
		return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
	} else if (auto literal_expr = reflect_cast<LiteralExpr>(expr)) {
		SsaOp op;
		s64   arg;
		if (literal_expr->expr_type->name == U"int"_b) {
			op = SsaOp::Loadi64Const;
			arg = literal_expr->s64_value;
		} else if (literal_expr->expr_type->name == U"float"_b) {
			op = SsaOp::Loadf32Const;
			arg = bitcast<u32>(literal_expr->f32_value);
		} else if (literal_expr->expr_type->name == U"double"_b) {
			op = SsaOp::Loadf64Const;
			arg = bitcast<u64>(literal_expr->f64_value);
		} else {
			return { NULL, format_error("Unsupported literal type: %", literal_expr->expr_type->name) };
		}
		auto inst = make_ssa_inst(block, op);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->args, arg);
		commit_inst(inst);
		auto ssa_expr = make_ssa_expr(block, inst->dst, false, false);
		return { ssa_expr, NULL };
	} else if (auto arr_access = reflect_cast<AstArrayAccess>(expr)) {
		auto [lhs, e0] = emit_expr(block, arr_access->lhs);
		if (e0) {
			return { NULL, e0 };
		}
		auto [rhs, e1] = emit_expr(block, arr_access->index);
		if (e1) {
			return { NULL, e1 };
		}
		auto inst = make_ssa_inst(block, SsaOp::Subscript);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->args, lhs->reg);
		add(&inst->args, rhs->reg);
		commit_inst(inst);
		auto ssa_expr = make_ssa_expr(block, inst->dst, false, false);
		return { ssa_expr, NULL };
	} else if (auto swizzle = reflect_cast<AstSwizzleExpr>(expr)) {
		auto [lhs, e0] = emit_expr(block, swizzle->lhs);
		if (e0) {
			return { NULL, e0 };
		}
		auto inst = make_ssa_inst(block, SsaOp::Swizzle);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->args, lhs->reg);
		add(&inst->args, swizzle->swizzle_len);
		for (auto i: range(swizzle->swizzle_len)) {
			add(&inst->args, swizzle->swizzle[i]);
		}
		commit_inst(inst);
		auto ssa_expr = make_ssa_expr(block, inst->dst, false, false);
		return { ssa_expr, NULL };
	} else if (auto postfix_expr = reflect_cast<CPostfixExpr>(expr)) {
		auto [lvalue, eo] = emit_lvalue(block, postfix_expr->lhs);
		if (eo) {
			return { NULL, eo };
		}
		auto [loaded, e2] = emit_load_lvalue(block, lvalue, postfix_expr->lhs->expr_type);
		if (e2) {
			return { NULL, e2 };
		}

		SsaOp op;
		if (postfix_expr->op == "++") {
			op = SsaOp::Add;
		} else if (postfix_expr->op == "--") {
			op = SsaOp::Sub;
		} else {
			return { NULL, format_error("Unsupported postfix operator: %", postfix_expr->op) };
		}
		auto const_load = make_ssa_inst(block, SsaOp::Loadi64Const);
		const_load->dst = alloc_reg(block->ssa);
		add(&const_load->args, 1);
		commit_inst(const_load);
		auto inst = make_ssa_inst(block, op);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->args, loaded->reg);
		add(&inst->args, const_load->dst);
		commit_inst(inst);
		return { make_ssa_expr(block, loaded->reg, false, false), NULL };
	} else if (auto access = reflect_cast<AstVarMemberAccess>(expr)) {
		auto [lhs, e] = emit_expr(block, access->lhs);
		if (e) {
			return { NULL, e };
		}
		auto const_load = make_ssa_inst(block, SsaOp::LoadLocalSlice);
		const_load->dst = alloc_reg(block->ssa);
		add(&const_load->args, lhs->reg);
		add(&const_load->args, access->member.offset);
		commit_inst(const_load);
		return { make_ssa_expr(block, const_load->dst, false, false), NULL };
	} else {
		return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
	}
}

Error* emit_stmt(SsaBasicBlock* block, AstNode* stmt) {
	if (auto expr = reflect_cast<AstExpr>(stmt)) {
		auto [var, e] = emit_expr(block, expr);
		if (e) {
			return e;
		}
		return NULL;
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		auto first = var_decl_group->var_decls[0];
		s64 init_reg = 0;
		if (first->init) {
			auto [var, e] = emit_expr(block, first->init);
			if (e) {
				return e;
			}
			init_reg = var->reg;
		}
		for (auto var_decl: var_decl_group->var_decls) {
			s64 var_reg;
			if (init_reg) {
				var_reg = init_reg;
			} else {
				auto inst = make_ssa_inst(block, SsaOp::LocalVar);
				inst->dst = alloc_reg(block->ssa);
				add(&inst->node_args, var_decl);
				commit_inst(inst);
				var_reg = inst->dst;
			}
			write_var(block, var_decl, var_reg);
		}
		return NULL;
	} else if (auto var_decl = reflect_cast<AstVarDecl>(stmt)) {
		s64 reg;
		if (var_decl->init) {
			auto [var, e] = emit_expr(block, var_decl->init);
			if (e) {
				return e;
			}
			reg = var->reg;
		} else {
			auto inst = make_ssa_inst(block, SsaOp::LocalVar);
			inst->dst = alloc_reg(block->ssa);
			add(&inst->node_args, var_decl);
			commit_inst(inst);
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

void print_ast_inst(SsaInst* inst) {
	if (inst->dst) {
		print("% = ", inst->dst);		
	}
	print("% ", inst->op);
	for (auto arg: inst->args) {
		print("% ", arg);
	}
	for (auto arg: inst->node_args) {
		if (auto var = reflect_cast<AstVar>(arg)) {
			print("%:%* ", var->name, var->var_type->name);
		} else if (auto symbol = reflect_cast<AstSymbol>(arg)) {
			print("% ", symbol->name);
		} else if (auto literal_expr = reflect_cast<LiteralExpr>(arg)) {
			if (literal_expr->expr_type->name == U"float"_b) {
				print("% ", literal_expr->f32_value);
			} else if (literal_expr->expr_type->name == U"double"_b) {
				print("% ", literal_expr->f64_value);
			} else if (literal_expr->expr_type->name == U"int"_b) {
				print("% ", literal_expr->s64_value);
			} else if (literal_expr->expr_type->name == U"bool"_b) {
				print("% ", (bool) literal_expr->u64_value);
			} else {
				print("Unknown literal type: %", literal_expr->expr_type->name);
			}
		} else {
			print("%* ", arg->type->name);
		}
	}
	println();
}

void print_ssa_block(SsaBasicBlock* block) {
	println("%", block->name);
	for (auto inst: block->insts) {
		print("  ");
		print_ast_inst(inst);
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
