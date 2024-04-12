#pragma once

#include "../error.h"
#include "../math/vector.h"
#include "../arena_allocator.h"
#include "../reflect.h"
#include "../array.h"
#include "../tuple.h"
#include "../string.h"
#include "../panic.h"

struct AstNode {
	Type* type;

	REFLECT(AstNode) {
		MEMBER(type);
			TAG(RealTypeMember{});
	}
};

template <typename T>
T* make_ast_node(Allocator allocator) {
	auto x = make<T>(allocator);
	x->type = reflect_type_of<T>();
	return x;
}

struct AstSymbol: AstNode {
	UnicodeString name;

	REFLECT(AstSymbol) {
		BASE_TYPE(AstNode);
		MEMBER(name);
	}
};

struct AstType: AstSymbol {
	REFLECT(AstType) {
		BASE_TYPE(AstSymbol);
	}
};

struct CTypeAlias: AstType {
	Type* c_type;
	
	REFLECT(CTypeAlias) {
		BASE_TYPE(AstType);
		MEMBER(c_type);
	}
};

struct AstExpr: AstNode {
	AstType* expr_type = NULL;
	bool     is_lvalue = false;

	REFLECT(AstExpr) {
		BASE_TYPE(AstNode);
		MEMBER(expr_type);
		MEMBER(is_lvalue);
	}
};

constexpr bool AST_EXPR_LVALUE = true;
constexpr bool AST_EXPR_RVALUE = false;

template <typename T>
T* make_ast_expr(Allocator allocator, AstType* expr_type, bool is_lvalue) {
	auto x = make_ast_node<T>(allocator);
	x->expr_type = expr_type;
	x->is_lvalue = is_lvalue;
	return x;
}

struct AstBlock: AstNode {
	Array<AstNode*> statements;

	REFLECT(AstBlock) {
		BASE_TYPE(AstNode);
		MEMBER(statements);
	}
};


struct CLikeProgram: AstNode {
	Array<AstNode*> globals;

	REFLECT(CLikeProgram) {
		BASE_TYPE(AstNode);
		MEMBER(globals);
	}
};

enum CTokenFlags {
	CTOKEN_FLAG_FLOATING_POINT = 1 << 0,
	CTOKEN_FLAG_INTEGER = 1 << 1,
};

struct CLikeParser {
	Allocator       allocator;
	CLikeProgram*   program;
	Array<AstNode*> scope;
	UnicodeString   str;
	s64             cursor = 0;
	u32             current_token_flags = 0;
	UnicodeString   current_token;
};


struct AstVar: AstSymbol {
	AstType* var_type;
	
	REFLECT(AstVar) {
		BASE_TYPE(AstSymbol);
		MEMBER(var_type);
	}
};

struct ShaderIntrinVar: AstVar {

	REFLECT(ShaderIntrinVar) {
		BASE_TYPE(AstVar);
	}
};

struct AstFunctionArg: AstNode {
	UnicodeString name;
	AstType*      arg_type = NULL;

	REFLECT(AstFunctionArg) {
		BASE_TYPE(AstNode);
		MEMBER(name);
		MEMBER(arg_type);
	}
};

struct AstFunction: AstSymbol {
	AstType*               return_type = NULL;
	Array<AstFunctionArg*> args;
	AstBlock*              block = NULL;

	REFLECT(AstFunction) {
		BASE_TYPE(AstSymbol);
		MEMBER(args);
		MEMBER(block);
	}
};

struct ShaderIntrinFunc: AstFunction {

	REFLECT(ShaderIntrinFunc) {
		BASE_TYPE(AstFunction);
	}
};

void add_c_type_alias(CLikeParser* p, Type* type, UnicodeString name) {
	// @TODO: check for duplicates
	auto node = make_ast_node<CTypeAlias>(p->allocator);
	node->c_type = type;
	node->name = name;
	p->program->globals.add(node);
}

void add_shader_intrinsic_var(CLikeParser* p, UnicodeString name, AstType* type) {
	// @TODO: check for duplicates
	if (!type) {
		panic("No type for shader intrinsic var");
	}
	auto node = make_ast_node<ShaderIntrinVar>(p->allocator);
	node->var_type = type;
	node->name = name;
	p->program->globals.add(node);
}

void add_shader_intrinsic_func(CLikeParser* p, UnicodeString name, AstType* return_type, std::initializer_list<AstType*> args) {
	// @TODO: check for duplicates
	auto node = make_ast_node<ShaderIntrinFunc>(p->allocator);
	node->args.allocator = p->allocator;
	node->name = name;
	node->return_type = return_type;
	s64 i = 0;
	for (auto arg: args) {
		auto arg_node = make_ast_node<AstFunctionArg>(p->allocator);
		arg_node->arg_type = arg;
		arg_node->name = sprint_unicode(p->allocator, U"arg_%"_b, i);
		node->args.add(arg_node);
		i += 1;
	}
	p->program->globals.add(node);
}

