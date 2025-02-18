#pragma once

#include "grd_base.h"
#include "grd_defer.h"
#include "grd_build.h"
#include "sync/grd_atomics.h"
#include "sync/grd_spinlock.h"

GRD_BUILD_RUN("ctx.params.add_lib('user32.lib')");

#ifndef GRD_USE_WINDOWS_H
	#define GRD_USE_WINDOWS_H 0
#endif

#if !defined(_WINDOWS_) && GRD_USE_WINDOWS_H
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	// #define GRD_WINBASEAPI WINBASEAPI
	// #define GRD_WINUSERAPI WINUSERAPI
	// #define GRD_WINAPI WINAPI
	// #define GRD_WIN_DWORD DWORD
	// #define GRD_WIN_LONG LONG
	// #define GRD_WIN_INFINITE INFINITE
	// #define GRD_WIN_HANDLE HANDLE
	// #define GRD_LPTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE
	// #define GRD_WIN32_CRITICAL_SECTION CRITICAL_SECTION
	// #define GRD_WIN_BOOL BOOL
	// #define GRD_WIN_LONGLONG LONGLONG
	// #define GRD_WIN_LARGE_INTEGER LARGE_INTEGER
	// #define GRD_WIN_UINT_PTR UINT_PTR
	// #define GRD_WIN_WPARAM WPARAM
	// #define GRD_WIN_LONG_PTR LONG_PTR
	// #define GRD_WIN_ULONG_PTR ULONG_PTR
	// #define GRD_WIN_UINT UINT
#elif !defined(_WINDOWS_)
	#define GRD_CRIPPLED_WIN32_API 1
#endif

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
typedef unsigned __int64 GRD_WIN_ULONG_PTR;
typedef GRD_WIN_LONG_PTR GRD_WIN_LPARAM;
typedef long long GRD_WIN_LONG64;
typedef GRD_WIN_DWORD (GRD_WINAPI *GRD_PTHREAD_START_ROUTINE)(void* lpThreadParameter);
typedef GRD_PTHREAD_START_ROUTINE GRD_LPTHREAD_START_ROUTINE;

#define GRD_WIN_MAX_PATH          260

#define GRD_WIN_ULONGLONG unsigned __int64
#define GRD_WIN_CALLBACK __stdcall
typedef GRD_WIN_HANDLE GRD_WIN_HWND;
typedef GRD_WIN_LONG_PTR GRD_WIN_LRESULT;
typedef GRD_WIN_LRESULT (GRD_WIN_CALLBACK* GRD_WIN_WNDPROC)(GRD_WIN_HWND, GRD_WIN_UINT, GRD_WIN_WPARAM, GRD_WIN_LPARAM);
typedef GRD_WIN_HANDLE GRD_WIN_HINSTANCE;
typedef GRD_WIN_HANDLE GRD_WIN_HICON;
typedef GRD_WIN_HICON GRD_WIN_HCURSOR;
typedef GRD_WIN_HANDLE GRD_WIN_HBRUSH;
typedef GRD_WIN_HINSTANCE GRD_WIN_HMODULE;
typedef GRD_WIN_HANDLE GRD_WIN_HMENU;

typedef struct GRD_WIN_WNDCLASSW {
    GRD_WIN_UINT        style;
    GRD_WIN_WNDPROC     lpfnWndProc;
    int                 cbClsExtra;
    int                 cbWndExtra;
    GRD_WIN_HINSTANCE   hInstance;
    GRD_WIN_HICON       hIcon;
    GRD_WIN_HCURSOR     hCursor;
    GRD_WIN_HBRUSH      hbrBackground;
    const wchar_t*      lpszMenuName;
    const wchar_t*      lpszClassName;
} GRD_WIN_WNDCLASSW;

typedef struct GRD_WIN_RECT {
	GRD_WIN_LONG left;
	GRD_WIN_LONG top;
	GRD_WIN_LONG right;
	GRD_WIN_LONG bottom;
} GRD_WIN_RECT;

