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

struct SsaId { 
	s64 v = 0;
};

enum class SsaOp {
	Nop = 0,
	Phi,
	Add,
	Sub,
	Mul,
	Div,
	Less,
	Greater,
	Const,
	Call,
	GlobalVar,
	MemberAccess,
	Swizzle,
	Store,
	Load,
	Alloca,
	Jump,
	CondJump,
	Return,
	GetElement,
	UnaryNeg,
	UnaryNot,
};
REFLECT(SsaOp) {
	ENUM_VALUE(Nop);
	ENUM_VALUE(Phi);
	ENUM_VALUE(Add);
	ENUM_VALUE(Sub);
	ENUM_VALUE(Mul);
	ENUM_VALUE(Div);
	ENUM_VALUE(Less);
	ENUM_VALUE(Greater);
	ENUM_VALUE(Const);
	ENUM_VALUE(Call);
	ENUM_VALUE(GlobalVar);
	ENUM_VALUE(MemberAccess);
	ENUM_VALUE(Swizzle);
	ENUM_VALUE(Store);
	ENUM_VALUE(Load);
	ENUM_VALUE(Alloca);
	ENUM_VALUE(Jump);
	ENUM_VALUE(CondJump);
	ENUM_VALUE(Return);
	ENUM_VALUE(GetElement);
	ENUM_VALUE(UnaryNeg);
	ENUM_VALUE(UnaryNot);
}

struct SsaBasicBlock;
struct Ssa;

struct SsaValue {
	SsaId            id;
	SsaOp            op = SsaOp::Nop;
	SsaBasicBlock*   block = NULL;
	AstType*         type = NULL;
	Array<SsaValue*> args;
	Any              aux;
};

struct SsaBasicBlock {
	String                      name;
	Ssa*                        ssa;
	SsaValue*                   ending = NULL;
	bool                        is_sealed = false;
	Array<SsaValue*>            values;
	Array<SsaValue*>            incomplete_phis;
	Array<SsaBasicBlock*>       pred;
	Array<SsaBasicBlock*>       successors;
	HashMap<AstVar*, SsaValue*> var_map;

	REFLECT(SsaBasicBlock) {
		MEMBER(name);
		MEMBER(ssa);
		MEMBER(ending);
		MEMBER(is_sealed);
		MEMBER(values);
		MEMBER(incomplete_phis);
		MEMBER(pred);
		MEMBER(successors);
		MEMBER(var_map);
	}
};

struct Ssa {
	Allocator             allocator = c_allocator;
	SsaBasicBlock*        entry = NULL;
	s64                   reg_counter = 0;
	SsaBasicBlock*        current_block = NULL;
	s64                   construct_id_counter = 0;

	void free() {
		free_allocator(allocator);
	}
};

SsaId alloc_id(Ssa* ssa) {
	return SsaId { ++ssa->reg_counter };
}

s64 alloc_construct_id(Ssa* ssa) {
	return ++ssa->construct_id_counter;
}

SsaBasicBlock* make_ssa_basic_block(Ssa* ssa, String name = ""_b, s64 construct_id = 0) {
	SsaBasicBlock* block = make<SsaBasicBlock>(ssa->allocator);
	if (construct_id > 0) {
		block->name = sprint(ssa->allocator, "%_%", construct_id, name);
	} else {
		block->name = name;
	}
	block->ssa = ssa;
	block->values = { .allocator = ssa->allocator };
	block->incomplete_phis = { .allocator = ssa->allocator };
	block->pred = { .allocator = ssa->allocator };
	block->successors = { .allocator = ssa->allocator };
	block->var_map = { .allocator = ssa->allocator };
	return block;
}

void write_var(SsaBasicBlock* block, AstVar* var, SsaValue* v) {
	put(&block->var_map, var, v);
}

SsaValue* read_var(SsaBasicBlock* block, AstVar* var);