struct CPostfixExpr: AstExpr {
	AstNode* lhs;
	UnicodeString op;

	REFLECT(CPostfixExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(op);
	}
};

struct CUnaryExpr: AstExpr {
	AstNode*      expr;
	UnicodeString op;

	REFLECT(CUnaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(expr);
		MEMBER(op);
	}
};

struct CBinaryExpr: AstExpr {
	AstNode*      lhs = NULL;
	AstNode*      rhs = NULL;
	UnicodeString op;

	REFLECT(CBinaryExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(rhs);
		MEMBER(op);
	}
};

struct AstVarDecl: AstVar { 
	AstType*      var_type;
	AstExpr*      init = NULL;

	REFLECT(AstVarDecl) {
		BASE_TYPE(AstVar);
		MEMBER(var_type);
		MEMBER(init);
	}
};


struct AstVarDeclGroup: AstNode {
	Array<AstVarDecl*> var_decls;

	REFLECT(AstVarDeclGroup) {
		BASE_TYPE(AstNode);
		MEMBER(var_decls);
	}
};

struct AstVarMemberAccess: AstExpr {
	AstNode*      lhs;
	UnicodeString member;

	REFLECT(AstVarMemberAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(member);
	}
};

struct AstArrayAccess: AstExpr {
	AstExpr*      lhs;
	AstExpr*      index;

	REFLECT(AstArrayAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(lhs);
		MEMBER(index);
	}
};

struct LiteralExpr: AstExpr {
	UnicodeString token;
	f64           f64_value = 0;
	u64           u64_value = 0;
	s64           s64_value = 0;

	REFLECT(LiteralExpr) {
		BASE_TYPE(AstNode);
		MEMBER(token);
		MEMBER(f64_value);
		MEMBER(u64_value);
		MEMBER(s64_value);
	}
};


struct CLikeParserError: Error {
	s64 start;
	s64 end;

	REFLECT(CLikeParserError) {
		BASE_TYPE(Error);
		MEMBER(start);
		MEMBER(end);
	}
};

CLikeParserError* parser_error_impl(CLikeParser* p, CodeLocation loc, UnicodeString str) {
	if (p->current_token.data == NULL) {
		return format_error_t(CLikeParserError, "current_token is not set!");
	}
	s64 idx = p->current_token.data - p->str.data;
	
	s64 bottom_lines_count = 0;
	s64 top_lines_count = 0;
	s64 end = len(p->str);
	s64 start = 0;
	for (auto i: range_from_to(idx, len(p->str))) {
		if (is_line_break(p->str[i])) {
			bottom_lines_count += 1;
			if (bottom_lines_count >= 3) {
				end = i + 1;
				break;
			}
		}
	}
	for (auto i: range(idx).reverse()) {
		if (is_line_break(p->str[i])) {
			top_lines_count += 1;
			if (top_lines_count >= 3) {
				start = i;
				break;
			}
		}
	}

	auto sb = build_unicode_string();
	defer { sb.free(); };
	auto local_text = slice(p->str, start, end - start);
	auto pre_text = slice(local_text, 0, idx - start);
	auto highlight_text = slice(local_text, idx - start, len(p->current_token));
	auto post_text = slice(local_text, idx - start + len(p->current_token));
	sb.append('\n');
	sb.append(str);
	sb.append('\n');
	sb.append("\x1b[0;97m");
	sb.append(pre_text);
	sb.append("\x1b[0m");
	sb.append("\x1b[4;31m");
	sb.append(highlight_text);
	sb.append("\x1b[0m");
	sb.append("\x1b[0;97m");
	sb.append(post_text);
	sb.append("\x1b[0m");
	auto error = format_error<CLikeParserError>(loc, sb.get_string());
	error->start = idx;
	error->end = idx + len(p->current_token);
	return error;
}

#define parser_error(p, ...) parser_error_impl(p, caller_loc(), sprint_unicode(p->allocator, __VA_ARGS__))

UnicodeString set_current_token(CLikeParser* p, s64 end, u32 flags = 0) {
	defer { p->cursor = end; };
	p->current_token_flags = flags;
	return p->current_token = slice(p->str, p->cursor, end - p->cursor);
}
s64 maybe_eat_floating_point_literal(CLikeParser* p) {
	s64 start = p->cursor;
	s64 c = p->cursor;
	
	for (; c < len(p->str); c++) {
		if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
			break;
		}
	}
	if (c >= len(p->str)) {
		return 0;
	}
	bool have_stuff_before_dot = c > start;
	bool have_dot = false;
	bool have_stuff_after_dot = false;
	if (p->str[c] == '.') {
		have_dot = true;
		c += 1;
		s64 decimal_start = c;
		for (; c < len(p->str); c++) {
			if (!(p->str[c] >= '0' && p->str[c] <= '9')) { 
				break;
			}
		}
		have_stuff_after_dot = c > decimal_start;
		if (!have_stuff_before_dot && !have_stuff_after_dot) {
			return 0;
		}
	}
	if (c == start) {
		return 0;
	}
	if (c < len(p->str)) {
		if (p->str[c] == 'e' || p->str[c] == 'E') {
			c += 1;
			if (c >= len(p->str)) {
				return 0;
			}
			if (p->str[c] == '+' || p->str[c] == '-') {
				c += 1;
				if (c >= len(p->str)) {
					return 0;
				}
			}
			for (; c < len(p->str); c++) {
				if (!(p->str[c] >= '0' && p->str[c] <= '9')) {
					break;
				}
			}
		}
	}
	if (c < len(p->str)) {
		if (p->str[c] == 'f' || p->str[c] == 'F' || p->str[c] == 'd' || p->str[c] == 'D') {
			c += 1;
		}
	}
	return c;
}