typedef struct GRD_WIN_POINT {
	GRD_WIN_LONG x;
	GRD_WIN_LONG y;
} GRD_WIN_POINT;

typedef struct GRD_WIN_CREATESTRUCTW {
    void*               lpCreateParams;
    GRD_WIN_HINSTANCE   hInstance;
    GRD_WIN_HMENU       hMenu;
    GRD_WIN_HWND        hwndParent;
    int                 cy;
    int                 cx;
    int                 y;
    int                 x;
    GRD_WIN_LONG        style;
    const wchar_t*      lpszName;
    const wchar_t*      lpszClass;
    GRD_WIN_DWORD       dwExStyle;
} GRD_WIN_CREATESTRUCTW;

typedef struct GRD_WIN_WINDOWPOS {
    GRD_WIN_HWND    hwnd;
    GRD_WIN_HWND    hwndInsertAfter;
    int             x;
    int             y;
    int             cx;
    int             cy;
    unsigned int    flags;
} GRD_WIN_WINDOWPOS;

typedef struct GRD_WIN_NCCALCSIZE_PARAMS {
    GRD_WIN_RECT       rgrc[3];
    GRD_WIN_WINDOWPOS* lppos;
} GRD_WIN_NCCALCSIZE_PARAMS;

typedef struct GRD_WIN_FILETIME {
	GRD_WIN_DWORD dwLowDateTime;
	GRD_WIN_DWORD dwHighDateTime;
} GRD_WIN_FILETIME;

typedef struct GRD_WIN_WIN32_FIND_DATAW {
	GRD_WIN_DWORD dwFileAttributes;
	GRD_WIN_FILETIME ftCreationTime;
	GRD_WIN_FILETIME ftLastAccessTime;
	GRD_WIN_FILETIME ftLastWriteTime;
	GRD_WIN_DWORD nFileSizeHigh;
	GRD_WIN_DWORD nFileSizeLow;
	GRD_WIN_DWORD dwReserved0;
	GRD_WIN_DWORD dwReserved1;
	wchar_t  cFileName[ GRD_WIN_MAX_PATH ];
	wchar_t  cAlternateFileName[ 14 ];
} GRD_WIN_WIN32_FIND_DATAW;

typedef struct GRD_WIN_MINMAXINFO {
    GRD_WIN_POINT ptReserved;
    GRD_WIN_POINT ptMaxSize;
    GRD_WIN_POINT ptMaxPosition;
    GRD_WIN_POINT ptMinTrackSize;
    GRD_WIN_POINT ptMaxTrackSize;
} GRD_WIN_MINMAXINFO;

typedef struct GRD_WIN_MOUSEMOVEPOINT {
    int   x;
    int   y;
    GRD_WIN_DWORD time;
    GRD_WIN_ULONG_PTR dwExtraInfo;
} GRD_WIN_MOUSEMOVEPOINT;

typedef struct GRD_WIN_MSG {
    GRD_WIN_HWND        hwnd;
    GRD_WIN_UINT        message;
    GRD_WIN_WPARAM      wParam;
    GRD_WIN_LPARAM      lParam;
    GRD_WIN_DWORD       time;
    GRD_WIN_POINT       pt;
} GRD_WIN_MSG;


// #define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
// #define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define GRD_WIN_LOWORD(l)           ((GRD_WIN_WORD)(((GRD_WIN_ULONG_PTR)(l)) & 0xffff))
#define GRD_WIN_HIWORD(l)           ((GRD_WIN_WORD)((((GRD_WIN_ULONG_PTR)(l)) >> 16) & 0xffff))
// #define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
// #define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#define GRD_WIN_GET_X_LPARAM(lp) ((int)(short)GRD_WIN_LOWORD(lp))
#define GRD_WIN_GET_Y_LPARAM(lp) ((int)(short)GRD_WIN_HIWORD(lp))

