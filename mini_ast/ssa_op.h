#pragma once

#include "../reflect.h"


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
	MemberAccess,
	Swizzle,
	Store,
	Load,
	Alloca,
	Jump,
	CondJump,
	Return,
	ReturnVoid,
	UnaryNeg,
	UnaryNot,
	ZeroInit,
	GetElementPtr,
	FunctionArg,
	InsertValue,
	ExtractValue,
	MakeStruct,
	GlobalVar,

	SsaOpSpirvStart = 2000,
	SpvUniform,
	SpvBuiltinPosition,
	SsaOpSpirvEnd = 3000,
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
	ENUM_VALUE(MemberAccess);
	ENUM_VALUE(Swizzle);
	ENUM_VALUE(Store);
	ENUM_VALUE(Load);
	ENUM_VALUE(Alloca);
	ENUM_VALUE(Jump);
	ENUM_VALUE(CondJump);
	ENUM_VALUE(Return);
	ENUM_VALUE(ReturnVoid);
	ENUM_VALUE(UnaryNeg);
	ENUM_VALUE(UnaryNot);
	ENUM_VALUE(ZeroInit);
	ENUM_VALUE(GetElementPtr);
	ENUM_VALUE(FunctionArg);
	ENUM_VALUE(InsertValue);
	ENUM_VALUE(ExtractValue);
	ENUM_VALUE(MakeStruct);
	ENUM_VALUE(GlobalVar);
	ENUM_VALUE(SsaOpSpirvStart);
	ENUM_VALUE(SpvUniform);
	ENUM_VALUE(SpvBuiltinPosition);
	ENUM_VALUE(SsaOpSpirvEnd);
}
