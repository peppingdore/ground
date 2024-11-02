#pragma once

#include "c_like_parser.h"

void print_ast_node(GrdAllocatedUnicodeString* sb, GrdcAstNode* ast_node);

void print_c_program(GrdAllocatedUnicodeString* sb, GrdcProgram* program) {
	for (auto it: program->globals) {
		print_ast_node(sb, it);
	}
}

void print_expr(GrdAllocatedUnicodeString* sb, GrdcAstNode* expr) {
	if (auto binary_op = grd_reflect_cast<GrdcAstBinaryExpr>(expr)) {
		grd_append(sb, "(");
		print_expr(sb, binary_op->lhs);
		grd_append(sb, " ");
		grd_append(sb, binary_op->op->op);
		grd_append(sb, " ");
		print_expr(sb, binary_op->rhs);
		grd_append(sb, ")");
	} else if (auto unary_op = grd_reflect_cast<GrdcAstUnaryExpr>(expr)) {
		grd_append(sb, "(");
		if (unary_op->op->flags & AST_OP_FLAG_PREFIX) {
			grd_append(sb, unary_op->op->op);
			print_expr(sb, unary_op->expr);
		} else {
			print_expr(sb, unary_op->expr);
			grd_append(sb, unary_op->op->op);
		}
		grd_append(sb, ")");
	} else if (auto member_access = grd_reflect_cast<GrdcAstVarMemberAccess>(expr)) {
		print_expr(sb, member_access->lhs);
		grd_append(sb, ".");
		grd_append(sb, member_access->member->name);
	} else if (auto var_decl = grd_reflect_cast<GrdcAstVar>(expr)) {
		grd_append(sb, var_decl->name);
	} else if (auto literal = grd_reflect_cast<GrdcAstLiteralExpr>(expr)) {
		auto any = grd_make_any(literal->lit_type, &literal->lit_value);
		format(sb, "%", any);
	} else if (auto ternary = grd_reflect_cast<GrdcAstTernary>(expr)) {
		grd_append(sb, "(");
		print_expr(sb, ternary->cond);
		grd_append(sb, " ? ");
		print_expr(sb, ternary->then);
		grd_append(sb, " : ");
		print_expr(sb, ternary->else_);
		grd_append(sb, ")");
	} else if (auto call = grd_reflect_cast<GrdcAstFunctionCall>(expr)) {
		grd_append(sb, grd_reflect_cast<GrdcAstSymbol>(call->f)->name);
		grd_append(sb, "(");
		for (auto it: call->args) {
			print_expr(sb, it);
			if (it != call->args[-1]) {
				grd_append(sb, ", ");
			}
		}
		grd_append(sb, ")");
	} else if (auto swizzle = grd_reflect_cast<GrdcAstSwizzleExpr>(expr)) {
		print_expr(sb, swizzle->lhs);
		grd_append(sb, ".#swizzle(");
		for (auto i: grd_range(swizzle->swizzle_len)) {
			grd_add(sb, swizzle->swizzle[i] + '0');
		}
		grd_append(sb, ")");
	} else if (auto array_access = grd_reflect_cast<GrdcAstGrdArrayAccess>(expr)) {
		print_expr(sb, array_access->lhs);
		grd_append(sb, "[");
		print_expr(sb, array_access->index);
		grd_append(sb, "]");
	} else if (auto var_access = grd_reflect_cast<GrdcAstVariableAccess>(expr)) {
		grd_append(sb, var_access->var->name);
	} else if (auto init = grd_reflect_cast<GrdcAstStructInitializer>(expr)) {
		grd_append(sb, init->struct_type->name);
		grd_append(sb, "{");
		s64 i = 0;
		for (auto it: init->members) {
			print_expr(sb, it.expr);
			if (i < grd_len(init->members) - 1) {
				grd_append(sb, ", ");
			}
			i += 1;
		}
		grd_append(sb, "}");
	} else {
		grd_append(sb, "Unknown expr: ");
		grd_append(sb, expr->type->name);
	}
}