#if GRD_CRIPPLED_WIN32_API
extern "C" {

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

	GRD_WINBASEAPI GRD_WIN_DWORD GRD_WINAPI FormatMessageA(u32 dwFlags, const void* lpSource, u32 dwMessageId, u32 dwLanguageId, char* lpBuffer, u32 nSize, va_list* Arguments);

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

	// FindFirstFileW
	GRD_WINBASEAPI GRD_WIN_HANDLE GRD_WINAPI FindFirstFileW(const wchar_t* lpFileName, GRD_WIN_WIN32_FIND_DATAW* lpFindFileData);
	// FindClose
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI FindClose(GRD_WIN_HANDLE hFindFile);
	// FindNextFileW
	GRD_WINBASEAPI GRD_WIN_BOOL GRD_WINAPI FindNextFileW(GRD_WIN_HANDLE hFindFile, GRD_WIN_WIN32_FIND_DATAW* lpFindFileData);

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
	GRD_WINUSERAPI GRD_WIN_HMODULE GRD_WINAPI GetModuleHandleA(const char* lpModuleName);
	GRD_WINUSERAPI GRD_WIN_HMODULE GRD_WINAPI GetModuleHandleW(const wchar_t* lpModuleName);

	GRD_WINUSERAPI GRD_WIN_WORD GRD_WINAPI RegisterClassW(const GRD_WIN_WNDCLASSW *lpWndClass);
	GRD_WINUSERAPI int GRD_WINAPI GetSystemMetrics(int nIndex);

	
	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI AdjustWindowRect(GRD_WIN_RECT* lpRect, GRD_WIN_DWORD dwStyle, GRD_WIN_BOOL bMenu);
	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI AdjustWindowRectEx(GRD_WIN_RECT* lpRect, GRD_WIN_DWORD dwStyle,
		GRD_WIN_BOOL bMenu,GRD_WIN_DWORD dwExStyle);

	GRD_WINUSERAPI GRD_WIN_HWND GRD_WINAPI CreateWindowExW(
		GRD_WIN_DWORD dwExStyle,
		const wchar_t* lpClassName,
		const wchar_t* lpWindowName,
		GRD_WIN_DWORD dwStyle,
		int X,
		int Y,
		int nWidth,
		int nHeight,
		GRD_WIN_HWND hWndParent,
		GRD_WIN_HMENU hMenu,
		GRD_WIN_HINSTANCE hInstance,
		void* lpParam);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI ShowWindow(GRD_WIN_HWND hWnd, int nCmdShow);
	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI SetForegroundWindow(GRD_WIN_HWND hWnd);
	GRD_WINUSERAPI GRD_WIN_LONG_PTR GRD_WINAPI GetWindowLongPtrW(GRD_WIN_HWND hWnd, int nIndex);
	GRD_WIN_LRESULT GRD_WINAPI DefWindowProcW(GRD_WIN_HWND hWnd, GRD_WIN_UINT Msg, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI ClientToScreen(GRD_WIN_HWND hWnd, GRD_WIN_POINT* lpPoint);
	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI ScreenToClient(GRD_WIN_HWND hWnd, GRD_WIN_POINT* lpPoint);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI IsZoomed(GRD_WIN_HWND hWnd);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI GetWindowRect(GRD_WIN_HWND hWnd, GRD_WIN_RECT* lpRect);

	GRD_WINUSERAPI GRD_WIN_LONG_PTR GRD_WINAPI SetWindowLongPtrW(GRD_WIN_HWND hWnd, int nIndex, GRD_WIN_LONG_PTR dwNewLong);

	GRD_WINUSERAPI GRD_WIN_ULONGLONG GRD_WINAPI GetTickCount64(void);
	GRD_WINUSERAPI GRD_WIN_LONG GRD_WINAPI GetMessageTime(void);

	GRD_WINUSERAPI int GRD_WINAPI GetMouseMovePointsEx(
		GRD_WIN_UINT cbSize, GRD_WIN_MOUSEMOVEPOINT* lppt, GRD_WIN_MOUSEMOVEPOINT* lpptBuf,
		int nBufPoints, GRD_WIN_DWORD resolution);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI PeekMessageW(
		GRD_WIN_MSG* lpMsg,
		GRD_WIN_HWND hWnd,
		GRD_WIN_UINT wMsgFilterMin,
		GRD_WIN_UINT wMsgFilterMax,
		GRD_WIN_UINT wRemoveMsg
	);

	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI TranslateMessage(const GRD_WIN_MSG* lpMsg);
	GRD_WINUSERAPI GRD_WIN_LRESULT GRD_WINAPI DispatchMessageA(const GRD_WIN_MSG* lpMsg);
	GRD_WINUSERAPI GRD_WIN_LRESULT GRD_WINAPI DispatchMessageW(const GRD_WIN_MSG* lpMsg);
	GRD_WINUSERAPI GRD_WIN_BOOL GRD_WINAPI DestroyWindow(GRD_WIN_HWND hWnd);
};
#endif

#define GRD_WIN_IS_MAXIMIZED(hWnd) (IsZoomed(hWnd))

#define GRD_WIN_FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define GRD_WIN_FORMAT_MESSAGE_FROM_STRING     0x00000400
#define GRD_WIN_FORMAT_MESSAGE_FROM_HMODULE    0x00000800
#define GRD_WIN_FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define GRD_WIN_FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
#define GRD_WIN_FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF
#define GRD_WIN_LANG_NEUTRAL                     0x00
#define GRD_WIN_SUBLANG_NEUTRAL                             0x00    // language neutral
#define GRD_WIN_SUBLANG_DEFAULT                             0x01    // user default
#define GRD_WIN_MAKELANGID(p, s)       ((((GRD_WIN_WORD  )(s)) << 10) | (GRD_WIN_WORD  )(p))

#define GRD_WIN_GENERIC_READ                     (0x80000000L)
#define GRD_WIN_GENERIC_WRITE                    (0x40000000L)
#define GRD_WIN_GENERIC_EXECUTE                  (0x20000000L)
#define GRD_WIN_GENERIC_ALL                      (0x10000000L)

#define GRD_WIN_CREATE_NEW          1
#define GRD_WIN_CREATE_ALWAYS       2
#define GRD_WIN_OPEN_EXISTING       3
#define GRD_WIN_OPEN_ALWAYS         4
#define GRD_WIN_TRUNCATE_EXISTING   5

#define GRD_WIN_FILE_SHARE_READ                 0x00000001  
#define GRD_WIN_FILE_SHARE_WRITE                0x00000002  
#define GRD_WIN_FILE_SHARE_DELETE               0x00000004  

#define GRD_WIN_FILE_ATTRIBUTE_NORMAL               0x00000080 
#define GRD_WIN_INVALID_HANDLE_VALUE ((GRD_WIN_HANDLE)(long long)-1)

#define GRD_WIN_FILE_BEGIN           0
#define GRD_WIN_FILE_CURRENT         1
#define GRD_WIN_FILE_END             2

#define GRD_WIN_ERROR_HANDLE_EOF                 38L

#define GRD_WIN_PM_NOREMOVE         0x0000
#define GRD_WIN_PM_REMOVE           0x0001
#define GRD_WIN_PM_NOYIELD          0x0002

#define GRD_WIN_GMMP_USE_DISPLAY_POINTS          1
#define GRD_WIN_GMMP_USE_HIGH_RESOLUTION_POINTS  2

#define GRD_WIN_WHEEL_DELTA                     120
#define GRD_WIN_GET_WHEEL_DELTA_WPARAM(wParam)  ((short)GRD_WIN_HIWORD(wParam))

#define GRD_WIN_HIGH_SURROGATE_START  0xd800
#define GRD_WIN_HIGH_SURROGATE_END    0xdbff
#define GRD_WIN_LOW_SURROGATE_START   0xdc00
#define GRD_WIN_LOW_SURROGATE_END     0xdfff
#define GRD_WIN_IS_HIGH_SURROGATE(wch) (((wch) >= GRD_WIN_HIGH_SURROGATE_START) && ((wch) <= GRD_WIN_HIGH_SURROGATE_END))
#define GRD_WIN_IS_LOW_SURROGATE(wch)  (((wch) >= GRD_WIN_LOW_SURROGATE_START) && ((wch) <= GRD_WIN_LOW_SURROGATE_END))
#define GRD_WIN_IS_SURROGATE_PAIR(hs, ls) (GRD_WIN_IS_HIGH_SURROGATE(hs) && GRD_WIN_IS_LOW_SURROGATE(ls))

#define GRD_WIN_MK_LBUTTON          0x0001
#define GRD_WIN_MK_RBUTTON          0x0002
#define GRD_WIN_MK_SHIFT            0x0004
#define GRD_WIN_MK_CONTROL          0x0008
#define GRD_WIN_MK_MBUTTON          0x0010
#define GRD_WIN_MK_XBUTTON1         0x0020
#define GRD_WIN_MK_XBUTTON2         0x0040

#define GRD_WIN_HTERROR             (-2)
#define GRD_WIN_HTTRANSPARENT       (-1)
#define GRD_WIN_HTNOWHERE           0
#define GRD_WIN_HTCLIENT            1
#define GRD_WIN_HTCAPTION           2
#define GRD_WIN_HTSYSMENU           3
#define GRD_WIN_HTGROWBOX           4
#define GRD_WIN_HTSIZE              GRD_WIN_HTGROWBOX
#define GRD_WIN_HTMENU              5
#define GRD_WIN_HTHSCROLL           6
#define GRD_WIN_HTVSCROLL           7
#define GRD_WIN_HTMINBUTTON         8
#define GRD_WIN_HTMAXBUTTON         9
#define GRD_WIN_HTLEFT              10
#define GRD_WIN_HTRIGHT             11
#define GRD_WIN_HTTOP               12
#define GRD_WIN_HTTOPLEFT           13
#define GRD_WIN_HTTOPRIGHT          14
#define GRD_WIN_HTBOTTOM            15
#define GRD_WIN_HTBOTTOMLEFT        16
#define GRD_WIN_HTBOTTOMRIGHT       17
#define GRD_WIN_HTBORDER            18
#define GRD_WIN_HTREDUCE            GRD_WIN_HTMINBUTTON
#define GRD_WIN_HTZOOM              GRD_WIN_HTMAXBUTTON
#define GRD_WIN_HTSIZEFIRST         GRD_WIN_HTLEFT
#define GRD_WIN_HTSIZELAST          GRD_WIN_HTBOTTOMRIGHT
#define GRD_WIN_HTOBJECT            19
#define GRD_WIN_HTCLOSE             20
#define GRD_WIN_HTHELP              21

#define GRD_WIN_GWLP_WNDPROC        (-4)
#define GRD_WIN_GWLP_HINSTANCE      (-6)
#define GRD_WIN_GWLP_HWNDPARENT     (-8)
#define GRD_WIN_GWLP_USERDATA       (-21)
#define GRD_WIN_GWLP_ID             (-12)

#define GRD_WIN_SW_HIDE             0
#define GRD_WIN_SW_SHOWNORMAL       1
#define GRD_WIN_SW_NORMAL           1
#define GRD_WIN_SW_SHOWMINIMIZED    2
#define GRD_WIN_SW_SHOWMAXIMIZED    3
#define GRD_WIN_SW_MAXIMIZE         3
#define GRD_WIN_SW_SHOWNOACTIVATE   4
#define GRD_WIN_SW_SHOW             5
#define GRD_WIN_SW_MINIMIZE         6
#define GRD_WIN_SW_SHOWMINNOACTIVE  7
#define GRD_WIN_SW_SHOWNA           8
#define GRD_WIN_SW_RESTORE          9
#define GRD_WIN_SW_SHOWDEFAULT      10
#define GRD_WIN_SW_FORCEMINIMIZE    11
#define GRD_WIN_SW_MAX              11



#define GRD_WIN_WS_OVERLAPPED       0x00000000L
#define GRD_WIN_WS_POPUP            0x80000000L
#define GRD_WIN_WS_CHILD            0x40000000L
#define GRD_WIN_WS_MINIMIZE         0x20000000L
#define GRD_WIN_WS_VISIBLE          0x10000000L
#define GRD_WIN_WS_DISABLED         0x08000000L
#define GRD_WIN_WS_CLIPSIBLINGS     0x04000000L
#define GRD_WIN_WS_CLIPCHILDREN     0x02000000L
#define GRD_WIN_WS_MAXIMIZE         0x01000000L
#define GRD_WIN_WS_CAPTION          0x00C00000L     /* GRD_WIN_WS_BORDER | GRD_WIN_WS_DLGFRAME  */
#define GRD_WIN_WS_BORDER           0x00800000L
#define GRD_WIN_WS_DLGFRAME         0x00400000L
#define GRD_WIN_WS_VSCROLL          0x00200000L
#define GRD_WIN_WS_HSCROLL          0x00100000L
#define GRD_WIN_WS_SYSMENU          0x00080000L
#define GRD_WIN_WS_THICKFRAME       0x00040000L
#define GRD_WIN_WS_GROUP            0x00020000L
#define GRD_WIN_WS_TABSTOP          0x00010000L

#define GRD_WIN_WS_MINIMIZEBOX      0x00020000L
#define GRD_WIN_WS_MAXIMIZEBOX      0x00010000L


#define GRD_WIN_WS_TILED            GRD_WIN_WS_OVERLAPPED
#define GRD_WIN_WS_ICONIC           GRD_WIN_WS_MINIMIZE
#define GRD_WIN_WS_SIZEBOX          GRD_WIN_WS_THICKFRAME
#define GRD_WIN_WS_TILEDWINDOW      GRD_WIN_WS_OVERLAPPEDWINDOW

#define GRD_WIN_WS_OVERLAPPEDWINDOW (GRD_WIN_WS_OVERLAPPED     | \
                             GRD_WIN_WS_CAPTION        | \
                             GRD_WIN_WS_SYSMENU        | \
                             GRD_WIN_WS_THICKFRAME     | \
                             GRD_WIN_WS_MINIMIZEBOX    | \
                             GRD_WIN_WS_MAXIMIZEBOX)

