#pragma once

#include "os/os_sync.h"
#include "../allocator.h"

struct Mutex {
	OsMutex os_mutex;

	void lock() {
		os_mutex_lock(&os_mutex);
	}

	void unlock() {
		os_mutex_unlock(&os_mutex);
	}

	void free() {
		os_mutex_destroy(&os_mutex);
	}
};

void grd_make_mutex(Mutex* out_mutex) {
	os_mutex_create(&out_mutex->os_mutex);
}

Mutex* grd_make_mutex() {
	auto x = grd_make<Mutex>();
	grd_make_mutex(x);
	return x;
}
