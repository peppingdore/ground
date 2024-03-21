#pragma once

template <typename T>
struct ScopedLockImpl {
	T& lock;
	ScopedLockImpl(T& lock): lock(lock) { lock.lock(); }
	~ScopedLockImpl() { lock.unlock(); }
};
#define ScopedLock(lock) ScopedLockImpl CONCAT(__scoped_lock, __LINE__)(lock);

template <typename T>
struct ScopedRestoreImpl {
	T& dst;
	T  value;

	ScopedRestoreImpl(T& dst): dst(dst), value(dst) {};
	~ScopedRestoreImpl() { dst = value; };
};
#define ScopedRestore(x) ScopedRestoreImpl CONCAT(__scoped_restore, __LINE__)(x);