#define GRD_WIN_WS_POPUPWINDOW      (GRD_WIN_WS_POPUP          | \
                             GRD_WIN_WS_BORDER         | \
                             GRD_WIN_WS_SYSMENU)

#define GRD_WIN_WS_CHILDWINDOW      (GRD_WIN_WS_CHILD)

/*
 * Extended Window Styles
 */
#define GRD_WIN_WS_EX_DLGMODALFRAME     0x00000001L
#define GRD_WIN_WS_EX_NOPARENTNOTIFY    0x00000004L
#define GRD_WIN_WS_EX_TOPMOST           0x00000008L
#define GRD_WIN_WS_EX_ACCEPTFILES       0x00000010L
#define GRD_WIN_WS_EX_TRANSPARENT       0x00000020L
#define GRD_WIN_WS_EX_MDICHILD          0x00000040L
#define GRD_WIN_WS_EX_TOOLWINDOW        0x00000080L
#define GRD_WIN_WS_EX_WINDOWEDGE        0x00000100L
#define GRD_WIN_WS_EX_CLIENTEDGE        0x00000200L
#define GRD_WIN_WS_EX_CONTEXTHELP       0x00000400L


#define GRD_WIN_WS_EX_RIGHT             0x00001000L
#define GRD_WIN_WS_EX_LEFT              0x00000000L
#define GRD_WIN_WS_EX_RTLREADING        0x00002000L
#define GRD_WIN_WS_EX_LTRREADING        0x00000000L
#define GRD_WIN_WS_EX_LEFTSCROLLBAR     0x00004000L
#define GRD_WIN_WS_EX_RIGHTSCROLLBAR    0x00000000L

