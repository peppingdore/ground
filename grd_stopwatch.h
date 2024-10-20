#pragma once

#include "grd_base.h"

#if GRD_OS_WINDOWS
	#include "Windows.h"
#elif GRD_OS_DARWIN
	#include <mach/clock.h>
	#include <mach/mach.h>
#elif GRD_OS_LINUX
	#include <time.h>
#endif

struct GrdStopwatch {
#if GRD_OS_WINDOWS
	s64 freq_per_second;
#endif
	s64 last_us;
};

s64 grd_get_current_nanos(GrdStopwatch* w) {
#if GRD_OS_WINDOWS
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	counter.QuadPart *= 1000000;
	return counter.QuadPart / w->freq_per_second;
#elif GRD_OS_LINUX
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_nsec / 1'000 + t.tv_sec * 1'000'000;
#elif GRD_OS_DARWIN
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	return mts.tv_nsec / 1'000 + mts.tv_sec * 1'000'000;
#endif
}

s64 grd_nanos_elapsed_s64(GrdStopwatch* w) {
	s64 new_time = grd_get_current_nanos(w);
	s64 delta = new_time - w->last_us;

	// Thread switching to another CPU may cause this.
	if (delta < 0) delta = 0;

	return delta;
}

s64 grd_millis_elapsed_s64(GrdStopwatch* w) {
	return grd_nanos_elapsed_s64(w) / 1000;
}

s64 grd_seconds_elapsed_s64(GrdStopwatch* w) {
	return grd_millis_elapsed_s64(w) / 1000;
}

f64 grd_millis_elapsed_f64(GrdStopwatch* w) {
	return f64(grd_nanos_elapsed_s64(w)) / 1000.0;
}

f64 grd_seconds_elapsed_f64(GrdStopwatch* w) {
	return grd_millis_elapsed_f64(w) / 1000.0;
}

void grd_reset(GrdStopwatch* w) {
	w->last_us = grd_get_current_nanos(w);
}

GrdStopwatch grd_make_stopwatch() {
	GrdStopwatch w;
#if GRD_OS_WINDOWS
	LARGE_INTEGER freq_li;
	QueryPerformanceFrequency(&freq_li);
	w.freq_per_second = freq_li.QuadPart;
#endif
	grd_reset(&w);
	return w;
}
