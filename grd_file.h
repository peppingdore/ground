#pragma once

#include "grd_error.h"
#include "grd_io.h"

#if GRD_IS_POSIX
	#include <fcntl.h>
#endif

using GrdOpenFileFlag = u32;
GRD_DEDUP constexpr GrdOpenFileFlag GRD_FILE_READ       = 1;
GRD_DEDUP constexpr GrdOpenFileFlag GRD_FILE_WRITE      = 1 << 1;
GRD_DEDUP constexpr GrdOpenFileFlag GRD_FILE_CREATE_NEW = 1 << 3;
GRD_DEDUP constexpr GrdOpenFileFlag GRD_FILE_APPEND     = 1 << 4;

#if GRD_OS_WINDOWS
	using GrdNativeFileHandle = HANDLE;
#elif GRD_IS_POSIX
	using GrdNativeFileHandle = int;
#else
	static_assert(false);
#endif

struct GrdFile {
	GrdNativeFileHandle handle;
};


GRD_DEDUP GrdFile grd_file_from_native_handle(GrdNativeFileHandle handle) {
	return { .handle = handle };
}

#if GRD_OS_WINDOWS
GRD_DEDUP GrdTuple<GrdFile, GrdError*> grd_open_file(GrdUnicodeString path, GrdOpenFileFlag flags = GRD_FILE_READ, u32 unix_perm=0777/*ignored*/) {
	int windows_flags = 0;
	if (flags & GRD_FILE_READ) {
		windows_flags |= GENERIC_READ;
	}
	if (flags & GRD_FILE_WRITE) {
		windows_flags |= GENERIC_WRITE;
	}
	if (windows_flags == 0) {
		return { {}, grd_make_error("flags must contain at least one of FILE_READ/FILE_WRITE") };
	}

	int creation_disposition = (flags & GRD_FILE_CREATE_NEW) ? CREATE_ALWAYS : OPEN_EXISTING;

	auto wide = grd_encode_utf16(path);
	grd_defer { wide.free(); };

	HANDLE handle = CreateFileW((wchar_t*) wide.data, windows_flags, FILE_SHARE_READ, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return { {}, grd_windows_error() };
	}

	if (flags & GRD_FILE_APPEND) {
		SetFilePointer(handle, 0, NULL, FILE_END);
	}
	return { grd_file_from_native_handle(handle), NULL };
}

GRD_DEDUP GrdTuple<s64, GrdError*> grd_os_write_file(GrdFile* file, void* data, s64 size) {
	DWORD to_write = grd_clamp_u64(0, u32_max, size);
	DWORD written = 0;
	BOOL  result = WriteFile(file->handle, data, to_write, &written, NULL);
	if (!result) {
		return { -1, grd_windows_error() };
	}
	return { written, NULL };
}

// Returns 0, NULL if EOF is reached.
GRD_DEDUP GrdTuple<s64, GrdError*> grd_os_read_file(GrdFile* file, void* buf, s64 size) {
	DWORD to_read = grd_clamp_u64(0, u32_max, size);
	DWORD read;
	BOOL  result = ReadFile(file->handle, buf, to_read, &read, NULL);
	if (!result) {
		if (GetLastError() == ERROR_HANDLE_EOF) {
			return { 0, NULL };
		}
		return { -1, grd_windows_error() };
	}
	return { read, NULL };
}

GRD_DEDUP void grd_close_file(GrdFile* file) {
	CloseHandle(file->handle);
}

GRD_DEDUP void grd_fsync(GrdFile* file) {
	FlushFileBuffers(file->handle);
}

#elif GRD_IS_POSIX

GRD_DEDUP GrdTuple<GrdFile, GrdError*> grd_open_file(GrdUnicodeString path, GrdOpenFileFlag flags = GRD_FILE_READ, u32 unix_perm = 0777) {
	GrdAllocatedString utf8_path = grd_encode_utf8(path);
	grd_defer { utf8_path.free(); };

	int o_flag;
	if ((flags & (GRD_FILE_WRITE | GRD_FILE_READ)) == (GRD_FILE_WRITE | GRD_FILE_READ)) {
		o_flag = O_RDWR;
	} else if (flags & GRD_FILE_WRITE) {
		o_flag = O_WRONLY;
	} else if (flags & GRD_FILE_READ) {
		o_flag = O_RDONLY;
	} else {
		return { {}, grd_make_error("Expecting any of FILE_WRITE/FILE_READ in |flags|") };
	}

	if (flags & GRD_FILE_CREATE_NEW) {
		o_flag |= O_TRUNC | O_CREAT;
	}

	int fd = open(utf8_path.data, o_flag, unix_perm);
	if (fd == -1) {
		return { {}, grd_posix_error() };
	}
	if (flags & GRD_FILE_APPEND) {
		lseek(fd, 0, SEEK_END);
	}
	return { fd, NULL };
}