SsaValue* make_ssa_val(SsaBasicBlock* block, SsaOp op) {
	auto v = make<SsaValue>(block->ssa->allocator);
	v->block = block;
	v->op = op;
	v->id = alloc_id(block->ssa);
	if (block->ending) {
		add(&block->values, v, len(block->values) - 1);
	} else {
		add(&block->values, v);
	}
	return v;
}
void add_phi_args(SsaValue* phi) {
	assert(len(phi->args) == 0);
	auto var = reflect_cast<AstVar>(phi->aux);
	if (!var) {
		panic("Expected var");
	}
	for (auto pred: phi->block->pred) {
		auto v = read_var(pred, var);
		assert(v);
		add(&phi->args, v);
	}
	phi->op = SsaOp::Phi;
}

SsaValue* read_var_recursive(SsaBasicBlock* block, AstVar* var) {
	SsaValue* v = NULL;
	if (!block->is_sealed) {
		auto phi = make_ssa_val(block, SsaOp::Phi);
		phi->aux = make_any(var);
		add(&block->incomplete_phis, phi);
		v = phi;
	} else if (len(block->pred) == 1) {
		assert(block->pred[0]->ending);
		v = read_var(block->pred[0], var);
	} else {
		auto phi = make_ssa_val(block, SsaOp::Phi);
		phi->aux = make_any(var);
		write_var(block, var, phi);
		add_phi_args(phi);
		v = phi;
	}
	assert(v != NULL);
	write_var(block, var, v);
	return v;
}

void seal_block(SsaBasicBlock* block) {
	for (auto phi: block->incomplete_phis) {
		add_phi_args(phi);
	}
	clear(&block->incomplete_phis);
	block->is_sealed = true;
}

void add_pred_inner(SsaBasicBlock* block, SsaBasicBlock* pred) {
	if (block->is_sealed) {
		panic("Cannot add pred to sealed block");
	}
	if (pred->ending == NULL) {
		panic("Cannot add unfilled pred");
	}
	add(&block->pred, pred);
	add(&pred->successors, block);
}

void end_block_jump(SsaBasicBlock* block, SsaBasicBlock* target) {
	assert(block->ending == NULL);
	auto v = make_ssa_val(block, SsaOp::Jump);
	v->aux = make_any(target);
	block->ending = v;
	add_pred_inner(target, block);
}

void end_block_ret(SsaBasicBlock* block, SsaValue* ret) {
	assert(block->ending == NULL);
	block->ending = make_ssa_val(block, SsaOp::Return);
	if (ret) {
		add(&block->ending->args, ret);
	}
}

struct SsaCondJump {
	SsaBasicBlock* t_block = NULL;
	SsaBasicBlock* f_block = NULL;

	REFLECT(SsaCondJump) {
		MEMBER(t_block);
		MEMBER(f_block);
	}
};

void end_block_cond_jump(SsaBasicBlock* block, SsaValue* cond, SsaBasicBlock* t_block, SsaBasicBlock* f_block) {
	assert(block->ending == NULL);
	auto v = make_ssa_val(block, SsaOp::CondJump);
	add(&v->args, cond);
	auto cj = make<SsaCondJump>(block->ssa->allocator);
	cj->t_block = t_block;
	cj->f_block = f_block;
	v->aux = make_any(cj);
	block->ending = v;
	add_pred_inner(t_block, block);
	add_pred_inner(f_block, block);
}

SsaValue* read_var(SsaBasicBlock* block, AstVar* var) {
	auto found = get(&block->var_map, var);
	if (found) {
		return *found;
	} else {
		return read_var_recursive(block, var);
	}
	return NULL;
}

template <typename T>
SsaValue* load_const(Ssa* ssa, T value) {
	Type* type = reflect_type_of<T>();
	auto inst = make_ssa_val(ssa->current_block, SsaOp::Const);
	// @MemoryLeak
	auto copied = copy(ssa->allocator, value);
	inst->aux = make_any(type, copied);
	return inst;
}

