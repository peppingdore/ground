#pragma once

#include "c_like_parser.h"

void print_ast_node(AllocatedUnicodeString* sb, AstNode* ast_node);

void print_c_program(AllocatedUnicodeString* sb, CLikeProgram* program) {
	for (auto it: program->globals) {
		print_ast_node(sb, it);
	}
}

void print_expr(AllocatedUnicodeString* sb, AstNode* expr) {
	if (auto binary_op = reflect_cast<AstBinaryExpr>(expr)) {
		append(sb, "(");
		print_expr(sb, binary_op->lhs);
		append(sb, " ");
		append(sb, binary_op->op->op);
		append(sb, " ");
		print_expr(sb, binary_op->rhs);
		append(sb, ")");
	} else if (auto unary_op = reflect_cast<AstUnaryExpr>(expr)) {
		append(sb, "(");
		if (unary_op->op->flags & AST_OP_FLAG_PREFIX) {
			append(sb, unary_op->op->op);
			print_expr(sb, unary_op->expr);
		} else {
			print_expr(sb, unary_op->expr);
			append(sb, unary_op->op->op);
		}
		append(sb, ")");
	} else if (auto member_access = reflect_cast<AstVarMemberAccess>(expr)) {
		print_expr(sb, member_access->lhs);
		append(sb, ".");
		append(sb, member_access->member->name);
	} else if (auto var_decl = reflect_cast<AstVar>(expr)) {
		append(sb, var_decl->name);
	} else if (auto literal = reflect_cast<AstLiteralExpr>(expr)) {
		auto any = make_any(literal->lit_type, &literal->lit_value);
		format(sb, "%", any);
	} else if (auto ternary = reflect_cast<AstTernary>(expr)) {
		append(sb, "(");
		print_expr(sb, ternary->cond);
		append(sb, " ? ");
		print_expr(sb, ternary->then);
		append(sb, " : ");
		print_expr(sb, ternary->else_);
		append(sb, ")");
	} else if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		append(sb, reflect_cast<AstSymbol>(call->f)->name);
		append(sb, "(");
		for (auto it: call->args) {
			print_expr(sb, it);
			if (it != call->args[-1]) {
				append(sb, ", ");
			}
		}
		append(sb, ")");
	} else if (auto swizzle = reflect_cast<AstSwizzleExpr>(expr)) {
		print_expr(sb, swizzle->lhs);
		append(sb, ".#swizzle(");
		for (auto i: range(swizzle->swizzle_len)) {
			add(sb, swizzle->swizzle[i] + '0');
		}
		append(sb, ")");
	} else if (auto array_access = reflect_cast<AstArrayAccess>(expr)) {
		print_expr(sb, array_access->lhs);
		append(sb, "[");
		print_expr(sb, array_access->index);
		append(sb, "]");
	} else if (auto var_access = reflect_cast<AstVariableAccess>(expr)) {
		append(sb, var_access->var->name);
	} else if (auto init = reflect_cast<AstStructInitializer>(expr)) {
		append(sb, init->struct_type->name);
		append(sb, "{");
		s64 i = 0;
		for (auto it: init->members) {
			print_expr(sb, it.expr);
			if (i < len(init->members) - 1) {
				append(sb, ", ");
			}
			i += 1;
		}
		append(sb, "}");
	} else {
		append(sb, "Unknown expr: ");
		append(sb, expr->type->name);
	}
}

void print_block(AllocatedUnicodeString* sb, AstBlock* block);

void print_for(AllocatedUnicodeString* sb, AstFor* for_stmt) {
	append(sb, "for (");
	print_expr(sb, for_stmt->init_expr);
	append(sb, ";");
	print_expr(sb, for_stmt->cond_expr);
	append(sb, ";");
	print_expr(sb, for_stmt->incr_expr);
	append(sb, ")");
	print_block(sb, reflect_cast<AstBlock>(for_stmt->body));
}

void print_var_decl_group(AllocatedUnicodeString* sb, AstVarDeclGroup* group) {
	assert(len(group->var_decls) > 0); 
	auto first = group->var_decls[0];
	add(sb, first->var_type->name);
	append(sb, " ");
	for (auto i: range(len(group->var_decls))) {
		auto var_decl = group->var_decls[i];
		add(sb, var_decl->name);
		if (i < len(group->var_decls) - 1) {
			append(sb, ", ");
		}
	}
	if (first->init) {
		append(sb, " = ");
		print_expr(sb, first->init);
	}
	append(sb, ";\n");
}

void print_stmt(AllocatedUnicodeString* sb, AstNode* stmt) {
	if (auto _for = reflect_cast<AstFor>(stmt)) {
		return print_for(sb, _for);
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		return print_var_decl_group(sb, var_decl_group);
	} else if (auto expr = reflect_cast<AstExpr>(stmt)) {
		return print_expr(sb, expr);
	} else {
		append(sb, "Unknown stmt type: ");
		append(sb, stmt->type->name);
	}
}

void print_block(AllocatedUnicodeString* sb, AstBlock* block) {
	append(sb, "{\n");
	for (auto it: block->statements) {
		print_stmt(sb, it);
		append(sb, "\n");
	}
	append(sb, "}");
}

void print_function(AllocatedUnicodeString* sb, AstFunction* function) {
	append(sb, function->return_type->name);
	append(sb, " ");
	append(sb, function->name);
	append(sb, "(");
	for (auto i: range(len(function->args))) {
		auto arg = function->args[i];
		append(sb, arg->var_type->name);
		append(sb, " ");
		append(sb, arg->name);
		if (i < len(function->args) - 1) {
			append(sb, ", ");
		}
	}
	append(sb, ")");
	if (function->block) {
		print_block(sb, function->block);
	} else {
		append(sb, ";\n");
	}
}

void print_ast_node(AllocatedUnicodeString* sb, AstNode* ast_node) {
	if (auto program = reflect_cast<CLikeProgram>(ast_node)) {
		return print_c_program(sb, program);
	} else if (auto decl = reflect_cast<AstVarDeclGroup>(ast_node)) {
		return print_var_decl_group(sb, decl);
	} else if (auto function = reflect_cast<AstFunction>(ast_node)) {
		return print_function(sb, function);
	} else {
		append(sb, "Unknown node type: ");
		append(sb, ast_node->type->name);
		append(sb, "\n");
	}
}

AllocatedUnicodeString print_ast_node(AstNode* ast_node) {
	AllocatedUnicodeString sb;
	print_ast_node(&sb, ast_node);
	return sb;
}