#define GRD_WIN_WS_EX_CONTROLPARENT     0x00010000L
#define GRD_WIN_WS_EX_STATICEDGE        0x00020000L
#define GRD_WIN_WS_EX_APPWINDOW         0x00040000L


#define GRD_WIN_WS_EX_OVERLAPPEDWINDOW  (GRD_WIN_WS_EX_WINDOWEDGE | GRD_WIN_WS_EX_CLIENTEDGE)
#define GRD_WIN_WS_EX_PALETTEWINDOW     (GRD_WIN_WS_EX_WINDOWEDGE | GRD_WIN_WS_EX_TOOLWINDOW | GRD_WIN_WS_EX_TOPMOST)


#define GRD_WIN_WS_EX_LAYERED           0x00080000

#define GRD_WIN_WS_EX_NOINHERITLAYOUT   0x00100000L // Disable inheritence of mirroring by children

#define GRD_WIN_WS_EX_NOREDIRECTIONBITMAP 0x00200000L

#define GRD_WIN_WS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring

#define GRD_WIN_WS_EX_COMPOSITED        0x02000000L
#define GRD_WIN_WS_EX_NOACTIVATE        0x08000000L



#define GRD_WIN_SM_CXSCREEN             0
#define GRD_WIN_SM_CYSCREEN             1
#define GRD_WIN_SM_CXVSCROLL            2
#define GRD_WIN_SM_CYHSCROLL            3
#define GRD_WIN_SM_CYCAPTION            4
#define GRD_WIN_SM_CXBORDER             5
#define GRD_WIN_SM_CYBORDER             6
#define GRD_WIN_SM_CXDLGFRAME           7
#define GRD_WIN_SM_CYDLGFRAME           8
#define GRD_WIN_SM_CYVTHUMB             9
#define GRD_WIN_SM_CXHTHUMB             10
#define GRD_WIN_SM_CXICON               11
#define GRD_WIN_SM_CYICON               12
#define GRD_WIN_SM_CXCURSOR             13
#define GRD_WIN_SM_CYCURSOR             14
#define GRD_WIN_SM_CYMENU               15
#define GRD_WIN_SM_CXFULLSCREEN         16
#define GRD_WIN_SM_CYFULLSCREEN         17
#define GRD_WIN_SM_CYKANJIWINDOW        18
#define GRD_WIN_SM_MOUSEPRESENT         19
#define GRD_WIN_SM_CYVSCROLL            20
#define GRD_WIN_SM_CXHSCROLL            21
#define GRD_WIN_SM_DEBUG                22
#define GRD_WIN_SM_SWAPBUTTON           23
#define GRD_WIN_SM_RESERVED1            24
#define GRD_WIN_SM_RESERVED2            25
#define GRD_WIN_SM_RESERVED3            26
#define GRD_WIN_SM_RESERVED4            27
#define GRD_WIN_SM_CXMIN                28
#define GRD_WIN_SM_CYMIN                29
#define GRD_WIN_SM_CXSIZE               30
#define GRD_WIN_SM_CYSIZE               31
#define GRD_WIN_SM_CXFRAME              32
#define GRD_WIN_SM_CYFRAME              33
#define GRD_WIN_SM_CXMINTRACK           34
#define GRD_WIN_SM_CYMINTRACK           35
#define GRD_WIN_SM_CXDOUBLECLK          36
#define GRD_WIN_SM_CYDOUBLECLK          37
#define GRD_WIN_SM_CXICONSPACING        38
#define GRD_WIN_SM_CYICONSPACING        39
#define GRD_WIN_SM_MENUDROPALIGNMENT    40
#define GRD_WIN_SM_PENWINDOWS           41
#define GRD_WIN_SM_DBCSENABLED          42
#define GRD_WIN_SM_CMOUSEBUTTONS        43

