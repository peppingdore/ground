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

	bool operator==(SsaId other) const {
		return v == other.v;
	}

	REFLECT(SsaId) {
		MEMBER(v);
	}
};

Hash64 hash_key(SsaId id) {
	return hash64(id.v);
}

enum class SsaOp {
	Nop = 0,
	Phi,
	Add,
	Sub,
	Mul,
	Div,
	Less,
	Greater,
	Equal,
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
	ReturnVoid,
	GetElement,
	UnaryNeg,
	UnaryNot,
	Lvalue,
	ZeroInit,
	Addr,
	FunctionArg,
	SpvUniform,
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
	ENUM_VALUE(Equal);
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
	ENUM_VALUE(ReturnVoid);
	ENUM_VALUE(GetElement);
	ENUM_VALUE(UnaryNeg);
	ENUM_VALUE(UnaryNot);
	ENUM_VALUE(Lvalue);
	ENUM_VALUE(ZeroInit);
	ENUM_VALUE(Addr);
	ENUM_VALUE(FunctionArg);
	ENUM_VALUE(SpvUniform);
}

struct SsaBasicBlock;
struct Ssa;

struct SsaValue {
	SsaId            id;
	SsaOp            op = SsaOp::Nop;
	SsaBasicBlock*   block = NULL;
	AstType*         type = NULL;
	Array<SsaValue*> args;
	Array<SsaValue*> uses;
	Any              aux;
	AstType*         v_type = NULL;
	bool             is_removed = false; // @TODO: only for debugging. remove later.
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
	HashMap<AstVar*, SsaValue*> ssa_vars;

	REFLECT(SsaBasicBlock) {
		MEMBER(name);
		MEMBER(ssa);
		MEMBER(ending);
		MEMBER(is_sealed);
		MEMBER(values);
		MEMBER(incomplete_phis);
		MEMBER(pred);
		MEMBER(successors);
		MEMBER(ssa_vars);
	}
};

struct Ssa {
	Allocator                   allocator = c_allocator;
	SsaBasicBlock*              entry = NULL;
	s64                         reg_counter = 0;
	SsaBasicBlock*              current_block = NULL;
	s64                         construct_id_counter = 0;
	HashMap<AstVar*, SsaValue*> non_ssa_vars;
	AstFunction*                function = NULL;
	Array<SsaBasicBlock*>       blocks;
	bool                        is_rewriting = false;

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
	block->ssa_vars = { .allocator = ssa->allocator };
	add(&ssa->blocks, block);
	return block;
}

bool can_ssa(AstVar* var) {
	if (var->is_global) {
		return false;
	}
	if (reflect_cast<AstPrimitiveType>(var->var_type)) {
		return true;
	}
	if (reflect_cast<AstFunctionArg>(var)) {
		return true;
	}
	return false;
}

void write_var(SsaBasicBlock* block, AstVar* var, SsaValue* v) {
	if (can_ssa(var)) {
		put(&block->ssa_vars, var, v);
	} else {
		put(&block->ssa->non_ssa_vars, var, v);
	}
}

SsaValue* read_var(SsaBasicBlock* block, AstVar* var);

SsaValue* init_ssa_val(SsaBasicBlock* block, SsaOp op) {
	auto v = make<SsaValue>(block->ssa->allocator);
	v->block = block;
	v->op = op;
	v->id = alloc_id(block->ssa);
	v->args.allocator = block->ssa->allocator;
	v->uses.allocator = block->ssa->allocator;
	return v;
}

SsaValue* make_ssa_val(SsaBasicBlock* block, SsaOp op) {
	assert(!block->ssa->is_rewriting);
	auto v = init_ssa_val(block, op);
	if (op == SsaOp::Phi) {
		add(&block->values, v, 0);
	} else if (block->ending) {
		add(&block->values, v, len(block->values) - 1);
	} else {
		add(&block->values, v);
	}
	return v;
}

void use(SsaValue* by, SsaValue* v) {
	add(&v->uses, by);
}

void add_arg(SsaValue* v, SsaValue* arg) {
	use(v, arg);
	add(&v->args, arg);
}


struct SsaRewriteAddValue {
	SsaValue* v = NULL;
	SsaValue* before = NULL;
};

struct SsaRewriter {
	Ssa*                      ssa = NULL;
	Array<SsaRewriteAddValue> add_values;
	Array<SsaValue*>          to_remove;
};

SsaRewriter start_rewrite(Ssa* ssa) {
	assert(!ssa->is_rewriting);
	SsaRewriter r;
	r.ssa = ssa;
	ssa->is_rewriting = true;
	return r;
}

void finish_rewrite(SsaRewriter* r) {
	for (auto it: r->add_values) {
		for (auto i: range(len(it.v->block->values))) {
			if (it.v->block->values[i] == it.before) {
				println("add: %* op % idx: %", it.v->id.v, it.v->op, i);
				println("block name: %*", it.v->block->name);
				println("before: %* op %", it.before->id.v, it.before->op);
				add(&it.v->block->values, it.v, i);
				break;
			}
		}
	}
	for (auto v: r->to_remove) {
		assert(len(v->uses) == 0);
		println("remove: %* op %", v->id.v, v->op);
		for (auto arg: v->args) {
			for (s64 i = 0; i < len(arg->uses); i++) {
				if (arg->uses[i] == v) {
					remove_at_index(&arg->uses, i);
					i -= 1;
				}
			}
		}
		for (auto i: range(len(v->block->values))) {
			if (v->block->values[i] == v) {
				remove_at_index(&v->block->values, i);
				break;
			}
		}
	}
	r->ssa->is_rewriting = false;
}

