#pragma once

template <typename T>
struct ScopedLockImpl {
	T& lock;
	ScopedLockImpl(T& lock): lock(lock) { lock.lock(); }
	~ScopedLockImpl() { lock.unlock(); }
};
#define ScopedLock(lock) ScopedLockImpl CONCAT(__scoped_lock, __LINE__)(lock);
