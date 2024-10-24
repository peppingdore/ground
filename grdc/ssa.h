#pragma once

#include "c_like_parser.h"
#include "grd_ssa_op.h"

struct GrdSsaId { 
	s64 v = 0;

	bool operator==(GrdSsaId other) const {
		return v == other.v;
	}

	GRD_REFLECT(GrdSsaId) {
		GRD_MEMBER(v);
	}
};

GrdHash64 grd_hash_key(GrdSsaId id) {
	return grd_hash64(id.v);
}

struct GrdSsaBasicBlock;
struct GrdSsa;

struct GrdSsaValue {
	GrdSsaId            id;
	GrdSsaOp            op = GrdSsaOp::Nop;
	GrdSsaBasicBlock*   block = NULL;
	GrdArray<GrdSsaValue*> args;
	GrdArray<GrdSsaValue*> uses;
	GrdAny              aux;
	GrdAstType*         v_type = NULL;
	bool                is_removed = false; // @TODO: only for debugging. remove later.
};

struct GrdSsaBasicBlock {
	GrdString                            name;
	GrdSsa*                              ssa;
	GrdSsaValue*                         ending = NULL;
	bool                                 is_sealed = false;
	GrdArray<GrdSsaValue*>               values;
	GrdArray<GrdSsaValue*>               incomplete_phis;
	GrdArray<GrdSsaBasicBlock*>          pred;
	GrdArray<GrdSsaBasicBlock*>          successors;
	GrdHashMap<GrdAstVar*, GrdSsaValue*> ssa_vars;

	GRD_REFLECT(GrdSsaBasicBlock) {
		GRD_MEMBER(name);
		GRD_MEMBER(ssa);
		GRD_MEMBER(ending);
		GRD_MEMBER(is_sealed);
		GRD_MEMBER(values);
		GRD_MEMBER(incomplete_phis);
		GRD_MEMBER(pred);
		GRD_MEMBER(successors);
		GRD_MEMBER(ssa_vars);
	}
};

struct GrdSsa {
	GrdAllocator                         allocator = c_allocator;
	GrdSsaBasicBlock*                    entry = NULL;
	s64                                  reg_counter = 0;
	GrdSsaBasicBlock*                    current_block = NULL;
	s64                                  construct_id_counter = 0;
	GrdHashMap<GrdAstVar*, GrdSsaValue*> non_ssa_vars;
	GrdAstFunction*                      function = NULL;
	GrdArray<GrdSsaBasicBlock*>          blocks;
	bool                                 is_rewriting = false;

	void free() {
		grd_free_allocator(allocator);
	}
};

GrdSsaId grd_alloc_id(GrdSsa* ssa) {
	return GrdSsaId { ++ssa->reg_counter };
}

s64 grd_alloc_construct_id(GrdSsa* ssa) {
	return ++ssa->construct_id_counter;
}

GrdSsaBasicBlock* grd_make_ssa_basic_block(GrdSsa* ssa, GrdString name = ""_b, s64 construct_id = 0) {
	GrdSsaBasicBlock* block = grd_make<GrdSsaBasicBlock>(ssa->allocator);
	if (construct_id > 0) {
		block->name = grd_sprint(ssa->allocator, "%_%", construct_id, name);
	} else {
		block->name = name;
	}
	block->ssa = ssa;
	block->values = { .allocator = ssa->allocator };
	block->incomplete_phis = { .allocator = ssa->allocator };
	block->pred = { .allocator = ssa->allocator };
	block->successors = { .allocator = ssa->allocator };
	block->ssa_vars = { .allocator = ssa->allocator };
	grd_add(&ssa->blocks, block);
	return block;
}

bool grd_can_ssa(GrdAstVar* var) {
	if (var->is_global) {
		return false;
	}
	if (grd_reflect_cast<GrdAstPrimitiveType>(var->var_ts->tp)) {
		return true;
	}
	if (grd_reflect_cast<GrdAstFunctionArg>(var)) {
		return true;
	}
	return false;
}

void grd_write_var(GrdSsaBasicBlock* block, GrdAstVar* var, GrdSsaValue* v) {
	if (grd_can_ssa(var)) {
		grd_put(&block->ssa_vars, var, v);
	} else {
		grd_put(&block->ssa->non_ssa_vars, var, v);
	}
}

GrdSsaValue* grd_read_var(GrdSsaBasicBlock* block, GrdAstVar* var);

GrdSsaValue* grd_init_ssa_val(GrdSsaBasicBlock* block, GrdSsaOp op) {
	auto v = grd_make<GrdSsaValue>(block->ssa->allocator);
	v->block = block;
	v->op = op;
	v->id = grd_alloc_id(block->ssa);
	v->args.allocator = block->ssa->allocator;
	v->uses.allocator = block->ssa->allocator;
	return v;
}

GrdSsaValue* grd_make_ssa_val(GrdSsaBasicBlock* block, GrdSsaOp op) {
	assert(!block->ssa->is_rewriting);
	auto v = grd_init_ssa_val(block, op);
	if (op == GrdSsaOp::Phi) {
		grd_add(&block->values, v, 0);
	} else if (block->ending) {
		grd_add(&block->values, v, grd_len(block->values) - 1);
	} else {
		grd_add(&block->values, v);
	}
	return v;
}

void grd_use(GrdSsaValue* by, GrdSsaValue* v) {
	grd_add(&v->uses, by);
}

void grd_add_arg(GrdSsaValue* v, GrdSsaValue* arg) {
	grd_use(v, arg);
	grd_add(&v->args, arg);
}


struct GrdSsaRewriteAddValue {
	GrdSsaValue* v = NULL;
	GrdSsaValue* before = NULL;
};

struct SsaRewriter {
	GrdSsa*                         ssa = NULL;
	GrdArray<GrdSsaRewriteAddValue> add_values;
	GrdArray<GrdSsaValue*>          to_remove;
};

SsaRewriter grd_start_rewrite(GrdSsa* ssa) {
	assert(!ssa->is_rewriting);
	SsaRewriter r;
	r.ssa = ssa;
	ssa->is_rewriting = true;
	return r;
}

void grd_finish_rewrite(SsaRewriter* r) {
	for (auto it: r->add_values) {
		for (auto i: grd_range(grd_len(it.v->block->values))) {
			if (it.v->block->values[i] == it.before) {
				// grd_println("add: %* op % idx: %", it.v->id.v, it.v->op, i);
				// grd_println("block name: %*", it.v->block->name);
				// grd_println("before: %* op %", it.before->id.v, it.before->op);
				grd_add(&it.v->block->values, it.v, i);
				break;
			}
		}
	}
	for (auto v: r->to_remove) {
		assert(grd_len(v->uses) == 0);
		// grd_println("remove: %* op %", v->id.v, v->op);
		for (auto arg: v->args) {
			for (s64 i = 0; i < grd_len(arg->uses); i++) {
				if (arg->uses[i] == v) {
					grd_remove(&arg->uses, i);
					i -= 1;
				}
			}
		}
		for (auto i: grd_range(grd_len(v->block->values))) {
			if (v->block->values[i] == v) {
				grd_remove(&v->block->values, i);
				break;
			}
		}
	}
	r->ssa->is_rewriting = false;
}

GrdSsaValue* grd_make_rewrite_ssa_val_before(SsaRewriter* r, GrdSsaBasicBlock* block, GrdSsaOp op, GrdSsaValue* before) {
	auto v = grd_init_ssa_val(block, op);
	GrdSsaRewriteAddValue x;
	x.v = v;
	x.before = before;
	grd_add(&r->add_values, x);
	return v;
}

void grd_replace_by(SsaRewriter* r, GrdSsaValue* v, GrdSsaValue* by) {
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
		for (auto i: grd_range(grd_len(use->args))) {
			if (use->args[i] == v) {
				use->args[i] = by;
				grd_add(&by->uses, use);
				did_find = true;
				// arg may be used more than one time in the same value,
				//  so we don't break here.
			}
		}
		// assert(did_find);
	}
	grd_clear(&v->uses);
}

void grd_remove_value(SsaRewriter* r, GrdSsaValue* v) {
	v->is_removed = true;
	grd_add(&r->to_remove, v);
}

