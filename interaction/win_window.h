#pragma once

#include "../build.h"
#include "window_interface.h"
#include "event.h"
#include "win_key.h"
#include "pointer.h"
#include "win_message_codes.h"
#include "../log.h"

#include <windowsx.h>

BUILD_RUN("params.add_lib('user32.lib')");


struct WindowsWindow: Window {
	HWND               hwnd;
	wchar_t*           utf16_title = NULL;
	Array<Event*>      events;
	char16_t           wm_char_high_surrogate = 0;
	LRESULT          (*wnd_proc)(WindowsWindow* window, HWND h, UINT m, WPARAM wParam, LPARAM lParam) = NULL;
	OsPointerIdMapper  os_pointer_mapper;
	LONG               last_wmmousemove_time = 0;
};

Spinlock WINDOWS_WINDOW_CREATE_LOCK;
bool     WINDOWS_IS_WINDOW_CLASS_CREATED = false;
// Windows sends WM_GETMINMAXINFO event before WM_NCCREATE,
//  which means we can't read Window* pointer from GWLP_USERDATA, because we set GWLP_USERDATA in WM_NCCREATE,
//  so the solution is to have a global lock with global Window* pointer, which we can use in WM_GETMINMAXINFO.
WindowsWindow* WINDOWS_GETMINMAXINFO_WINDOW_POINTER = NULL;

LRESULT ground_wnd_proc(HWND h, UINT m, WPARAM wParam, LPARAM lParam);

const wchar_t* windows_get_window_class() {
	// This function assumes you have taken the creation lock.
	auto class_name = L"GROUND_WINDOW_CLASS";
	if (!WINDOWS_IS_WINDOW_CLASS_CREATED) {
		WINDOWS_IS_WINDOW_CLASS_CREATED = true;
		WNDCLASSW wc = { 
			.lpfnWndProc = ground_wnd_proc,
			.hInstance = GetModuleHandle(NULL),
			.lpszClassName = class_name,
		};
		RegisterClassW(&wc);
	}
	return class_name;
}

Tuple<WindowsWindow*, Error*> os_create_window(WindowParams params) {
	ScopedLock(WINDOWS_WINDOW_CREATE_LOCK);

	s32 window_width  = GetSystemMetrics(SM_CXSCREEN) / 2 - params.initial_size.x / 2;
	s32 window_height = GetSystemMetrics(SM_CYSCREEN) / 2 - params.initial_size.y / 2;
	RECT window_rect = { window_width, window_height, window_width + (s32) params.initial_size.x, window_height + (s32) params.initial_size.y };
	DWORD window_style = WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	AdjustWindowRect(&window_rect, window_style, false);

	auto window = make<WindowsWindow>();
	window->type = reflect.type_of<WindowsWindow>();
	window->params = params;
	window->utf16_title = (wchar_t*) encode_utf16(params.title)._0;
	
	WINDOWS_GETMINMAXINFO_WINDOW_POINTER = window;

	window->hwnd = CreateWindowExW(
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

	if (window->hwnd == NULL) {
		c_allocator.free(window->utf16_title);
		c_allocator.free(window);
		return { NULL, windows_error() };
	}

	ShowWindow(window->hwnd, SW_SHOWDEFAULT);
	SetForegroundWindow(window->hwnd);
	return { window, NULL };
}

LRESULT ground_wnd_proc(HWND h, UINT m, WPARAM wParam, LPARAM lParam) {
	auto window = (WindowsWindow*) GetWindowLongPtrW(h, GWLP_USERDATA);

	switch (m) {
		case WM_NCHITTEST: {
			if (!window->params.borderless) {
				return DefWindowProcW(h, m, wParam, lParam);
			}

			POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			POINT ptMouseWindowLocal = ptMouse;
			ScreenToClient(h, &ptMouseWindowLocal);

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

			// Determine if the hit test is for resizing. Default middle (1,1).
			USHORT row = 1;
			USHORT column = 1;
			bool   on_top_resize_border = false;

			// Determine if the point is at the top or bottom of the window.
			if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + TOPEXTENDWIDTH) {
				on_top_resize_border = (ptMouse.y < (rcWindow.top + 4));
				row = 0;
			} else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - BOTTOMEXTENDWIDTH) {
				row = 2;
			}

			// Determine if the point is at the left or right of the window.
			if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + LEFTEXTENDWIDTH) {
				column = 0;
			} else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - RIGHTEXTENDWIDTH) {
				column = 2;
			}

			if (IsMaximized(h)) {
				if (row > 0) {
					return HTCLIENT;
				}
				if (column != 1) {
					return HTCLIENT;
				}
				return HTCAPTION;
			}

			LRESULT hit_tests[3][3] = {
				{ HTTOPLEFT,    on_top_resize_border ? HTTOP : HTCAPTION, HTTOPRIGHT },
				{ HTLEFT,       HTCLIENT, HTRIGHT },
				{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
			};
			return hit_tests[row][column];
		}
		break;

		case WM_NCCREATE: {
			CREATESTRUCTW* createstruct = (CREATESTRUCTW*) lParam;
			SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR) createstruct->lpCreateParams);
			return DefWindowProcW(h, m, wParam, lParam);
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
			}
		}
		break;
		case WM_GETMINMAXINFO: {
			window = WINDOWS_GETMINMAXINFO_WINDOW_POINTER;
			LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
			lpMMI->ptMinTrackSize.x = window->params.min_size.x;
			lpMMI->ptMinTrackSize.y = window->params.min_size.y;
		}
		break;

		default: {
			if (window->wnd_proc) {
				return window->wnd_proc(window, h, m, wParam, lParam);
			}
		}
		break;
	}
	return DefWindowProcW(h, m, wParam, lParam);
}

