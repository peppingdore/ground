#pragma once

#include "atomics.h"
#include "os/os_sync.h"
#include "../type_utils.h"
#include "../thread/os/os_thread.h"

static_assert(
	sizeof(ThreadId) == 4 ||
	sizeof(ThreadId) == 8
);
using Spinlock_Num_Type = std::conditional<sizeof(ThreadId) == 4, s32, s64>::type;
struct alignas(sizeof(ThreadId) * 2) SpinlockAtomicData {
	ThreadId          locking_thread_id = 0;
	Spinlock_Num_Type lock_count = 0;
};
static_assert(
	sizeof(SpinlockAtomicData) == 8 ||
	sizeof(SpinlockAtomicData) == 16
);
static_assert(!does_type_have_padding<SpinlockAtomicData>());

struct Spinlock {
	SpinlockAtomicData data;

	void lock() {
		auto current = atomic_load(&data);
		auto thread_id = current_thread_id();

		if (current.locking_thread_id == thread_id && data.lock_count != 0) {
			assert(data.lock_count > 0);
			atomic_load_add<Spinlock_Num_Type>(&data.lock_count, 1);
			return;
		}

		SpinlockAtomicData store_value = {
			.locking_thread_id = thread_id,
			.lock_count = 1
		};

		while (true) {
			auto prev = compare_and_swap<SpinlockAtomicData, MemoryOrder::Acquire>(&data, {}, store_value);
			if (prev.lock_count == 0) {
				break;
			}
		}
	}

	void unlock() {
		auto current = atomic_load(&data);
		assert(current.lock_count >= 1);
		assert(current.locking_thread_id == current_thread_id());

		if (current.lock_count > 1) {
			atomic_load_add<Spinlock_Num_Type, MemoryOrder::Release>(&data.lock_count, -1);
		} else {
			atomic_store<SpinlockAtomicData, MemoryOrder::Release>(&data, {});
		}
	}
};
