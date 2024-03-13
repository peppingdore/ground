#pragma once

#include "base.h"
#include "math.h"
#include <stdio.h>

template <typename... Args>
char* heap_sprintf(const char* format, Args... args) {
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

char* heap_join(const char* start, const char* joiner, const char* end, auto... args) {
	char buf[512];
	int  cursor = 0;

	auto append = [&](const char* x) {
		auto len = strlen(x);
		auto remaining = s64(sizeof(buf)) - cursor - 1;
		auto copy_len = min_s64(remaining, len);
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

	return heap_sprintf(buf, args...);
}