Tuple<SsaValue*, Error*> lvalue(Ssa* ssa, AstExpr* expr) {
	if (!expr->is_lvalue) {
		return { NULL, format_error("Expected lvalue, got %*", expr->type->name) };
	}

	if (auto var_access = reflect_cast<AstVarMemberAccess>(expr)) {
		auto [lhs, e] = lvalue(ssa, var_access->lhs);
		if (e) {
			return { NULL, e };
		}
		auto v = make_ssa_val(ssa->current_block, SsaOp::MemberAccess);
		add(&v->args, lhs);
		v->aux = make_any(var_access->member);
		return { v, NULL };
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		if (var_access->var->is_global) {
			auto v = make_ssa_val(ssa->current_block, SsaOp::GlobalVar);
			v->aux = make_any(var_access->var);
			return { v, NULL };
		}
		auto v = read_var(ssa->current_block, var_access->var);
		if (!v) {
			return { NULL, format_error("Unknown variable: %*", var_access->var->type->name) };
		}
		return { v, NULL };
	} else if (auto sw = reflect_cast<AstSwizzleExpr>(expr)) {
		auto [lhs, e] = lvalue(ssa, sw->lhs);
		if (e) {
			return { NULL, e };
		}
		auto v = make_ssa_val(ssa->current_block, SsaOp::Swizzle);
		add(&v->args, lhs);
		v->aux = make_any(sw);
		return { v, NULL };
	}
	return { NULL, format_error("Unsupported lvalue: %*", expr->type->name) };
}

Tuple<SsaValue*, Error*> load(Ssa* ssa, SsaValue* lhs) {
	auto v = make_ssa_val(ssa->current_block, SsaOp::Load);
	add(&v->args, lhs);
	return { v, NULL };
}

SsaValue* store(Ssa* ssa, SsaValue* lhs, SsaValue* rhs) {
	auto v = make_ssa_val(ssa->current_block, SsaOp::Store);
	add(&v->args, lhs);
	add(&v->args, rhs);
	return v;
}

SsaValue* ssa_alloca(Ssa* ssa, u64 size) {
	auto v = make_ssa_val(ssa->current_block, SsaOp::Alloca);
	add(&v->args, load_const(ssa, size));
	return v;
}

Tuple<SsaValue*, Error*> emit_binary_op(Ssa* ssa, UnicodeString op, SsaValue* lhs, SsaValue* rhs) {
	SsaOp ssa_op = SsaOp::Nop;
	if (op == "+") {
		ssa_op = SsaOp::Add;
	} else if (op == "-") {
		ssa_op = SsaOp::Sub;
	} else if (op == "*") {
		ssa_op = SsaOp::Mul;
	} else if (op == "/") {
		ssa_op = SsaOp::Div;
	} else if (op == "<") {
		ssa_op = SsaOp::Less;
	} else if (op == ">") {
		ssa_op = SsaOp::Greater;
	} else if (op == ",") {
		return { rhs, NULL };
	} else {
		return { NULL, format_error("Unsupported binary operator: %", op) };
	}
	auto v = make_ssa_val(ssa->current_block, ssa_op);
	add(&v->args, lhs);
	add(&v->args, rhs);
	return { v, NULL };
}

Tuple<SsaValue*, Error*> emit_unary_op(Ssa* ssa, UnicodeString op, SsaValue* lhs) {
	SsaOp ssa_op = SsaOp::Nop;
	if (op == "+") {
		return { lhs, NULL };
	} else if (op == "-") {
		ssa_op = SsaOp::UnaryNeg;
	} else if (op == "!") {
		ssa_op = SsaOp::UnaryNot;
	} else {
		return { NULL, format_error("Unsupported unary operator: %", op) };
	}
	auto v = make_ssa_val(ssa->current_block, ssa_op);
	add(&v->args, lhs);
	return { v, NULL };
}

