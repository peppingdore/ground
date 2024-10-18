#pragma once

#include "grd_base.h"
#include "data_ops.h"

enum ByteOrder {
	BYTE_ORDER_LITTLE_ENDIAN = 0,
	BYTE_ORDER_BIG_ENDIAN    = 1,
};

void swap_endianness(void* data, u64 size) {
	reverse((u8*) data, size);
}

void swap_endianness(auto* x) {
	swap_endianness(x, sizeof(*x));
}
