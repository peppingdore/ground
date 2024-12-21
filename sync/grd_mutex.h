#pragma once

#include "os/grd_os_sync.h"
#include "../grd_allocator.h"

struct GrdMutex {
	GrdOsMutex os_mutex;

	void lock() {
		grd_os_mutex_lock(&os_mutex);
	}

	void unlock() {
		grd_os_mutex_unlock(&os_mutex);
	}

	void free() {
		grd_os_mutex_destroy(&os_mutex);
	}
};

GRD_DEDUP void grd_make_mutex(GrdMutex* out_mutex) {
	grd_os_mutex_create(&out_mutex->os_mutex);
}

GRD_DEDUP GrdMutex* grd_make_mutex() {
	auto x = grd_make<GrdMutex>();
	grd_make_mutex(x);
	return x;
}
