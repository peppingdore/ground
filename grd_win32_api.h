#pragma once

#include "grd_base.h"
#include "grd_defer.h"
#include "sync/grd_atomics.h"
#include "sync/grd_spinlock.h"

#define GRD_USE_WINDOWS_HEADER 0

#if !defined(_WINDOWS_) && GRD_USE_WINDOWS_HEADER
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#define GRD_WINBASEAPI WINBASEAPI
	#define GRD_WINAPI WINAPI
	#define GRD_WIN_DWORD DWORD
	#define GRD_WIN_LONG LONG
	#define GRD_WIN_INFINITE INFINITE
	#define GRD_WIN_HANDLE HANDLE
	#define GRD_LPTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE
	#define GRD_WIN32_CRITICAL_SECTION CRITICAL_SECTION
	#define GRD_WIN_BOOL BOOL

#elif !defined(_WINDOWS_)
	// #pragma comment(lib, "kernel32.lib")

	#define GRD_WINBASEAPI __declspec(dllimport)
	#define GRD_WIN_IMPORT_API extern "C" GRD_WINBASEAPI
	#define GRD_WINAPI __stdcall
	#define GRD_WIN_DWORD u32
	#define GRD_WIN_BOOL int
	#define GRD_WIN_LONG long
	#define GRD_WIN_INFINITE 0xFFFFFFFF
	#define GRD_WIN_HANDLE void*
	#define GRD_WIN_WORD unsigned short
	typedef long long GRD_WIN_LONG64;

	typedef GRD_WIN_DWORD (GRD_WINAPI *GRD_PTHREAD_START_ROUTINE)(void* lpThreadParameter);
	typedef GRD_PTHREAD_START_ROUTINE GRD_LPTHREAD_START_ROUTINE;
	

	struct alignas(8) GRD_WIN32_CRITICAL_SECTION {
		char data[40];
	};

	struct alignas(8) GRD_WIN32_SECURITY_ATTRIBUTES {
		char data[24];
	};

	GRD_WIN_IMPORT_API void GRD_WINAPI InitializeCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WIN_IMPORT_API void GRD_WINAPI EnterCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WIN_IMPORT_API void GRD_WINAPI LeaveCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WIN_IMPORT_API void GRD_WINAPI DeleteCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WIN_IMPORT_API GRD_WIN_HANDLE GRD_WINAPI CreateSemaphoreA(GRD_WIN32_SECURITY_ATTRIBUTES* lpSecurityAttributes, long lInitialCount, long lMaximumCount, const char* lpName);
	GRD_WIN_IMPORT_API u32 GRD_WINAPI WaitForSingleObject(GRD_WIN_HANDLE handle, u32 dwMilliseconds);
	GRD_WIN_IMPORT_API int GRD_WINAPI ReleaseSemaphore(GRD_WIN_HANDLE handle, long lReleaseCount, long* lpPreviousCount);
	GRD_WIN_IMPORT_API int GRD_WINAPI CloseHandle(GRD_WIN_HANDLE handle);
	GRD_WIN_IMPORT_API void GRD_WINAPI Sleep(u32 dwMilliseconds);
	// GetCurrentThreadId
	GRD_WIN_IMPORT_API GRD_WIN_DWORD GRD_WINAPI GetCurrentThreadId(void);
	// CreateThread
	GRD_WIN_IMPORT_API GRD_WIN_HANDLE GRD_WINAPI CreateThread(GRD_WIN32_SECURITY_ATTRIBUTES* lpThreadAttributes, u64 dwStackSize, GRD_LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter, u32 dwCreationFlags, GRD_WIN_DWORD* lpThreadId);
	// GetThreadId
	GRD_WIN_IMPORT_API GRD_WIN_DWORD GRD_WINAPI GetThreadId(GRD_WIN_HANDLE thread);
	// GetLastError
	GRD_WIN_IMPORT_API u32 GRD_WINAPI GetLastError(void);