s64 maybe_eat_integer_literal(CLikeParser* p) {
	auto start = slice(p->str, p->cursor);

	s32 prefix_len = 0;
	if (len(start) > 0 && (start[0] >= '0' && start[0] <= '9')) {
		prefix_len = 1;
	} else if (
		starts_with(start, U"0x"_b) ||
		starts_with(start, U"0X"_b) ||
		starts_with(start, U"0b"_b) ||
		starts_with(start, U"0B"_b) ||
		starts_with(start, U"0o"_b) ||
		starts_with(start, U"0O"_b))
	{
		prefix_len = 2;
	}

	for (auto i: range_from_to(p->cursor + prefix_len, len(p->str))) {
		if (p->str[i] >= '0' && p->str[i] <= '9') {
			continue;
		}
		if (prefix_len > 0) {
			if (p->str[i] >= 'a' && p->str[i] <= 'z') {
				continue;
			}
			if (p->str[i] >= 'A' && p->str[i] <= 'Z') {
				continue;
			}
		}
		return i != p->cursor ? i : 0;
	}
	return prefix_len > 0 ? len(p->str) : 0;
}

UnicodeString next(CLikeParser* p) {
	for (auto i: range_from_to(p->cursor, len(p->str))) {
		if (is_whitespace(p->str[i])) {
			if (p->cursor < i) {
				return set_current_token(p, i);
			} else {
				p->cursor = i + 1;
				continue;
			}
		}

		s64 fp_end = maybe_eat_floating_point_literal(p);
		if (fp_end != 0) {
			return set_current_token(p, fp_end, CTOKEN_FLAG_FLOATING_POINT);
		}

		s64 int_end = maybe_eat_integer_literal(p);
		if (int_end != 0) {
			return set_current_token(p, int_end, CTOKEN_FLAG_INTEGER);
		}

		for (auto c: {',',';','/','.','-','=','(',')','?',':','+','*','-','<','>'}) {
			if (p->str[i] == c) {
				if (p->cursor < i) {
					return set_current_token(p, i);
				} else {
					// parse multichar operators.
					auto txt = slice(p->str, i);
					if (starts_with(txt, U"=="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"!="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"+="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"-="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"*="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"/="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"%="_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"++"_b)) return set_current_token(p, i + 2);
					if (starts_with(txt, U"--"_b)) return set_current_token(p, i + 2);
					return set_current_token(p, i + 1);
				}
			}
		}
	}
	return set_current_token(p, len(p->str));
}

UnicodeString peek(CLikeParser* p) {
	if (len(p->current_token) == 0) {
		return next(p);
	}
	return p->current_token;
}

AstNode* resolve_symbol(AstNode* node, UnicodeString name) {
	if (auto sym = reflect_cast<AstSymbol>(node)) {
		if (sym->name == name) {
			return sym;
		}
	}
	if (auto var_decl_group = reflect_cast<AstVarDeclGroup>(node)) {
		for (auto it: var_decl_group->var_decls) {
			if (auto symbol = resolve_symbol(it, name)) {
				return symbol;
			}
		}
	}
	return NULL;
}

AstNode* lookup_symbol(CLikeParser* p, UnicodeString name) {
	for (auto i: range(len(p->scope)).reverse()) {
		auto scope = *p->scope[i];
		if (auto program = reflect_cast<CLikeProgram>(scope)) {
			for (auto child: program->globals) {
				if (auto symbol = resolve_symbol(child, name)) {
					return symbol;
				}
			}
		}
		if (auto block = reflect_cast<AstBlock>(scope)) {
			for (auto child: block->statements) {
				if (auto symbol = resolve_symbol(child, name)) {
					return symbol;
				}
			}
		}
	}
	return NULL;
}

AstType* find_type(CLikeParser* p, UnicodeString name) {
	auto symbol = lookup_symbol(p, name);
	if (symbol) {
		if (auto tp = reflect_cast<AstType>(symbol)) {
			return tp;
		}
	}
	return NULL;
}

