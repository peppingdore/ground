#pragma once

template <typename T>
struct Scoped_Lock_Impl {
	T& lock;
	Scoped_Lock_Impl(T& lock): lock(lock) { lock.lock(); }
	~Scoped_Lock_Impl() { lock.unlock(); }
};
#define Scoped_Lock(lock) Scoped_Lock_Impl CONCAT(__scoped_lock, __LINE__)(lock);