SsaValue* make_rewrite_ssa_val_before(SsaRewriter* r, SsaBasicBlock* block, SsaOp op, SsaValue* before) {
	auto v = init_ssa_val(block, op);
	SsaRewriteAddValue x;
	x.v = v;
	x.before = before;
	add(&r->add_values, x);
	return v;
}

void replace_by(SsaRewriter* r, SsaValue* v, SsaValue* by) {
	for (auto entry: v->block->ssa_vars) {
		if (entry->value == v) {
			entry->value = by;
		}
	}
	for (auto use: v->uses) {
		if (use == v) {
			continue;
		}
		bool did_find = false;
		for (auto i: range(len(use->args))) {
			if (use->args[i] == v) {
				use->args[i] = by;
				add(&by->uses, use);
				did_find = true;
				// arg may be used more than one time in the same value,
				//  so we don't break here.
			}
		}
		// assert(did_find);
	}
	clear(&v->uses);
}

void remove_value(SsaRewriter* r, SsaValue* v) {
	v->is_removed = true;
	add(&r->to_remove, v);
}

SsaValue* try_remove_trivial_phi(SsaValue* phi) {
	assert(phi->op == SsaOp::Phi);
	SsaValue* same = NULL;
	for (auto arg: phi->args) {
		if (arg == same || arg == phi) {
			continue;
		}
		if (same) {
			return phi;
		}
		same = arg;
	}
	assert(same);
	// Array<SsaValue*> uses;
	// for (auto use: phi->uses) {
	// 	if (use == phi) {
	// 		continue;
	// 	}
	// 	if (!contains(uses, use)) {
	// 		add(&uses, use);
	// 	}
	// }
	
	auto rw = start_rewrite(phi->block->ssa);
	replace_by(&rw, phi, same);
	finish_rewrite(&rw);

	for (auto use: phi->uses) {
		if (use == phi) {
			continue;
		}
		if (use->op == SsaOp::Phi) {
			try_remove_trivial_phi(use);
		}
	}

	rw = start_rewrite(phi->block->ssa);
	remove_value(&rw, phi);
	finish_rewrite(&rw);

	return same;
}

SsaValue* add_phi_args(SsaValue* phi) {
	assert(phi->block->is_sealed);
	assert(len(phi->args) == 0);
	auto var = reflect_cast<AstVar>(phi->aux);
	if (!var) {
		panic("Expected var");
	}
	for (auto pred: phi->block->pred) {
		auto v = read_var(pred, var);
		assert(v);
		add_arg(phi, v);
	}
	return try_remove_trivial_phi(phi);
}

SsaValue* read_var_recursive(SsaBasicBlock* block, AstVar* var) {
	SsaValue* v = NULL;
	if (!block->is_sealed) {
		auto phi = make_ssa_val(block, SsaOp::Phi);
		phi->aux = make_any(var);
		phi->v_type = var->var_type;
		add(&block->incomplete_phis, phi);
		v = phi;
	} else if (len(block->pred) == 1) {
		assert(block->pred[0]->ending);
		v = read_var(block->pred[0], var);
	} else {
		auto phi = make_ssa_val(block, SsaOp::Phi);
		phi->aux = make_any(var);
		phi->v_type = var->var_type;
		write_var(block, var, phi);
		v = add_phi_args(phi);
	}
	assert(v != NULL);
	write_var(block, var, v);
	return v;
}

void seal_block(SsaBasicBlock* block) {
	block->is_sealed = true;
	for (auto phi: block->incomplete_phis) {
		assert(len(block->pred) > 0);
		add_phi_args(phi);
	}
	clear(&block->incomplete_phis);
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
		add_arg(block->ending, ret);
	}
}

void end_block_ret_void(SsaBasicBlock* block) {
	assert(block->ending == NULL);
	block->ending = make_ssa_val(block, SsaOp::ReturnVoid);
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
	add_arg(v, cond);
	auto cj = make<SsaCondJump>(block->ssa->allocator);
	cj->t_block = t_block;
	cj->f_block = f_block;
	v->aux = make_any(cj);
	block->ending = v;
	add_pred_inner(t_block, block);
	add_pred_inner(f_block, block);
}

