#pragma once

#pragma once

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
T* make_ast_node() {
	auto x = make<T>();
	x->type = reflect_type_of<T>();
	return x;
}

struct AstStmt: AstNode {

	REFLECT(AstStmt) {
		BASE_TYPE(AstNode);
	}
};

struct AstExpr: AstStmt {
	REFLECT(AstExpr) {
		BASE_TYPE(AstStmt);
	}
};

template <typename T>
struct AstExprTyped: AstExpr {
	REFLECT(AstExprTyped) {}
};

struct AstMemberExpr: AstExpr {
	AstExpr*    sub;
	const char* name = NULL;

	REFLECT(AstMemberExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(sub);
		MEMBER(name);
	}
};

AstMemberExpr* Member(AstExpr* expr, const char* name) {
	auto node = make_ast_node<AstMemberExpr>();
	node->sub = expr;
	node->name = name;
	return node;
}

struct AstBlock: AstStmt {
	Array<AstStmt*> statements;

	REFLECT(AstBlock) {
		BASE_TYPE(AstStmt);
		MEMBER(statements);
	}
};

AstBlock* make_block() {
	return make_ast_node<AstBlock>();
}

template <typename T>
struct ExtractFunctionType;

template <typename R, typename... Args>
struct ExtractFunctionType<R(Args...)> {
	using Ret = R;
	using Args_Tuple = Tuple<Args...>;
};

struct AstIf: AstStmt {
	AstExpr*    cond  = NULL;
	AstBlock*   body  = NULL;
	AstIf*      elif  = NULL;
	AstBlock*   else_ = NULL;

	void operator=(auto func) {
		auto body = make_block();
		func();
	}

	REFLECT(AstIf) {
		BASE_TYPE(AstStmt);
		MEMBER(cond);
		MEMBER(body);
		MEMBER(elif);
		MEMBER(else_);
	}
};

struct AstFunction;

struct AstFunctionCall;

struct AstFunctionCall: AstExpr {
	AstFunction*    f;
	Array<AstExpr*> args;

	REFLECT(AstFunctionCall) {
		BASE_TYPE(AstExpr);
		MEMBER(args);
	}
};

template <typename R, typename... Types>
constexpr size_t get_arg_count(R(*f)(Types...)) {
	return sizeof...(Types);
}

struct ArgExpr: AstExpr {
	Type* arg_type = NULL;

	REFLECT(ArgExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(arg_type);
	}
};

struct AstFunction: AstNode {
	AstBlock*       block = NULL;
	Array<ArgExpr*> args;
	String          name;
	s64             var_id_counter = 0;

	template <typename R, typename... Args>
	AstFunction(R(*func)(AstFunction*, Args...)) {
		block = make_block();
		type = reflect_type_of<AstFunction>();
		auto arg = [&]<typename T>(AstExprTyped<T>* arg) {
			auto node = make_ast_node<ArgExpr>();
			node->arg_type = reflect_type_of<T>();
			args.add(node);
			return (AstExprTyped<T>*) node;
		};
		func(this, arg(Args{})...);
		assert(name.data && "AstFunction name is not set");
	}

	void add(AstStmt* stmt) {
		block->statements.add(stmt);
	}

	AstFunctionCall* operator()(auto... args) {
		auto node = make_ast_node<AstFunctionCall>();
		node->f = this;
		std::initializer_list<AstExpr*> x = { args... };
		for (auto it: x) {
			node->args.add(it);
		}
		return node;
	}

	REFLECT(AstFunction) {
		BASE_TYPE(AstNode);
		MEMBER(block);
	}
};

AstIf& If(AstFunction* f, AstExpr* expr) {
	auto node = make_ast_node<AstIf>();
	node->cond = expr;
	f->add(node);
	return *node;
}

void Stmt(AstFunction* f, AstStmt* stmt) {
	f->add(stmt);
}

struct AstReturn: AstStmt {
	AstExpr* expr = NULL;

	REFLECT(AstReturn) {
		BASE_TYPE(AstStmt);
		MEMBER(expr);
	}
};

void Return(AstFunction* f, AstExpr* expr) {
	auto ret = make_ast_node<AstReturn>();
	ret->expr = expr;
	f->add(ret);
}

struct AstIntrinCall: AstExpr {
	const char*     name = NULL;
	Array<AstExpr*> args;
	AstFunction*    fallback = NULL;

	REFLECT(AstIntrinCall) {
		BASE_TYPE(AstExpr);
		MEMBER(name);
		MEMBER(args);
		MEMBER(fallback);
	}
};

AstIntrinCall* Intrin(const char* name, auto... args) {
	auto node = make_ast_node<AstIntrinCall>();
	node->name = name;
	std::initializer_list<AstExpr*> x = { args... };
	for (auto it: x) {
		node->args.add(it);
	}
	return node;
}

AstIntrinCall* IntrinWithFallback(const char* name, AstFunction* f, auto... args) {
	auto node = make_ast_node<AstIntrinCall>();
	node->name = name;
	// @TODO: check whether fallbacks arguments are correct.
	node->fallback = f;
	std::initializer_list<AstExpr*> x = { args... };
	for (auto it: x) {
		node->args.add(it);
	}
	return node;
}

template <typename T>
AstFunction Mul = +[](AstFunction* f, AstExprTyped<T>* lhs, AstExprTyped<T>* rhs) {
	f->name = "Mul"_b;
	Return(f, Intrin("mul", lhs, rhs));
};

template <typename T>
AstFunction Add = +[](AstFunction* f, AstExprTyped<T>* lhs, AstExprTyped<T>* rhs) {
	f->name = "Add"_b;
	Return(f, Intrin("add", lhs, rhs));
};

template <typename T>
AstFunction Sub = +[](AstFunction* f, AstExprTyped<T>* lhs, AstExprTyped<T>* rhs) {
	f->name = "Sub"_b;
	Return(f, Intrin("sub", lhs, rhs));
};

struct AstMakeStructExpr: AstExpr {
	Type*           struct_type;
	Array<AstExpr*> members;

	REFLECT(AstMakeStructExpr) {
		BASE_TYPE(AstExpr);
		MEMBER(struct_type);
		MEMBER(members);
	}
};

struct AstVarDecl: AstStmt {
	Type*         var_type;
	UnicodeString name;
	s64           id = 0;
	AstExpr*      init = NULL;

	REFLECT(AstVarDecl) {
		BASE_TYPE(AstStmt);
		MEMBER(var_type);
		MEMBER(name);
		MEMBER(id);
		MEMBER(init);
	}
};

template <typename T>
AstVarDecl* DeclVar(AstFunction* f, const char* name = NULL) {
	auto var = make_ast_node<AstVarDecl>();
	var->var_type = reflect_type_of<T>();
	var->id = f->var_id_counter;
	var->name = name;
	f->var_id_counter += 1;
	f->add(var);
	auto expr = make_ast_node<AstVarDecl>();
	expr->var = var;
	return expr;
}

template <typename T>
AstMakeStructExpr* Make(auto... args) {
	auto node = make_ast_node<AstMakeStructExpr>();
	node->struct_type = reflect_type_of<T>();
	AstExpr* x[] = { args... };
	for (auto it: x) {
		node->members.add(it);
	}
	return node;
}