Tuple<SsaValue*, Error*> emit_expr(Ssa* ssa, AstExpr* expr) {
	if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		auto v = make_ssa_val(ssa->current_block, SsaOp::Call);
		v->aux = make_any(call->f);
		for (auto arg: call->args) {
			auto [arg_v, e] = emit_expr(ssa, arg);
			if (e) {
				return { NULL, e };
			}
			add(&v->args, arg_v);
		}
		return { v, NULL };
	} else if (auto binary_expr = reflect_cast<AstBinaryExpr>(expr)) {
		auto [rhs, e0] = emit_expr(ssa, binary_expr->rhs);
		if (e0) {
			return { NULL, e0 };
		}
		if (binary_expr->op->op == "=") {
			auto [lhs, e1] = lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto v = store(ssa, lhs, rhs);
			return { rhs, NULL };
		} else if (binary_expr->op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			auto [lhs, e1] = lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto [loaded, e2] = load(ssa, lhs);
			if (e2) {
				return { NULL, e2 };
			}
			auto [oped, e3] = emit_binary_op(ssa, binary_expr->pure_op->op, loaded, rhs);
			if (e3) {
				return { NULL, e3 };
			}
			store(ssa, lhs, oped);
			return { oped, NULL };
		} else {
			auto [lhs, e] = emit_expr(ssa, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto [oped, e2] = emit_binary_op(ssa, binary_expr->op->op, lhs, rhs);
			if (e2) {
				return { NULL, e2 };
			}
			return { oped, NULL };
		}
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		auto [lv, e] = lvalue(ssa, var_access);
		if (e) {
			return { NULL, e };
		}
		auto [loaded, e2] = load(ssa, lv);
		if (e2) {
			return { NULL, e2 };
		}
		return { loaded, NULL };
	} else if (auto literal_expr = reflect_cast<AstLiteralExpr>(expr)) {
		auto any = make_any(literal_expr->lit_type, &literal_expr->lit_value);
		auto inst = make_ssa_val(ssa->current_block, SsaOp::Const);
		inst->aux = any;
		return { inst, NULL };
	} else if (auto ternary = reflect_cast<AstTernary>(expr)) {
		auto construct_id = alloc_construct_id(ssa);

		auto cond_block = make_ssa_basic_block(ssa, "ternary_cond"_b, construct_id);
		auto pred = ssa->current_block;
		ssa->current_block = cond_block;
		auto [cond_value, e1] = emit_expr(ssa, ternary->cond);
		if (e1) {
			return { NULL, e1 };
		}
		seal_block(pred);
		end_block_jump(pred, cond_block);
		seal_block(cond_block);
		ssa->current_block = make_ssa_basic_block(ssa, "ternary_then"_b, construct_id);
		auto [then_expr, e2] = emit_expr(ssa, ternary->then);
		if (e2) {
			return { NULL, e2 };
		}
		auto then_block = ssa->current_block;
		ssa->current_block = make_ssa_basic_block(ssa, "ternary_else"_b, construct_id);
		auto [else_expr, e3] = emit_expr(ssa, ternary->else_);
		if (e3) {
			return { NULL, e3 };
		}
		auto else_block = ssa->current_block;
		end_block_cond_jump(cond_block, cond_value, then_block, else_block);
		seal_block(then_block);
		seal_block(else_block);

		ssa->current_block = make_ssa_basic_block(ssa, "ternary_after"_b, construct_id);
		end_block_jump(then_block, ssa->current_block);
		end_block_jump(else_block, ssa->current_block);

		auto phi = make_ssa_val(ssa->current_block, SsaOp::Phi);
		add(&phi->args, then_expr);
		add(&phi->args, else_expr);
		return { phi, NULL };
	} else if (auto init = reflect_cast<AstStructInitializer>(expr)) {
		auto slot = ssa_alloca(ssa, init->struct_type->size);
		for (auto m: init->members) {
			auto [expr, e] = emit_expr(ssa, m.expr);
			if (e) {
				return { NULL, e };
			}
			auto ptr = make_ssa_val(ssa->current_block, SsaOp::GetElement);
			add(&ptr->args, slot);
			add(&ptr->args, load_const(ssa, m.member.offset));
			store(ssa, ptr, expr);
		}
		return { slot, NULL };
	} else if (auto unary = reflect_cast<AstUnaryExpr>(expr)) {
		auto [lhs, e] = emit_expr(ssa, unary->expr);
		if (e) {
			return { NULL, e };
		}
		if (unary->op->op == "--" || unary->op->op == "++") {
			auto [loaded, e2] = load(ssa, lhs);
			if (e2) {
				return { NULL, e2 };
			}
			auto const_one = load_const(ssa, (s64) 1);
			auto [oped, e3] = emit_binary_op(ssa, U"+"_b, loaded, const_one);
			if (e3) {
				return { NULL, e3 };
			}
			store(ssa, lhs, oped);

			if (unary->op->flags & AST_OP_FLAG_POSTFIX) {
				return { loaded, NULL };
			} else {
				return { oped, NULL };
			}
		} else {
			auto [oped, e2] = emit_unary_op(ssa, unary->op->op, lhs);
			if (e2) {
				return { NULL, e2 };
			}
			return { oped, NULL };
		}
	} else {
		auto [lv, e] = lvalue(ssa, expr);
		if (e == NULL) {
			auto [loaded, e2] = load(ssa, lv);
			if (e2) {
				return { NULL, e2 };
			}
			return { loaded, NULL };
		}
	}
	return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
}

