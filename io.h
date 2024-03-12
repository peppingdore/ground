#pragma once

#include "base.h"

struct Writer {
	void*  item;
	void (*write_proc) (void* item, void* data, s64 size);
};