#define GRD_WIN_SM_CXFIXEDFRAME           GRD_WIN_SM_CXDLGFRAME  /* ;win40 name change */
#define GRD_WIN_SM_CYFIXEDFRAME           GRD_WIN_SM_CYDLGFRAME  /* ;win40 name change */
#define GRD_WIN_SM_CXSIZEFRAME            GRD_WIN_SM_CXFRAME     /* ;win40 name change */
#define GRD_WIN_SM_CYSIZEFRAME            GRD_WIN_SM_CYFRAME     /* ;win40 name change */

#define GRD_WIN_SM_SECURE               44
#define GRD_WIN_SM_CXEDGE               45
#define GRD_WIN_SM_CYEDGE               46
#define GRD_WIN_SM_CXMINSPACING         47
#define GRD_WIN_SM_CYMINSPACING         48
#define GRD_WIN_SM_CXSMICON             49
#define GRD_WIN_SM_CYSMICON             50
#define GRD_WIN_SM_CYSMCAPTION          51
#define GRD_WIN_SM_CXSMSIZE             52
#define GRD_WIN_SM_CYSMSIZE             53
#define GRD_WIN_SM_CXMENUSIZE           54
#define GRD_WIN_SM_CYMENUSIZE           55
#define GRD_WIN_SM_ARRANGE              56
#define GRD_WIN_SM_CXMINIMIZED          57
#define GRD_WIN_SM_CYMINIMIZED          58
#define GRD_WIN_SM_CXMAXTRACK           59
#define GRD_WIN_SM_CYMAXTRACK           60
#define GRD_WIN_SM_CXMAXIMIZED          61
#define GRD_WIN_SM_CYMAXIMIZED          62
#define GRD_WIN_SM_NETWORK              63
#define GRD_WIN_SM_CLEANBOOT            67
#define GRD_WIN_SM_CXDRAG               68
#define GRD_WIN_SM_CYDRAG               69
#define GRD_WIN_SM_SHOWSOUNDS           70
#define GRD_WIN_SM_CXMENUCHECK          71   /* Use instead of GetMenuCheckMarkDimensions()! */
#define GRD_WIN_SM_CYMENUCHECK          72
#define GRD_WIN_SM_SLOWMACHINE          73
#define GRD_WIN_SM_MIDEASTENABLED       74

