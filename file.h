#pragma once

#include "error.h"
#include "io.h"

#if IS_POSIX
	#include <fcntl.h>
#endif

using OpenFileFlag = u32;
constexpr OpenFileFlag FILE_READ       = 1;
constexpr OpenFileFlag FILE_WRITE      = 1 << 1;
constexpr OpenFileFlag FILE_CREATE_NEW = 1 << 3;
constexpr OpenFileFlag FILE_APPEND     = 1 << 4;

#if OS_WINDOWS
	using Native_File_Handle = HANDLE;
#elif IS_POSIX
	using Native_File_Handle = int;
#else
	PLATFORM_IS_NOT_SUPPORTED();
#endif

struct File {
	Native_File_Handle handle;
};


File file_from_native_handle(Native_File_Handle handle) {
	return { .handle = handle };
}

#if OS_WINDOWS
Tuple<File, Error*> open_file(UnicodeString path, OpenFileFlag flags = FILE_READ, u32 unix_perm=0777/*ignored*/) {
	int windows_flags = 0;
	if (flags & FILE_READ) {
		windows_flags |= GENERIC_READ;
	}
	if (flags & FILE_WRITE) {
		windows_flags |= GENERIC_WRITE;
	}
	if (windows_flags == 0) {
		return { {}, make_error("flags must contain at least one of FILE_READ/FILE_WRITE") };
	}

	int creation_disposition = (flags & FILE_CREATE_NEW) ? CREATE_ALWAYS : OPEN_EXISTING;

	auto wide = encode_utf16(path);
	defer { wide.free(); };

	HANDLE handle = CreateFileW((wchar_t*) wide.data, windows_flags, FILE_SHARE_READ, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return { {}, windows_error() };
	}

	if (flags & FILE_APPEND) {
		SetFilePointer(handle, 0, NULL, FILE_END);
	}
	return { file_from_native_handle(handle), NULL };
}

Tuple<s64, Error*> os_write_file(File* file, void* data, s64 size) {
	DWORD to_write = clamp_u64(0, u32_max, size);
	DWORD written = 0;
	BOOL  result = WriteFile(file->handle, data, to_write, &written, NULL);
	if (!result) {
		return { -1, windows_error() };
	}
	return { written, NULL };
}

// Returns 0, NULL if EOF is reached.
Tuple<s64, Error*> os_read_file(File* file, void* buf, s64 size) {
	DWORD to_read = clamp_u64(0, u32_max, size);
	DWORD read;
	BOOL  result = ReadFile(file->handle, buf, to_read, &read, NULL);
	if (!result) {
		if (GetLastError() == ERROR_HANDLE_EOF) {
			return { 0, NULL };
		}
		return { -1, windows_error() };
	}
	return { read, NULL };
}

#elif IS_POSIX

Tuple<File, Error*> open_file(UnicodeString path, OpenFileFlag flags = FILE_READ, u32 unix_perm = 0777) {
	String utf8_path = encode_utf8(path);
	defer { utf8_path.free(); };

	int o_flag;
	if ((flags & (FILE_WRITE | FILE_READ)) == (FILE_WRITE | FILE_READ)) {
		o_flag = O_RDWR;
	} else if (flags & FILE_WRITE) {
		o_flag = O_WRONLY;
	} else if (flags & FILE_READ) {
		o_flag = O_RDONLY;
	} else {
		return { {}, make_error("expecting any FILE_WRITE/FILE_READ in |flags|") };
	}

	if (flags & FILE_CREATE_NEW) {
		o_flag |= O_TRUNC | O_CREAT;
	}

	int fd = open(utf8_path.data, o_flag, unix_perm);
	if (fd == -1) {
		return { {}, posix_error() };
	}
	if (flags & FILE_APPEND) {
		lseek(fd, 0, SEEK_END);
	}
	return { fd, NULL };
}

Tuple<s64, Error*> os_write_file(File* file, void* data, s64 size) {
	ssize_t result = write(file->handle, data, (u64) size);
	if (result == -1) {
		return { -1, posix_error() };
	}
	return { result, NULL };
}

// Returns 0, NULL if EOF is reached.
Tuple<s64, Error*> os_read_file(File* file, void* buf, s64 size) {
	ssize_t result = read(file->handle, buf, (u64) size);
	if (result == -1) {
		return { -1, posix_error() };
	}
	return { result, NULL };
}

#endif

Error* write_file(File* file, void* data, s64 size) {
	s64 total_written = 0;
	while (total_written < size) {
		auto [written, e] = os_write_file(file, ptr_add(data, total_written), size - total_written);
		if (e) {
			return e;
		}
		total_written += written;
	}
	return NULL;
}

Tuple<s64, Error*> read_file(File* file, void* buf, s64 size) {
	s64 total_read = 0;
	while (total_read < size) {
		auto [read, e] = os_read_file(file, ptr_add(buf, total_read), size - total_read);
		if (e) {
			return { total_read, e };
		}
		if (read == 0) {
			break;
		}
		total_read += read;
	}
	return { total_read, NULL };
}

Writer file_writer(File* file) {
	Writer w = {
		.item = file,
		.write_proc = +[] (void* item, void* data, s64 size) -> Error* {
			return write_file((File*) item, data, size);
		}
	};
	return w;
}

Reader file_reader(File* file) {
	Reader r = {
		.item = file,
		.read_proc = +[] (void* item, void* buf, s64 size) -> Tuple<s64, Error*> {
			return os_read_file((File*) item, buf, size);
		}
	};
	return r;
}

Tuple<String, Error*> read_text_at_path(Allocator allocator, UnicodeString path) {
	auto [f, e] = open_file(path);
	if (e) {
		return { {}, e };
	}
	auto [text, err] = read_text(file_reader(&f));
	if (err) {
		return { {}, err };
	}
	return { text, NULL };
}

Tuple<String, Error*> read_text_at_path(UnicodeString path) {
	return read_text_at_path(c_allocator, path);
}