f64 get_windows_time_from_start(DWORD message_time) {
	u64 time = GetTickCount64();
	// Adjust the lower bits.
	*((u32*) &time) = message_time;
	return ((f64) time) / 1000.0;
}

void push_windows_window_event(WindowsWindow* window, WindowEvent* event, Optional<DWORD> custom_time = {}) {
	// GetMessageTime() is like GetTickCount(), but casted to LONG instead of DWORD.
	auto time = custom_time.has_value ? custom_time.value : bitcast<DWORD>(GetMessageTime());
	event->time_from_system_start = get_windows_time_from_start(time);
	window->events.add(event);
}

PointerButtonFlags wparam_to_pointer_buttons(WPARAM wParam) {
	u64 buttons = 0;
	if (wParam & MK_LBUTTON)  buttons |= 1 << 0;
	if (wParam & MK_RBUTTON)  buttons |= 1 << 1;
	if (wParam & MK_MBUTTON)  buttons |= 1 << 2;
	if (wParam & MK_XBUTTON1) buttons |= 1 << 3;
	if (wParam & MK_XBUTTON2) buttons |= 1 << 4;
	return (PointerButtonFlags) buttons;
}

Vector2 convert_windows_coords(HWND hwnd, POINT p) {
	RECT window_rect;
	GetWindowRect(hwnd, &window_rect);
	return Vector2::make(p.x, window_rect.bottom - p.y);
}

void push_windows_mouse_button_event(WindowsWindow* window, WPARAM wParam, LPARAM lParam, PointerAction action, PointerButtonFlags button) {
	POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	Pointer pointer = {
		.id       = map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
		.kind     = PointerKind::Mouse,
		.position = convert_windows_coords(window->hwnd, p),
		.buttons  = wparam_to_pointer_buttons(wParam),
	};
	PointerEvent* event = make_event<PointerEvent>();
	event->pointer = pointer;
	event->action = action;
	event->changed_buttons = button;
	push_windows_window_event(window, event);
}

