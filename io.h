#pragma once

#include "base.h"
#include "error.h"
#include "tuple.h"
#include "array.h"

struct Writer {
	void*    item;
	Error* (*write_proc) (void* item, void* data, s64 size);

	Error* write(void* data, s64 size) {
		return write_proc(item, data, size);
	}
};

struct Reader {
	void*                item;
	Tuple<s64, Error*> (*read_proc) (void* item, void* buf, s64 size);

	Tuple<s64, Error*> read(void* buf, s64 size) {
		return read_proc(item, buf, size);
	}
};

Tuple<Array<u8>, Error*> read_all(Allocator allocator, Reader reader) {
	Array<u8> data = { .allocator = allocator };
	while (true) {
		void* dst = reserve(&data, 128);
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

Tuple<Array<u8>, Error*> read_all(Reader reader) {
	return read_all(c_allocator, reader);
}

Tuple<String, Error*> read_text(Allocator allocator, Reader reader) {
	auto [data, e] = read_all(allocator, reader);
	if (e) {
		return { {}, e };
	}
	return { make_string((char*) data.data, len(data)), NULL };
}

Tuple<String, Error*> read_text(Reader reader) {
	return read_text(c_allocator, reader);
}