#define GRD_WIN_SM_MOUSEWHEELPRESENT    75
#define GRD_WIN_SM_XVIRTUALSCREEN       76
#define GRD_WIN_SM_YVIRTUALSCREEN       77
#define GRD_WIN_SM_CXVIRTUALSCREEN      78
#define GRD_WIN_SM_CYVIRTUALSCREEN      79
#define GRD_WIN_SM_CMONITORS            80
#define GRD_WIN_SM_SAMEDISPLAYFORMAT    81
#define GRD_WIN_SM_IMMENABLED           82
#define GRD_WIN_SM_CXFOCUSBORDER        83
#define GRD_WIN_SM_CYFOCUSBORDER        84
#define GRD_WIN_SM_TABLETPC             86
#define GRD_WIN_SM_MEDIACENTER          87
#define GRD_WIN_SM_STARTER              88
#define GRD_WIN_SM_SERVERR2             89
#define GRD_WIN_SM_MOUSEHORIZONTALWHEELPRESENT    91
#define GRD_WIN_SM_CXPADDEDBORDER       92

#define GRD_WIN_SM_DIGITIZER            94
#define GRD_WIN_SM_MAXIMUMTOUCHES       95
#define GRD_WIN_SM_CMETRICS             97

#define GRD_WIN_SM_REMOTESESSION        0x1000
#define GRD_WIN_SM_SHUTTINGDOWN           0x2000
#define GRD_WIN_SM_REMOTECONTROL          0x2001
#define GRD_WIN_SM_CARETBLINKINGENABLED   0x2002

#define GRD_WIN_SM_CONVERTIBLESLATEMODE   0x2003
#define GRD_WIN_SM_SYSTEMDOCKED           0x2004



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