// 	WINBASEAPI
// _Success_(return != 0)
// DWORD
// WINAPI
// FormatMessageA(
//     _In_     DWORD dwFlags,
//     _In_opt_ LPCVOID lpSource,
//     _In_     DWORD dwMessageId,
//     _In_     DWORD dwLanguageId,
//     _When_((dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPSTR*)lpBuffer, _Outptr_result_z_))
//     _When_((dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(nSize))
//              LPSTR lpBuffer,
//     _In_     DWORD nSize,
//     _In_opt_ va_list *Arguments
//     );

	#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
	#define FORMAT_MESSAGE_FROM_STRING     0x00000400
	#define FORMAT_MESSAGE_FROM_HMODULE    0x00000800
	#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
	#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
	#define FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF
	#define LANG_NEUTRAL                     0x00
	#define SUBLANG_NEUTRAL                             0x00    // language neutral
	#define SUBLANG_DEFAULT                             0x01    // user default
	#define MAKELANGID(p, s)       ((((GRD_WIN_WORD  )(s)) << 10) | (GRD_WIN_WORD  )(p))

	GRD_WIN_IMPORT_API GRD_WIN_DWORD GRD_WINAPI FormatMessageA(u32 dwFlags, const void* lpSource, u32 dwMessageId, u32 dwLanguageId, char* lpBuffer, u32 nSize, va_list* Arguments);

	
	#define GENERIC_READ                     (0x80000000L)
	#define GENERIC_WRITE                    (0x40000000L)
	#define GENERIC_EXECUTE                  (0x20000000L)
	#define GENERIC_ALL                      (0x10000000L)
	
	#define CREATE_NEW          1
	#define CREATE_ALWAYS       2
	#define OPEN_EXISTING       3
	#define OPEN_ALWAYS         4
	#define TRUNCATE_EXISTING   5

	#define FILE_SHARE_READ                 0x00000001  
	#define FILE_SHARE_WRITE                0x00000002  
	#define FILE_SHARE_DELETE               0x00000004  

	#define FILE_ATTRIBUTE_NORMAL               0x00000080 
	#define INVALID_HANDLE_VALUE ((GRD_WIN_HANDLE)(long long)-1)

	#define FILE_BEGIN           0
	#define FILE_CURRENT         1
	#define FILE_END             2

	#define ERROR_HANDLE_EOF                 38L

	#define MAX_PATH          260

	// CreateFileW
	GRD_WIN_IMPORT_API GRD_WIN_HANDLE GRD_WINAPI CreateFileW(const wchar_t* lpFileName, u32 dwDesiredAccess, u32 dwShareMode, GRD_WIN32_SECURITY_ATTRIBUTES* lpSecurityAttributes, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, GRD_WIN_HANDLE hTemplateFile);
	// SetFilePointer
	GRD_WIN_IMPORT_API GRD_WIN_LONG GRD_WINAPI SetFilePointer(GRD_WIN_HANDLE hFile, GRD_WIN_LONG lDistanceToMove, GRD_WIN_DWORD* lpDistanceToMoveHigh, u32 dwMoveMethod);
	// WriteFile
	GRD_WIN_IMPORT_API GRD_WIN_BOOL GRD_WINAPI WriteFile(GRD_WIN_HANDLE hFile, const void* lpBuffer, GRD_WIN_DWORD nNumberOfBytesToWrite, GRD_WIN_DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
	// ReadFile
	GRD_WIN_IMPORT_API GRD_WIN_BOOL GRD_WINAPI ReadFile(GRD_WIN_HANDLE hFile, void* lpBuffer, GRD_WIN_DWORD nNumberOfBytesToRead, GRD_WIN_DWORD* lpNumberOfBytesRead, void* lpOverlapped);
	// FlusFileBuffers
	GRD_WIN_IMPORT_API GRD_WIN_BOOL GRD_WINAPI FlushFileBuffers(GRD_WIN_HANDLE hFile);

	typedef struct _FILETIME {
		GRD_WIN_DWORD dwLowDateTime;
		GRD_WIN_DWORD dwHighDateTime;
	} FILETIME, *PFILETIME, *LPFILETIME;

	typedef struct _WIN32_FIND_DATAW {
		GRD_WIN_DWORD dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		GRD_WIN_DWORD nFileSizeHigh;
		GRD_WIN_DWORD nFileSizeLow;
		GRD_WIN_DWORD dwReserved0;
		GRD_WIN_DWORD dwReserved1;
		_Field_z_ wchar_t  cFileName[ MAX_PATH ];
		_Field_z_ wchar_t  cAlternateFileName[ 14 ];
	} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

	// FindFirstFileW
	GRD_WIN_IMPORT_API GRD_WIN_HANDLE GRD_WINAPI FindFirstFileW(const wchar_t* lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
	// FindClose
	GRD_WIN_IMPORT_API GRD_WIN_BOOL GRD_WINAPI FindClose(GRD_WIN_HANDLE hFindFile);
	// FindNextFileW
	GRD_WIN_IMPORT_API GRD_WIN_BOOL GRD_WINAPI FindNextFileW(GRD_WIN_HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);

	#define ERROR_NO_MORE_FILES              18L


	// GRD_WIN_LONG64 _InterlockedCompareExchange64 (
	// 	GRD_WIN_LONG64 volatile *Destination,
	// 	GRD_WIN_LONG64 ExChange,
	// 	GRD_WIN_LONG64 Comperand
    // );
	
	// __int64 _InterlockedCompareExchange64 (
	// 	__int64 volatile *Destination,
	// 	__int64 ExChange,
	// 	__int64 Comperand
    // );
	// #pragma intrinsic(_InterlockedCompareExchange64)

#endif
