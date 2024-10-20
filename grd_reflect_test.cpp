#pragma once

#include "grd_testing.h"
#include "grd_reflect.h"
#include "grd_format.h"

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
	GRD_EXPECT(type->kind == GrdStructType::KIND);
	auto casted = (GrdStructType*) type;
	GRD_EXPECT(casted->unflattened_members.count == 3);
	GRD_EXPECT(casted->unflattened_members[0]->offset == 0);
	GRD_EXPECT(casted->unflattened_members[1]->offset == sizeof(int) * 2);
	GRD_EXPECT(casted->unflattened_members[2]->offset == sizeof(int) * 4);
	GRD_EXPECT(casted->unflattened_members[0]->kind == GRD_STRUCT_MEMBER_KIND_BASE);
	GRD_EXPECT(casted->unflattened_members[1]->kind == GRD_STRUCT_MEMBER_KIND_BASE);
	GRD_EXPECT(casted->unflattened_members[2]->kind == GRD_STRUCT_MEMBER_KIND_PLAIN);
	GRD_EXPECT(casted->members.count == 5);
	GRD_EXPECT(casted->members[0]->offset == 0);
	GRD_EXPECT(casted->members[1]->offset == sizeof(int) * 1);
	GRD_EXPECT(casted->members[2]->offset == sizeof(int) * 2);
	GRD_EXPECT(casted->members[3]->offset == sizeof(int) * 3);
	GRD_EXPECT(casted->members[4]->offset == sizeof(int) * 4);
}


struct Base {
	GrdType* type;

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
	GRD_EXPECT(type == grd_reflect_type_of<Derived>());
}
