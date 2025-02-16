#pragma once

#include "grd_base.h"
#include "math/grd_math_base.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

template <typename... Args>
GRD_DEDUP char* grd_heap_sprintf(const char* format, Args... args) {
	char buf[256];
	int written = snprintf(buf, sizeof(buf), format, args...);
	if (written < 0) {
		auto str = (char*) malloc(1);
		str[0] = '\0';
		return str;
	}
	if (written < sizeof(buf)) {
		auto str = (char*) malloc(written + 1);
		strcpy(str, buf);
		return str;
	}
	auto str = (char*) malloc(written + 1);
	written = snprintf(str, written + 1, format, args...);
	if (written < 0) {
		str[0] = '\0';
	}
	return str;
}

GRD_DEDUP char* grd_heap_join(const char* start, const char* joiner, const char* end, auto... args) {
	char buf[512];
	int  cursor = 0;

	auto append = [&](const char* x) {
		auto grd_len = strlen(x);
		auto remaining = s64(sizeof(buf)) - cursor - 1;
		auto copy_len = grd_min(remaining, grd_len);
		memcpy(buf + cursor, x, copy_len);
		cursor += copy_len;
		assert(cursor <= sizeof(buf));
	};

	append(start);
	for (int i = 0; i < sizeof...(args); i++) {
		append(joiner);
		if (i != sizeof...(args) - 1) {
			append(", ");
		}
	}
	append(end);

	if (cursor == sizeof(buf)) {
		buf[sizeof(buf) - 1] = '\0';
	}

	return grd_heap_sprintf(buf, args...);
}
