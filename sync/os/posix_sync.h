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


using OsMutex = pthread_mutex_t;

void os_mutex_create(OsMutex* mutex) {
	*mutex = {};
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mutex, &attr);
}

void os_mutex_lock(OsMutex* mutex) {
	pthread_mutex_lock(mutex);
}

void os_mutex_unlock(OsMutex* mutex) {
	pthread_mutex_unlock(mutex);
}

void os_mutex_destroy(OsMutex* mutex) {
	pthread_mutex_destroy(mutex);
}

#if OS_DARWIN
	using OsSemaphore = semaphore_t;

	void os_semaphore_create(OsSemaphore* sem, u32 initial_value) {
		kern_return_t result = semaphore_create(current_task(), sem, SYNC_POLICY_FIFO, initial_value);
	}

	void os_semaphore_wait_and_decrement(OsSemaphore* sem) {
		while (true) {
			kern_return_t result = semaphore_wait(*sem);
			if (result != KERN_ABORTED) {
				break;
			}
		}
	}

	void os_semaphore_increment(OsSemaphore* sem) {
		semaphore_signal(*sem);
	}

	void os_semaphore_destroy(OsSemaphore* sem) {
		semaphore_destroy(current_task(), *sem);
	}

#else
	using OsSemaphore = sem_t;

	void os_semaphore_create(OsSemaphore* sem, u32 initial_value) {
	    int result = sem_init(sem, 0, initial_value);
	}

	void os_semaphore_wait_and_decrement(OsSemaphore* sem) {
		sem_wait(sem);
	}

	void os_semaphore_increment(OsSemaphore* sem) {
		sem_post(sem);
	}

	void os_semaphore_destroy(OsSemaphore* sem) {
		sem_destroy(sem);
	}
#endif
