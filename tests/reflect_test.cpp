#if 0
	`dirname "$0"`/../build.sh $0 $@; exit
#endif

#include "../grd_reflect.h"
#include "../grd_testing.h"
#include "../grd_format.h"

GRD_TEST(reflect_type_names) {
	GRD_EXPECT(strcmp(grd_reflect_type_of<int>()->name, "s32") == 0, grd_make_string(grd_reflect_type_of<int>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<char>()->name, "char") == 0, grd_make_string(grd_reflect_type_of<char>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<short>()->name, "s16") == 0, grd_make_string(grd_reflect_type_of<short>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<float>()->name, "f32") == 0, grd_make_string(grd_reflect_type_of<float>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<double>()->name, "f64") == 0, grd_make_string(grd_reflect_type_of<double>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<bool>()->name, "bool") == 0, grd_make_string(grd_reflect_type_of<bool>()->name));
	GRD_EXPECT(strcmp(grd_reflect_type_of<void>()->name, "void") == 0, grd_make_string(grd_reflect_type_of<void>()->name));
}

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
	GRD_EXPECT(type->kind == GrdStructType::KIND, grd_type_kind_as_c_str(&type->kind));
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
