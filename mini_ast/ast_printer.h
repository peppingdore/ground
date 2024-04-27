#pragma once

#include "c_like_parser.h"

void print_ast_node(AllocatedUnicodeString* sb, AstNode* ast_node);

void print_c_program(AllocatedUnicodeString* sb, CLikeProgram* program) {
	for (auto it: program->globals) {
		print_ast_node(sb, it);
	}
}

void print_expr(AllocatedUnicodeString* sb, AstNode* expr) {
	if (auto binary_op = reflect_cast<CBinaryExpr>(expr)) {
		add(sb, "(");
		print_expr(sb, binary_op->lhs);
		add(sb, " ");
		add(sb, binary_op->op);
		add(sb, " ");
		print_expr(sb, binary_op->rhs);
		add(sb, ")");
	} else if (auto unary_op = reflect_cast<CUnaryExpr>(expr)) {
		add(sb, "(");
		add(sb, unary_op->op);
		print_expr(sb, unary_op->expr);
		add(sb, ")");
	} else if (auto postfix_op = reflect_cast<CPostfixExpr>(expr)) {
		add(sb, "(");
		print_expr(sb, postfix_op->lhs);
		add(sb, postfix_op->op);
		add(sb, ")");
	} else if (auto member_access = reflect_cast<AstVarMemberAccess>(expr)) {
		print_expr(sb, member_access->lhs);
		add(sb, ".");
		add(sb, member_access->member);
	} else if (auto var_decl = reflect_cast<AstVar>(expr)) {
		add(sb, var_decl->name);
	} else if (auto literal = reflect_cast<LiteralExpr>(expr)) {
		add(sb, literal->tok.str);
	} else if (auto ternary = reflect_cast<AstTernary>(expr)) {
		add(sb, "(");
		print_expr(sb, ternary->cond);
		add(sb, " ? ");
		print_expr(sb, ternary->then);
		add(sb, " : ");
		print_expr(sb, ternary->else_);
		add(sb, ")");
	} else if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		add(sb, reflect_cast<AstSymbol>(call->f)->name);
		add(sb, "(");
		for (auto it: call->args) {
			print_expr(sb, it);
			if (it != call->args[-1]) {
				add(sb, ", ");
			}
		}
		add(sb, ")");
	} else {
		add(sb, "Unknown expr: ");
		add(sb, expr->type->name);
	}
}

void print_block(AllocatedUnicodeString* sb, AstBlock* block);

void print_for(AllocatedUnicodeString* sb, AstFor* for_stmt) {
	add(sb, "for (");
	print_expr(sb, for_stmt->init_expr);
	add(sb, ";");
	print_expr(sb, for_stmt->cond_expr);
	add(sb, ";");
	print_expr(sb, for_stmt->incr_expr);
	add(sb, ")");
	print_block(sb, reflect_cast<AstBlock>(for_stmt->body));
}

void print_var_decl_group(AllocatedUnicodeString* sb, AstVarDeclGroup* group) {
	assert(len(group->var_decls) > 0); 
	auto first = group->var_decls[0];
	add(sb, first->var_type->name);
	add(sb, " ");
	for (auto i: range(len(group->var_decls))) {
		auto var_decl = group->var_decls[i];
		add(sb, var_decl->name);
		if (i < len(group->var_decls) - 1) {
			add(sb, ", ");
		}
	}
	if (first->init) {
		add(sb, " = ");
		print_expr(sb, first->init);
	}
	add(sb, ";\n");
}

void print_stmt(AllocatedUnicodeString* sb, AstNode* stmt) {
	if (auto _for = reflect_cast<AstFor>(stmt)) {
		return print_for(sb, _for);
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		return print_var_decl_group(sb, var_decl_group);
	} else if (auto expr = reflect_cast<AstExpr>(stmt)) {
		return print_expr(sb, expr);
	} else {
		add(sb, "Unknown stmt type: ");
		add(sb, stmt->type->name);
	}
}

void print_block(AllocatedUnicodeString* sb, AstBlock* block) {
	add(sb, "{\n");
	for (auto it: block->statements) {
		print_stmt(sb, it);
		add(sb, "\n");
	}
	add(sb, "}");
}

void print_function(AllocatedUnicodeString* sb, AstFunction* function) {
	add(sb, function->return_type->name);
	add(sb, " ");
	add(sb, function->name);
	add(sb, "(");
	for (auto i: range(len(function->args))) {
		auto arg = function->args[i];
		add(sb, arg->arg_type->name);
		add(sb, " ");
		add(sb, arg->name);
		if (i < len(function->args) - 1) {
			add(sb, ", ");
		}
	}
	add(sb, ")");
	if (function->block) {
		print_block(sb, function->block);
	} else {
		add(sb, ";\n");
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
		add(sb, "Unknown node type: ");
		add(sb, ast_node->type->name);
		add(sb, "\n");
	}
}

AllocatedUnicodeString print_ast_node(AstNode* ast_node) {
	AllocatedUnicodeString sb;
	print_ast_node(&sb, ast_node);
	return sb;
}
