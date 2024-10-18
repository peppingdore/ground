#pragma once

#include "grd_base.h"

template <typename T>
struct GrdScopedLockImpl {
	T& x;
	GrdScopedLockImpl(T& x): x(x) { grd_lock(&x); }
	~GrdScopedLockImpl() { grd_unlock(&x); }
};
#define GrdScopedLock(x) GrdScopedLockImpl GRD_CONCAT(__scoped_lock, __LINE__)(x);

template <typename T>
struct GrdScopedRestoreImpl {
	T& dst;
	T  value;

	GrdScopedRestoreImpl(T& dst): dst(dst), value(dst) {};
	~GrdScopedRestoreImpl() { dst = value; };
};
#define GrdScopedRestore(x) GrdScopedRestoreImpl GRD_CONCAT(__scoped_restore, __LINE__)(x);