GrdSsaValue* grd_try_remove_trivial_phi(GrdSsaValue* phi) {
	assert(phi->op == GrdSsaOp::Phi);
	GrdSsaValue* same = NULL;
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
	// GrdArray<GrdSsaValue*> uses;
	// for (auto use: phi->uses) {
	// 	if (use == phi) {
	// 		continue;
	// 	}
	// 	if (!grd_contains(uses, use)) {
	// 		grd_add(&uses, use);
	// 	}
	// }
	
	auto rw = grd_start_rewrite(phi->block->ssa);
	grd_replace_by(&rw, phi, same);
	grd_finish_rewrite(&rw);

	for (auto use: phi->uses) {
		if (use == phi) {
			continue;
		}
		if (use->op == GrdSsaOp::Phi) {
			grd_try_remove_trivial_phi(use);
		}
	}

	rw = grd_start_rewrite(phi->block->ssa);
	grd_remove_value(&rw, phi);
	grd_finish_rewrite(&rw);

	return same;
}

GrdSsaValue* grd_add_phi_args(GrdSsaValue* phi) {
	assert(phi->block->is_sealed);
	assert(grd_len(phi->args) == 0);
	auto var = grd_reflect_cast<GrdAstVar>(phi->aux);
	if (!var) {
		grd_panic("Expected var");
	}
	for (auto pred: phi->block->pred) {
		auto v = grd_read_var(pred, var);
		assert(v);
		grd_add_arg(phi, v);
	}
	return grd_try_remove_trivial_phi(phi);
}

GrdSsaValue* grd_read_var_recursive(GrdSsaBasicBlock* block, GrdAstVar* var) {
	GrdSsaValue* v = NULL;
	if (!block->is_sealed) {
		auto phi = grd_make_ssa_val(block, GrdSsaOp::Phi);
		phi->aux = grd_make_any(var);
		phi->v_type = var->var_ts->tp;
		grd_add(&block->incomplete_phis, phi);
		v = phi;
	} else if (grd_len(block->pred) == 1) {
		assert(block->pred[0]->ending);
		v = grd_read_var(block->pred[0], var);
	} else {
		auto phi = grd_make_ssa_val(block, GrdSsaOp::Phi);
		phi->aux = grd_make_any(var);
		phi->v_type = var->var_ts->tp;
		grd_write_var(block, var, phi);
		v = grd_add_phi_args(phi);
	}
	assert(v != NULL);
	grd_write_var(block, var, v);
	return v;
}

void grd_seal_block(GrdSsaBasicBlock* block) {
	block->is_sealed = true;
	for (auto phi: block->incomplete_phis) {
		assert(grd_len(block->pred) > 0);
		grd_add_phi_args(phi);
	}
	grd_clear(&block->incomplete_phis);
}

void grd_add_pred_inner(GrdSsaBasicBlock* block, GrdSsaBasicBlock* pred) {
	if (block->is_sealed) {
		grd_panic("Cannot add pred to sealed block");
	}
	if (pred->ending == NULL) {
		grd_panic("Cannot add unfilled pred");
	}
	grd_add(&block->pred, pred);
	grd_add(&pred->successors, block);
}

void grd_end_block_jump(GrdSsaBasicBlock* block, GrdSsaBasicBlock* target) {
	assert(block->ending == NULL);
	auto v = grd_make_ssa_val(block, GrdSsaOp::Jump);
	v->aux = grd_make_any(target);
	block->ending = v;
	grd_add_pred_inner(target, block);
}

void grd_end_block_ret(GrdSsaBasicBlock* block, GrdSsaValue* ret) {
	assert(block->ending == NULL);
	block->ending = grd_make_ssa_val(block, GrdSsaOp::Return);
	if (ret) {
		grd_add_arg(block->ending, ret);
	}
}

void grd_end_block_ret_void(GrdSsaBasicBlock* block) {
	assert(block->ending == NULL);
	block->ending = grd_make_ssa_val(block, GrdSsaOp::ReturnVoid);
}

struct GrdSsaCondJump {
	GrdSsaBasicBlock* t_block = NULL;
	GrdSsaBasicBlock* f_block = NULL;

	GRD_REFLECT(GrdSsaCondJump) {
		GRD_MEMBER(t_block);
		GRD_MEMBER(f_block);
	}
};

void grd_end_block_cond_jump(GrdSsaBasicBlock* block, GrdSsaValue* cond, GrdSsaBasicBlock* t_block, GrdSsaBasicBlock* f_block) {
	assert(block->ending == NULL);
	auto v = grd_make_ssa_val(block, GrdSsaOp::CondJump);
	grd_add_arg(v, cond);
	auto cj = grd_make<GrdSsaCondJump>(block->ssa->allocator);
	cj->t_block = t_block;
	cj->f_block = f_block;
	v->aux = grd_make_any(cj);
	block->ending = v;
	grd_add_pred_inner(t_block, block);
	grd_add_pred_inner(f_block, block);
}

GrdSsaValue* grd_read_var(GrdSsaBasicBlock* block, GrdAstVar* var) {
	if (grd_can_ssa(var)) {
		auto found = get(&block->ssa_vars, var);
		if (found) {
			assert(!(*found)->is_removed);
			return *found;
		} else {
			return grd_read_var_recursive(block, var);
		}
		return NULL;
	} else {
		auto v = get(&block->ssa->non_ssa_vars, var);
		if (v == NULL) {
			grd_panic("Unknown variable: %*", var->type->name);
		}
		return *v;
	}
}

template <typename T>
GrdSsaValue* grd_load_const(GrdSsa* ssa, T value) {
	GrdType* type = grd_reflect_type_of<T>();
	auto inst = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Const);
	// @MemoryLeak
	auto copied = copy(ssa->allocator, value);
	inst->aux = grd_make_any(type, copied);
	return inst;
}

GrdTuple<GrdSsaValue*, GrdError*> grd_emit_expr(GrdSsa* ssa, GrdAstExpr* expr);

enum GrdSsaLVKind {
	GrdSsaLVKindSsa,
	GrdSsaLVKindPtr,
};

struct GrdSsaLV {
	GrdSsaLVKind            kind = GrdSsaLVKindSsa;
	GrdArray<GrdSsaValue*>  args;
	GrdAstVar*              var = NULL;
	GrdSsaValue*            value = NULL;
};

Tuple<GrdSsaLV, GrdSsaLVKind, GrdError*> grd_fill_lvalue(GrdSsa* ssa, GrdAstExpr* expr) {
	if (!expr->is_lvalue) {
		return { {}, {}, grd_format_error("Expected lvalue, got %*", expr->type->name) };
	}

	if (auto var_access = grd_reflect_cast<AstVarMemberAccess>(expr)) {
		auto [v, kind, e] = grd_fill_lvalue(ssa, var_access->lhs);
		if (e) {
			return { {}, {}, e };
		}
		if (grd_reflect_cast<AstGrdPointerType>(var_access->lhs->expr_type)) {
			// I think we might want to grd_emit_expr(lhs) instead of grd_fill_lvalue(lhs).
			auto idx = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Const);
			auto zero = grd_make<s64>(ssa->allocator);
			idx->aux = grd_make_any(zero);
			idx->v_type = ssa->function->p->s64_tp;
			grd_add(&v.args, idx);
			kind = GrdSsaLVKindPtr;
		}
		auto access = grd_make_ssa_val(ssa->current_block, GrdSsaOp::MemberAccess);
		access->aux = grd_make_any(var_access->member);
		access->v_type = var_access->member->member_ts->tp;
		grd_add(&v.args, access);
		return { v, kind, NULL };
	} else if (auto var_access = grd_reflect_cast<AstVariableAccess>(expr)) {
		if (var_access->var->is_global) {
			GrdSsaLV lv;
			lv.var = var_access->var;
			lv.value = grd_make_ssa_val(ssa->current_block, GrdSsaOp::GlobalVar);
			lv.value->aux = grd_make_any(var_access->var);
			lv.args.allocator = ssa->allocator;
			return { lv, GrdSsaLVKindPtr, NULL };
		} else {
			if (grd_can_ssa(var_access->var)) {
				GrdSsaValue* var = grd_read_var(ssa->current_block, var_access->var);
				if (!var) {
					return { {}, {}, grd_format_error("Unknown variable: %*", var_access->var->type->name) };
				}
				GrdSsaLV lv;
				lv.var = var_access->var;
				lv.value = var;
				lv.args.allocator = ssa->allocator;
				return { lv, GrdSsaLVKindSsa, NULL };
			} else {
				GrdSsaValue* var = grd_read_var(ssa->current_block, var_access->var);
				if (!var) {
					return { {}, {}, grd_format_error("Unknown variable: %*", var_access->var->type->name) };
				}
				GrdSsaLV lv;
				lv.var = var_access->var;
				lv.value = var;
				lv.args.allocator = ssa->allocator;
				return { lv, GrdSsaLVKindPtr, NULL };
			}
		}
	} else if (auto sw = grd_reflect_cast<AstSwizzleExpr>(expr)) {
		auto [v, kind, e] = grd_fill_lvalue(ssa, sw->lhs);
		if (e) {
			return { {}, {}, e };
		}
		auto swz = grd_make_ssa_val(ssa->current_block, GrdSsaOp::SwizzleIndices);
		swz->aux = grd_make_any(sw);
		swz->v_type = sw->expr_type;
		grd_add(&v.args, swz);
		return { v, kind, NULL };
	} else if (auto deref = grd_reflect_cast<AstDerefExpr>(expr)) {
		auto [addr, e] = grd_emit_expr(ssa, deref->lhs);
		if (e) {
			return { {}, {}, e };
		}
		GrdSsaLV lv;
		lv.args.allocator = ssa->allocator;
		lv.value = addr;
		return { lv, GrdSsaLVKindPtr, NULL };
	} else if (auto array_access = grd_reflect_cast<AstGrdArrayAccess>(expr)) {
		auto [v, kind, e] = grd_fill_lvalue(ssa, array_access->lhs);
		if (e) {
			return { {}, {}, e };
		}
		if (grd_reflect_cast<AstGrdPointerType>(array_access->lhs->expr_type)) {
			kind = GrdSsaLVKindPtr;
		}
		auto [idx, e2] = grd_emit_expr(ssa, array_access->index);
		if (e2) {
			return { {}, {}, e2 };
		}
		grd_add(&v.args, idx);
		return { v, kind, NULL };
	} else {
		return { {}, {}, grd_format_error("Unsupported lvalue: %*", expr->type->name) };
	}
}

