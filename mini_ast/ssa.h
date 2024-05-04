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
	Call       = 1,
	LocalVar   = 2,
	Phi        = 3,
	Store      = 4,
	Modify     = 5,
	Load       = 6,
	LoadIndir  = 7,
	Div        = 8,
	Mul        = 9,
	Add        = 10,
	Sub        = 11,
	LoadConst  = 12,
	Subscript  = 13,
	Swizzle    = 14,
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
	ENUM_VALUE(Div);
	ENUM_VALUE(Mul);
	ENUM_VALUE(Add);
	ENUM_VALUE(Sub);
	ENUM_VALUE(LoadConst);
	ENUM_VALUE(Subscript);
	ENUM_VALUE(Swizzle);
}

struct Ssa;
struct SsaBasicBlock;

struct SsaInst {
	SsaBasicBlock*  block = NULL;
	SsaOp           op = SsaOp::Nop;
	s64             dst = 0;
	Array<s64>      args;
	Array<AstNode*> node_args;
};

struct SsaBasicBlock {
	String                name;
	Ssa*                  ssa;
	bool                  is_sealed = false;
	Array<SsaInst*>       insts;
	Array<SsaInst*>       incomplete_phis;
	Array<SsaBasicBlock*> pred;
	HashMap<AstVar*, s64> var_map;
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

s64 alloc_reg(Ssa* ssa) {
	return ++ssa->reg_counter;
}

SsaBasicBlock* make_ssa_basic_block(Ssa* ssa, String name = ""_b) {
	SsaBasicBlock* block = make<SsaBasicBlock>(ssa->allocator);
	block->name = name;
	block->ssa = ssa;
	block->insts = { .allocator = ssa->allocator };
	block->incomplete_phis = { .allocator = ssa->allocator };
	block->pred = { .allocator = ssa->allocator };
	block->var_map = { .allocator = ssa->allocator };
	return block;
}

void write_var(SsaBasicBlock* block, AstVar* var, s64 reg) {
	put(&block->var_map, var, reg);
}

s64 read_var_recursive(SsaBasicBlock* block, AstVar* var) {
	if (!block->is_sealed) {
		
	}
}

s64 read_var(SsaBasicBlock* block, AstVar* var) {
	assert(var->is_global);
	auto found = get(&block->var_map, var);
	if (found) {
		return *found;
	}

}

SsaInst* make_ssa_inst(SsaBasicBlock* block, SsaOp op) {
	SsaInst* inst = make<SsaInst>(block->ssa->allocator);
	inst->block = block;
	inst->op = op;
	inst->args = { .allocator = block->ssa->allocator };
	inst->node_args = { .allocator = block->ssa->allocator };
	return inst;
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
			put(&block->ssa->var_map, dst->var, src);
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

Tuple<SsaExpr*, Error*> emit_lvalue(SsaBasicBlock* block, AstExpr* expr) {
	if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		auto var_expr = make_ssa_expr(block, 0, true, false);
		var_expr->var = var_access->var;
		return { var_expr, NULL };
	} else {
		println("%*+", expr);
		return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
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
			auto var = get(&block->ssa->var_map, expr->var);
			if (!var) {
				return { NULL, format_error("Unknown variable: %", expr->var->name) };
			}
			return { make_ssa_expr(block, *var, true, false), NULL };
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
				auto var = get(&block->ssa->var_map, var_access->var);
				if (!var) {
					return { NULL, format_error("Unknown variable: %", var_access->var->name) };
				}
				return { make_ssa_expr(block, *var, false, false), NULL };
			}
		}
		return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
	} else if (auto literal_expr = reflect_cast<LiteralExpr>(expr)) {
		auto inst = make_ssa_inst(block, SsaOp::LoadConst);
		inst->dst = alloc_reg(block->ssa);
		add(&inst->node_args, literal_expr);
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
			put(&block->ssa->var_map, var_decl, var_reg);
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
		put(&block->ssa->var_map, var_decl, reg);
		return NULL;
	} else if (auto _for = reflect_cast<AstFor>(stmt)) {

	} else {
		return format_error("Unsupported statement: %*", stmt->type->name);
	}
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

Tuple<Ssa, Error*> emit_function_ssa(Allocator allocator, AstFunction* f) {
	Ssa ssa;
	ssa.allocator = make_arena_allocator(allocator, 1024 * 1024);
	if (!f->block) {
		return { {}, format_error("Function % has no body", f->name) };
	}
	auto entry_block = make_ssa_basic_block(&ssa);
	defer { 
		for (auto inst: entry_block->insts) {
			print_ast_inst(inst);
		}
	};
	for (auto stmt: f->block->statements) {
		auto e = emit_stmt(entry_block, stmt);
		if (e) {
			return { {}, e };
		}
	}
	return { ssa, NULL };
}
