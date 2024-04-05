#pragma once

#include "c_like_parser.h"
#include "../string_builder.h"

void print_ast_node(UnicodeStringBuilder* sb, AstNode* ast_node);

void print_c_program(UnicodeStringBuilder* sb, CLikeProgram* program) {
	for (auto it: program->globals) {
		print_ast_node(sb, it);
	}
}

void print_expr(UnicodeStringBuilder* sb, AstNode* expr) {
	if (auto binary_op = reflect_cast<CBinaryExpr>(expr)) {
		sb->append("(");
		print_expr(sb, binary_op->lhs);
		sb->append(" ");
		sb->append(binary_op->op);
		sb->append(" ");
		print_expr(sb, binary_op->rhs);
		sb->append(")");
	} else if (auto unary_op = reflect_cast<CUnaryExpr>(expr)) {
		sb->append("(");
		sb->append(unary_op->op);
		print_expr(sb, unary_op->expr);
		sb->append(")");
	} else if (auto postfix_op = reflect_cast<CPostfixExpr>(expr)) {
		sb->append("(");
		print_expr(sb, postfix_op->lhs);
		sb->append(postfix_op->op);
		sb->append(")");
	} else if (auto member_access = reflect_cast<AstVarMemberAccess>(expr)) {
		print_expr(sb, member_access->lhs);
		sb->append(".");
		sb->append(member_access->member);
	} else if (auto var_decl = reflect_cast<AstVar>(expr)) {
		sb->append(var_decl->name);
	} else if (auto literal = reflect_cast<CAstLiteral>(expr)) {
		sb->append(literal->value);
	} else if (auto ternary = reflect_cast<AstTernary>(expr)) {
		sb->append("(");
		print_expr(sb, ternary->cond);
		sb->append(" ? ");
		print_expr(sb, ternary->then);
		sb->append(" : ");
		print_expr(sb, ternary->else_);
		sb->append(")");
	} else if (auto call = reflect_cast<AstFunctionCall>(expr)) {
		sb->append(reflect_cast<AstSymbol>(call->f)->name);
		sb->append("(");
		for (auto it: call->args) {
			print_expr(sb, it);
			if (it != *call->args.last()) {
				sb->append(", ");
			}
		}
		sb->append(")");
	} else {
		sb->append("Unknown expr: ");
		sb->append(expr->type->name);
	}
}

void print_block(UnicodeStringBuilder* sb, AstBlock* block);

void print_for(UnicodeStringBuilder* sb, AstFor* for_stmt) {
	sb->append("for (");
	print_expr(sb, for_stmt->init_expr);
	sb->append(";");
	print_expr(sb, for_stmt->cond_expr);
	sb->append(";");
	print_expr(sb, for_stmt->incr_expr);
	sb->append(")");
	print_block(sb, reflect_cast<AstBlock>(for_stmt->body));
}

void print_var_decl_group(UnicodeStringBuilder* sb, AstVarDeclGroup* group) {
	assert(len(group->var_decls) > 0); 
	auto first = *group->var_decls[0];
	sb->append(first->var_type->name);
	sb->append(" ");
	for (auto i: range(len(group->var_decls))) {
		auto var_decl = *group->var_decls[i];
		sb->append(var_decl->name);
		if (i < len(group->var_decls) - 1) {
			sb->append(", ");
		}
	}
	if (first->init) {
		sb->append(" = ");
		print_expr(sb, first->init);
	}
	sb->append(";\n");
}

void print_stmt(UnicodeStringBuilder* sb, AstNode* stmt) {
	if (auto _for = reflect_cast<AstFor>(stmt)) {
		return print_for(sb, _for);
	} else if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(stmt)) {
		return print_var_decl_group(sb, var_decl_group);
	} else if (auto expr = reflect_cast<AstExpr>(stmt)) {
		return print_expr(sb, expr);
	} else {
		sb->append("Unknown stmt type: ");
		sb->append(stmt->type->name);
	}
}

void print_block(UnicodeStringBuilder* sb, AstBlock* block) {
	sb->append("{\n");
	for (auto it: block->statements) {
		print_stmt(sb, it);
		sb->append("\n");
	}
	sb->append("}");
}

void print_function(UnicodeStringBuilder* sb, AstFunction* function) {
	sb->append(function->return_type->name);
	sb->append(" ");
	sb->append(function->name);
	sb->append("(");
	for (auto i: range(len(function->args))) {
		auto arg = *function->args[i];
		sb->append(arg->arg_type->name);
		sb->append(" ");
		sb->append(arg->name);
		if (i < len(function->args) - 1) {
			sb->append(", ");
		}
	}
	sb->append(")");
	if (function->block) {
		print_block(sb, function->block);
	} else {
		sb->append(";\n");
	}
}

void print_ast_node(UnicodeStringBuilder* sb, AstNode* ast_node) {
	if (auto program = reflect_cast<CLikeProgram>(ast_node)) {
		return print_c_program(sb, program);
	} else if (auto decl = reflect_cast<AstVarDeclGroup>(ast_node)) {
		return print_var_decl_group(sb, decl);
	} else if (auto function = reflect_cast<AstFunction>(ast_node)) {
		return print_function(sb, function);
	} else {
		sb->append("Unknown node type: ");
		sb->append(ast_node->type->name);
		sb->append("\n");
	}
}

UnicodeString print_ast_node(AstNode* ast_node) {
	auto sb = build_unicode_string();
	print_ast_node(&sb, ast_node);
	return sb.get_string();
}