Tuple<GrdSsaLV, GrdError*> grd_lvalue(GrdSsa* ssa, GrdAstExpr* expr) {
	auto [lv, kind, e] = grd_fill_lvalue(ssa, expr);
	if (e) {
		return { {}, e };
	}
	lv.kind = kind;
	if (lv.kind != GrdSsaLVKindSsa) {
		// @TODO: this can happen in the grd_fill_lvalue() itself.
		if (grd_len(lv.args) == 0) {
			return { lv, NULL };
		}
		auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::GetElementPtr);
		grd_add_arg(v, lv.value);
		for (auto arg: lv.args) {
			grd_add_arg(v, arg);
		}
		lv.value = v;
	}
	return { lv, NULL };
}


Tuple<GrdSsaValue*, GrdError*> grd_load(GrdSsa* ssa, GrdSsaLV lv, GrdAstType* type) {
	if (lv.kind == GrdSsaLVKindSsa) {
		if (grd_len(lv.args) == 0) {
			return { lv.value, NULL };
		} else {
			auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::ExtractValue);
			grd_add_arg(v, lv.value);
			for (auto arg: lv.args) {
				grd_add_arg(v, arg);
			}
			v->v_type = type;
			return { v, NULL };
		}
	}
	auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Load);
	grd_add_arg(v, lv.value);
	v->v_type = type;
	return { v, NULL };
}

GrdError* grd_store(GrdSsa* ssa, GrdSsaLV lv, GrdSsaValue* rhs) {
	if (lv.kind == GrdSsaLVKindSsa) {
		if (grd_len(lv.args) == 0) {
			grd_write_var(ssa->current_block, lv.var, rhs);
		} else {
			auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::InsertValue);
			grd_add_arg(v, lv.value);
			for (auto arg: lv.args) {
				grd_add_arg(v, arg);
			}
			grd_add_arg(v, rhs);
			grd_write_var(ssa->current_block, lv.var, v);
		}
		return NULL;
	}
	auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Store);
	grd_add_arg(v, lv.value);
	grd_add_arg(v, rhs);
	return NULL;
}

GrdSsaValue* grd_ssa_alloca(GrdSsa* ssa, GrdAstType* type) {
	auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Alloca);
	v->aux = grd_make_any(type);
	v->v_type = type;
	return v;
}

Tuple<GrdSsaValue*, GrdError*> grd_emit_binary_op(GrdSsa* ssa, GrdUnicodeString op, GrdSsaValue* lhs, GrdSsaValue* rhs) {
	GrdSsaOp ssa_op = GrdSsaOp::Nop;
	if (op == "+") {
		ssa_op = GrdSsaOp::Add;
	} else if (op == "-") {
		ssa_op = GrdSsaOp::Sub;
	} else if (op == "*") {
		ssa_op = GrdSsaOp::Mul;
	} else if (op == "/") {
		ssa_op = GrdSsaOp::Div;
	} else if (op == "<") {
		ssa_op = GrdSsaOp::Less;
	} else if (op == ">") {
		ssa_op = GrdSsaOp::Greater;
	} else if (op == "==") {
		ssa_op = GrdSsaOp::Equal;
	} else if (op == ",") {
		return { rhs, NULL };
	} else {
		return { NULL, grd_format_error("Unsupported binary operator: %", op) };
	}
	auto v = grd_make_ssa_val(ssa->current_block, ssa_op);
	grd_add_arg(v, lhs);
	grd_add_arg(v, rhs);
	return { v, NULL };
}

Tuple<GrdSsaValue*, GrdError*> grd_emit_unary_op(GrdSsa* ssa, GrdUnicodeString op, GrdSsaValue* lhs) {
	GrdSsaOp ssa_op = GrdSsaOp::Nop;
	if (op == "+") {
		return { lhs, NULL };
	} else if (op == "-") {
		ssa_op = GrdSsaOp::UnaryNeg;
	} else if (op == "!") {
		ssa_op = GrdSsaOp::UnaryNot;
	} else {
		return { NULL, grd_format_error("Unsupported unary operator: %", op) };
	}
	auto v = grd_make_ssa_val(ssa->current_block, ssa_op);
	grd_add_arg(v, lhs);
	return { v, NULL };
}