Error* emit_block(Ssa* ssa, AstBlock* block);

Error* emit_stmt(Ssa* ssa, AstNode* stmt) {
	if (auto expr = reflect_cast<AstExpr>(stmt)) {
		auto [var, e] = emit_expr(ssa, expr);
		if (e) {
			return e;
		}
		return NULL;
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		auto first = var_decl_group->var_decls[0];
		SsaValue* v = NULL;
		if (first->init) {
			auto [var, e] = emit_expr(ssa, first->init);
			if (e) {
				return e;
			}
			v = var;
		}
		for (auto var_decl: var_decl_group->var_decls) {
			auto slot = ssa_alloca(ssa, var_decl->type->size);
			write_var(ssa->current_block, var_decl, slot);
			if (v) {
				auto stored = store(ssa, slot, v);
				write_var(ssa->current_block, var_decl, stored);
			}
		}
		return NULL;
	} else if (auto var_decl = reflect_cast<AstVarDecl>(stmt)) {
		SsaValue* v = NULL;
		if (var_decl->init) {
			auto [var, e] = emit_expr(ssa, var_decl->init);
			if (e) {
				return e;
			}
			v = var;
		}
		auto var_v = make_ssa_val(ssa->current_block, SsaOp::Alloca);
		var_v->aux = make_any(var_decl);
		write_var(ssa->current_block, var_decl, var_v);
		if (v) {
			auto stored = store(ssa, var_v, v);
			write_var(ssa->current_block, var_decl, stored);
		}
		return NULL;
	} else if (auto _for = reflect_cast<AstFor>(stmt)) {
		auto construct_id = alloc_construct_id(ssa);

		seal_block(ssa->current_block);
		auto pred = ssa->current_block;
		// b_s = block start, b_e = block end
		auto init_b_s = make_ssa_basic_block(ssa, "for_init"_b, construct_id);
		ssa->current_block = init_b_s;
		auto [_, e0] = emit_expr(ssa, _for->init_expr);
		if (e0) {
			return e0;
		}
		auto init_b_e = ssa->current_block;
		seal_block(pred);
		end_block_jump(pred, init_b_s);
		seal_block(init_b_e);

		auto cond_b_s = make_ssa_basic_block(ssa, "for_cond"_b, construct_id);
		end_block_jump(init_b_e, cond_b_s);
		ssa->current_block = cond_b_s;
		auto [cond_value, e1] = emit_expr(ssa, _for->cond_expr);
		if (e1) {
			return e1;
		}
		auto cond_b_e = ssa->current_block;

		auto body_b_s = make_ssa_basic_block(ssa, "for_body"_b, construct_id);
		auto after_b_s = make_ssa_basic_block(ssa, "for_after"_b, construct_id);
		end_block_cond_jump(cond_b_e, cond_value, body_b_s, after_b_s);
		
		ssa->current_block = body_b_s;
		auto e3 = emit_block(ssa, _for->body);
		if (e3) {
			return e3;
		}
		auto body_b_e = ssa->current_block;

		auto incr_b_s = make_ssa_basic_block(ssa, "for_incr"_b, construct_id);
		end_block_jump(body_b_e, incr_b_s);

		ssa->current_block = incr_b_s;
		auto [xxxx, e2] = emit_expr(ssa, _for->incr_expr);
		if (e2) {
			return e2;
		}
		auto incr_b_e = ssa->current_block;
		end_block_jump(incr_b_e, cond_b_s);

		seal_block(body_b_e);
		seal_block(incr_b_e);
		seal_block(cond_b_e);

		ssa->current_block = after_b_s;
		return NULL;
	} else if (auto if_ = reflect_cast<AstIf>(stmt)) {
		auto construct_id = alloc_construct_id(ssa);

		seal_block(ssa->current_block);
		auto pred = ssa->current_block;
		// b_s = block start, b_e = block end
		auto cond_b_s = make_ssa_basic_block(ssa, "if_cond"_b, construct_id);
		ssa->current_block = cond_b_s;
		auto [cond_value, e1] = emit_expr(ssa, if_->cond);
		if (e1) {
			return e1;
		}
		seal_block(pred);
		end_block_jump(pred, cond_b_s);
		seal_block(cond_b_s);
		auto cond_b_e = ssa->current_block;

		auto t_b_s = make_ssa_basic_block(ssa, "if_then"_b, construct_id);
		auto else_b_s = make_ssa_basic_block(ssa, "if_else"_b, construct_id);
		end_block_cond_jump(cond_b_e, cond_value, t_b_s, else_b_s);

		ssa->current_block = t_b_s;
		auto e3 = emit_block(ssa, if_->then);
		if (e3) {
			return e3;
		}
		auto t_b_e = ssa->current_block;
		end_block_jump(t_b_e, else_b_s);

		if (if_->else_if) {
			ssa->current_block = else_b_s;
			auto e3 = emit_stmt(ssa, if_->else_if);
			if (e3) {
				return e3;
			}
		} else if (if_->else_block) {
			ssa->current_block = else_b_s;
			auto e3 = emit_block(ssa, if_->else_block);
			if (e3) {
				return e3;
			}
		}

	} else {
		return format_error("Unsupported statement: %*", stmt->type->name);
	}
}