Tuple<AstType*, Error*> parse_type(CLikeParser* p) {
	auto tok = peek(p);
	auto c_str = encode_utf8(tok);
	defer { c_str.free(); };
	AstType* type = find_type(p, tok);
	if (!type) {
		return { NULL, parser_error(p, U"Could not find type."_b) };
	}
	next(p);
	return { type, NULL };
}

Tuple<UnicodeString, Error*> parse_ident(CLikeParser* p) {
	auto tok = peek(p);
	if (len(tok) > 0) {
		next(p);
		return { tok, NULL };
	}
	return { {}, parser_error(p, U"Expected an identifier."_b) };
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p);
Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec);

Tuple<AstNode*, Error*> parse_var_decl(CLikeParser* p, AstType* type, UnicodeString first_ident) {
	Array<AstVarDecl*> var_decls = { .allocator = p->allocator };
	defer { var_decls.free(); };

	auto current_ident = first_ident;
	while (true) {
		auto node = make_ast_node<AstVarDecl>(p->allocator);
		node->var_type = type;
		node->name = current_ident;
		var_decls.add(node);

		auto tok = peek(p);
		if (tok != U","_b) {
			break;
		}
		next(p);
		auto [ident, e] = parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		current_ident = ident;
	}

	auto tok = peek(p);
	if (tok == U"="_b) {
		next(p);
		auto [expr, e] = parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		for (auto it: var_decls) {
			it->init = (AstExpr*) expr;
		}
	}

	tok = peek(p);
	if (tok != U";"_b) {
		return { NULL, parser_error(p, U"Expected , or ;"_b) };
	}
	

	if (len(var_decls) > 1) {
		auto node = make_ast_node<AstVarDeclGroup>(p->allocator);
		node->var_decls = var_decls;
		var_decls = {};
		return { node, NULL };
	} else {
		return { *var_decls[0], NULL };
	}
}

Tuple<AstNode*, Error*> parse_unary_expr(CLikeParser* p, UnicodeString op, s32 min_prec) {
	auto [expr, e] = parse_expr(p, min_prec);
	if (e) {
		return { NULL, e };
	}
	auto unary = make_ast_node<CUnaryExpr>(p->allocator);
	unary->expr = (AstExpr*) expr;
	unary->op = op;
	return { unary, NULL };
}

Tuple<s32, bool> get_postfix_unary_operator_prec_assoc(UnicodeString op) {
	if (op == "++" || op == "--") {
		return { 16, false };
	}
	return { 0, false };
}

Tuple<s32, bool> get_prefix_unary_operator_prec_assoc(UnicodeString op) {
	if (op == "!" || op == "~") {
		return { 15, false };
	}
	if (op == "+" || op == "-") { 
		return { 15, false };
	}
	if (op == "++" || op == "--") {
		return { 15, false };
	}
	if (op == "*" || op == "&") {
		return { 15, false };
	}
	if (op == "++" || op == "--") {
		return { 15, false };
	}
	return { 0, false };
}

Tuple<s32, bool> get_binary_operator_prec_assoc(UnicodeString op) { 
	if (op == ",") {
		return { 3, true };
	}

	if (op == "?") {
		return { 3, false };
	}

	if (op == "+=" ||
		op == "-=" ||
		op == "*=" ||
		op == "/=" ||
		op == "%=" ||
		op == "=") {
		return { 4, false };
	}

	if (op == "||") {
		return { 5, true };
	}
	if (op == "&&") {
		return { 6, true };
	}
	if (op == "|") {
		return { 7, true };
	}
	if (op == "^") {
		return { 8, true };
	}
	if (op == "&") {
		return { 9, true };
	}
	if (op == "==" || op == "!=") {
		return { 10, true };
	}
	if (op == "<" || op == ">" || op == "<=" || op == ">=") {
		return { 11, true };
	}
	if (op == "<<" || op == ">>") {
		return { 12, true };
	}
	if (op == "+" || op == "-") {
		return { 13, true };
	}
	if (op == "*" || op == "/" || op == "%") {
		return { 14, true };
	}
	return { 0, false };
}

struct AstFunctionCall: AstExpr {
	AstNode*        f;
	Array<AstExpr*> args;

	REFLECT(AstFunctionCall) {
		BASE_TYPE(AstExpr);
		MEMBER(args);
	}
};

Tuple<AstExpr*, Error*> parse_function_call(CLikeParser* p, AstFunction* func) {
	auto tok = peek(p);
	if (tok != "("_b) {
		return { NULL, parser_error(p, U"Expected ("_b) };
	}
	auto call = make_ast_expr<AstFunctionCall>(p->allocator, func->return_type, AST_EXPR_RVALUE);
	call->args.allocator = p->allocator;
	call->f = func;
	next(p);
	while (true) {
		auto tok = peek(p);
		if (tok == ")"_b) {
			next(p);
			break;
		}
		auto [comma_prec, _] = get_binary_operator_prec_assoc(U","_b);
		auto [expr, e] = parse_expr(p, comma_prec + 1);
		if (e) {
			return { NULL, e };
		}
		call->args.add(expr);
		if (peek(p) == ","_b) {
			next(p);
		}
	}
	return { call, NULL };
}