Tuple<GrdSsaValue*, GrdError*> grd_emit_expr(GrdSsa* ssa, GrdAstExpr* expr) {
	if (auto call = grd_reflect_cast<AstFunctionCall>(expr)) {
		auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Call);
		v->aux = grd_make_any(call->f);
		v->v_type = call->expr_type;
		for (auto arg: call->args) {
			auto [arg_v, e] = grd_emit_expr(ssa, arg);
			if (e) {
				return { NULL, e };
			}
			grd_add_arg(v, arg_v);
		}
		return { v, NULL };
	} else if (auto binary_expr = grd_reflect_cast<AstBinaryExpr>(expr)) {
		if (binary_expr->op->op == "=") {
			auto [lhs, e1] = grd_lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto [rhs, e0] = grd_emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
			}
			auto e = grd_store(ssa, lhs, rhs);
			if (e) {
				return { NULL, e };
			}
			return { rhs, NULL };
		} else if (binary_expr->op->flags & AST_OP_FLAG_MOD_ASSIGN) {
			auto [lhs, e1] = grd_lvalue(ssa, binary_expr->lhs);
			if (e1) {
				return { NULL, e1 };
			}
			auto [loaded, e2] = grd_load(ssa, lhs, binary_expr->lhs->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			auto [rhs, e0] = grd_emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
			}
			auto [oped, e3] = grd_emit_binary_op(ssa, binary_expr->pure_op->op, loaded, rhs);
			if (e3) {
				return { NULL, e3 };
			}
			auto e = grd_store(ssa, lhs, oped);
			if (e) {
				return { NULL, e };
			}
			return { oped, NULL };
		} else {
			assert(binary_expr->op->flags & AST_OP_FLAG_LEFT_ASSOC);
			auto [lhs, e] = grd_emit_expr(ssa, binary_expr->lhs);
			if (e) {
				return { NULL, e };
			}
			auto [rhs, e0] = grd_emit_expr(ssa, binary_expr->rhs);
			if (e0) {
				return { NULL, e0 };
			}
			auto [oped, e2] = grd_emit_binary_op(ssa, binary_expr->op->op, lhs, rhs);
			if (e2) {
				return { NULL, e2 };
			}
			return { oped, NULL };
		}
	} else if (auto var_access = grd_reflect_cast<AstVariableAccess>(expr)) {
		auto [lv, e] = grd_lvalue(ssa, var_access);
		if (e) {
			return { NULL, e };
		}
		auto [loaded, e2] = grd_load(ssa, lv, var_access->var->var_ts->tp);
		if (e2) {
			return { NULL, e2 };
		}
		return { loaded, NULL };
	} else if (auto literal_expr = grd_reflect_cast<AstLiteralExpr>(expr)) {
		auto any = grd_make_any(literal_expr->lit_type, &literal_expr->lit_value);
		auto inst = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Const);
		inst->aux = any;
		return { inst, NULL };
	} else if (auto ternary = grd_reflect_cast<AstTernary>(expr)) {
		auto construct_id = grd_alloc_construct_id(ssa);
		// @TODO: This is not correct! Rewrite with starting and ending blocks.

		auto cond_block = grd_make_ssa_basic_block(ssa, "ternary_cond"_b, construct_id);
		auto pred = ssa->current_block;
		ssa->current_block = cond_block;
		auto [cond_value, e1] = grd_emit_expr(ssa, ternary->cond);
		if (e1) {
			return { NULL, e1 };
		}
		grd_seal_block(pred);
		grd_end_block_jump(pred, cond_block);
		grd_seal_block(cond_block);
		ssa->current_block = grd_make_ssa_basic_block(ssa, "ternary_then"_b, construct_id);
		auto [then_expr, e2] = grd_emit_expr(ssa, ternary->then);
		if (e2) {
			return { NULL, e2 };
		}
		auto then_block = ssa->current_block;
		ssa->current_block = grd_make_ssa_basic_block(ssa, "ternary_else"_b, construct_id);
		auto [else_expr, e3] = grd_emit_expr(ssa, ternary->else_);
		if (e3) {
			return { NULL, e3 };
		}
		auto else_block = ssa->current_block;
		grd_end_block_cond_jump(cond_block, cond_value, then_block, else_block);
		grd_seal_block(then_block);
		grd_seal_block(else_block);

		ssa->current_block = grd_make_ssa_basic_block(ssa, "ternary_after"_b, construct_id);
		grd_end_block_jump(then_block, ssa->current_block);
		grd_end_block_jump(else_block, ssa->current_block);

		auto phi = grd_make_ssa_val(ssa->current_block, GrdSsaOp::Phi);
		grd_add_arg(phi, then_expr);
		grd_add_arg(phi, else_expr);
		phi->v_type = ternary->then->expr_type;
		return { phi, NULL };
	} else if (auto init = grd_reflect_cast<AstStructInitializer>(expr)) {
		auto slot = grd_ssa_alloca(ssa, init->struct_type);
		for (auto m: init->members) {
			auto [expr, e] = grd_emit_expr(ssa, m.expr);
			if (e) {
				return { NULL, e };
			}
			auto member = grd_make_ssa_val(ssa->current_block, GrdSsaOp::MemberAccess);
			member->aux = grd_make_any(m.member);
			member->v_type = m.member->member_ts->tp;
			auto lv = GrdSsaLV { .kind = GrdSsaLVKindPtr, .value = slot };
			lv.value = grd_make_ssa_val(ssa->current_block, GrdSsaOp::GetElementPtr);
			grd_add_arg(lv.value, slot);
			grd_add_arg(lv.value, member);
			grd_store(ssa, lv, expr);
		}
		auto lv = GrdSsaLV { .kind = GrdSsaLVKindPtr, .value = slot };
		auto [ld, e] = grd_load(ssa, lv, init->struct_type);
		if (e) {
			return { NULL, e };
		}
		return { ld, NULL };
	} else if (auto unary = grd_reflect_cast<AstUnaryExpr>(expr)) {
		if (unary->op->op == "--" || unary->op->op == "++") {
			auto [lv, e] = grd_lvalue(ssa, unary->expr);
			if (e) {
				return { NULL, e };
			}
			auto [loaded, e2] = grd_load(ssa, lv, unary->expr->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			auto const_one = grd_load_const(ssa, (s64) 1);
			auto [oped, e3] = grd_emit_binary_op(ssa, U"+"_b, loaded, const_one);
			if (e3) {
				return { NULL, e3 };
			}
			grd_store(ssa, lv, oped);

			if (unary->op->flags & AST_OP_FLAG_POSTFIX) {
				return { loaded, NULL };
			} else {
				return { oped, NULL };
			}
		} else {
			auto [lhs, e] = grd_emit_expr(ssa, unary->expr);
			if (e) {
				return { NULL, e };
			}
			auto [oped, e2] = grd_emit_unary_op(ssa, unary->op->op, lhs);
			if (e2) {
				return { NULL, e2 };
			}
			return { oped, NULL };
		}
	} else if (auto swizzle = grd_reflect_cast<AstSwizzleExpr>(expr)) {
		auto [lhs, e] = grd_emit_expr(ssa, swizzle->lhs);
		if (e) {
			return { NULL, e };
		}
		auto swz = grd_make_ssa_val(ssa->current_block, GrdSsaOp::SwizzleIndices);
		swz->aux = grd_make_any(swizzle);
		swz->v_type = swizzle->expr_type;
		auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::SwizzleExpr);
		grd_add_arg(v, lhs);
		grd_add_arg(v, swz);
		return { v, NULL };
	} else if (auto access = grd_reflect_cast<AstVarMemberAccess>(expr)) {
		auto [lhs, e] = grd_emit_expr(ssa, access->lhs);
		if (e) {
			return { NULL, e };
		}
		auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::MemberAccess);
		v->aux = grd_make_any(access->member);
		auto ext = grd_make_ssa_val(ssa->current_block, GrdSsaOp::ExtractValue);
		grd_add_arg(ext, lhs);
		grd_add_arg(ext, v);
		return { ext, NULL };
	} else {
		auto [lv, e] = grd_lvalue(ssa, expr);
		if (e == NULL) {
			auto [loaded, e2] = grd_load(ssa, lv, expr->expr_type);
			if (e2) {
				return { NULL, e2 };
			}
			return { loaded, NULL };
		} else {
			grd_println(e);
			e->free();
		}
	}
	return { NULL, grd_format_error("Unsupported expression: %*", expr->type->name) };
}

GrdError* grd_emit_block(GrdSsa* ssa, AstBlock* block);

GrdError* grd_decl_var(GrdSsa* ssa, GrdAstVarDecl* var_decl, GrdSsaValue* init) {
	if (grd_can_ssa(var_decl)) {
		if (init == NULL) {
			init = grd_make_ssa_val(ssa->current_block, GrdSsaOp::ZeroInit);
			init->aux = grd_make_any(var_decl->var_ts->tp);
		}
		grd_write_var(ssa->current_block, var_decl, init);
	} else {
		auto slot = grd_ssa_alloca(ssa, var_decl->var_ts->tp);
		grd_write_var(ssa->current_block, var_decl, slot);
		if (init) {
			auto lv = GrdSsaLV { .kind = GrdSsaLVKindPtr, .value = slot };
			auto e = grd_store(ssa, lv, init);
			if (e) {
				return e;
			}
		}
	}
	return NULL;
}

