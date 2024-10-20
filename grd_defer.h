#pragma once

#include "grd_base.h"

template<typename T>
struct GrdDeferScope {
	T f;
	GrdDeferScope(T f): f(f) {}
	~GrdDeferScope() { f(); }
	GrdDeferScope(const GrdDeferScope&);
	GrdDeferScope& operator=(const GrdDeferScope&);
};

struct GrdDeferMaker {
	template<typename T>
	GrdDeferScope<T> operator+(T t) {
		return t;
	}
};

#define grd_defer const auto& GRD_CONCAT(grd_defer__, __LINE__ ) = GrdDeferMaker() + [&]()
#define grd_defer_x(stmt) grd_defer { stmt; };
