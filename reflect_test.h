#pragma once

#include "grd_testing.h"
#include "reflect.h"
#include "format.h"

struct A {
	int a;
	int b;
	GRD_REFLECT(A) {
		GRD_MEMBER(a); GRD_TAG("a");
		GRD_MEMBER(b);
	}
};

struct B {
	int c;
	int d;
	GRD_REFLECT(B) {
		GRD_MEMBER(c);
		GRD_MEMBER(d);
	}
};

struct C: A, B {
	int e;
	GRD_REFLECT(C) {
		GRD_BASE_TYPE(A);
		GRD_BASE_TYPE(B);
		GRD_MEMBER(e);
	}
};

GRD_TEST(reflect_struct_offset) {
	auto type = grd_reflect_type_of<C>();
	EXPECT(type->kind == StructType::KIND);
	auto casted = (StructType*) type;
	EXPECT(casted->unflattened_members.count == 3);
	EXPECT(casted->unflattened_members[0]->offset == 0);
	EXPECT(casted->unflattened_members[1]->offset == sizeof(int) * 2);
	EXPECT(casted->unflattened_members[2]->offset == sizeof(int) * 4);
	EXPECT(casted->unflattened_members[0]->kind == STRUCT_GRD_MEMBER_KIND_BASE);
	EXPECT(casted->unflattened_members[1]->kind == STRUCT_GRD_MEMBER_KIND_BASE);
	EXPECT(casted->unflattened_members[2]->kind == STRUCT_GRD_MEMBER_KIND_PLAIN);
	EXPECT(casted->members.count == 5);
	EXPECT(casted->members[0]->offset == 0);
	EXPECT(casted->members[1]->offset == sizeof(int) * 1);
	EXPECT(casted->members[2]->offset == sizeof(int) * 2);
	EXPECT(casted->members[3]->offset == sizeof(int) * 3);
	EXPECT(casted->members[4]->offset == sizeof(int) * 4);
}


struct Base {
	Type* type;

	GRD_REFLECT(Base) {
		GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
	}
};

struct Derived: Base {

	GRD_REFLECT(Derived) {
		GRD_BASE_TYPE(Base);
	}
};

GRD_TEST(reflect_real_type_test) {
	Derived d;
	d.type = grd_reflect_type_of<decltype(d)>();

	Base* b = &d;

	auto type = grd_get_real_type(grd_reflect_type_of(*b), b);
	EXPECT(type == grd_reflect_type_of<Derived>());
}