GrdError* grd_emit_stmt(GrdSsa* ssa, AstNode* stmt) {
	if (auto expr = grd_reflect_cast<GrdAstExpr>(stmt)) {
		auto [var, e] = grd_emit_expr(ssa, expr);
		if (e) {
			return e;
		}
		return NULL;
	} else if (auto var_decl_group = grd_reflect_cast<AstVarDeclGroup>(stmt)) {
		auto first = var_decl_group->var_decls[0];
		GrdSsaValue* v = NULL;
		if (first->init) {
			auto [var, e] = grd_emit_expr(ssa, first->init);
			if (e) {
				return e;
			}
			v = var;
		}
		for (auto var_decl: var_decl_group->var_decls) {
			auto e = grd_decl_var(ssa, var_decl, v);
			if (e) {
				return e;
			}
		}
		return NULL;
	} else if (auto var_decl = grd_reflect_cast<GrdAstVarDecl>(stmt)) {
		GrdSsaValue* v = NULL;
		if (var_decl->init) {
			auto [var, e] = grd_emit_expr(ssa, var_decl->init);
			if (e) {
				return e;
			}
			v = var;
		}
		auto e = grd_decl_var(ssa, var_decl, v);
		if (e) {
			return e;
		}
		return NULL;
	} else if (auto _for = grd_reflect_cast<AstFor>(stmt)) {
		auto construct_id = grd_alloc_construct_id(ssa);

		grd_seal_block(ssa->current_block);
		auto pred = ssa->current_block;
		// b_s = block start, b_e = block end
		auto init_b_s = grd_make_ssa_basic_block(ssa, "for_init"_b, construct_id);
		ssa->current_block = init_b_s;
		auto [_, e0] = grd_emit_expr(ssa, _for->init_expr);
		if (e0) {
			return e0;
		}
		auto init_b_e = ssa->current_block;
		grd_seal_block(pred);
		grd_end_block_jump(pred, init_b_s);
		grd_seal_block(init_b_e);

		auto cond_b_s = grd_make_ssa_basic_block(ssa, "for_cond"_b, construct_id);
		grd_end_block_jump(init_b_e, cond_b_s);
		ssa->current_block = cond_b_s;
		auto [cond_value, e1] = grd_emit_expr(ssa, _for->cond_expr);
		if (e1) {
			return e1;
		}
		auto cond_b_e = ssa->current_block;

		auto body_b_s = grd_make_ssa_basic_block(ssa, "for_body"_b, construct_id);
		auto after_b_s = grd_make_ssa_basic_block(ssa, "for_after"_b, construct_id);
		grd_end_block_cond_jump(cond_b_e, cond_value, body_b_s, after_b_s);
		
		ssa->current_block = body_b_s;
		auto e3 = grd_emit_block(ssa, _for->body);
		if (e3) {
			return e3;
		}
		auto body_b_e = ssa->current_block;

		auto incr_b_s = grd_make_ssa_basic_block(ssa, "for_incr"_b, construct_id);
		grd_end_block_jump(body_b_e, incr_b_s);

		ssa->current_block = incr_b_s;
		auto [xxxx, e2] = grd_emit_expr(ssa, _for->incr_expr);
		if (e2) {
			return e2;
		}
		auto incr_b_e = ssa->current_block;
		grd_end_block_jump(incr_b_e, cond_b_s);

		grd_seal_block(body_b_e);
		grd_seal_block(incr_b_e);
		grd_seal_block(cond_b_e);

		ssa->current_block = after_b_s;
		grd_seal_block(after_b_s);
		return NULL;
	} else if (auto if_ = grd_reflect_cast<AstIf>(stmt)) {
		auto after_c_id = grd_alloc_construct_id(ssa);

		auto pred = ssa->current_block;
		grd_seal_block(pred);

		auto if_after = grd_make_ssa_basic_block(ssa, "if_after"_b, after_c_id);
		
		auto ptr_b_s = grd_make_ssa_basic_block(ssa, "if_cond"_b, after_c_id);
		grd_end_block_jump(pred, ptr_b_s);
		grd_seal_block(ptr_b_s);

		auto curr_if = if_;
		while (curr_if) {
			auto construct_id = grd_alloc_construct_id(ssa);

			auto cond_b_s = ptr_b_s;
			ssa->current_block = cond_b_s;
			auto [cond_value, e1] = grd_emit_expr(ssa, curr_if->cond);
			if (e1) {
				return e1;
			}
			auto cond_b_e = ssa->current_block;
			auto body_b_s = grd_make_ssa_basic_block(ssa, "if_body"_b, construct_id);
			ptr_b_s = grd_make_ssa_basic_block(ssa, "if_else"_b, construct_id);
			grd_end_block_cond_jump(cond_b_e, cond_value, body_b_s, ptr_b_s);
			grd_seal_block(body_b_s);
			grd_seal_block(ptr_b_s);
			ssa->current_block = body_b_s;
			auto e = grd_emit_block(ssa, curr_if->then);
			if (e) {
				return e;
			}
			auto body_b_e = ssa->current_block;
			grd_end_block_jump(body_b_e, if_after);

			if (curr_if->else_if) {
				curr_if = curr_if->else_if;
				continue;
			} else if (curr_if->else_block) {
				ssa->current_block = ptr_b_s;
				auto e = grd_emit_block(ssa, curr_if->else_block);
				if (e) {
					return e;
				}
				auto else_b_e = ssa->current_block;
				grd_end_block_jump(else_b_e, if_after);
				break;
			} else {
				break;
			}
		}
		grd_seal_block(if_after);
		ssa->current_block = if_after;
		return NULL;
	} else if (auto ret = grd_reflect_cast<AstReturn>(stmt)) {
		if (ret->rhs) {
			auto [ret_v, e] = grd_emit_expr(ssa, ret->rhs);
			if (e) {
				return e;
			}
			grd_end_block_ret(ssa->current_block, ret_v);
			return NULL;
		} else {
			grd_end_block_ret_void(ssa->current_block);
			return NULL;
		}
		ssa->current_block = grd_make_ssa_basic_block(ssa, "after_ret"_b, grd_alloc_construct_id(ssa));
		grd_seal_block(ssa->current_block);	
	} else {
		return grd_format_error("Unsupported statement: %*", stmt->type->name);
	}
}

GrdError* grd_emit_block(GrdSsa* ssa, AstBlock* block) {
	for (auto stmt: block->statements) {
		auto e = grd_emit_stmt(ssa, stmt);
		if (e) {
			return e;
		}
	}
	return NULL;
}


struct GrdSpvUniformSetBinding {
	s64 set = 0;
	s64 binding = 0;

	GRD_REFLECT(GrdSpvUniformSetBinding) {
		GRD_MEMBER(set);
		GRD_MEMBER(binding);
	}
};

void grd_print_ssa_value(GrdSsaValue* inst) { 
	grd_print("$% = ", inst->id.v);
	grd_print("% ", inst->op);
	if (inst->v_type) {
		grd_print(" {%*} ", inst->v_type->name);
	}
	for (auto arg: inst->args) {
		grd_print("$%\\(%) ", arg->id.v, arg->op);
	}
	if (inst->aux.ptr) {
		grd_print(" %* ", inst->aux.type->name);
		if (inst->op == GrdSsaOp::Const) {
			grd_print(" '%*' ", inst->aux);
		} else if (auto var = grd_reflect_cast<GrdAstVar>(inst->aux)) {
			grd_print(" '%' ", var->name);
		} else if (auto block = grd_reflect_cast<GrdSsaBasicBlock>(inst->aux)) {
			grd_print(" %* ", block->name);
		} else if (auto cj = grd_reflect_cast<GrdSsaCondJump>(inst->aux)) {
			grd_print(" %*, %* ", cj->t_block->name, cj->f_block->name);
		} else if (auto arg = grd_reflect_cast<GrdAstFunctionArg>(inst->aux)) {
			grd_print(" '%*' ", arg->var_ts->tp->name);
		} else if (auto spv = grd_reflect_cast<GrdSpvUniformSetBinding>(inst->aux)) {
			grd_print(" (%, %) ", spv->set, spv->binding);
		}
	}
	grd_print("     used by ");
	for (auto use: inst->uses) {
		grd_print("$%\\(%) ", use->id.v, use->op);
	}
}

void grd_print_ssa_block(GrdSsaBasicBlock* block, GrdArray<GrdSsaBasicBlock*>* printed_blocks) {
	if (!block->is_sealed) {
		grd_panic("Block % is not sealed", block->name);
	}
	if (grd_contains(*printed_blocks, block)) {
		return;
	}
	grd_add(printed_blocks, block);
	grd_println("Block: %", block->name);
	grd_print("Predecessors: ");
	for (auto pred: block->pred) {
		grd_print(" % ", pred->name);
	}
	grd_println();
	for (auto v: block->values) {
		grd_print("  ");
		grd_print_ssa_value(v);
		grd_println();
	}
	grd_println();
	for (auto succ: block->successors) {
		grd_print_ssa_block(succ, printed_blocks);
	}
}

void grd_print_ssa(GrdSsaBasicBlock* entry) {
	GrdArray<GrdSsaBasicBlock*> printed_blocks;
	grd_print_ssa_block(entry, &printed_blocks);
	printed_blocks.free();
}