LRESULT read_event_wnd_proc(WindowsWindow* window, HWND h, UINT m, WPARAM wParam, LPARAM lParam) {
	switch (m) {
		case WM_ACTIVATE: {
			auto event = make_event<FocusEvent>();
			event->have_focus = LOWORD(wParam) != 0;
			push_windows_window_event(window, event);
		}
		break;

		case WM_CHAR: {
			if (wParam < 32) {
				return true;
			}

			switch (wParam) {
				case VK_BACK:
				case VK_TAB:
				case VK_RETURN:
				case VK_ESCAPE:
				case 127: // Ctrl + backspace
					return true;
			}

			if (IS_HIGH_SURROGATE(wParam)) {
				window->wm_char_high_surrogate = wParam;
				return 0;
			}

			char32_t c;
			if (window->wm_char_high_surrogate && wParam >= 0xDC00) {
				c = (window->wm_char_high_surrogate - 0xD800) << 10;
				c |= ((char32_t) wParam) - 0xDC00;
				c += 0x10000;
			} else {
				c = (char32_t) wParam;
			}
			auto event = make_event<CharEvent>();
			event->character = c;
			push_windows_window_event(window, event);

			window->wm_char_high_surrogate = 0;
		}
		break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			auto event = make_event<KeyEvent>();
			event->action = (m == WM_KEYDOWN || m == WM_SYSKEYDOWN) ? KeyAction::Down : KeyAction::Up;
			event->key = map_windows_key(wParam, lParam);
			event->os_key_code = wParam;
			push_windows_window_event(window, event);
		}
		break;

		case WM_MOUSEWHEEL: {
			s32 int_delta   =     -GET_WHEEL_DELTA_WPARAM(wParam)  /     WHEEL_DELTA;
			f32 float_delta = f32(-GET_WHEEL_DELTA_WPARAM(wParam)) / f32(WHEEL_DELTA);

			// WM_MOUSEWHEEL has screen relative coords, unlike WM_LBUTTONDOWN for example, which has client relative coords.
			POINT p = { .x = GET_X_LPARAM(lParam), .y = GET_Y_LPARAM(lParam) };
			ScreenToClient(window->hwnd, &p);

			Pointer pointer = {
				.id = map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
				.kind = PointerKind::Mouse,
				.position = convert_windows_coords(window->hwnd, p),
				.buttons = wparam_to_pointer_buttons(wParam),
			};

			PointerEvent* event = make_event<PointerEvent>();
			event->pointer = pointer;
			event->action = PointerAction::Scroll;
			event->scroll_delta = { 0, float_delta };
			event->is_scroll_in_lines = true;
			push_windows_window_event(window, event);
		}
		break;

		case WM_MOUSEMOVE: {
			// https://developer.blender.org/rBd5c59913de95b6b6952088f175a8393bef376d27
			// This link helped to correct the initial code.
			
			POINT p = { .x = GET_X_LPARAM(lParam), .y = GET_Y_LPARAM(lParam) };
			ClientToScreen(window->hwnd, &p);

			MOUSEMOVEPOINT current_point = {
				.x    = p.x,
				.y    = p.y,
				.time = bitcast<DWORD>(GetMessageTime()),
			};

			MOUSEMOVEPOINT points[65]; // 64 + 1 (+1 for the current_point)

			int count = GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), &current_point, points, 64, GMMP_USE_DISPLAY_POINTS);
			int i = 0;
			while (i < count) {
				if (points[i].x > 32767) points[i].x -= 65536;
				if (points[i].y > 32767) points[i].y -= 65536;

				auto point_time = get_windows_time_from_start(points[i].time);
				if (point_time < window->last_wmmousemove_time) {
					break;
				}
				if (point_time == window->last_wmmousemove_time &&
					points[i].x == p.x &&
					points[i].y == p.y) {
					break;
				}
				i += 1;
			}

			points[i] = current_point;
			i += 1;

			while (--i >= 0) {
				POINT p = { .x = points[i].x, .y = points[i].y };
				ScreenToClient(window->hwnd, &p);
				Pointer pointer = {
					.id       = map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
					.kind     = PointerKind::Mouse,
					.position = convert_windows_coords(window->hwnd, p),
					.buttons  = wparam_to_pointer_buttons(wParam),
				};

				PointerEvent* event = make_event<PointerEvent>();
				event->pointer = pointer;
				event->action = PointerAction::Move;
				push_windows_window_event(window, event, points[i].time);
			}
			window->last_wmmousemove_time = GetMessageTime();
		}
		break;

		case WM_LBUTTONDOWN: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Down, POINTER_BUTTON_MOUSE_LEFT); break;
		case WM_RBUTTONDOWN: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Down, POINTER_BUTTON_MOUSE_RIGHT); break;
		case WM_MBUTTONDOWN: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Down, POINTER_BUTTON_MOUSE_MIDDLE); break;
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Up, POINTER_BUTTON_MOUSE_LEFT); break;
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Up, POINTER_BUTTON_MOUSE_RIGHT); break;
		case WM_MBUTTONUP:
		case WM_NCMBUTTONUP: push_windows_mouse_button_event(window, wParam, lParam, PointerAction::Up, POINTER_BUTTON_MOUSE_MIDDLE); break;

		case WM_DESTROY:
		case WM_CLOSE: {
			auto event = make_event<WindowCloseEvent>();
			push_windows_window_event(window, event);
		}
		break;
	}

	return DefWindowProcW(h, m, wParam, lParam);
}

// @TODO: Add a check that makes sure that this function is called from the same thread that created the window, because Windows requires that.
//   And the check must be present on all OSs (not Windows only) for consistency.
Array<Event*> os_read_window_events(WindowsWindow* window) {
	ScopedRestore(window->wnd_proc);
	window->wnd_proc = read_event_wnd_proc;

	MSG msg;
	while (PeekMessageW(&msg, window->hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	auto result = window->events;
	window->events = { .allocator = result.allocator };
	return result;
}