SsaValue* read_var(SsaBasicBlock* block, AstVar* var) {
	if (can_ssa(var)) {
		auto found = get(&block->ssa_vars, var);
		if (found) {
			assert(!(*found)->is_removed);
			return *found;
		} else {
			return read_var_recursive(block, var);
		}
		return NULL;
	} else {
		auto v = get(&block->ssa->non_ssa_vars, var);
		if (v == NULL) {
			panic("Unknown variable: %*", var->type->name);
		}
		return *v;
	}
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

Tuple<SsaValue*, Error*> get_lvalue_root(Ssa* ssa, SsaValue* lvalue) {
	if (lvalue->op == SsaOp::MemberAccess) {
		auto lhs = lvalue->args[0];
		return get_lvalue_root(ssa, lhs);
	} else if (lvalue->op == SsaOp::GlobalVar) {
		return { lvalue, NULL };
	} else if (lvalue->op == SsaOp::Swizzle) {
		auto lhs = lvalue->args[0];
		return get_lvalue_root(ssa, lhs);
	} else if (lvalue->op == SsaOp::Lvalue) {
		return { lvalue, NULL };
	} else if (lvalue->op == SsaOp::Alloca) {
		return { lvalue, NULL };
	} else if (lvalue->op == SsaOp::Addr) {
		return { lvalue, NULL };
	} else if (lvalue->op == SsaOp::FunctionArg) {
		return { lvalue, NULL };
	} else {
		return { NULL, format_error("Unsupported lvalue op: %", lvalue->op) }; 
	}
}

Tuple<SsaValue*, Error*> emit_expr(Ssa* ssa, AstExpr* expr);

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
		add_arg(v, lhs);
		v->v_type = var_access->member->member_type;
		v->aux = make_any(var_access->member);
		return { v, NULL };
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		if (var_access->var->is_global) {
			auto v = make_ssa_val(ssa->current_block, SsaOp::GlobalVar);
			v->v_type = var_access->var->var_type;
			v->aux = make_any(var_access->var);
			return { v, NULL };
		} else {
			if (can_ssa(var_access->var)) {
				SsaValue* v = read_var(ssa->current_block, var_access->var);
				if (!v) {
					return { NULL, format_error("Unknown variable: %*", var_access->var->type->name) };
				}
				auto lv = make_ssa_val(ssa->current_block, SsaOp::Lvalue);
				assert(var_access->var->var_type);
				lv->aux = make_any(var_access->var);
				lv->v_type = var_access->var->var_type;
				add_arg(lv, v);
				return { lv, NULL };
			} else {
				SsaValue* v = read_var(ssa->current_block, var_access->var);
				if (!v) {
					return { NULL, format_error("Unknown variable: %*", var_access->var->type->name) };
				}
				return { v, NULL };
			}
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
		add_arg(v, lhs);
		v->aux = make_any(sw);
		return { v, NULL };
	} else if (auto deref = reflect_cast<AstDerefExpr>(expr)) {
		auto [addr, e] = emit_expr(ssa, deref->lhs);
		if (e) {
			return { NULL, e };
		}
		auto v = make_ssa_val(ssa->current_block, SsaOp::Addr);
		add_arg(v, addr);
		v->v_type = deref->lhs->expr_type;
		return { v, NULL };
	}
	return { NULL, format_error("Unsupported lvalue: %*", expr->type->name) };
}

Tuple<SsaValue*, Error*> load(Ssa* ssa, SsaValue* lhs, AstType* type) {
	auto [root, e] = get_lvalue_root(ssa, lhs);
	if (e) {
		return { NULL, e };
	}
	auto v = make_ssa_val(ssa->current_block, SsaOp::Load);
	add_arg(v, lhs);
	v->v_type = type;
	return { v, NULL };
}

Error* store(Ssa* ssa, SsaValue* lhs, SsaValue* rhs) {
	auto [root, e] = get_lvalue_root(ssa, lhs);
	if (e) {
		return e;
	}
	if (root->op == SsaOp::Lvalue) {
		// @TODO: this is wrong!.
		auto var = reflect_cast<AstVar>(root->aux);
		if (!var) {
			return format_error("Expected var");
		}
		write_var(ssa->current_block, var, rhs);
		return NULL;
	}
	auto v = make_ssa_val(ssa->current_block, SsaOp::Store);
	add_arg(v, lhs);
	add_arg(v, rhs);
	return NULL;
}

SsaValue* ssa_alloca(Ssa* ssa, AstType* type) {
	auto v = make_ssa_val(ssa->current_block, SsaOp::Alloca);
	v->aux = make_any(type);
	v->v_type = type;
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
	} else if (op == "==") {
		ssa_op = SsaOp::Equal;
	} else if (op == ",") {
		return { rhs, NULL };
	} else {
		return { NULL, format_error("Unsupported binary operator: %", op) };
	}
	auto v = make_ssa_val(ssa->current_block, ssa_op);
	add_arg(v, lhs);
	add_arg(v, rhs);
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
	add_arg(v, lhs);
	return { v, NULL };
}

