#pragma once

#include "grd_atomics.h"
#include "os/os_sync.h"
#include "../grd_type_utils.h"
#include "../thread/os/grd_os_thread.h"

static_assert(
	sizeof(GrdThreadId) == 4 ||
	sizeof(GrdThreadId) == 8
);
using GrdSpinlockNumType = std::conditional<sizeof(GrdThreadId) == 4, s32, s64>::type;
struct alignas(sizeof(GrdThreadId) * 2) GrdSpinlock {
	GrdThreadId        locking_thread_id = 0;
	GrdSpinlockNumType lock_count = 0;
};
static_assert(
	sizeof(GrdSpinlock) == 8 ||
	sizeof(GrdSpinlock) == 16
);
static_assert(!grd_does_type_have_padding<GrdSpinlock>());

void grd_lock(GrdSpinlock* x) {
	assert(grd_is_aligned(x, sizeof(GrdSpinlock)));
	auto loaded = grd_atomic_load(x);
	auto thread_id = grd_current_thread_id();

	if (loaded.locking_thread_id == thread_id && x->lock_count != 0) {
		assert(x->lock_count > 0);
		grd_atomic_load_add<GrdSpinlockNumType>(&x->lock_count, 1LL);
		return;
	}

	GrdSpinlock store_value = {
		.locking_thread_id = thread_id,
		.lock_count = 1
	};

	while (true) {
		auto prev = grd_compare_and_swap<GrdSpinlock, GrdMemoryOrder::Acquire>(x, {}, store_value);
		if (prev.lock_count == 0) {
			break;
		}
	}
}

void grd_unlock(GrdSpinlock* x) {
	assert(grd_is_aligned(x, sizeof(GrdSpinlock)));
	auto prev = grd_atomic_load(x);
	assert(prev.lock_count >= 1);
	assert(prev.locking_thread_id == grd_current_thread_id());

	if (prev.lock_count > 1) {
		grd_atomic_load_add<GrdSpinlockNumType, GrdMemoryOrder::Release>(&x->lock_count, -1LL);
	} else {
		grd_atomic_store<GrdSpinlock, GrdMemoryOrder::Release>(x, {});
	}
}