struct AstTernary: AstExpr {
	AstNode* cond;
	AstNode* then;
	AstNode* else_;

	REFLECT(AstTernary) {
		BASE_TYPE(AstExpr);
		MEMBER(cond);
		MEMBER(then);
		MEMBER(else_);
	}
};

bool is_floating_point(AstType* type) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		if (c_alias->c_type == reflect_type_of<float>() ||
			c_alias->c_type == reflect_type_of<double>()) {
			return true;
		}
	}
	return false;
}

bool is_integer(AstType* type) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		if (c_alias->c_type == reflect_type_of<s32>() ||
			c_alias->c_type == reflect_type_of<s64>())
		{			
			return true;
		}
	}
	return false;
}

bool is_numeric(AstType* type) {
	return is_floating_point(type) || is_integer(type);
}

struct AstPointerType: AstType {
	AstType* pointee = NULL;

	REFLECT(AstPointerType) {
		BASE_TYPE(AstType);
		MEMBER(pointee);
	}
};

bool is_pointer(AstType* type) {
	if (auto ptr = reflect_cast<AstPointerType>(type)) {
		return true;
	}
	return false;
}

bool is_array(AstType* type) {
	// @TODO: implement.
	return false;
}

AstType* get_element_type(AstType* type) {
	if (auto ptr = reflect_cast<AstPointerType>(type)) {
		return ptr->pointee;
	}
	return NULL;
}

struct AstVariableAccess: AstExpr {
	AstVar* var = NULL;

	REFLECT(AstVariableAccess) {
		BASE_TYPE(AstExpr);
		MEMBER(var);
	}
};

bool is_struct(AstType* type) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (c_type->as<StructType>()) {
			return true;
		}
	}
	return false;
}

struct AstStructMember {
	AstType*      type = NULL;
	UnicodeString name;
	s64           offset = 0;

	REFLECT(AstStructMember) {
		MEMBER(type);
		MEMBER(name);
		MEMBER(offset);
	}
};

AstStructMember make_ast_struct_member(AstType* type, UnicodeString name, s64 offset) {
	AstStructMember member;
	member.type = type;
	member.name = name;
	member.offset = offset;
	return member;
}

Optional<AstStructMember> get_struct_member(CLikeParser* p, AstType* type, UnicodeString name) {
	if (auto c_alias = reflect_cast<CTypeAlias>(type)) {
		auto c_type = c_alias->c_type;
		if (auto c_struct_type = c_type->as<StructType>()) {
			for (auto& member: c_struct_type->members) {
				if (make_string(member.name) == name) {
					auto unicode_str = make_string(member.name).copy_unicode_string(p->allocator);
					auto found = find_type(p, unicode_str);
					if (!found) {
						panic("Type '%' not found", unicode_str);
					}
					return make_ast_struct_member(found, unicode_str, member.offset);
				}
			}
		}
	}
	return {};
}

