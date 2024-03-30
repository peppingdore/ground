#pragma once

#include "testing.h"
#include "reflect.h"
#include "format.h"

struct A {
	int a;
	int b;
	REFLECT(A) {
		MEMBER(a); TAG("a");
		MEMBER(b);
	}
};

struct B {
	int c;
	int d;
	REFLECT(B) {
		MEMBER(c);
		MEMBER(d);
	}
};

struct C: A, B {
	int e;
	REFLECT(C) {
		BASE_TYPE(A);
		BASE_TYPE(B);
		MEMBER(e);
	}
};

TEST(reflect_struct_offset) {
	auto type = reflect_type_of<C>();
	EXPECT(type->kind == StructType::KIND);
	auto casted = (StructType*) type;
	EXPECT(casted->unflattened_members.count == 3);
	EXPECT(casted->unflattened_members[0]->offset == 0);
	EXPECT(casted->unflattened_members[1]->offset == sizeof(int) * 2);
	EXPECT(casted->unflattened_members[2]->offset == sizeof(int) * 4);
	EXPECT(casted->unflattened_members[0]->kind == STRUCT_MEMBER_KIND_BASE);
	EXPECT(casted->unflattened_members[1]->kind == STRUCT_MEMBER_KIND_BASE);
	EXPECT(casted->unflattened_members[2]->kind == STRUCT_MEMBER_KIND_PLAIN);
	EXPECT(casted->members.count == 5);
	EXPECT(casted->members[0]->offset == 0);
	EXPECT(casted->members[1]->offset == sizeof(int) * 1);
	EXPECT(casted->members[2]->offset == sizeof(int) * 2);
	EXPECT(casted->members[3]->offset == sizeof(int) * 3);
	EXPECT(casted->members[4]->offset == sizeof(int) * 4);
}
