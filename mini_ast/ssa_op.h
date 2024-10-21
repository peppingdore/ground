#pragma once

#include "../grd_reflect.h"

#define GRD_SSA_OP_LIST\
	GRD_SSA_OP_WITH_VALUE(Nop, 0)\
	GRD_SSA_OP(Phi)\
	GRD_SSA_OP(Add)\
	GRD_SSA_OP(Sub)\
	GRD_SSA_OP(Mul)\
	GRD_SSA_OP(Div)\
	GRD_SSA_OP(Less)\
	GRD_SSA_OP(Greater)\
	GRD_SSA_OP(Equal)\
	GRD_SSA_OP(Const)\
	GRD_SSA_OP(Call)\
	GRD_SSA_OP(MemberAccess)\
	GRD_SSA_OP(SwizzleExpr)\
	GRD_SSA_OP(SwizzleIndices)\
	GRD_SSA_OP(Store)\
	GRD_SSA_OP(Load)\
	GRD_SSA_OP(Alloca)\
	GRD_SSA_OP(Jump)\
	GRD_SSA_OP(CondJump)\
	GRD_SSA_OP(Return)\
	GRD_SSA_OP(ReturnVoid)\
	GRD_SSA_OP(UnaryNeg)\
	GRD_SSA_OP(UnaryNot)\
	GRD_SSA_OP(ZeroInit)\
	GRD_SSA_OP(GetElementPtr)\
	GRD_SSA_OP(FunctionArg)\
	GRD_SSA_OP(InsertValue)\
	GRD_SSA_OP(ExtractValue)\
	GRD_SSA_OP(MakeStruct)\
	GRD_SSA_OP(GlobalVar)\
	GRD_SSA_OP_WITH_VALUE(SsaOpSpirvStart, 2000)\
	GRD_SSA_OP(SpvUniform)\
	GRD_SSA_OP(SpvBuiltinPosition)\
	GRD_SSA_OP_WITH_VALUE(SsaOpSpirvEnd, 3000)

enum class GrdSsaOp {
	#define GRD_SSA_OP_WITH_VALUE(name, value) name = value,
	#define GRD_SSA_OP(name) name,
	GRD_SSA_OP_LIST
};
GRD_REFLECT(GrdSsaOp) {
	#define GRD_SSA_OP_WITH_VALUE(name, value) GRD_ENUM_VALUE(name);
	#define GRD_SSA_OP(name) GRD_ENUM_VALUE(name);
	GRD_SSA_OP_LIST
}