Tuple<SsaValue*, Error*> emit_expr(Ssa* ssa, AstExpr* expr) {
	if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		auto v = make_ssa_val(ssa->current_block, SsaOp::Call);
		v->aux = make_any(call->f);
		v->v_type = call->expr_type;
		for (auto arg: call->args) {
			auto [arg_v, e] = emit_expr(ssa, arg);
			if (e) {
				return { NULL, e };
			}
			add_arg(v, arg_v);
		}
		return { v, NULL };
	} else if (auto binary_expr = reflect_cast<AstBinaryExpr>(expr)) {
		if (binary_expr->op->op == "=") {
			auto [lhs, e1] = lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto [rhs, e0] = emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
			}
			auto e = store(ssa, lhs, rhs);
			if (e) {
				return { NULL, e };
			}
			return { rhs, NULL };
		} else if (binary_expr->op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			auto [lhs, e1] = lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto [loaded, e2] = load(ssa, lhs, binary_expr->lhs->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			auto [rhs, e0] = emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
			}
			auto [oped, e3] = emit_binary_op(ssa, binary_expr->pure_op->op, loaded, rhs);
			if (e3) {
				return { NULL, e3 };
			}
			auto e = store(ssa, lhs, oped);
			if (e) {
				return { NULL, e };
			}
			return { oped, NULL };
		} else {
			assert(binary_expr->op->flags & AST_OP_FLAG_LEFT_ASSOC);
			auto [lhs, e] = emit_expr(ssa, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto [rhs, e0] = emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
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
		auto [loaded, e2] = load(ssa, lv, var_access->var->var_type);
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
		// @TODO: This is not correct! Rewrite with starting and ending blocks.

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
		add_arg(phi, then_expr);
		add_arg(phi, else_expr);
		phi->v_type = ternary->then->expr_type;
		return { phi, NULL };
	} else if (auto init = reflect_cast<AstStructInitializer>(expr)) {
		auto slot = ssa_alloca(ssa, init->struct_type);
		for (auto m: init->members) {
			auto [expr, e] = emit_expr(ssa, m.expr);
			if (e) {
				return { NULL, e };
			}
			auto offset = load_const(ssa, m.member->offset);
			auto ptr = make_ssa_val(ssa->current_block, SsaOp::GetElement);
			add_arg(ptr, slot);
			add_arg(ptr, offset);
			store(ssa, ptr, expr);
		}
		return { slot, NULL };
	} else if (auto unary = reflect_cast<AstUnaryExpr>(expr)) {
		if (unary->op->op == "--" || unary->op->op == "++") {
			auto [lv, e] = lvalue(ssa, unary->expr);
			if (e) {
				return { NULL, e };
			}
			auto [loaded, e2] = load(ssa, lv, unary->expr->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			auto const_one = load_const(ssa, (s64) 1);
			auto [oped, e3] = emit_binary_op(ssa, U"+"_b, loaded, const_one);
			if (e3) {
				return { NULL, e3 };
			}
			store(ssa, lv, oped);

			if (unary->op->flags & AST_OP_FLAG_POSTFIX) {
				return { loaded, NULL };
			} else {
				return { oped, NULL };
			}
		} else {
			auto [lhs, e] = emit_expr(ssa, unary->expr);
			if (e) {
				return { NULL, e };
			}
			auto [oped, e2] = emit_unary_op(ssa, unary->op->op, lhs);
			if (e2) {
				return { NULL, e2 };
			}
			return { oped, NULL };
		}
	} else {
		auto [lv, e] = lvalue(ssa, expr);
		if (e == NULL) {
			auto [loaded, e2] = load(ssa, lv, expr->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			return { loaded, NULL };
		}
	}
	return { NULL, format_error("Unsupported expression: %*", expr->type->name) };
}

Error* emit_block(Ssa* ssa, AstBlock* block);

Error* decl_var(Ssa* ssa, AstVarDecl* var_decl, SsaValue* init) {
	if (can_ssa(var_decl)) {
		if (init == NULL) {
			init = make_ssa_val(ssa->current_block, SsaOp::ZeroInit);
			init->aux = make_any(var_decl->var_type);
		}
		write_var(ssa->current_block, var_decl, init);
	} else {
		auto slot = ssa_alloca(ssa, var_decl->var_type);
		write_var(ssa->current_block, var_decl, slot);
		if (init) {
			auto e = store(ssa, slot, init);
			if (e) {
				return e;
			}
		}
	}
	return NULL;
}

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
			auto e = decl_var(ssa, var_decl, v);
			if (e) {
				return e;
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
		auto e = decl_var(ssa, var_decl, v);
		if (e) {
			return e;
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
		seal_block(after_b_s);
		return NULL;
	} else if (auto if_ = reflect_cast<AstIf>(stmt)) {
		auto after_c_id = alloc_construct_id(ssa);

		auto pred = ssa->current_block;
		seal_block(pred);

		auto if_after = make_ssa_basic_block(ssa, "if_after"_b, after_c_id);
		
		auto ptr_b_s = make_ssa_basic_block(ssa, "if_cond"_b, after_c_id);
		end_block_jump(pred, ptr_b_s);
		seal_block(ptr_b_s);

		auto curr_if = if_;
		while (curr_if) {
			auto construct_id = alloc_construct_id(ssa);

			auto cond_b_s = ptr_b_s;
			ssa->current_block = cond_b_s;
			auto [cond_value, e1] = emit_expr(ssa, curr_if->cond);
			if (e1) {
				return e1;
			}
			auto cond_b_e = ssa->current_block;
			auto body_b_s = make_ssa_basic_block(ssa, "if_body"_b, construct_id);
			ptr_b_s = make_ssa_basic_block(ssa, "if_else"_b, construct_id);
			end_block_cond_jump(cond_b_e, cond_value, body_b_s, ptr_b_s);
			seal_block(body_b_s);
			seal_block(ptr_b_s);
			ssa->current_block = body_b_s;
			auto e = emit_block(ssa, curr_if->then);
			if (e) {
				return e;
			}
			auto body_b_e = ssa->current_block;
			end_block_jump(body_b_e, if_after);

			if (curr_if->else_if) {
				curr_if = curr_if->else_if;
				continue;
			} else if (curr_if->else_block) {
				ssa->current_block = ptr_b_s;
				auto e = emit_block(ssa, curr_if->else_block);
				if (e) {
					return e;
				}
				auto else_b_e = ssa->current_block;
				end_block_jump(else_b_e, if_after);
				break;
			} else {
				break;
			}
		}
		seal_block(if_after);
		ssa->current_block = if_after;
		return NULL;
	} else if (auto ret = reflect_cast<AstReturn>(stmt)) {
		if (ret->rhs) {
			auto [ret_v, e] = emit_expr(ssa, ret->rhs);
			if (e) {
				return e;
			}
			end_block_ret(ssa->current_block, ret_v);
			return NULL;
		} else {
			end_block_ret_void(ssa->current_block);
			return NULL;
		}
		ssa->current_block = make_ssa_basic_block(ssa, "after_ret"_b, alloc_construct_id(ssa));
		seal_block(ssa->current_block);	
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
	if (inst->v_type) {
		print(" {%*} ", inst->v_type->name);
	}
	for (auto arg: inst->args) {
		print("$%\\(%) ", arg->id.v, arg->op);
	}
	if (inst->aux.ptr) {
		print(" %* ", inst->aux.type->name);
		if (inst->op == SsaOp::Const) {
			print(" '%*' ", inst->aux);
		} else if (auto var = reflect_cast<AstVar>(inst->aux)) {
			print(" '%' ", var->name);
		} else if (auto block = reflect_cast<SsaBasicBlock>(inst->aux)) {
			print(" %* ", block->name);
		} else if (auto cj = reflect_cast<SsaCondJump>(inst->aux)) {
			print(" %*, %* ", cj->t_block->name, cj->f_block->name);
		} else if (auto arg = reflect_cast<AstFunctionArg>(inst->aux)) {
			print(" '%*' ", arg->var_type->name);
		}
	}
	print("     used by ");
	for (auto use: inst->uses) {
		print("$%\\(%) ", use->id.v, use->op);
	}
}

void print_ssa_block(SsaBasicBlock* block, Array<SsaBasicBlock*>* printed_blocks) {
	if (!block->is_sealed) {
		panic("Block % is not sealed", block->name);
	}
	if (contains(*printed_blocks, block)) {
		return;
	}
	add(printed_blocks, block);
	println("Block: %", block->name);
	print("Predecessors: ");
	for (auto pred: block->pred) {
		print(" % ", pred->name);
	}
	println();
	for (auto v: block->values) {
		print("  ");
		print_ssa_value(v);
		println();
	}
	println();
	for (auto succ: block->successors) {
		print_ssa_block(succ, printed_blocks);
	}
}

void print_ssa(SsaBasicBlock* entry) {
	Array<SsaBasicBlock*> printed_blocks;
	print_ssa_block(entry, &printed_blocks);
	printed_blocks.free();
}

Tuple<Ssa*, Error*> emit_function_ssa(Allocator allocator, AstFunction* f) {
	auto arena = make_arena_allocator(allocator, 1024 * 1024);
	Ssa* ssa = make<Ssa>(arena);
	ssa->function = f;
	ssa->allocator = arena;
	if (!f->block) {
		return { {}, format_error("Function % has no body", f->name) };
	}
	auto entry_block = make_ssa_basic_block(ssa);
	ssa->entry = entry_block;
	ssa->current_block = entry_block;
	ssa->non_ssa_vars.allocator = ssa->allocator;
	seal_block(entry_block);
	for (auto arg: f->args) {
		auto v = make_ssa_val(ssa->current_block, SsaOp::FunctionArg);
		v->aux = make_any(arg);
		v->v_type = arg->var_type;
		write_var(ssa->current_block, arg, v);
	}
	auto e = emit_block(ssa, f->block);
	if (e) {
		return { {}, e };
	}
	if (ssa->current_block->ending == NULL) {
		end_block_ret_void(ssa->current_block);
	}
	return { ssa, NULL };
}

#include "../third_party/spirv_reflect.h"

struct SpirvEntryPoint {
	AstFunction* f;
	HashMap<AstFunctionArg*, u32> uniforms;
	HashMap<AstStructMember*, u32> inputs;
	Array<u32>   interf;
	u32          function_id;
};

struct SpirvEmitter {
	Allocator               allocator;
	CLikeParser*            p = NULL;
	Array<u32>              spv;
	s64                     spv_cursor = 0;
	HashMap<AstType*, u32>  type_ids;
	HashMap<Tuple<AstType*, u32>, u32> ptr_type_ids;
	u32                     id_counter = 0;
	HashMap<SsaId, u32>     id_map;
	Array<SpirvEntryPoint*> entry_points;
	SpirvEntryPoint*        ep = NULL;
};

u32 alloc_id(SpirvEmitter* m) {
	return ++m->id_counter;
}

void spv_word(SpirvEmitter* m, u32 word) {
	add(&m->spv, word, m->spv_cursor);
	m->spv_cursor += 1;
}

void spv_opcode(SpirvEmitter* m, SpvOp op, u32 word_count) {
	spv_word(m, u32(op) | ((word_count + 1) << 16));
}

struct SpvOpOperand {
	u32       op = 0;
	Span<u32> span = {};
	bool      is_span = false;

	SpvOpOperand(u32 x) {
		op = x;
		is_span = false;
	}

	SpvOpOperand(Span<u32> x) {
		span = x;
		is_span = true;
	}
};

void spv_op(SpirvEmitter* m, SpvOp op, std::initializer_list<SpvOpOperand> words) {
	s64 words_count = 0;
	for (auto word: words) {
		if (word.is_span) {
			words_count += len(word.span);
		} else {
			words_count += 1;
		}
	}
	
	spv_opcode(m, op, u32(words_count));
	for (auto word: words) {
		if (word.is_span) {
			for (auto w: word.span) {
				spv_word(m, w);
			}
		} else {
			spv_word(m, word.op); 
		}
	}

	if (false) {
		println("%", op);
		for (auto word: words) {
			if (word.is_span) {
				for (auto w: word.span) {
					print(" % ", w);
				}
			} else {
				print(" % ", word.op);
			}
		}
		println();
	}
}

Tuple<u32, Error*> get_type_id(SpirvEmitter* m, AstType* type) {
	type = resolve_type_alias(type);
	auto found = get(&m->type_ids, type);
	if (found) {
		return { *found, NULL };
	}
	u32 id = 0;
	if (auto prim = reflect_cast<AstPrimitiveType>(type)) {
		id = alloc_id(m);
		switch (prim->c_tp->primitive_kind) {
			case PrimitiveKind::P_void:
				spv_op(m, SpvOpTypeVoid, { id });
				break;
			case PrimitiveKind::P_u32:
				spv_op(m, SpvOpTypeInt, { id, 32, 0 });
				break;
			case PrimitiveKind::P_f32:
				spv_op(m, SpvOpTypeFloat, { id, 32 });
				break;
			case PrimitiveKind::P_f64:
				spv_op(m, SpvOpTypeFloat, { id, 64 });
				break;
			default:
				return { 0, format_error("Unsupported type: %*", type->name) };
		}
	} else if (auto tp = reflect_cast<AstStructType>(type)) {
		id = alloc_id(m);
		if (tp == m->p->float2_tp || tp == m->p->float3_tp || tp == m->p->float4_tp) {
			auto [comp_id, e] = get_type_id(m, m->p->f32_tp);
			if (e) {
				return { 0, e };
			}
			if (tp == m->p->float2_tp) {
				spv_op(m, SpvOpTypeVector, { id, comp_id, 2 });
			} else if (tp == m->p->float3_tp) {
				spv_op(m, SpvOpTypeVector, { id, comp_id, 3 });
			} else if (tp == m->p->float4_tp) {
				spv_op(m, SpvOpTypeVector, { id, comp_id, 4 });
			} else {
				return { 0, format_error("Unsupported type: %*", type->name) };
			}
		} else {
			return { 0, format_error("Unsupported struct type: %*", type->name) };
		}
	} else {
		return { 0, format_error("Unsupported type: %*", type->name) };
	}
	put(&m->type_ids, type, id);
	return { id, NULL };
}

Tuple<u32, Error*> decl_pointer_type(SpirvEmitter* m, AstType* type, u32 storage_class) {
	Tuple<AstType*, u32> key = { type, storage_class };
	auto found = get(&m->ptr_type_ids, key);
	if (found) {
		return { *found, NULL };
	}
	auto [id, e] = get_type_id(m, type);
	if (e) {
		return { 0, e };
	}
	auto res = alloc_id(m);
	spv_op(m, SpvOpTypePointer, { res, storage_class, id });
	put(&m->ptr_type_ids, key, res);
	return { res, NULL };
}

SpirvEmitter make_spirv_emitter(CLikeParser* p, Allocator allocator) {
	SpirvEmitter m;
	m.allocator = allocator;
	m.p = p;
	m.spv.allocator = allocator;
	return m;
}

void spv_map_id(SpirvEmitter* emitter, SsaId id, u32 value) {
	put(&emitter->id_map, id, value);
}

Tuple<u32, Error*> get_pointer_id(SpirvEmitter* m, SsaValue* v) {
	if (v->op == SsaOp::Load) {
		return get_pointer_id(m, v->args[0]);
	} else if (v->op == SsaOp::Lvalue) {
		return get_pointer_id(m, v->args[0]);
	} else if (v->op == SsaOp::FunctionArg) {
		auto arg = reflect_cast<AstFunctionArg>(v->aux);
		if (!arg) {
			return { 0, format_error("Expected function arg") };
		}
		for (auto entry: m->ep->uniforms) {
			if (entry->key == arg) {
				return { entry->value, NULL };
			}
		}
		return { 0, format_error("Unknown function arg: %*", arg->name) };
	} else {
		return { 0, format_error("Unsupported op: %", v->op) };
	}
}

#if 0
Error* emit_spirv_block(SpirvEmitter* m, SsaBasicBlock* block) {
	for (auto v: block->values) {
		switch (v->op) {
			case SsaOp::Const: {
				auto id = alloc_id(m);
				if (auto f = reflect_cast<f32>(v->aux)) {
					auto [tp, e] = get_type_id(m, m->p->f32_tp);
					if (e) {
						return e;
					}
					u32 w = bitcast<u32>(*f);
					spv_op(m, SpvOpConstant, { tp, id, w });
				} else if (auto f = reflect_cast<f64>(v->aux)) {
					auto [tp, e] = get_type_id(m, m->p->f64_tp);
					if (e) {
						return e;
					}
					u64 w = bitcast<u64>(*f);
					u32 w0 = w & 0xffffffff;
					u32 w1 = w >> 32;
					spv_op(m, SpvOpConstant, { tp, id, w0, w1 });
				} else if (auto i = reflect_cast<u32>(v->aux)) {
					auto [tp, e] = get_type_id(m, m->p->u32_tp);
					if (e) {
						return e;
					}
					spv_op(m, SpvOpConstant, { tp, id, *i });
				} else {
					return format_error("Unsupported constant type: %*", v->aux.type->name);
				}
				spv_map_id(m, v->id, id);
			}
			break;
			case SsaOp::ZeroInit: {
				auto type = reflect_cast<AstType>(v->aux);
				auto [tp, e] = get_type_id(m, type);
				if (e) {
					return e;
				}
				auto id = alloc_id(m);
				spv_op(m, SpvOpConstantNull, { tp, id });
				spv_map_id(m, v->id, id);
			}
			break;
			case SsaOp::GlobalVar: {
				return format_error("Global variables are not supported");
			}
			break;
			case SsaOp::FunctionArg: {
				// nop
			}
			break;
			case SsaOp::Load: {
				auto [root, e1] = get_lvalue_root(v->block->ssa, v->args[0]);
				if (e1) {
					return e1;
				}
				if (root->op == SsaOp::Lvalue) {
					continue;
				}
				auto found = get(&m->id_map, v->args[0]->id);
				if (!found) {
					return format_error("Unknown variable: %*", v->args[0]->id);
				}
				auto id = alloc_id(m);
				assert(v->v_type);
				auto [tp, e] = get_type_id(m, v->v_type);
				if (e) {
					return e;
				}
				spv_op(m, SpvOpLoad, { tp, id, *found });
			}
			break;
			case SsaOp::Addr: {
				auto id = alloc_id(m);
				auto [ptr_id, e] = get_pointer_id(m, v->args[0]);
				if (e) {
					return e;
				}
				spv_op(m, SpvOpAccessChain, { id, ptr_id });
				spv_map_id(m, v->id, id);
			}
			break;
			case SsaOp::Lvalue: {
				if (v->args[0]->op == SsaOp::FunctionArg) {
					auto arg = reflect_cast<AstFunctionArg>(v->args[0]->aux);
					if (!arg) {
						return format_error("Expected function arg");
					}
					bool handled = false;
					for (auto attr: arg->attrs) {
						if (attr->name == "stage_in") {
							handled = true;
							break;
						}
					}
					if (handled) {
						continue;
					}
					for (auto entry: m->ep->uniforms) {
						if (entry->key == arg) {
							put(&m->id_map, v->id, entry->value);
							handled = true;
							break;
						}
					}
					if (!handled) {
						return format_error("Unknown function arg: %*", arg->name);
					}
				}
			}
			break;
			case SsaOp::MemberAccess: {
				auto member = reflect_cast<AstStructMember>(v->aux);
				if (!member) {
					return format_error("Expected struct member in aux");
				}
				bool handled = false;
				for (auto attr: member->attrs) {
					if (attr->name == "position") {
						auto id = alloc_id(m);
						auto [tp_id, e] = get_type_id(m, m->p->float4_tp);
						if (e) {
							return e;
						}
						spv_op(m, SpvOpVariable, { id, tp_id, SpvStorageClassInput });
						auto ac_id = alloc_id(m);
						spv_op(m, SpvOpAccessChain, { ac_id, id });
						spv_map_id(m, v->id, ac_id);
						handled = true;
						break;
					}
				}
				if (handled) {
					break;
				}
				println("member: %*", member->name);
				auto found_input = get(&m->ep->inputs, member);
				println("found_input: %", found_input);
				if (found_input) {
					auto id = alloc_id(m);
					spv_op(m, SpvOpAccessChain, { id, *found_input });
					spv_map_id(m, v->id, id);
					break;
				}
				auto [ptr_id, e] = get_pointer_id(m, v->args[0]);
				if (e) {
					return e;
				}
				auto id = alloc_id(m);
				s64 idx = 0;
				for (auto x: member->struct_type->members) {
					if (x == member) {
						break;
					}
					idx += 1;
				}
				spv_op(m, SpvOpAccessChain, { id, ptr_id, idx });
				spv_map_id(m, v->id, id);
			}
			break;
			case SsaOp::Div: {
				auto lhs = get(&m->id_map, v->args[0]->id);
				if (!lhs) {
					return format_error("Expected lhs");
				}
				auto rhs = get(&m->id_map, v->args[1]->id);
				if (!rhs) {
					return format_error("Expected rhs");
				}

			}
			break;
			default: {
				return format_error("Unsupported op: %", v->op);
			}
		}
	}
	return NULL;
}
#endif

Tuple<s64, Error*> eval_const_int(AstExpr* expr) {
	auto p = expr->p;
	if (auto i = reflect_cast<AstLiteralExpr>(expr)) {
		if (i->lit_type == reflect_type_of<s64>()) {
			return { i->lit_value.s64_value, NULL };
		} else if (i->lit_type == reflect_type_of<s32>()) {
			return { i->lit_value.s32_value, NULL };
		} else {
			return { 0, simple_parser_error(p, current_loc(), expr->text_region, "Expected s32 or s64 literal, got '%*'", i->lit_type->name) };
		}
	} else {
		return { 0, simple_parser_error(p, current_loc(), expr->text_region, "Expected literal, got '%*'", expr->type->name) };
	}
}

Tuple<AstType*, Error*> strip_pointer_type(AstFunctionArg* arg) {
	if (auto ptr = reflect_cast<AstPointerType>(arg->var_type)) {
		return { ptr->pointee, NULL };
	} else {
		return { NULL, simple_parser_error(arg->p, current_loc(), arg->text_region, "Expected pointer type, got '%*'", arg->var_type->name) };
	}
}

AstAttr* find_attr(Span<AstAttr*> attrs, UnicodeString name) {
	for (auto attr: attrs) {
		if (attr->name == name) {
			return attr;
		}
	}
	return NULL;
}

Error* emit_entry_point_arg(SpirvEmitter* m, AstFunction* f, AstFunctionArg* arg) {
	auto p = f->p;
	for (auto attr: arg->attrs) {
		if (attr->name == "vk_uniform") {
			s64 set = 0;
			s64 binding = 0;
			if (len(attr->args) == 1) {
				set = 0;
				auto [i, e] = eval_const_int(attr->args[0]);
				if (e) {
					return e;
				}
				binding = i;
			} else if (len(attr->args) == 2) {
				auto [i, e] = eval_const_int(attr->args[0]);
				if (e) {
					return e;
				}
				set = i;
				auto [j, e2] = eval_const_int(attr->args[1]);
				if (e2) {
					return e2;
				}
				binding = j;
			} else {
				// expect (set, binding) or (set).
				auto reg = ProgramTextRegion { 0, 0 };
				if (attr->text_region.has_value) {
					reg = attr->text_region.value;
				}
				return simple_parser_error(f->p, current_loc(), reg, "Expected (set, binding) or (set).");
			}
			auto var_id = alloc_id(m);
			auto [tp, e] = strip_pointer_type(arg);
			if (e) {
				return e;
			}
			auto [tp_id, e2] = decl_pointer_type(m, tp, SpvStorageClassUniform);
			if (e2) {
				return e2;
			}
			put(&m->ep->uniforms, arg, var_id);
			spv_op(m, SpvOpVariable, { tp_id, var_id, SpvStorageClassUniform });
		} else if (attr->name == "stage_in") {
			auto tp = reflect_cast<AstStructType>(arg->var_type);
			if (!tp) {
				return simple_parser_error(f->p, current_loc(), attr->text_region, "Expected struct type");
			}
			if (f->kind == AstFunctionKind::Vertex) {
				// @TODO
			} else if (f->kind == AstFunctionKind::Fragment) {
				s64 idx = 0;
				for (auto member: tp->members) {
					if (find_attr(member->attrs, U"position"_b)) {
						continue;
					}
					auto [tp_id, e] = decl_pointer_type(m, member->member_type, SpvStorageClassInput);
					if (e) {
						return e;
					}
					auto id = alloc_id(m);
					spv_op(m, SpvOpVariable, { tp_id, id, SpvStorageClassInput });
					add(&m->ep->interf, id);
					put(&m->ep->inputs, member, id);
					idx += 1;
				}
			} else {
				return simple_parser_error(f->p, current_loc(), attr->text_region, "Expected vertex or fragment function");
			}
		}
	}
	return NULL;
}

Error* emit_entry_point_header(SpirvEmitter* m, AstFunction* f) {
	for (auto arg: f->args) {
		auto e = emit_entry_point_arg(m, f, arg);
		if (e) {
			return e;
		}
	}
	return NULL;
}

void collect_phis(Array<SsaValue*>* phis, SsaValue* v) {
	assert(v->op == SsaOp::Phi);
	for (auto arg: v->args) {
		if (arg->op == SsaOp::Phi) {
			if (contains(*phis, arg)) {
				continue;
			}
			add(phis, arg);
			collect_phis(phis, arg);
		}
	}
}

Error* perform_spirv_rewrites(SpirvEmitter* m, Ssa* ssa) {
	auto rw = start_rewrite(ssa);

	for (auto block: ssa->blocks) {
		while (true) {
			for (auto v: block->values) {
				if (v->op == SsaOp::FunctionArg) {
					auto arg = reflect_cast<AstFunctionArg>(v->aux);
					assert(arg);

					auto new_v = make_rewrite_ssa_val_before(&rw, block, SsaOp::SpvUniform, v);

					for (auto use: v->uses) {
						println("use: %* op %", use->id.v, use->op);
						if (use->op == SsaOp::Lvalue) {
							replace_by(&rw, use, new_v);
							remove_value(&rw, use);
						} else {
							assert(arg);
							return simple_parser_error(v->block->ssa->function->p, current_loc(), arg->text_region, "FunctionArg is expected to be used only by Lvalue");
						}
					}
					remove_value(&rw, v);
				}
			}
			break;
			next: int kl = 43;
		}
	}
	finish_rewrite(&rw);
	return NULL;
}

Error* emit_spirv_function(SpirvEmitter* m, Ssa* ssa) {
	auto f = ssa->function;
	u32 function_id = alloc_id(m);
	u32 function_type_id = alloc_id(m);
	u32 return_type_id = 0;

	if (f->kind == AstFunctionKind::Vertex || f->kind == AstFunctionKind::Fragment) {
		m->ep = make<SpirvEntryPoint>(m->allocator);
		m->ep->f = f;
		m->ep->function_id = function_id;
		m->ep->interf.allocator = m->allocator;
		add(&m->entry_points, m->ep);
	}

	auto e = perform_spirv_rewrites(m, ssa);
	if (e) {
		return e;
	}


	if (f->kind == AstFunctionKind::Plain) {
		auto [ret_type_id, e] = get_type_id(m, f->return_type);
		if (e) {
			return e;
		}
		return_type_id = ret_type_id;
		spv_opcode(m, SpvOpTypeFunction, 1 + 1 + len(f->args));
		spv_word(m, function_type_id);
		spv_word(m, ret_type_id);

		// Regular function args.
		for (auto arg: f->args) {
			auto [id, e] = decl_pointer_type(m, arg->var_type, SpvStorageClassFunction);
			if (e) {
				return e;
			}
			spv_word(m, id);
		}
	} else {
		auto [ret_tp_id, e] = get_type_id(m, f->p->void_tp);
		if (e) {
			return e;
		}
		return_type_id = ret_tp_id;
		spv_op(m, SpvOpTypeFunction, { function_type_id, ret_tp_id });
		e = emit_entry_point_header(m, f);
		if (e) {
			return e;
		}
	}
	assert(return_type_id != 0);
	spv_opcode(m, SpvOpFunction, 4);
	spv_word(m, return_type_id);
	spv_word(m, function_id);
	spv_word(m, SpvFunctionControlMaskNone);
	spv_word(m, function_type_id);
	// auto e = emit_spirv_block(m, ssa->entry);
	// if (e) {
	// 	return e;
	// }
	spv_opcode(m, SpvOpFunctionEnd, 0);
	return NULL;
}

Error* finalize_spirv(SpirvEmitter* m) {
	m->spv_cursor = 0;
	spv_word(m, 0x07230203); // magic
	spv_word(m, 0x00010000); // version
	spv_word(m, 0x00000000); // generator magic
	spv_word(m, 65536); // id bound // @TODO: patch it later.
	spv_word(m, 0); // reserved
	spv_op(m, SpvOpCapability, { SpvCapabilityShader });
	spv_op(m, SpvOpMemoryModel, { SpvAddressingModelLogical, SpvMemoryModelGLSL450 });
	for (auto ep: m->entry_points) {
		u32 exec_mode = 0;
		if (ep->f->kind == AstFunctionKind::Vertex) {
			exec_mode = SpvExecutionModelVertex;
		} else if (ep->f->kind == AstFunctionKind::Fragment) {
			exec_mode = SpvExecutionModelFragment;
		} else {
			return simple_parser_error(m->p, current_loc(), ep->f->text_region, "Unsupported entry point kind: %", ep->f->kind);
		}

		auto str = encode_utf8(m->allocator, ep->f->name);
		defer { str.free(); };
		s64 pad = align(len(str), 4) - len(str);
		for (auto i: range(pad)) {
			add(&str, 0);
		}
		add(&str, 0);
		add(&str, 0);
		add(&str, 0);
		add(&str, 0);

		auto casted_str = *(Span<u32>*) &str;
		casted_str.count /= 4;

		spv_op(m, SpvOpEntryPoint, { exec_mode, ep->function_id, casted_str, ep->interf });
	}
	return NULL;
}