Error* emit_block(Ssa* ssa, AstBlock* block) {
	for (auto stmt: block->statements) {
		auto e = emit_stmt(ssa, stmt);
		if (e) {
			return e;
		}
	}
	return NULL;
}

void print_ssa_value(SsaValue* inst) { 
	print("$% = ", inst->id.v);
	print("% ", inst->op);
	for (auto arg: inst->args) {
		print("$% ", arg->id.v);
	}
	if (inst->aux.ptr) {
		print(" %* ", inst->aux.type->name);
		if (auto var = reflect_cast<AstVar>(inst->aux)) {
			print(" '%' ", var->name);
		} else if (auto block = reflect_cast<SsaBasicBlock>(inst->aux)) {
			print(" %* ", block->name);
		} else if (auto cj = reflect_cast<SsaCondJump>(inst->aux)) {
			print(" %*, %* ", cj->t_block->name, cj->f_block->name);
		}
	}
}

void print_ssa_block(SsaBasicBlock* block, Array<SsaBasicBlock*>* printed_blocks) {
	if (contains(*printed_blocks, block)) {
		return;
	}
	add(printed_blocks, block);
	println("%", block->name);
	for (auto v: block->values) {
		print_ssa_value(v);
		println();
	}
	println();
	for (auto succ: block->successors) {
		print_ssa_block(succ, printed_blocks);
	}
}

Tuple<Ssa, Error*> emit_function_ssa(Allocator allocator, AstFunction* f) {
	Ssa ssa;
	ssa.allocator = make_arena_allocator(allocator, 1024 * 1024);
	if (!f->block) {
		return { {}, format_error("Function % has no body", f->name) };
	}
	auto entry_block = make_ssa_basic_block(&ssa);
	Array<SsaBasicBlock*> printed_blocks;
	printed_blocks.allocator = ssa.allocator;
	defer { printed_blocks.free(); };
	defer { 
		print_ssa_block(entry_block, &printed_blocks);
	};
	ssa.entry = entry_block;
	ssa.current_block = entry_block;
	auto e = emit_block(&ssa, f->block);
	if (e) {
		return { {}, e };
	}
	return { ssa, NULL };
}