Tuple<GrdSsa*, GrdError*> emit_function_ssa(GrdAllocator allocator, GrdAstFunction* f) {
	auto arena = grd_make_arena_allocator(allocator, 1024 * 1024);
	GrdSsa* ssa = grd_make<GrdSsa>(arena);
	ssa->function = f;
	ssa->allocator = arena;
	if (!f->block) {
		return { {}, grd_format_error("Function % has no body", f->name) };
	}
	auto entry_block = grd_make_ssa_basic_block(ssa);
	ssa->entry = entry_block;
	ssa->current_block = entry_block;
	ssa->non_ssa_vars.allocator = ssa->allocator;
	grd_seal_block(entry_block);
	for (auto arg: f->args) {
		auto v = grd_make_ssa_val(ssa->current_block, GrdSsaOp::FunctionArg);
		v->aux = grd_make_any(arg);
		v->v_type = arg->var_ts->tp;
		grd_write_var(ssa->current_block, arg, v);
	}
	auto e = grd_emit_block(ssa, f->block);
	if (e) {
		return { {}, e };
	}
	if (ssa->current_block->ending == NULL) {
		grd_end_block_ret_void(ssa->current_block);
	}
	return { ssa, NULL };
}

#include "../third_party/spirv_grd_reflect.h"

struct GrdSpirvEntryPoint {
	GrdAstFunction*                   f;
	GrdHashMap<GrdAstFunctionArg*, u32>  uniforms;
	GrdHashMap<AstStructMember*, u32> inputs;
	GrdArray<u32>                     interf;
	u32                               function_id;
};

struct GrdSpirvEmitter {
	GrdAllocator                  allocator;
	GrdCcParser*                  p = NULL;
	GrdArray<u32>                 spv;
	s64                           spv_cursor = 0;
	GrdHashMap<GrdAstType*, u32>  type_ids;
	GrdHashMap<Tuple<GrdAstType*, u32>, u32> ptr_type_ids;
	u32                           id_counter = 0;
	GrdHashMap<GrdSsaId, u32>     id_map;
	GrdArray<GrdSpirvEntryPoint*> entry_points;
	GrdSpirvEntryPoint*           ep = NULL;
};

u32 grd_alloc_id(GrdSpirvEmitter* m) {
	return ++m->id_counter;
}

void grd_spv_word(GrdSpirvEmitter* m, u32 word) {
	grd_add(&m->spv, word, m->spv_cursor);
	m->spv_cursor += 1;
}

void grd_spv_opcode(GrdSpirvEmitter* m, SpvOp op, u32 word_count) {
	grd_spv_word(m, u32(op) | ((word_count + 1) << 16));
}

struct GrdSpvOpOperand {
	u32          op = 0;
	GrdSpan<u32> span = {};
	bool         is_span = false;

	GrdSpvOpOperand(u32 x) {
		op = x;
		is_span = false;
	}

	GrdSpvOpOperand(GrdSpan<u32> x) {
		span = x;
		is_span = true;
	}
};

void grd_spv_op(GrdSpirvEmitter* m, SpvOp op, std::initializer_list<GrdSpvOpOperand> words) {
	s64 words_count = 0;
	for (auto word: words) {
		if (word.is_span) {
			words_count += grd_len(word.span);
		} else {
			words_count += 1;
		}
	}
	
	grd_spv_opcode(m, op, u32(words_count));
	for (auto word: words) {
		if (word.is_span) {
			for (auto w: word.span) {
				grd_spv_word(m, w);
			}
		} else {
			grd_spv_word(m, word.op); 
		}
	}

	if (false) {
		grd_println("%", op);
		for (auto word: words) {
			if (word.is_span) {
				for (auto w: word.span) {
					print(" % ", w);
				}
			} else {
				print(" % ", word.op);
			}
		}
		grd_println();
	}
}

GrdTuple<u32, GrdError*> grd_get_type_id(GrdSpirvEmitter* m, GrdAstType* type) {
	type = grd_resolve_type_alias(type);
	auto found = grd_get(&m->type_ids, type);
	if (found) {
		return { *found, NULL };
	}
	u32 id = 0;
	if (auto prim = grd_reflect_cast<GrdAstPrimitiveType>(type)) {
		id = grd_alloc_id(m);
		switch (prim->c_tp->primitive_kind) {
			case GrdPrimitiveKind::P_void:
				grd_spv_op(m, SpvOpTypeVoid, { id });
				break;
			case GrdPrimitiveKind::P_u32:
				grd_spv_op(m, SpvOpTypeInt, { id, 32, 0 });
				break;
			case GrdPrimitiveKind::P_f32:
				grd_spv_op(m, SpvOpTypeFloat, { id, 32 });
				break;
			case GrdPrimitiveKind::P_f64:
				grd_spv_op(m, SpvOpTypeFloat, { id, 64 });
				break;
			default:
				return { 0, grd_format_error("Unsupported type: %*", type->name) };
		}
	} else if (auto tp = grd_reflect_cast<GrdAstStructType>(type)) {
		id = grd_alloc_id(m);
		if (tp == m->p->float2_tp || tp == m->p->float3_tp || tp == m->p->float4_tp) {
			auto [comp_id, e] = grd_get_type_id(m, m->p->f32_tp);
			if (e) {
				return { 0, e };
			}
			if (tp == m->p->float2_tp) {
				grd_spv_op(m, SpvOpTypeVector, { id, comp_id, 2 });
			} else if (tp == m->p->float3_tp) {
				grd_spv_op(m, SpvOpTypeVector, { id, comp_id, 3 });
			} else if (tp == m->p->float4_tp) {
				grd_spv_op(m, SpvOpTypeVector, { id, comp_id, 4 });
			} else {
				return { 0, grd_format_error("Unsupported type: %*", type->name) };
			}
		} else {
			return { 0, grd_format_error("Unsupported struct type: %*", type->name) };
		}
	} else {
		return { 0, grd_format_error("Unsupported type: %*", type->name) };
	}
	grd_put(&m->type_ids, type, id);
	return { id, NULL };
}

GrdTuple<u32, GrdError*> decl_pointer_type(GrdSpirvEmitter* m, GrdAstType* type, u32 storage_class) {
	GrdTuple<GrdAstType*, u32> key = { type, storage_class };
	auto found = get(&m->ptr_type_ids, key);
	if (found) {
		return { *found, NULL };
	}
	auto [id, e] = grd_get_type_id(m, type);
	if (e) {
		return { 0, e };
	}
	auto res = grd_alloc_id(m);
	grd_spv_op(m, SpvOpTypePointer, { res, storage_class, id });
	grd_put(&m->ptr_type_ids, key, res);
	return { res, NULL };
}

GrdSpirvEmitter grd_make_spirv_emitter(GrdCcParser* p, GrdAllocator allocator) {
	GrdSpirvEmitter m;
	m.allocator = allocator;
	m.p = p;
	m.spv.allocator = allocator;
	return m;
}

void grd_spv_map_id(GrdSpirvEmitter* emitter, GrdSsaId id, u32 value) {
	put(&emitter->id_map, id, value);
}

