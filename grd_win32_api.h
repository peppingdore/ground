#pragma once

#include "grd_base.h"
#include "grd_defer.h"
#include "grd_build.h"
#include "sync/grd_atomics.h"
#include "sync/grd_spinlock.h"

GRD_BUILD_RUN("ctx.params.add_lib('user32.lib')");

#ifndef GRD_USE_WINDOWS_HEADER
	#define GRD_USE_WINDOWS_HEADER 0
#endif

#if !defined(_WINDOWS_) && GRD_USE_WINDOWS_HEADER
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#define GRD_WINBASEAPI WINBASEAPI
	#define GRD_WINUSERAPI WINUSERAPI
	#define GRD_WINAPI WINAPI
	#define GRD_WIN_DWORD DWORD
	#define GRD_WIN_LONG LONG
	#define GRD_WIN_INFINITE INFINITE
	#define GRD_WIN_HANDLE HANDLE
	#define GRD_LPTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE
	#define GRD_WIN32_CRITICAL_SECTION CRITICAL_SECTION
	#define GRD_WIN_BOOL BOOL
	#define GRD_WIN_LONGLONG LONGLONG
	#define GRD_WIN_LARGE_INTEGER LARGE_INTEGER
	#define GRD_WIN_UINT_PTR UINT_PTR
	#define GRD_WIN_WPARAM WPARAM
	#define GRD_WIN_LONG_PTR LONG_PTR
	#define GRD_WIN_UINT UINT