Tuple<AstExpr*, Error*> parse_primary_expr(CLikeParser* p) {
	auto tok = peek(p);

	if (p->current_token_flags & CTOKEN_FLAG_FLOATING_POINT) {
		f64 f;
		bool ok = parse_float(p->current_token, &f);
		if (!ok) {
			return { NULL, parser_error(p, U"Could not parse floating point literal."_b) };
		}
		auto literal = make_ast_expr<LiteralExpr>(p->allocator, find_type(p, U"double"_b), AST_EXPR_RVALUE);
		literal->token = p->current_token;
		literal->f64_value = f;
		next(p);
		return { literal, NULL };
	}

	if (p->current_token_flags & CTOKEN_FLAG_INTEGER) {
		s64 u;
		bool ok = parse_integer(p->current_token, &u);
		if (!ok) {
			return { NULL, parser_error(p, U"Could not parse integer literal."_b) };
		}
		auto literal = make_ast_expr<LiteralExpr>(p->allocator, find_type(p, U"int"_b), AST_EXPR_RVALUE);
		literal->token = p->current_token;
		literal->s64_value = u;
		next(p);
		return { literal, NULL };
	}

	auto [unary_op_prec, _] = get_prefix_unary_operator_prec_assoc(tok);
	if (unary_op_prec > 0) {
		auto op = tok;
		next(p);
		auto [expr, e] = parse_expr(p, unary_op_prec);
		if (e) {
			return { NULL, e };
		}
		if (tok == "*") {
			if (!is_pointer(expr->expr_type)) {
				return { NULL, parser_error(p, U"Operator '*' can only be applied to pointer types.", op) };
			}
		}
		if (tok == "&") {
			if (!expr->is_lvalue) {
				return { NULL, parser_error(p, U"Operator '&' can only be applied to lvalues.", op) };
			}
		}
		if (tok == "++" || tok == "--") {
			if (!expr->is_lvalue) {
				return { NULL, parser_error(p, U"Operator '%' can only be applied to lvalues.", op) };
			}
		}
		if (tok == "!" || tok == "~") {
			if (!is_integer(expr->expr_type)) {
				return { NULL, parser_error(p, U"Operator '%' can only be applied to integer types.", op) };
			}
		}
		if (!is_numeric(expr->expr_type)) {
			return { NULL, parser_error(p, U"Operator '%' can only be applied to numeric types, got '%'.", op, expr->expr_type->name) };
		}
		auto unary = make_ast_expr<CUnaryExpr>(p->allocator, expr->expr_type, AST_EXPR_RVALUE);
		unary->expr = expr;
		unary->op = tok;
		return { unary, NULL };
	}

	if (tok == "("_b) {
		next(p);
		auto [expr, e] = parse_expr(p, 0);
		if (e) {
			return { NULL, e };
		}
		if (peek(p) != ")"_b) {
			return { NULL, parser_error(p, U"Expected a )"_b) };
		}
		next(p);
		return { expr, NULL };
	}

	AstNode* sym = lookup_symbol(p, tok);
	if (!sym) {
		return { NULL, parser_error(p, U"Unknown identifier"_b) };
	}
	AstExpr* lhs = NULL;
	if (auto type = reflect_cast<AstType>(sym)) {
		next(p);
		// @TODO: parse initializer.
		return { NULL, parser_error(p, U"TODO: type initializer"_b) };
	} else if (auto var = reflect_cast<AstVar>(sym)) {
		next(p);
		auto expr = make_ast_expr<AstVariableAccess>(p->allocator, var->var_type, AST_EXPR_LVALUE);
		expr->var = var;
		lhs = expr;
	} else {		
		return { NULL, parser_error(p, U"Expected type or variable"_b) };
	}

	while (true) {
		auto tok = peek(p);

		auto [postfix_op_prec, left_assoc] = get_postfix_unary_operator_prec_assoc(tok);
		if (postfix_op_prec > 0) {
			if (tok == "++"_b || tok == "--"_b) {
				if (!is_numeric(lhs->expr_type)) {
					return { NULL, parser_error(p, U"Operator '%' can only be applied to numeric types.", tok) };
				}
				if (!lhs->is_lvalue) {
					return { NULL, parser_error(p, U"Operator '%' can only be applied to lvalues.", tok) };
				}
			}
			next(p);
			auto node = make_ast_node<CPostfixExpr>(p->allocator);
			node->lhs = lhs;
			node->op = tok;
			lhs = node;
			continue;
		}
		if (tok == "("_b) {
			auto func = reflect_cast<AstFunction>(lhs);
			if (!func) {
				return { NULL, parser_error(p, U"Expected a function, got %"_b, lhs->type->name) };
			}
			auto [call, e] = parse_function_call(p, func);
			if (e) {
				return { NULL, e };
			}
			lhs = call;
			continue;
		}
		if (tok == "."_b) {
			if (!is_struct(lhs->expr_type)) {
				return { NULL, parser_error(p, U"Expected a struct, got %"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [ident, e] = parse_ident(p);
			if (e) {
				return { NULL, e };
			}
			auto [member, ok] = get_struct_member(p, lhs->expr_type, ident);
			if (!ok) {
				return { NULL, parser_error(p, U"Member '%' not found in struct '%'.", ident, lhs->expr_type->name) };
			}
			auto node = make_ast_expr<AstVarMemberAccess>(p->allocator, member.type, lhs->is_lvalue);
			node->lhs = lhs;
			node->member = ident;
			lhs = node;
			continue;
		}
		if (tok == "["_b) {
			bool ok = is_pointer(lhs->expr_type) || is_array(lhs->expr_type);
			if (!ok) {
				return { NULL, parser_error(p, U"Expected an array or pointer, got %"_b, lhs->expr_type->name) };
			}
			next(p);
			auto [expr, e] = parse_expr(p, 0);
			if (e) {
				return { NULL, e };
			}
			if (!is_integer(expr->expr_type)) {
				return { NULL, parser_error(p, U"Array index must be an integer, got %"_b, expr->expr_type->name) };
			}
			if (peek(p) != "]"_b) {
				return { NULL, parser_error(p, U"Expected a ]"_b) };
			}
			next(p);
			auto element_type = get_element_type(lhs->expr_type);
			if (!element_type) {
				panic(p, U"Unexpected error. expected an array or pointer, got %"_b, lhs->expr_type->name);
			}
			auto node = make_ast_expr<AstArrayAccess>(p->allocator, element_type, AST_EXPR_LVALUE);
			node->lhs = lhs;
			node->index = expr;
			lhs = node;
			continue;
		}
		break;
	}

	return { lhs, NULL };
}

Tuple<AstExpr*, Error*> parse_expr(CLikeParser* p, s32 min_prec) {
	auto [lhs, e] = parse_primary_expr(p);
	if (e) {
		return { NULL, e };
	}

	while (true) {
		auto tok = peek(p);
		auto [prec, left_assoc] = get_binary_operator_prec_assoc(tok);
		if (prec == 0) {
			break;
		}
		if (prec < min_prec) {
			break;
		}
		if (tok == "?"_b) {
			next(p);
			auto [then_expr, e2] = parse_expr(p, 0);
			if (e2) {
				return { NULL, e2 };
			}
			auto tok = peek(p);
			if (tok != ":"_b) {
				return { NULL, parser_error(p, U"Expected a :"_b) };
			}
			next(p);
			auto [else_expr, e3] = parse_expr(p, left_assoc ? prec + 1 : prec);
			if (e3) {
				return { NULL, e3 };
			}
			auto node = make_ast_node<AstTernary>(p->allocator);
			node->cond = lhs;
			node->then = then_expr;
			node->else_ = else_expr;
			lhs = node;
			continue;
		}
		next(p);
		auto [rhs, e] = parse_expr(p, left_assoc ? prec + 1 : prec);
		if (e) {
			return { NULL, e };
		}
		auto node = make_ast_node<CBinaryExpr>(p->allocator);
		node->lhs = lhs;
		node->rhs = rhs;
		node->op = tok;
		lhs = node;
	}
	return { lhs, NULL };
}

Tuple<AstNode*, Error*> parse_if(CLikeParser* p) {
	if (peek(p) != "("_b) {
		return { NULL, parser_error(p, U"Expected ("_b) };
	}
	next(p);
	auto [expr, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	return { NULL, parser_error(p, U"TODO: parse if"_b) };
}

Tuple<AstNode*, Error*> parse_block(CLikeParser* p);
Tuple<AstNode*, Error*> parse_stmt(CLikeParser* p);

Tuple<AstNode*, Error*> parse_block_or_one_stmt(CLikeParser* p) {
	auto tok = peek(p);
	if (tok == "{"_b) {
		next(p);
		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		return { block, NULL };
	} else {
		auto [stmt, e] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		auto block = make_ast_node<AstBlock>(p->allocator);
		block->statements.add(stmt);
		return { block, NULL };
	}
}

struct AstFor: AstNode {
	AstNode* init_expr = NULL;
	AstNode* cond_expr = NULL;
	AstNode* incr_expr = NULL;
	AstNode* body = NULL;

	REFLECT(AstFor) {
		BASE_TYPE(AstNode);
		MEMBER(init_expr);
		MEMBER(cond_expr);
		MEMBER(incr_expr);
		MEMBER(body);
	}
};

Tuple<AstNode*, Error*> parse_for(CLikeParser* p) {
	if (peek(p) != "("_b) {
		return { NULL, parser_error(p, U"Expected ("_b) };
	}
	next(p);
	auto [init_expr, e] = parse_expr(p, 0);
	if (e) {
		return { NULL, e };
	}
	if (peek(p) != ";"_b) {
		return { NULL, parser_error(p, U"Expected ;"_b) };
	}
	next(p);
	auto [cond_expr, e2] = parse_expr(p, 0);
	if (e2) {
		return { NULL, e2 };
	}
	if (peek(p) != ";"_b) {
		return { NULL, parser_error(p, U"Expected ;"_b) };
	}
	next(p);
	auto [incr_expr, e3] = parse_expr(p, 0);
	if (e3) {
		return { NULL, e3 };
	}
	if (peek(p) != ")"_b) {
		return { NULL, parser_error(p, U"Expected )"_b) };
	}
	next(p);
	auto [body, e4] = parse_block_or_one_stmt(p);
	if (e4) {
		return { NULL, e4 };
	}
	auto node = make_ast_node<AstFor>(p->allocator);
	node->init_expr = init_expr;
	node->cond_expr = cond_expr;
	node->incr_expr = incr_expr;
	node->body = body;
	return { node, NULL };
}

Tuple<AstNode*, Error*> parse_stmt(CLikeParser* p) {
	auto tok = peek(p);
	if (tok == "if"_b) {
		next(p);
		return parse_if(p);
	} else if (tok == "for"_b) {
		next(p);
		return parse_for(p);
	}

	auto type = find_type(p, tok);
	if (type) {
		next(p);
		auto [ident, e] = parse_ident(p);
		if (e) {
			return { NULL, e };
		}
		auto [decl, ee] = parse_var_decl(p, type, ident);
		if (ee) {
			return { NULL, ee };
		}
		return { decl, NULL };
	}
	auto [expr, e] = parse_expr(p, 0);
	return { expr, e };
}

Tuple<AstNode*, Error*> parse_block(CLikeParser* p) {
	auto tok = peek(p);
	if (tok != U"{"_b) {
		return { NULL, parser_error(p, U"Expected a {"_b) };
	}
	next(p);
	auto block = make_ast_node<AstBlock>(p->allocator);
	block->statements.allocator = p->allocator;
	p->scope.add(block);
	defer { p->scope.pop_last(); };
	
	while (true) {
		auto tok = peek(p);
		if (tok == U"}"_b) {
			next(p);
			break;
		}
		auto [stmt, e] = parse_stmt(p);
		if (e) {
			return { NULL, e };
		}
		tok = peek(p);
		if (tok != U";"_b) {
			return { NULL, parser_error(p, U"Expected ;"_b) };
		}
		next(p);
		block->statements.add(stmt);
	}
	return { block, NULL };
}

Tuple<AstNode*, Error*> parse_function(CLikeParser* p, AstType* return_type, UnicodeString name) {
	Array<AstFunctionArg*> args = { .allocator = p->allocator };
	defer { args.free(); };

	while (true) {
		auto tok = next(p);
		if (tok == U")"_b) {
			next(p);
			break;
		} else if (tok != U","_b) {
			return { NULL, parser_error(p, U"Expected ')' or ','"_b) };
		}
		next(p);
		auto [type, e] = parse_type(p);
		if (e) {
			return { NULL, e };
		}
		auto [arg_name, e2] = parse_ident(p);
		if (e2) {
			return { NULL, e2 };
		}
		auto arg_node = make_ast_node<AstFunctionArg>(p->allocator);
		arg_node->name = arg_name;
		arg_node->arg_type = type;
		args.add(arg_node);
	}

	auto f = make_ast_node<AstFunction>(p->allocator);
	f->args.allocator = p->allocator;
	f->return_type = return_type;
	f->name = name;
	f->args = args;

	auto tok = peek(p);
	if (tok == ";") {
		next(p);
		return { f, NULL };
	}
	if (tok == "{") {
		auto [block, e] = parse_block(p);
		if (e) {
			return { NULL, e };
		}
		f->block = (AstBlock*) block;
		return { f, NULL };
	}

	return { NULL, parser_error(p, U"Expected ; or { after function header"_b) }; 
}

Tuple<AstNode*, Error*> parse_top_level(CLikeParser* p) {
	auto tok = peek(p);
	auto [type, error] = parse_type(p);
	if (error) {
		return { NULL, error };
	}
	auto [ident, e] = parse_ident(p);
	if (e) {
		return { NULL, e };
	}
	tok = peek(p);
	if (tok == U"("_b) {
		return parse_function(p, type, ident);
	} else if (tok == U","_b || tok == U";"_b || tok == U"="_b) {
		return parse_var_decl(p, type, ident);
	} else {
		return { NULL, parser_error(p, U"Expected ( or , or ; or ="_b) };
	}
}

Tuple<AstNode*, Error*> parse_c_like(UnicodeString str) {
	CLikeParser p;
	p.allocator = make_arena_allocator();
	p.program = make_ast_node<CLikeProgram>(p.allocator);
	p.program->globals.allocator = p.allocator;
	p.str = str;
	p.scope.add(p.program);

	add_c_type_alias(&p, reflect_type_of<s64>(), U"int"_b);
	add_c_type_alias(&p, reflect_type_of<double>(), U"double"_b);
	add_c_type_alias(&p, reflect_type_of<float>(), U"float"_b);
	add_c_type_alias(&p, reflect_type_of<void>(), U"void"_b);
	add_c_type_alias(&p, reflect_type_of<Vector2_f32>(), U"vec2"_b);
	add_c_type_alias(&p, reflect_type_of<Vector3_f32>(), U"vec3"_b);
	add_c_type_alias(&p, reflect_type_of<Vector4_f64>(), U"vec4"_b);

	auto float_type = find_type(&p, U"float"_b);

	add_shader_intrinsic_var(&p, U"FC"_b, find_type(&p, U"vec2"_b)); // frag coord
	add_shader_intrinsic_var(&p, U"r"_b, find_type(&p, U"vec2"_b)); // resolution
	add_shader_intrinsic_var(&p, U"o"_b, find_type(&p, U"vec4"_b)); // out color
	add_shader_intrinsic_var(&p, U"t"_b, find_type(&p, U"float"_b)); // time?
	add_shader_intrinsic_func(&p, U"hsv"_b, find_type(&p, U"vec3"_b), { float_type, float_type, float_type });
	add_shader_intrinsic_func(&p, U"log"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"sin"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"cos"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"dot_vec3"_b, float_type, { find_type(&p, U"vec3"_b), find_type(&p, U"vec3"_b) });
	add_shader_intrinsic_func(&p, U"atan"_b, float_type, { float_type });
	add_shader_intrinsic_func(&p, U"length"_b, float_type, { find_type(&p, U"vec3"_b) });
	add_shader_intrinsic_func(&p, U"abs"_b, float_type, { float_type });

	while (peek(&p) != ""_b) {
		if (peek(&p) == U";"_b) {
			next(&p);
			continue;
		}
		auto [node, e] = parse_top_level(&p);
		if (e) {
			return { NULL, e };
		}
		p.program->globals.add(node);
	}
	return { p.program, NULL };
}