#if 0
GrdError* emit_spirv_block(GrdSpirvEmitter* m, GrdSsaBasicBlock* block) {
	for (auto v: block->values) {
		switch (v->op) {
			case GrdSsaOp::Const: {
				auto id = grd_alloc_id(m);
				if (auto f = grd_reflect_cast<f32>(v->aux)) {
					auto [tp, e] = grd_get_type_id(m, m->p->f32_tp);
					if (e) {
						return e;
					}
					u32 w = bitcast<u32>(*f);
					grd_spv_op(m, SpvOpConstant, { tp, id, w });
				} else if (auto f = grd_reflect_cast<f64>(v->aux)) {
					auto [tp, e] = grd_get_type_id(m, m->p->f64_tp);
					if (e) {
						return e;
					}
					u64 w = bitcast<u64>(*f);
					u32 w0 = w & 0xffffffff;
					u32 w1 = w >> 32;
					grd_spv_op(m, SpvOpConstant, { tp, id, w0, w1 });
				} else if (auto i = grd_reflect_cast<u32>(v->aux)) {
					auto [tp, e] = grd_get_type_id(m, m->p->u32_tp);
					if (e) {
						return e;
					}
					grd_spv_op(m, SpvOpConstant, { tp, id, *i });
				} else {
					return grd_format_error("Unsupported constant type: %*", v->aux.type->name);
				}
				grd_spv_map_id(m, v->id, id);
			}
			break;
			case GrdSsaOp::ZeroInit: {
				auto type = grd_reflect_cast<GrdAstType>(v->aux);
				auto [tp, e] = grd_get_type_id(m, type);
				if (e) {
					return e;
				}
				auto id = grd_alloc_id(m);
				grd_spv_op(m, SpvOpConstantNull, { tp, id });
				grd_spv_map_id(m, v->id, id);
			}
			break;
			case GrdSsaOp::GlobalVar: {
				return grd_format_error("Global variables are not supported");
			}
			break;
			case GrdSsaOp::FunctionArg: {
				// nop
			}
			break;
			case GrdSsaOp::Load: {
				auto [root, e1] = get_lvalue_root(v->block->ssa, v->args[0]);
				if (e1) {
					return e1;
				}
				if (root->op == GrdSsaOp::Lvalue) {
					continue;
				}
				auto found = get(&m->id_map, v->args[0]->id);
				if (!found) {
					return grd_format_error("Unknown variable: %*", v->args[0]->id);
				}
				auto id = grd_alloc_id(m);
				assert(v->v_type);
				auto [tp, e] = grd_get_type_id(m, v->v_type);
				if (e) {
					return e;
				}
				grd_spv_op(m, SpvOpLoad, { tp, id, *found });
			}
			break;
			case GrdSsaOp::Addr: {
				auto id = grd_alloc_id(m);
				auto [ptr_id, e] = get_pointer_id(m, v->args[0]);
				if (e) {
					return e;
				}
				grd_spv_op(m, SpvOpAccessChain, { id, ptr_id });
				grd_spv_map_id(m, v->id, id);
			}
			break;
			case GrdSsaOp::Lvalue: {
				if (v->args[0]->op == GrdSsaOp::FunctionArg) {
					auto arg = grd_reflect_cast<GrdAstFunctionArg>(v->args[0]->aux);
					if (!arg) {
						return grd_format_error("Expected function arg");
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
						return grd_format_error("Unknown function arg: %*", arg->name);
					}
				}
			}
			break;
			case GrdSsaOp::MemberAccess: {
				auto member = grd_reflect_cast<AstStructMember>(v->aux);
				if (!member) {
					return grd_format_error("Expected struct member in aux");
				}
				bool handled = false;
				for (auto attr: member->attrs) {
					if (attr->name == "position") {
						auto id = grd_alloc_id(m);
						auto [tp_id, e] = grd_get_type_id(m, m->p->float4_tp);
						if (e) {
							return e;
						}
						grd_spv_op(m, SpvOpVariable, { id, tp_id, SpvStorageClassInput });
						auto ac_id = grd_alloc_id(m);
						grd_spv_op(m, SpvOpAccessChain, { ac_id, id });
						grd_spv_map_id(m, v->id, ac_id);
						handled = true;
						break;
					}
				}
				if (handled) {
					break;
				}
				grd_println("member: %*", member->name);
				auto found_input = get(&m->ep->inputs, member);
				grd_println("found_input: %", found_input);
				if (found_input) {
					auto id = grd_alloc_id(m);
					grd_spv_op(m, SpvOpAccessChain, { id, *found_input });
					grd_spv_map_id(m, v->id, id);
					break;
				}
				auto [ptr_id, e] = get_pointer_id(m, v->args[0]);
				if (e) {
					return e;
				}
				auto id = grd_alloc_id(m);
				s64 idx = 0;
				for (auto x: member->struct_type->members) {
					if (x == member) {
						break;
					}
					idx += 1;
				}
				grd_spv_op(m, SpvOpAccessChain, { id, ptr_id, idx });
				grd_spv_map_id(m, v->id, id);
			}
			break;
			case GrdSsaOp::Div: {
				auto lhs = get(&m->id_map, v->args[0]->id);
				if (!lhs) {
					return grd_format_error("Expected lhs");
				}
				auto rhs = get(&m->id_map, v->args[1]->id);
				if (!rhs) {
					return grd_format_error("Expected rhs");
				}

			}
			break;
			default: {
				return grd_format_error("Unsupported op: %", v->op);
			}
		}
	}
	return NULL;
}
#endif

GrdTuple<GrdAstType*, GrdError*> grd_strip_pointer_type(GrdAstFunctionArg* arg) {
	if (auto ptr = grd_reflect_cast<AstGrdPointerType>(arg->var_ts->tp)) {
		return { ptr->pointee, NULL };
	} else {
		return { NULL, grd_simple_parser_error(arg->p, grd_current_loc(), arg->text_region, "Expected pointer type, got '%*'", arg->var_ts->tp->name) };
	}
}

GrdAstAttr* grd_find_attr(GrdSpan<GrdAstAttr*> attrs, GrdUnicodeString name) {
	for (auto attr: attrs) {
		if (attr->name == name) {
			return attr;
		}
	}
	return NULL;
}

GrdError* grd_emit_entry_point_arg(GrdSpirvEmitter* m, GrdAstFunction* f, GrdAstFunctionArg* arg) {
	auto p = f->p;
	for (auto attr: arg->attrs) {
		if (attr->name == "vk_uniform") {
			
			auto var_id = grd_alloc_id(m);
			auto [tp, e] = grd_strip_pointer_type(arg);
			if (e) {
				return e;
			}
			auto [tp_id, e2] = decl_pointer_type(m, tp, SpvStorageClassUniform);
			if (e2) {
				return e2;
			}
			put(&m->ep->uniforms, arg, var_id);
			grd_spv_op(m, SpvOpVariable, { tp_id, var_id, SpvStorageClassUniform });
		} else if (attr->name == "stage_in") {
			auto tp = grd_reflect_cast<GrdAstStructType>(arg->var_ts->tp);
			if (!tp) {
				return grd_simple_parser_error(f->p, grd_current_loc(), attr->text_region, "Expected struct type");
			}
			if (f->kind == GrdAstFunctionKind::Vertex) {
				// @TODO
			} else if (f->kind == GrdAstFunctionKind::Fragment) {
				s64 idx = 0;
				for (auto member: tp->members) {
					if (grd_find_attr(member->attrs, U"position"_b)) {
						continue;
					}
					auto [tp_id, e] = decl_pointer_type(m, member->member_ts->tp, SpvStorageClassInput);
					if (e) {
						return e;
					}
					auto id = grd_alloc_id(m);
					grd_spv_op(m, SpvOpVariable, { tp_id, id, SpvStorageClassInput });
					grd_add(&m->ep->interf, id);
					put(&m->ep->inputs, member, id);
					idx += 1;
				}
			} else {
				return grd_simple_parser_error(f->p, grd_current_loc(), attr->text_region, "Expected vertex or fragment function");
			}
		}
	}
	return NULL;
}

GrdError* emit_entry_point_header(GrdSpirvEmitter* m, GrdAstFunction* f) {
	for (auto arg: f->args) {
		auto e = grd_emit_entry_point_arg(m, f, arg);
		if (e) {
			return e;
		}
	}
	return NULL;
}

void collect_phis(GrdArray<GrdSsaValue*>* phis, GrdSsaValue* v) {
	assert(v->op == GrdSsaOp::Phi);
	for (auto arg: v->args) {
		if (arg->op == GrdSsaOp::Phi) {
			if (grd_contains(*phis, arg)) {
				continue;
			}
			grd_add(phis, arg);
			collect_phis(phis, arg);
		}
	}
}

void replace_arg(GrdSsaValue* v, s64 arg, GrdSsaValue* by) {
	auto rp = v->args[arg];
	for (auto i: grd_range(grd_len(rp->uses))) {
		if (rp->uses[i] == v) {
			grd_remove(&rp->uses, i);
			break;
		}
	}
	v->args[arg] = by;
	grd_add(&by->uses, v);
}

