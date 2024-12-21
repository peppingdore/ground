#pragma once

#include "grd_base.h"
#include "grd_data_ops.h"

enum GrdByteOrder {
	GRD_BYTE_ORDER_LITTLE_ENDIAN = 0,
	GRD_BYTE_ORDER_BIG_ENDIAN    = 1,
};

GRD_DEDUP void grd_swap_endianness(void* data, u64 size) {
	grd_reverse((u8*) data, size);
}

GRD_DEDUP void grd_swap_endianness(auto* x) {
	grd_swap_endianness(x, sizeof(*x));
}