void print_block(GrdAllocatedUnicodeString* sb, GrdcAstBlock* block);

void print_for(GrdAllocatedUnicodeString* sb, GrdcAstFor* for_stmt) {
	grd_append(sb, "for (");
	print_expr(sb, for_stmt->init_expr);
	grd_append(sb, ";");
	print_expr(sb, for_stmt->cond_expr);
	grd_append(sb, ";");
	print_expr(sb, for_stmt->incr_expr);
	grd_append(sb, ")");
	print_block(sb, grd_reflect_cast<GrdcAstBlock>(for_stmt->body));
}

void print_var_decl_group(GrdAllocatedUnicodeString* sb, GrdcAstVarDeclGroup* group) {
	assert(grd_len(group->var_decls) > 0); 
	auto first = group->var_decls[0];
	grd_add(sb, first->var_ts->tp->name);
	grd_append(sb, " ");
	for (auto i: grd_range(grd_len(group->var_decls))) {
		auto var_decl = group->var_decls[i];
		grd_add(sb, var_decl->name);
		if (i < grd_len(group->var_decls) - 1) {
			grd_append(sb, ", ");
		}
	}
	if (first->init) {
		grd_append(sb, " = ");
		print_expr(sb, first->init);
	}
	grd_append(sb, ";\n");
}

void print_stmt(GrdAllocatedUnicodeString* sb, GrdcAstNode* stmt) {
	if (auto _for = grd_reflect_cast<GrdcAstFor>(stmt)) {
		return print_for(sb, _for);
	} else if (auto var_decl_group = grd_reflect_cast<GrdcAstVarDeclGroup>(stmt)) {
		return print_var_decl_group(sb, var_decl_group);
	} else if (auto expr = grd_reflect_cast<GrdcAstExpr>(stmt)) {
		return print_expr(sb, expr);
	} else {
		grd_append(sb, "Unknown stmt type: ");
		grd_append(sb, stmt->type->name);
	}
}

void print_block(GrdAllocatedUnicodeString* sb, GrdcAstBlock* block) {
	grd_append(sb, "{\n");
	for (auto it: block->statements) {
		print_stmt(sb, it);
		grd_append(sb, "\n");
	}
	grd_append(sb, "}");
}

void print_function(GrdAllocatedUnicodeString* sb, GrdcAstFunction* function) {
	grd_append(sb, function->return_ts->tp->name);
	grd_append(sb, " ");
	grd_append(sb, function->name);
	grd_append(sb, "(");
	for (auto i: grd_range(grd_len(function->args))) {
		auto arg = function->args[i];
		grd_append(sb, arg->var_ts->tp->name);
		grd_append(sb, " ");
		grd_append(sb, arg->name);
		if (i < grd_len(function->args) - 1) {
			grd_append(sb, ", ");
		}
	}
	grd_append(sb, ")");
	if (function->block) {
		print_block(sb, function->block);
	} else {
		grd_append(sb, ";\n");
	}
}

void print_ast_node(GrdAllocatedUnicodeString* sb, GrdcAstNode* ast_node) {
	if (auto program = grd_reflect_cast<GrdcProgram>(ast_node)) {
		return print_c_program(sb, program);
	} else if (auto decl = grd_reflect_cast<GrdcAstVarDeclGroup>(ast_node)) {
		return print_var_decl_group(sb, decl);
	} else if (auto function = grd_reflect_cast<GrdcAstFunction>(ast_node)) {
		return print_function(sb, function);
	} else {
		grd_append(sb, "Unknown node type: ");
		grd_append(sb, ast_node->type->name);
		grd_append(sb, "\n");
	}
}

GrdAllocatedUnicodeString print_ast_node(GrdcAstNode* ast_node) {
	GrdAllocatedUnicodeString sb;
	print_ast_node(&sb, ast_node);
	return sb;
}