#if 0
GrdError* rewrite_spirv_function_arg(GrdSpirvEmitter* m, GrdSsa* ssa, SsaRewriter* rw, GrdSsaValue* v) {
	auto arg = grd_reflect_cast<GrdAstFunctionArg>(v->aux);
	assert(arg);

	auto f = ssa->function;

	for (auto attr: arg->attrs) {
		if (attr->name == "vk_uniform") {
			s64 set = 0;
			s64 binding = 0;
			if (grd_len(attr->args) == 1) {
				set = 0;
				auto [i, e] = eval_const_int(attr->args[0]);
				if (e) {
					return e;
				}
				binding = i;
			} else if (grd_len(attr->args) == 2) {
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
				return grd_simple_parser_error(f->p, grd_current_loc(), reg, "Expected (set, binding) or (set).");
			}
			auto new_v = grd_make_rewrite_ssa_val_before(rw, v->block, GrdSsaOp::SpvUniform, v);
			GrdSpvUniformSetBinding sb = { .set = set, .binding = binding };
			auto cp = copy(m->allocator, sb);
			new_v->v_type = v->v_type;
			new_v->aux = grd_make_any(cp);

			grd_replace_by(rw, v, new_v);
			grd_remove_value(rw, v);
			return NULL;
		} else if (attr->name == "stage_in") {
			auto tp = grd_reflect_cast<GrdAstStructType>(arg->var_ts->tp);
			if (!tp) {
				return grd_simple_parser_error(f->p, grd_current_loc(), arg->text_region, "Expected struct type for [[stage_in]]");
			}
			bool need_as_value = false;
			for (auto use: v->uses) {
				if (use->op != GrdSsaOp::ExtractValue) {
					need_as_value = true;
				}
			}

			GrdArray<GrdSsaValue*> member_values;
			for (auto member: tp->members) {
				for (auto attr: member->attrs) {
					if (attr->name == "position") {
						auto pos = grd_make_rewrite_ssa_val_before(rw, v->block, GrdSsaOp::SpvBuiltinPosition, v);
						grd_add(&member_values, pos);
						break;
					} else {
						return grd_simple_parser_error(v->block->ssa->function->p, grd_current_loc(), attr->text_region, "Unknown attribute", attr->name);
					}
				}
			}

			GrdSsaValue* as_value = NULL;
			if (need_as_value) {
				as_value = grd_make_rewrite_ssa_val_before(rw, v->block, GrdSsaOp::MakeStruct, v);
				for (auto it: member_values) {
					grd_add_arg(as_value, it);
				}
			}

			for (auto use: v->uses) {
				if (use->op == GrdSsaOp::ExtractValue) {
					auto arg = use->args[1];
					if (arg->op != GrdSsaOp::MemberAccess) {
						grd_panic("Expected member access");
					}
					auto member = grd_reflect_cast<AstStructMember>(arg->aux);
					if (!member) {
						grd_panic("Expected struct member");
					}
					GrdSsaValue* member_value = NULL;
					s64 idx = 0;
					for (auto it: tp->members) {
						if (it == member) {
							member_value = member_values[idx];
							break;
						}
						idx += 1;
					}
					assert(member_value);
					if (grd_len(use->args) == 2) {
						s64 idx = 0;
						grd_replace_by(rw, use, member_value);
						grd_remove_value(rw, use);
					} else if (grd_len(use->args) > 2) {
						auto new_v = grd_make_rewrite_ssa_val_before(rw, v->block, GrdSsaOp::ExtractValue, use);
						grd_add_arg(new_v, member_value);
						for (auto it: use->args[2, {}]) {
							grd_add_arg(new_v, it);
						}
						grd_replace_by(rw, use, new_v);
						grd_remove_value(rw, use);
					} else {
						grd_panic("Unsupported number of args");
					}
				}
			}
			if (as_value) {
				grd_replace_by(rw, v, as_value);
			}
			grd_remove_value(rw, v);
			return NULL;
		}
	}
	return NULL;
}

GrdError* perform_spirv_rewrites(GrdSpirvEmitter* m, GrdSsa* ssa) {
	auto rw = grd_start_rewrite(ssa);
	for (auto block: ssa->blocks) {
		for (auto v: block->values) {
			if (v->op == GrdSsaOp::FunctionArg) {
				auto e = rewrite_spirv_function_arg(m, ssa, &rw, v);
				if (e) {
					return e;
				}
			}
		}
	}
	grd_finish_rewrite(&rw);
	return NULL;
}
#endif

GrdError* emit_spirv_function(GrdSpirvEmitter* m, GrdSsa* ssa) {
	auto f = ssa->function;
	u32 function_id = grd_alloc_id(m);
	u32 function_type_id = grd_alloc_id(m);
	u32 return_type_id = 0;

	if (f->kind == GrdAstFunctionKind::Vertex || f->kind == GrdAstFunctionKind::Fragment) {
		m->ep = grd_make<GrdSpirvEntryPoint>(m->allocator);
		m->ep->f = f;
		m->ep->function_id = function_id;
		m->ep->interf.allocator = m->allocator;
		grd_add(&m->entry_points, m->ep);
	}

	// auto e = perform_spirv_rewrites(m, ssa);
	// if (e) {
	// 	return e;
	// }


	// if (f->kind == GrdAstFunctionKind::Plain) {
	// 	auto [ret_type_id, e] = grd_get_type_id(m, f->return_type);
	// 	if (e) {
	// 		return e;
	// 	}
	// 	return_type_id = ret_type_id;
	// 	grd_spv_opcode(m, SpvOpTypeFunction, 1 + 1 + grd_len(f->args));
	// 	grd_spv_word(m, function_type_id);
	// 	grd_spv_word(m, ret_type_id);

	// 	// Regular function args.
	// 	for (auto arg: f->args) {
	// 		auto [id, e] = decl_pointer_type(m, arg->var_type, SpvStorageClassFunction);
	// 		if (e) {
	// 			return e;
	// 		}
	// 		grd_spv_word(m, id);
	// 	}
	// } else {
	// 	auto [ret_tp_id, e] = grd_get_type_id(m, f->p->void_tp);
	// 	if (e) {
	// 		return e;
	// 	}
	// 	return_type_id = ret_tp_id;
	// 	grd_spv_op(m, SpvOpTypeFunction, { function_type_id, ret_tp_id });
	// 	e = emit_entry_point_header(m, f);
	// 	if (e) {
	// 		return e;
	// 	}
	// }
	// assert(return_type_id != 0);
	grd_spv_opcode(m, SpvOpFunction, 4);
	grd_spv_word(m, return_type_id);
	grd_spv_word(m, function_id);
	grd_spv_word(m, SpvFunctionControlMaskNone);
	grd_spv_word(m, function_type_id);
	// auto e = emit_spirv_block(m, ssa->entry);
	// if (e) {
	// 	return e;
	// }
	grd_spv_opcode(m, SpvOpFunctionEnd, 0);
	return NULL;
}

GrdError* finalize_spirv(GrdSpirvEmitter* m) {
	m->spv_cursor = 0;
	grd_spv_word(m, 0x07230203); // magic
	grd_spv_word(m, 0x00010000); // version
	grd_spv_word(m, 0x00000000); // generator magic
	grd_spv_word(m, 65536); // id bound // @TODO: patch it later.
	grd_spv_word(m, 0); // reserved
	grd_spv_op(m, SpvOpCapability, { SpvCapabilityShader });
	grd_spv_op(m, SpvOpMemoryModel, { SpvAddressingModelLogical, SpvMemoryModelGLSL450 });
	for (auto ep: m->entry_points) {
		u32 exec_mode = 0;
		if (ep->f->kind == GrdAstFunctionKind::Vertex) {
			exec_mode = SpvExecutionModelVertex;
		} else if (ep->f->kind == GrdAstFunctionKind::Fragment) {
			exec_mode = SpvExecutionModelFragment;
		} else {
			return grd_simple_parser_error(m->p, grd_current_loc(), ep->f->text_region, "Unsupported entry point kind: %", ep->f->kind);
		}

		auto str = grd_encode_utf8(m->allocator, ep->f->name);
		grd_defer { str.free(); };
		s64 pad = align(grd_len(str), 4) - grd_len(str);
		for (auto i: grd_range(pad)) {
			grd_add(&str, 0);
		}
		grd_add(&str, 0);
		grd_add(&str, 0);
		grd_add(&str, 0);
		grd_add(&str, 0);

		auto casted_str = *(GrdSpan<u32>*) &str;
		casted_str.count /= 4;

		grd_spv_op(m, SpvOpEntryPoint, { exec_mode, ep->function_id, casted_str, ep->interf });
	}
	return NULL;
}