GRD_DEDUP GrdTuple<s64, GrdError*> grd_os_write_file(GrdFile* file, void* data, s64 size) {
	ssize_t result = write(file->handle, data, (u64) size);
	if (result == -1) {
		return { -1, grd_posix_error() };
	}
	return { result, NULL };
}

// Returns 0, NULL if EOF is reached.
GRD_DEDUP GrdTuple<s64, GrdError*> grd_os_read_file(GrdFile* file, void* buf, s64 size) {
	ssize_t result = read(file->handle, buf, (u64) size);
	if (result == -1) {
		return { -1, grd_posix_error() };
	}
	return { result, NULL };
}

GRD_DEDUP void grd_close_file(GrdFile* file) {
	close(file->handle);
}

GRD_DEDUP void grd_fsync(GrdFile* file) {
	fsync(file->handle);
}
#endif

GRD_DEDUP GrdError* grd_write_file(GrdFile* file, void* data, s64 size) {
	s64 total_written = 0;
	while (total_written < size) {
		auto [written, e] = grd_os_write_file(file, grd_ptr_add(data, total_written), size - total_written);
		if (e) {
			return e;
		}
		total_written += written;
	}
	return NULL;
}

GRD_DEDUP GrdTuple<s64, GrdError*> grd_read_file(GrdFile* file, void* buf, s64 size) {
	s64 total_read = 0;
	while (total_read < size) {
		auto [read, e] = grd_os_read_file(file, grd_ptr_add(buf, total_read), size - total_read);
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

GRD_DEDUP GrdWriter grd_file_writer(GrdFile* file) {
	GrdWriter w = {
		.item = file,
		.write_proc = +[] (void* item, void* data, s64 size) -> GrdError* {
			return grd_write_file((GrdFile*) item, data, size);
		}
	};
	return w;
}

GRD_DEDUP GrdReader grd_file_reader(GrdFile* file) {
	GrdReader r = {
		.item = file,
		.read_proc = +[] (void* item, void* buf, s64 size) -> GrdTuple<s64, GrdError*> {
			return grd_os_read_file((GrdFile*) item, buf, size);
		}
	};
	return r;
}

GRD_DEDUP GrdTuple<GrdAllocatedString, GrdError*> grd_read_text_at_path(GrdAllocator allocator, GrdUnicodeString path) {
	auto [f, e] = grd_open_file(path);
	if (e) {
		return { {}, e };
	}
	auto [text, err] = grd_read_text(grd_file_reader(&f));
	if (err) {
		return { {}, err };
	}
	return { text, NULL };
}

GRD_DEDUP GrdTuple<GrdAllocatedString, GrdError*> grd_read_text_at_path(GrdUnicodeString path) {
	return grd_read_text_at_path(c_allocator, path);
}

GRD_DEDUP GrdError* write_string_to_file(GrdString str, GrdUnicodeString path) {
	auto [f, e] = grd_open_file(path, GRD_FILE_WRITE | GRD_FILE_CREATE_NEW);
	if (e) {
		return e;
	}
	auto e1 = grd_write_file(&f, str.data, grd_len(str) * sizeof(str[0]));
	if (e1) {
		return e;
	}
	return NULL;
}

GRD_DEF grd_iterate_folder(GrdUnicodeString path) -> GrdGenerator<GrdTuple<GrdError*, GrdAllocatedUnicodeString>> {
#if GRD_OS_WINDOWS
	WIN32_FIND_DATAW findFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	auto wide_path = grd_encode_utf16(path);
	grd_add(&wide_path, u"/*\0"_b);
	grd_defer_x(wide_path.free());
	hFind = FindFirstFileW((wchar_t*) wide_path.data, &findFileData);
	grd_defer_x(FindClose(hFind));
	if (hFind == INVALID_HANDLE_VALUE) {
		co_yield { grd_windows_error() };
		co_return;
	}
	while (true) {
		auto name = grd_decode_utf16((char16_t*) findFileData.cFileName);
		if (name != "." && name != "..") {
			co_yield { NULL, name };
		} else {
			name.free();
		}
		auto res = FindNextFileW(hFind, &findFileData);
		if (res == 0) {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				break;
			}
			co_yield { grd_windows_error() };
		}
	}
#elif GRD_IS_POSIX
	auto c_str = grd_encode_utf8(path);
	grd_defer_x(c_str.free());
	DIR* dir = opendir(c_str.data);
	if (!dir) {
		co_yield { grd_posix_error() };
	}
	grd_defer_x(closedir(dir));
	struct dirent* entry;
	while (true) {
		errno = 0;
		entry = readdir(dir);
		if (entry == NULL) {
			if (errno == 0) {
				break;
			}
			co_yield { grd_posix_error() };
		}
		GrdAllocatedUnicodeString name = grd_decode_utf8(entry->d_name);
		if (name == "." || name == "..") {
			name.free();
			continue;
		}
		co_yield { NULL, name };
	}
#else
	static_assert(false);
#endif
}