#elif !defined(_WINDOWS_)
extern "C" {
	#define GRD_CRIPPLED_WIN32_API 1

	// #pragma comment(lib, "kernel32.lib")

	#define GRD_WINBASEAPI __declspec(dllimport)
	#define GRD_WINUSERAPI __declspec(dllimport)
	#define GRD_WINAPI __stdcall
	#define GRD_WIN_DWORD u32
	#define GRD_WIN_BOOL int
	#define GRD_WIN_LONG long
	#define GRD_WIN_LONGLONG __int64
	#define GRD_WIN_INFINITE 0xFFFFFFFF
	#define GRD_WIN_HANDLE void*
	#define GRD_WIN_WORD unsigned short
	#define GRD_WIN_UINT unsigned int
	typedef unsigned __int64 GRD_WIN_UINT_PTR;
	typedef GRD_WIN_UINT_PTR GRD_WIN_WPARAM;
	typedef __int64 GRD_WIN_LONG_PTR; 
	typedef GRD_WIN_LONG_PTR GRD_WIN_LPARAM;
	typedef long long GRD_WIN_LONG64;

	typedef GRD_WIN_DWORD (GRD_WINAPI *GRD_PTHREAD_START_ROUTINE)(void* lpThreadParameter);
	typedef GRD_PTHREAD_START_ROUTINE GRD_LPTHREAD_START_ROUTINE;
	

	struct alignas(8) GRD_WIN32_CRITICAL_SECTION {
		char data[40];
	};

	struct alignas(8) GRD_WIN32_SECURITY_ATTRIBUTES {
		char data[24];
	};

	GRD_WINBASEAPI void GRD_WINAPI InitializeCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WINBASEAPI void GRD_WINAPI EnterCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WINBASEAPI void GRD_WINAPI LeaveCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WINBASEAPI void GRD_WINAPI DeleteCriticalSection(GRD_WIN32_CRITICAL_SECTION* lpCriticalSection);
	GRD_WINBASEAPI GRD_WIN_HANDLE GRD_WINAPI CreateSemaphoreA(GRD_WIN32_SECURITY_ATTRIBUTES* lpSecurityAttributes, long lInitialCount, long lMaximumCount, const char* lpName);
	GRD_WINBASEAPI u32 GRD_WINAPI WaitForSingleObject(GRD_WIN_HANDLE handle, u32 dwMilliseconds);
	GRD_WINBASEAPI int GRD_WINAPI ReleaseSemaphore(GRD_WIN_HANDLE handle, long lReleaseCount, long* lpPreviousCount);
	GRD_WINBASEAPI int GRD_WINAPI CloseHandle(GRD_WIN_HANDLE handle);
	GRD_WINBASEAPI void GRD_WINAPI Sleep(u32 dwMilliseconds);
	// GetCurrentThreadId
	GRD_WINBASEAPI GRD_WIN_DWORD GRD_WINAPI GetCurrentThreadId(void);
	// CreateThread
	GRD_WINBASEAPI GRD_WIN_HANDLE GRD_WINAPI CreateThread(GRD_WIN32_SECURITY_ATTRIBUTES* lpThreadAttributes, u64 dwStackSize, GRD_LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter, u32 dwCreationFlags, GRD_WIN_DWORD* lpThreadId);
	// GetThreadId
	GRD_WINBASEAPI GRD_WIN_DWORD GRD_WINAPI GetThreadId(GRD_WIN_HANDLE thread);
	// GetLastError
	GRD_WINBASEAPI u32 GRD_WINAPI GetLastError(void);

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

	GRD_WINBASEAPI GRD_WIN_DWORD GRD_WINAPI FormatMessageA(u32 dwFlags, const void* lpSource, u32 dwMessageId, u32 dwLanguageId, char* lpBuffer, u32 nSize, va_list* Arguments);

	
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
	GRD_WINBASEAPI GRD_WIN_HANDLE GRD_WINAPI CreateFileW(const wchar_t* lpFileName, u32 dwDesiredAccess, u32 dwShareMode, GRD_WIN32_SECURITY_ATTRIBUTES* lpSecurityAttributes, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, GRD_WIN_HANDLE hTemplateFile);
	// SetFilePointer
	GRD_WINBASEAPI GRD_WIN_LONG GRD_WINAPI SetFilePointer(GRD_WIN_HANDLE hFile, GRD_WIN_LONG lDistanceToMove, GRD_WIN_DWORD* lpDistanceToMoveHigh, u32 dwMoveMethod);
	// WriteFile
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI WriteFile(GRD_WIN_HANDLE hFile, const void* lpBuffer, GRD_WIN_DWORD nNumberOfBytesToWrite, GRD_WIN_DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
	// ReadFile
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI ReadFile(GRD_WIN_HANDLE hFile, void* lpBuffer, GRD_WIN_DWORD nNumberOfBytesToRead, GRD_WIN_DWORD* lpNumberOfBytesRead, void* lpOverlapped);
	// FlusFileBuffers
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI FlushFileBuffers(GRD_WIN_HANDLE hFile);

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
	GRD_WINBASEAPI GRD_WIN_HANDLE GRD_WINAPI FindFirstFileW(const wchar_t* lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
	// FindClose
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI FindClose(GRD_WIN_HANDLE hFindFile);
	// FindNextFileW
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI FindNextFileW(GRD_WIN_HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);

	#define ERROR_NO_MORE_FILES              18L

	typedef union _GRD_WIN_LARGE_INTEGER {
		struct {
			GRD_WIN_DWORD LowPart;
			GRD_WIN_LONG  HighPart;
		} DUMMYSTRUCTNAME;
		struct {
			GRD_WIN_DWORD LowPart;
			GRD_WIN_LONG  HighPart;
		} u;
		GRD_WIN_LONGLONG QuadPart;
	  } GRD_WIN_LARGE_INTEGER;

	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI QueryPerformanceCounter(GRD_WIN_LARGE_INTEGER* lpPerformanceCount);
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI QueryPerformanceFrequency(GRD_WIN_LARGE_INTEGER* lpFrequency);

	GRD_WINUSERAPI GRD_WIN_UINT GRD_WINAPI MapVirtualKeyW(GRD_WIN_UINT uCode, GRD_WIN_UINT uMapType);
};
#endif

#define GRD_WIN_MAPVK_VK_TO_VSC     (0)
#define GRD_WIN_MAPVK_VSC_TO_VK     (1)
#define GRD_WIN_MAPVK_VK_TO_CHAR    (2)
#define GRD_WIN_MAPVK_VSC_TO_VK_EX  (3)

/*
 * Virtual Keys, Standard Set
 */
#define GRD_WIN_VK_LBUTTON        0x01
#define GRD_WIN_VK_RBUTTON        0x02
#define GRD_WIN_VK_CANCEL         0x03
#define GRD_WIN_VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define GRD_WIN_VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define GRD_WIN_VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */

/*
 * 0x07 : reserved
 */


#define GRD_WIN_VK_BACK           0x08
#define GRD_WIN_VK_TAB            0x09

/*
 * 0x0A - 0x0B : reserved
 */

#define GRD_WIN_VK_CLEAR          0x0C
#define GRD_WIN_VK_RETURN         0x0D

/*
 * 0x0E - 0x0F : unassigned
 */

#define GRD_WIN_VK_SHIFT          0x10
#define GRD_WIN_VK_CONTROL        0x11
#define GRD_WIN_VK_MENU           0x12
#define GRD_WIN_VK_PAUSE          0x13
#define GRD_WIN_VK_CAPITAL        0x14

#define GRD_WIN_VK_KANA           0x15
#define GRD_WIN_VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define GRD_WIN_VK_HANGUL         0x15
#define GRD_WIN_VK_IME_ON         0x16
#define GRD_WIN_VK_JUNJA          0x17
#define GRD_WIN_VK_FINAL          0x18
#define GRD_WIN_VK_HANJA          0x19
#define GRD_WIN_VK_KANJI          0x19
#define GRD_WIN_VK_IME_OFF        0x1A

#define GRD_WIN_VK_ESCAPE         0x1B

#define GRD_WIN_VK_CONVERT        0x1C
#define GRD_WIN_VK_NONCONVERT     0x1D
#define GRD_WIN_VK_ACCEPT         0x1E
#define GRD_WIN_VK_MODECHANGE     0x1F

#define GRD_WIN_VK_SPACE          0x20
#define GRD_WIN_VK_PRIOR          0x21
#define GRD_WIN_VK_NEXT           0x22
#define GRD_WIN_VK_END            0x23
#define GRD_WIN_VK_HOME           0x24
#define GRD_WIN_VK_LEFT           0x25
#define GRD_WIN_VK_UP             0x26
#define GRD_WIN_VK_RIGHT          0x27
#define GRD_WIN_VK_DOWN           0x28
#define GRD_WIN_VK_SELECT         0x29
#define GRD_WIN_VK_PRINT          0x2A
#define GRD_WIN_VK_EXECUTE        0x2B
#define GRD_WIN_VK_SNAPSHOT       0x2C
#define GRD_WIN_VK_INSERT         0x2D
#define GRD_WIN_VK_DELETE         0x2E
#define GRD_WIN_VK_HELP           0x2F

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define GRD_WIN_VK_LWIN           0x5B
#define GRD_WIN_VK_RWIN           0x5C
#define GRD_WIN_VK_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define GRD_WIN_VK_SLEEP          0x5F

#define GRD_WIN_VK_NUMPAD0        0x60
#define GRD_WIN_VK_NUMPAD1        0x61
#define GRD_WIN_VK_NUMPAD2        0x62
#define GRD_WIN_VK_NUMPAD3        0x63
#define GRD_WIN_VK_NUMPAD4        0x64
#define GRD_WIN_VK_NUMPAD5        0x65
#define GRD_WIN_VK_NUMPAD6        0x66
#define GRD_WIN_VK_NUMPAD7        0x67
#define GRD_WIN_VK_NUMPAD8        0x68
#define GRD_WIN_VK_NUMPAD9        0x69
#define GRD_WIN_VK_MULTIPLY       0x6A
#define GRD_WIN_VK_ADD            0x6B
#define GRD_WIN_VK_SEPARATOR      0x6C
#define GRD_WIN_VK_SUBTRACT       0x6D
#define GRD_WIN_VK_DECIMAL        0x6E
#define GRD_WIN_VK_DIVIDE         0x6F
#define GRD_WIN_VK_F1             0x70
#define GRD_WIN_VK_F2             0x71
#define GRD_WIN_VK_F3             0x72
#define GRD_WIN_VK_F4             0x73
#define GRD_WIN_VK_F5             0x74
#define GRD_WIN_VK_F6             0x75
#define GRD_WIN_VK_F7             0x76
#define GRD_WIN_VK_F8             0x77
#define GRD_WIN_VK_F9             0x78
#define GRD_WIN_VK_F10            0x79
#define GRD_WIN_VK_F11            0x7A
#define GRD_WIN_VK_F12            0x7B
#define GRD_WIN_VK_F13            0x7C
#define GRD_WIN_VK_F14            0x7D
#define GRD_WIN_VK_F15            0x7E
#define GRD_WIN_VK_F16            0x7F
#define GRD_WIN_VK_F17            0x80
#define GRD_WIN_VK_F18            0x81
#define GRD_WIN_VK_F19            0x82
#define GRD_WIN_VK_F20            0x83
#define GRD_WIN_VK_F21            0x84
#define GRD_WIN_VK_F22            0x85
#define GRD_WIN_VK_F23            0x86
#define GRD_WIN_VK_F24            0x87

/*
 * 0x88 - 0x8F : UI navigation
 */

#define GRD_WIN_VK_NAVIGATION_VIEW     0x88 // reserved
#define GRD_WIN_VK_NAVIGATION_MENU     0x89 // reserved
#define GRD_WIN_VK_NAVIGATION_UP       0x8A // reserved
#define GRD_WIN_VK_NAVIGATION_DOWN     0x8B // reserved
#define GRD_WIN_VK_NAVIGATION_LEFT     0x8C // reserved
#define GRD_WIN_VK_NAVIGATION_RIGHT    0x8D // reserved
#define GRD_WIN_VK_NAVIGATION_ACCEPT   0x8E // reserved
#define GRD_WIN_VK_NAVIGATION_CANCEL   0x8F // reserved

#define GRD_WIN_VK_NUMLOCK        0x90
#define GRD_WIN_VK_SCROLL         0x91

/*
 * NEC PC-9800 kbd definitions
 */
#define GRD_WIN_VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad

/*
 * Fujitsu/OASYS kbd definitions
 */
#define GRD_WIN_VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define GRD_WIN_VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define GRD_WIN_VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define GRD_WIN_VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define GRD_WIN_VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define GRD_WIN_VK_LSHIFT         0xA0
#define GRD_WIN_VK_RSHIFT         0xA1
#define GRD_WIN_VK_LCONTROL       0xA2
#define GRD_WIN_VK_RCONTROL       0xA3
#define GRD_WIN_VK_LMENU          0xA4
#define GRD_WIN_VK_RMENU          0xA5

#define GRD_WIN_VK_BROWSER_BACK        0xA6
#define GRD_WIN_VK_BROWSER_FORWARD     0xA7
#define GRD_WIN_VK_BROWSER_REFRESH     0xA8
#define GRD_WIN_VK_BROWSER_STOP        0xA9
#define GRD_WIN_VK_BROWSER_SEARCH      0xAA
#define GRD_WIN_VK_BROWSER_FAVORITES   0xAB
#define GRD_WIN_VK_BROWSER_HOME        0xAC

#define GRD_WIN_VK_VOLUME_MUTE         0xAD
#define GRD_WIN_VK_VOLUME_DOWN         0xAE
#define GRD_WIN_VK_VOLUME_UP           0xAF
#define GRD_WIN_VK_MEDIA_NEXT_TRACK    0xB0
#define GRD_WIN_VK_MEDIA_PREV_TRACK    0xB1
#define GRD_WIN_VK_MEDIA_STOP          0xB2
#define GRD_WIN_VK_MEDIA_PLAY_PAUSE    0xB3
#define GRD_WIN_VK_LAUNCH_MAIL         0xB4
#define GRD_WIN_VK_LAUNCH_MEDIA_SELECT 0xB5
#define GRD_WIN_VK_LAUNCH_APP1         0xB6
#define GRD_WIN_VK_LAUNCH_APP2         0xB7


/*
 * 0xB8 - 0xB9 : reserved
 */

#define GRD_WIN_VK_OEM_1          0xBA   // ';:' for US
#define GRD_WIN_VK_OEM_PLUS       0xBB   // '+' any country
#define GRD_WIN_VK_OEM_COMMA      0xBC   // ',' any country
#define GRD_WIN_VK_OEM_MINUS      0xBD   // '-' any country
#define GRD_WIN_VK_OEM_PERIOD     0xBE   // '.' any country
#define GRD_WIN_VK_OEM_2          0xBF   // '/?' for US
#define GRD_WIN_VK_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xC2 : reserved
 */


/*
 * 0xC3 - 0xDA : Gamepad input
 */

#define GRD_WIN_VK_GAMEPAD_A                         0xC3 // reserved
#define GRD_WIN_VK_GAMEPAD_B                         0xC4 // reserved
#define GRD_WIN_VK_GAMEPAD_X                         0xC5 // reserved
#define GRD_WIN_VK_GAMEPAD_Y                         0xC6 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_SHOULDER            0xC7 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_SHOULDER             0xC8 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_TRIGGER              0xC9 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_TRIGGER             0xCA // reserved
#define GRD_WIN_VK_GAMEPAD_DPAD_UP                   0xCB // reserved
#define GRD_WIN_VK_GAMEPAD_DPAD_DOWN                 0xCC // reserved
#define GRD_WIN_VK_GAMEPAD_DPAD_LEFT                 0xCD // reserved
#define GRD_WIN_VK_GAMEPAD_DPAD_RIGHT                0xCE // reserved
#define GRD_WIN_VK_GAMEPAD_MENU                      0xCF // reserved
#define GRD_WIN_VK_GAMEPAD_VIEW                      0xD0 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5 // reserved
#define GRD_WIN_VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9 // reserved
#define GRD_WIN_VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA // reserved



#define GRD_WIN_VK_OEM_4          0xDB  //  '[{' for US
#define GRD_WIN_VK_OEM_5          0xDC  //  '\|' for US
#define GRD_WIN_VK_OEM_6          0xDD  //  ']}' for US
#define GRD_WIN_VK_OEM_7          0xDE  //  ''"' for US
#define GRD_WIN_VK_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */
#define GRD_WIN_VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define GRD_WIN_VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define GRD_WIN_VK_ICO_HELP       0xE3  //  Help key on ICO
#define GRD_WIN_VK_ICO_00         0xE4  //  00 key on ICO

#define GRD_WIN_VK_PROCESSKEY     0xE5
#define GRD_WIN_VK_ICO_CLEAR      0xE6


#define GRD_WIN_VK_PACKET         0xE7

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */
#define GRD_WIN_VK_OEM_RESET      0xE9
#define GRD_WIN_VK_OEM_JUMP       0xEA
#define GRD_WIN_VK_OEM_PA1        0xEB
#define GRD_WIN_VK_OEM_PA2        0xEC
#define GRD_WIN_VK_OEM_PA3        0xED
#define GRD_WIN_VK_OEM_WSCTRL     0xEE
#define GRD_WIN_VK_OEM_CUSEL      0xEF
#define GRD_WIN_VK_OEM_ATTN       0xF0
#define GRD_WIN_VK_OEM_FINISH     0xF1
#define GRD_WIN_VK_OEM_COPY       0xF2
#define GRD_WIN_VK_OEM_AUTO       0xF3
#define GRD_WIN_VK_OEM_ENLW       0xF4
#define GRD_WIN_VK_OEM_BACKTAB    0xF5

#define GRD_WIN_VK_ATTN           0xF6
#define GRD_WIN_VK_CRSEL          0xF7
#define GRD_WIN_VK_EXSEL          0xF8
#define GRD_WIN_VK_EREOF          0xF9
#define GRD_WIN_VK_PLAY           0xFA
#define GRD_WIN_VK_ZOOM           0xFB
#define GRD_WIN_VK_NONAME         0xFC
#define GRD_WIN_VK_PA1            0xFD
#define GRD_WIN_VK_OEM_CLEAR      0xFE
