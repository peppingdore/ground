#pragma once

#include "../string.h"
#include "../sync_primitives.h"
#include "../scoped.h"
#include "../math/vector.h"

struct WindowParams {
	String   title;
	Vector2i initial_size = { 1280, 720 };
};

struct Window {
	WindowParams params;
};


struct WindowsWindow: Window {
	wchar_t* utf16_title = NULL;
};

Spinlock WINDOWS_WINDOW_CREATE_LOCK;
bool     WINDOWS_IS_WINDOW_CLASS_CREATED = false;
// Windows sends WM_GETMINMAXINFO event before WM_NCCREATE,
//  which means we can't read Window* pointer from GWLP_USERDATA, because we set GWLP_USERDATA in WM_NCCREATE,
//  so the solution is to have a global lock with global Window* pointer, which we can use in WM_GETMINMAXINFO.
Window*  WINDOWS_GETMINMAXINFO_WINDOW_POINTER = NULL;

LRESULT ground_wnd_proc(HWND h, UINT m, WPARAMS wParam, LPARAM lParam);

const wchar_t* windows_get_window_class() {
	// This function assumes you have taken the creation lock.
	auto class_name = L"GROUND_WINDOW_CLASS";
	if (!WINDOWS_IS_WINDOW_CLASS_CREATED) {
		WINDOWS_IS_WINDOW_CLASS_CREATED = true;
		WNDCLASS wc = { 
			.hInstance = GetModuleHandle(NULL),
			.lpszClassName = class_name,
			.lpfnWndProc = ground_wnd_proc,
		};
		RegisterClass(&wc);
	}
	return class_name;
}

WindowsWindow* create_window(WindowParams params) {
	ScopedLock(WINDOWS_WINDOW_CREATE_LOCK);

	s32 window_width  = GetSystemMetrics(SM_CXSCREEN) / 2 - params.initial_size.x / 2;
	s32 window_height = GetSystemMetrics(SM_CYSCREEN) / 2 - params.initial_size.y / 2;
	RECT window_rect = { window_width, window_height, window_width + params.initial_size.x, window_height + params.initial_size.y };
	DWORD window_style = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	AdjustWindowRect(&window_rect, window_style, false);

	auto window = make<WindowsWindow>();
	window->params = params;
	window->utf16_title = (wchar_t*) encode_utf16(params.title).data;
	
	WINDOWS_GETMINMAXINFO_WINDOW_POINTER = window;

	HWND hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		windows_get_window_class(),
		window->utf16_title,
		window_style,
		window_rect.left,
		window_rect.top,
		window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top,
		NULL,   
		NULL,
		GetModuleHandle(NULL),
		window
	);

	if (hwnd == NULL) {
		c_allocator.free(window->utf16_title);
		c_allocator.free(window);
		auto error = windows_error();
		Log("Failed to CreateWindowEx(), %", e);
		error->free();
		return NULL;
	}
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	SetForegroundWindow(hwnd);
	return window;
}


