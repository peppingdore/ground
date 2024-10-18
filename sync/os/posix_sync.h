#pragma once

#include "../../grd_base.h"

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


using GrdOsMutex = pthread_mutex_t;

void grd_os_mutex_create(GrdOsMutex* mutex) {
	*mutex = {};
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mutex, &attr);
}

void grd_os_mutex_lock(GrdOsMutex* mutex) {
	pthread_mutex_lock(mutex);
}

void grd_os_mutex_unlock(GrdOsMutex* mutex) {
	pthread_mutex_unlock(mutex);
}

void grd_os_mutex_destroy(GrdOsMutex* mutex) {
	pthread_mutex_destroy(mutex);
}

#if OS_DARWIN
	using GrdOsSemaphore = semaphore_t;

	void grd_os_semaphore_create(GrdOsSemaphore* sem, u32 initial_value) {
		kern_return_t result = semaphore_create(current_task(), sem, SYNC_POLICY_FIFO, initial_value);
	}

	void grd_os_semaphore_wait_and_decrement(GrdOsSemaphore* sem) {
		while (true) {
			kern_return_t result = semaphore_wait(*sem);
			if (result != KERN_ABORTED) {
				break;
			}
		}
	}

	void grd_os_semaphore_increment(GrdOsSemaphore* sem) {
		semaphore_signal(*sem);
	}

	void grd_os_semaphore_destroy(GrdOsSemaphore* sem) {
		semaphore_destroy(current_task(), *sem);
	}

#else
	using GrdOsSemaphore = sem_t;

	void grd_os_semaphore_create(GrdOsSemaphore* sem, u32 initial_value) {
	    int result = sem_init(sem, 0, initial_value);
	}

	void grd_os_semaphore_wait_and_decrement(GrdOsSemaphore* sem) {
		sem_wait(sem);
	}

	void grd_os_semaphore_increment(GrdOsSemaphore* sem) {
		sem_post(sem);
	}

	void grd_os_semaphore_destroy(GrdOsSemaphore* sem) {
		sem_destroy(sem);
	}
#endif
