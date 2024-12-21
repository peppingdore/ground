#pragma once

#include "grd_base.h"
#include "grd_error.h"
#include "grd_tuple.h"
#include "grd_array.h"

struct GrdWriter {
	void*      item;
	GrdError* (*write_proc) (void* item, void* data, s64 size);

	// @TODO: demethodize.
	GrdError* write(void* data, s64 size) {
		return write_proc(item, data, size);
	}
};

struct GrdReader {
	void*                      item;
	GrdTuple<s64, GrdError*> (*read_proc) (void* item, void* buf, s64 size);

	// @TODO: demethodize.
	GrdTuple<s64, GrdError*> read(void* buf, s64 size) {
		return read_proc(item, buf, size);
	}
};

GRD_DEDUP GrdTuple<GrdArray<u8>, GrdError*> grd_read_all(GrdAllocator allocator, GrdReader reader) {
	GrdArray<u8> data = { .allocator = allocator };
	while (true) {
		void* dst = grd_reserve(&data, 128);
		auto [read, e] = reader.read(dst, 128);
		if (e) {
			data.count -= 128;
			return { {}, e };
		}
		data.count -= (128 - read);
		if (read == 0) {
			break;
		}
	}
	return { data, NULL };
}

GRD_DEDUP GrdTuple<GrdArray<u8>, GrdError*> grd_read_all(GrdReader reader) {
	return grd_read_all(c_allocator, reader);
}

GRD_DEDUP GrdTuple<GrdAllocatedString, GrdError*> grd_read_text(GrdAllocator allocator, GrdReader reader) {
	auto [data, e] = grd_read_all(allocator, reader);
	if (e) {
		return { {}, e };
	}
	return { *((GrdAllocatedString*) &data) };
}

GRD_DEDUP GrdTuple<GrdAllocatedString, GrdError*> grd_read_text(GrdReader reader) {
	return grd_read_text(c_allocator, reader);
}