LRESULT ground_wnd_proc(HWND h, UINT m, WPARAMS wParam, LPARAM lParam) {
	auto window = (Window*) GetWindowLongPtrW(h, GWLP_USERDATA);
	
	switch (m) {
		case WM_ACTIVATE: {
			// if (window->params.borderless) {
			// 	MARGINS margins = { 0, 0, 0, 0 };
			// 	DwmExtendFrameIntoClientArea(h, &margins);
			// 	DWORD huy = DWMNCRP_ENABLED;
			// 	DwmSetWindowAttribute(h, DWMWA_NCRENDERING_POLICY, &huy, sizeof(DWORD));
			// }

			window->has_focus = LOWORD(wParam) != 0;
			if (window->windows_event_callback) {
				return window->windows_event_callback(h, m, wParam, lParam);
			}
		}
		break;
		case WM_NCHITTEST: {
			POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			POINT ptMouseWindowLocal = ptMouse;
			ScreenToClient(h, &ptMouseWindowLocal);

			auto mouse_position = Vector2i::make(ptMouseWindowLocal.x, window->size.y - ptMouseWindowLocal.y);
			
			auto maybe_return_user_provided_region = [&](LRESULT result) -> LRESULT {
				if (result == HTCLIENT && window->params.get_window_region_for_point) {
					return window_region_to_windows(window->params.get_window_region_for_point(window, mouse_position));
				}
				return result;
			};
			
			if (!window->params.borderless) {
				auto default_result = DefWindowProcW(h, m, wParam, lParam);
				return maybe_return_user_provided_region(default_result);
			}

			int LEFTEXTENDWIDTH = 8;
			int RIGHTEXTENDWIDTH = 8;
			int BOTTOMEXTENDWIDTH = 8;
			int TOPEXTENDWIDTH = 8; // @TODO: is this correct??

			if (IsMaximized(h)) {
				TOPEXTENDWIDTH += 8;
			}

			// Get the window rectangle.
			RECT rcWindow;
			GetWindowRect(h, &rcWindow);

			// Get the frame rectangle, adjusted for the style without a caption.
			// RECT rcFrame = { 0 };
			// AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, NULL);

			// Determine if the hit test is for resizing. Default middle (1,1).
			USHORT uRow = 1;
			USHORT uCol = 1;
			bool fOnResizeBorder = false;

			// Determine if the point is at the top or bottom of the window.
			if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + TOPEXTENDWIDTH) {
				fOnResizeBorder = (ptMouse.y < (rcWindow.top + 4));
				uRow = 0;
			} else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - BOTTOMEXTENDWIDTH) {
				uRow = 2;
			}

			// Determine if the point is at the left or right of the window.
			if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + LEFTEXTENDWIDTH) {
				uCol = 0; // left side
			} else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - RIGHTEXTENDWIDTH) {
				uCol = 2; // right side
			}


			if (uRow == 0 && uCol == 1) {
				if (window->params.get_window_region_for_point) {
					if (window->params.get_window_region_for_point(window, mouse_position) == Window_Region::Client) {
						return window_region_to_windows(Window_Region::Client);
					}
				}
				// Need further detection.
			}

			if (IsMaximized(h)) {
				if (uRow > 0) {
					return maybe_return_user_provided_region(HTCLIENT);
				}
				if (uCol != 1) {
					return maybe_return_user_provided_region(HTCLIENT);
				}
				return HTCAPTION;
			}

			LRESULT hitTests[3][3] = {
				{ HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION, HTTOPRIGHT },
				{ HTLEFT,       HTCLIENT, HTRIGHT },
				{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
			};
			return maybe_return_user_provided_region(hitTests[uRow][uCol]);
		}
		break;

		case WM_NCCREATE: {
			CREATESTRUCTW* createstruct = (CREATESTRUCTW*) lParam;
			SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR) createstruct->lpCreateParams);
			return DefWindowProc(h, m, wParam, lParam);
		}
		break;
		
		case WM_CREATE: {
			SetWindowPos(h, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		}
		break;

		case WM_DESTROY:
		case WM_CLOSE: {
			window->wants_to_close = true;
		}
		break;

		case WM_ENTERSIZEMOVE: {
			window->is_being_resized = true;
		}
		break;
		case WM_EXITSIZEMOVE: {
			window->is_being_resized = false;
		}
		break;

		case WM_WINDOWPOSCHANGED: {
			WINDOWPOS* window_pos = (WINDOWPOS*)lParam;
			RECT new_client_rect;
			GetClientRect(h, &new_client_rect);
			int new_width  = new_client_rect.right  - new_client_rect.left;
			int new_height = new_client_rect.bottom - new_client_rect.top;
			handle_window_resize(window, Vector2i::make(new_width, new_height));
		}
		break;

		case WM_SIZING: {
			RECT new_client_rect;
			GetClientRect(h, &new_client_rect);
			int new_width  = new_client_rect.right  - new_client_rect.left;
			int new_height = new_client_rect.bottom - new_client_rect.top;
			handle_window_resize(window, Vector2i::make(new_width, new_height));
			return DefWindowProc(h, m, wParam, lParam);
		}
		break;

		case WM_NCCALCSIZE: {
			if (window->params.borderless) {
				if (IsMaximized(h)) {
					NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
					union {
						LPARAM lparam;
						RECT* rect;
					} params = { .lparam = lParam };
					RECT nonclient = *params.rect;
					DefWindowProcW(h, WM_NCCALCSIZE, wParam, lParam);
					RECT client = *params.rect;
					pncsp->rgrc[0] = nonclient;
					pncsp->rgrc[0].top    += 8;
					pncsp->rgrc[0].bottom -= 8;
					pncsp->rgrc[0].left   += 8;
					pncsp->rgrc[0].right  -= 8;
				} else {
					int m_ncTop = 0;
					int m_ncBottom = -8;
					int m_ncLeft = -8;
					int m_ncRight = -8;
					if (wParam == TRUE) {
						NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
						params->rgrc[0].top    -= m_ncTop;
						params->rgrc[0].bottom += m_ncBottom;
						params->rgrc[0].left   -= m_ncLeft;
						params->rgrc[0].right  += m_ncRight;
						return 0;
					}
				}
				return 0;
			} else {
				return DefWindowProcW(h, WM_NCCALCSIZE, wParam, lParam);
			}
		}
		break;
		case WM_GETMINMAXINFO: {
			window = windows_brokenness_window_being_created;
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = window->params.min_size.x;
			lpMMI->ptMinTrackSize.y = window->params.min_size.y;
		}
		break;
		default: {
			if (window->windows_event_callback) {
				return window->windows_event_callback(h, m, wParam, lParam);
			}
			return DefWindowProc(h, m, wParam, lParam);
		}
	}
	return false;
}
