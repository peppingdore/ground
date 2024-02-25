#pragma once

#include "atomics.h"
#include "os/os_sync.h"
#include "../type_utils.h"

static_assert(
	sizeof(Thread_Id) == 4 ||
	sizeof(Thread_Id) == 8
);
using Spinlock_Num_Type = std::conditional<sizeof(Thread_Id) == 4, s32, s64>::type;
struct alignas(sizeof(Thread_Id) * 2) Spinlock_Atomic_Data {
	Thread_Id         locking_thread_id = 0;
	Spinlock_Num_Type lock_count = 0;
};
static_assert(
	sizeof(Spinlock_Atomic_Data) == 8 ||
	sizeof(Spinlock_Atomic_Data) == 16
);
static_assert(!does_type_have_padding<Spinlock_Atomic_Data>());

struct Spinlock {
	Spinlock_Atomic_Data data;

	void lock() {
		auto current = atomic_load(&data);
		auto thread_id = current_thread_id();

		if (current.locking_thread_id == current_thread_id() && data.lock_count != 0) {
			assert(data.lock_count > 0);
			atomic_load_add<Spinlock_Num_Type>(&data.lock_count, 1);
			return;
		}


		Spinlock_Atomic_Data store_value = {
			.locking_thread_id = thread_id,
			.lock_count = 1
		};

		while (true) {
			auto prev = compare_and_swap<Spinlock_Atomic_Data, Memory_Order::Acquire>(&data, {}, store_value);
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
			atomic_load_add<Spinlock_Num_Type, Memory_Order::Release>(&data.lock_count, -1);
		} else {
			atomic_store<Spinlock_Atomic_Data, Memory_Order::Release>(&data, {});
		}
	}
};
