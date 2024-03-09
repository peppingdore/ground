#pragma once

#include "../../base.h"

#if OS_DARWIN
	#include <mach/task.h>
	#include <mach/semaphore.h>
	#include <mach/mach_init.h>
#else
	#include <semaphore.h>
#endif

#include <pthread.h>
#include <unistd.h>
#include <signal.h>

using Thread_Id = pthread_t;

void os_sleep(u32 ms) {
	usleep(ms * 1000);
}

Thread_Id current_thread_id() {
	return pthread_self();
}

using Os_Mutex = pthread_mutex_t;

void os_mutex_create(Os_Mutex* mutex) {
	*mutex = {};
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mutex, &attr);
}

void os_mutex_lock(Os_Mutex* mutex) {
	pthread_mutex_lock(mutex);
}

void os_mutex_unlock(Os_Mutex* mutex) {
	pthread_mutex_unlock(mutex);
}

void os_mutex_destroy(Os_Mutex* mutex) {
	pthread_mutex_destroy(mutex);
}

#if OS_DARWIN
	using Os_Semaphore = semaphore_t;

	void os_semaphore_create(Os_Semaphore* sem, u32 initial_value) {
		kern_return_t result = semaphore_create(current_task(), sem, SYNC_POLICY_FIFO, initial_value);
	}

	void os_semaphore_wait_and_decrement(Os_Semaphore* sem) {
		while (true) {
			kern_return_t result = semaphore_wait(*sem);
			if (result != KERN_ABORTED) {
				break;
			}
		}
	}

	void os_semaphore_increment(Os_Semaphore* sem) {
		semaphore_signal(*sem);
	}

	void os_semaphore_destroy(Os_Semaphore* sem) {
		semaphore_destroy(current_task(), *sem);
	}

#else
	using Os_Semaphore = sem_t;

	void os_semaphore_create(Os_Semaphore* sem, u32 initial_value) {
	    int result = sem_init(sem, 0, initial_value);
	}

	void os_semaphore_wait_and_decrement(Os_Semaphore* sem) {
		sem_wait(sem);
	}

	void os_semaphore_increment(Os_Semaphore* sem) {
		sem_post(sem);
	}

	void os_semaphore_destroy(Os_Semaphore* sem) {
		sem_destroy(sem);
	}
#endif

using Os_Thread = pthread_t;
using Os_Thread_Return_Type = void*;

bool os_thread_start(Os_Thread* thread, Os_Thread_Return_Type (*proc)(void*), void* data) {
	pthread_t handle;
	int result = pthread_create(&handle, NULL, proc, data);
	if (result != 0) {
		return false;
	}
	*thread = handle;
	return true;
}

void os_thread_join(Os_Thread* thread) {
	pthread_join(*thread, NULL);
}

Thread_Id os_thread_get_id(Os_Thread* thread) {
	return *thread;
}
