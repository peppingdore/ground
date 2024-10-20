#pragma once

#include "../grd_reflect.h"


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
	SwizzleExpr,
	SwizzleIndices,
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
GRD_REFLECT(SsaOp) {
	GRD_ENUM_VALUE(Nop);
	GRD_ENUM_VALUE(Phi);
	GRD_ENUM_VALUE(Add);
	GRD_ENUM_VALUE(Sub);
	GRD_ENUM_VALUE(Mul);
	GRD_ENUM_VALUE(Div);
	GRD_ENUM_VALUE(Less);
	GRD_ENUM_VALUE(Greater);
	GRD_ENUM_VALUE(Equal);
	GRD_ENUM_VALUE(Const);
	GRD_ENUM_VALUE(Call);
	GRD_ENUM_VALUE(MemberAccess);
	GRD_ENUM_VALUE(SwizzleExpr);
	GRD_ENUM_VALUE(SwizzleIndices);
	GRD_ENUM_VALUE(Store);
	GRD_ENUM_VALUE(Load);
	GRD_ENUM_VALUE(Alloca);
	GRD_ENUM_VALUE(Jump);
	GRD_ENUM_VALUE(CondJump);
	GRD_ENUM_VALUE(Return);
	GRD_ENUM_VALUE(ReturnVoid);
	GRD_ENUM_VALUE(UnaryNeg);
	GRD_ENUM_VALUE(UnaryNot);
	GRD_ENUM_VALUE(ZeroInit);
	GRD_ENUM_VALUE(GetElementPtr);
	GRD_ENUM_VALUE(FunctionArg);
	GRD_ENUM_VALUE(InsertValue);
	GRD_ENUM_VALUE(ExtractValue);
	GRD_ENUM_VALUE(MakeStruct);
	GRD_ENUM_VALUE(GlobalVar);
	GRD_ENUM_VALUE(SsaOpSpirvStart);
	GRD_ENUM_VALUE(SpvUniform);
	GRD_ENUM_VALUE(SpvBuiltinPosition);
	GRD_ENUM_VALUE(SsaOpSpirvEnd);
}
