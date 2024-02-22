#pragma once

#include "base.h"

template<typename T>
struct DeferScope {
	T lambda;
	DeferScope(T lambda): lambda(lambda) {}
	~DeferScope() { lambda(); }
	DeferScope(const DeferScope&);
	DeferScope& operator=(const DeferScope&);
};

struct DeferMaker {
	template<typename T>
	DeferScope<T> operator+(T t) {
		return t;
	}
};

#define defer const auto& CONCAT(defer__, __LINE__ ) = DeferMaker() + [&]()
