#pragma once

#include "../grd_build.h"
#include "grd_window_base.h"
#include "grd_event.h"
#include "grd_win_key.h"
#include "grd_pointer.h"
#include "grd_win_message_codes.h"
#include "../grd_log.h"

#include <windowsx.h>

GRD_BUILD_RUN("ctx.params.add_lib('user32.lib')");


struct GrdWindowsWindow: GrdWindow {
	GRD_WIN_HWND           hwnd;
	wchar_t*               utf16_title = NULL;
	GrdArray<GrdEvent*>    events;
	char16_t               wm_char_high_surrogate = 0;
	GRD_WIN_LRESULT      (*wnd_proc)(GrdWindowsWindow* window, GRD_WIN_HWND h, GRD_WIN_UINT m, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam) = NULL;
	GrdOsPointerIdMapper   os_pointer_mapper;
	GRD_WIN_LONG           last_wmmousemove_time = 0; 
};

GRD_DEDUP GrdSpinlock GRD_WINDOWS_WINDOW_CREATE_LOCK;
GRD_DEDUP bool        GRD_WINDOWS_IS_WINDOW_CLASS_CREATED = false;
//  Windows sends WM_GETMINMAXINFO event before WM_NCCREATE, which is a problem.
// 
//  It means we can't read Window* pointer from GWLP_USERDATA in WM_GETMINMAXINFO,
//  because we set GWLP_USERDATA in WM_NCCREATE which happens later,
//  so the solution is to have a global lock with global GrdWindowsWindow* pointer,
//  from which we can read max/min size in WM_GETMINMAXINFO while window is being created.
GRD_DEDUP GrdWindowsWindow* GRD_WINDOWS_GETMINMAXINFO_WINDOW_POINTER = NULL;

GRD_DEDUP GRD_WIN_LRESULT grd_wnd_proc(GRD_WIN_HWND h, GRD_WIN_UINT m, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam);

GRD_DEDUP const wchar_t* grd_windows_get_window_class() {
	// This function assumes you have taken |GRD_WINDOWS_WINDOW_CREATE_LOCK|.
	auto class_name = L"GRD_WINDOW_CLASS";
	if (!GRD_WINDOWS_IS_WINDOW_CLASS_CREATED) {
		GRD_WINDOWS_IS_WINDOW_CLASS_CREATED = true;
		GRD_WIN_WNDCLASSW wc = { 
			.lpfnWndProc = grd_wnd_proc,
			.hInstance = GetModuleHandleW(NULL),
			.lpszClassName = class_name,
		};
		RegisterClassW(&wc);
	}
	return class_name;
}

GRD_DEDUP GrdTuple<GrdWindowsWindow*, GrdError*> grd_os_create_window(GrdWindowParams params) {
	GrdScopedLock(GRD_WINDOWS_WINDOW_CREATE_LOCK);

	auto window = grd_make<GrdWindowsWindow>();
	grd_init_window(window, params, grd_reflect_type_of<GrdWindowsWindow>());

	GRD_WINDOWS_GETMINMAXINFO_WINDOW_POINTER = window;

	s32 window_width  = GetSystemMetrics(GRD_WIN_SM_CXSCREEN) / 2 - params.initial_size.x / 2;
	s32 window_height = GetSystemMetrics(GRD_WIN_SM_CYSCREEN) / 2 - params.initial_size.y / 2;
	GRD_WIN_RECT window_rect = { window_width, window_height, window_width + (s32) params.initial_size.x, window_height + (s32) params.initial_size.y };
	GRD_WIN_DWORD window_style = GRD_WIN_WS_CAPTION | GRD_WIN_WS_SYSMENU | GRD_WIN_WS_THICKFRAME | GRD_WIN_WS_MINIMIZEBOX | GRD_WIN_WS_MAXIMIZEBOX;
	AdjustWindowRect(&window_rect, window_style, false);

	window->utf16_title = (wchar_t*) grd_encode_utf16(params.title).data;
	window->hwnd = CreateWindowExW(
		GRD_WIN_WS_EX_APPWINDOW,
		grd_windows_get_window_class(),
		window->utf16_title,
		window_style,
		window_rect.left,
		window_rect.top,
		window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top,
		NULL,   
		NULL,
		GetModuleHandleA(NULL),
		window
	);

	if (window->hwnd == NULL) {
		GrdFree(window->utf16_title);
		GrdFree(window);
		return { NULL, grd_windows_error() };
	}

	ShowWindow(window->hwnd, GRD_WIN_SW_SHOWDEFAULT);
	SetForegroundWindow(window->hwnd);
	return { window, NULL };
}

GRD_DEDUP GRD_WIN_LRESULT grd_wnd_proc(GRD_WIN_HWND h, GRD_WIN_UINT m, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam) {
	auto window = (GrdWindowsWindow*) GetWindowLongPtrW(h, GRD_WIN_GWLP_USERDATA);

	switch (m) {
		case GRD_WIN_WM_NCHITTEST: {
			if (!window->params.borderless) {
				return DefWindowProcW(h, m, wParam, lParam);
			}

			GRD_WIN_POINT ptMouse = { GRD_WIN_GET_X_LPARAM(lParam), GRD_WIN_GET_Y_LPARAM(lParam) };
			GRD_WIN_POINT ptMouseWindowLocal = ptMouse;
			ScreenToClient(h, &ptMouseWindowLocal);

			int LEFTEXTENDWIDTH = 8;
			int RIGHTEXTENDWIDTH = 8;
			int BOTTOMEXTENDWIDTH = 8;
			int TOPEXTENDWIDTH = 8; // @TODO: is this correct??

			if (GRD_WIN_IS_MAXIMIZED(h)) {
				TOPEXTENDWIDTH += 8;
			}

			// Get the window rectangle.
			GRD_WIN_RECT rcWindow;
			GetWindowRect(h, &rcWindow);

			// Determine if the hit test is for resizing. Default middle (1,1).
			u16  row = 1;
			u16  column = 1;
			bool on_top_resize_border = false;

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
					return GRD_WIN_HTCLIENT;
				}
				if (column != 1) {
					return GRD_WIN_HTCLIENT;
				}
				return GRD_WIN_HTCAPTION;
			}

			GRD_WIN_LRESULT hit_tests[3][3] = {
				{ GRD_WIN_HTTOPLEFT,    on_top_resize_border ? GRD_WIN_HTTOP : GRD_WIN_HTCAPTION, GRD_WIN_HTTOPRIGHT },
				{ GRD_WIN_HTLEFT,       GRD_WIN_HTCLIENT, GRD_WIN_HTRIGHT },
				{ GRD_WIN_HTBOTTOMLEFT, GRD_WIN_HTBOTTOM, GRD_WIN_HTBOTTOMRIGHT },
			};
			return hit_tests[row][column];
		}
		break;

		case GRD_WIN_WM_NCCREATE: {
			auto* createstruct = (GRD_WIN_CREATESTRUCTW*) lParam;
			SetWindowLongPtrW(h, GRD_WIN_GWLP_USERDATA, (GRD_WIN_LONG_PTR) createstruct->lpCreateParams);
			return DefWindowProcW(h, m, wParam, lParam);
		}
		break;
		
		case GRD_WIN_WM_NCCALCSIZE: {
			if (window->params.borderless) {
				if (IsMaximized(h)) {
					auto* pncsp = reinterpret_cast<GRD_WIN_NCCALCSIZE_PARAMS*>(lParam);
					union {
						GRD_WIN_LPARAM lparam;
						GRD_WIN_RECT* rect;
					} params = { .lparam = lParam };
					GRD_WIN_RECT nonclient = *params.rect;
					DefWindowProcW(h, GRD_WIN_WM_NCCALCSIZE, wParam, lParam);
					GRD_WIN_RECT client = *params.rect;
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
					if (wParam == 1) {
						auto* params = (GRD_WIN_NCCALCSIZE_PARAMS*)lParam;
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
		case GRD_WIN_WM_GETMINMAXINFO: {
			window = GRD_WINDOWS_GETMINMAXINFO_WINDOW_POINTER;
			auto* lpMMI = (GRD_WIN_MINMAXINFO*) lParam;
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

GRD_DEDUP f64 grd_get_windows_time_from_start(GRD_WIN_DWORD message_time) {
	u64 time = GetTickCount64();
	// Adjust the lower bits.
	*((u32*) &time) = message_time;
	return ((f64) time) / 1000.0;
}

GRD_DEDUP void grd_push_windows_window_event(GrdWindowsWindow* window, GrdWindowEvent* event, GrdOptional<GRD_WIN_DWORD> custom_time = {}) {
	// GetMessageTime() is like GetTickCount(), but casted to LONG instead of DWORD.
	auto time = custom_time.has_value ? custom_time.value : grd_bitcast<GRD_WIN_DWORD>(GetMessageTime());
	event->time_from_system_start = grd_get_windows_time_from_start(time);
	grd_add(&window->events, event);
}

GRD_DEDUP GrdPointerButtonFlags grd_wparam_to_pointer_buttons(GRD_WIN_WPARAM wParam) {
	u64 buttons = 0;
	if (wParam & GRD_WIN_MK_LBUTTON)  buttons |= 1 << 0;
	if (wParam & GRD_WIN_MK_RBUTTON)  buttons |= 1 << 1;
	if (wParam & GRD_WIN_MK_MBUTTON)  buttons |= 1 << 2;
	if (wParam & GRD_WIN_MK_XBUTTON1) buttons |= 1 << 3;
	if (wParam & GRD_WIN_MK_XBUTTON2) buttons |= 1 << 4;
	return (GrdPointerButtonFlags) buttons;
}

GRD_DEDUP GrdVector2 grd_convert_windows_coords(GRD_WIN_HWND hwnd, GRD_WIN_POINT p) {
	GRD_WIN_RECT window_rect;
	GetWindowRect(hwnd, &window_rect);
	return GrdVector2 { (f64) p.x, (f64) (window_rect.bottom - p.y) };
}

GRD_DEDUP void grd_push_windows_mouse_button_event(GrdWindowsWindow* window, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam, GrdPointerAction action, GrdPointerButtonFlags button) {
	GRD_WIN_POINT p = { GRD_WIN_GET_X_LPARAM(lParam), GRD_WIN_GET_Y_LPARAM(lParam) };
	GrdPointer pointer = {
		.id       = grd_map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
		.kind     = GrdPointerKind::Mouse,
		.position = grd_convert_windows_coords(window->hwnd, p),
		.buttons  = grd_wparam_to_pointer_buttons(wParam),
	};
	GrdPointerEvent* event = grd_make_event<GrdPointerEvent>();
	event->pointer = pointer;
	event->action = action;
	event->changed_buttons = button;
	grd_push_windows_window_event(window, event);
}

GRD_DEDUP GRD_WIN_LRESULT grd_read_event_wnd_proc(GrdWindowsWindow* window, GRD_WIN_HWND h, GRD_WIN_UINT m, GRD_WIN_WPARAM wParam, GRD_WIN_LPARAM lParam) {
	switch (m) {
		case GRD_WIN_WM_ACTIVATE: {
			auto event = grd_make_event<GrdFocusEvent>();
			event->have_focus = GRD_WIN_LOWORD(wParam) != 0;
			grd_push_windows_window_event(window, event);
		}
		break;

		case GRD_WIN_WM_CHAR: {
			if (wParam < 32) {
				return true;
			}

			switch (wParam) {
				case GRD_WIN_VK_BACK:
				case GRD_WIN_VK_TAB:
				case GRD_WIN_VK_RETURN:
				case GRD_WIN_VK_ESCAPE:
				case 127: // Ctrl + backspace
					return true;
			}

			if (GRD_WIN_IS_HIGH_SURROGATE(wParam)) {
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
			auto event = grd_make_event<GrdCharEvent>();
			event->character = c;
			grd_push_windows_window_event(window, event);

			window->wm_char_high_surrogate = 0;
		}
		break;

		case GRD_WIN_WM_KEYUP:
		case GRD_WIN_WM_SYSKEYUP:
		case GRD_WIN_WM_KEYDOWN:
		case GRD_WIN_WM_SYSKEYDOWN: {
			auto event = grd_make_event<GrdKeyEvent>();
			event->action = (m == GRD_WIN_WM_KEYDOWN || m == GRD_WIN_WM_SYSKEYDOWN) ? GrdKeyAction::Down : GrdKeyAction::Up;
			event->key = grd_map_windows_key(wParam, lParam);
			event->os_key_code = wParam;
			grd_push_windows_window_event(window, event);
		}
		break;

		case GRD_WIN_WM_MOUSEWHEEL: {
			s32 int_delta   =     -GRD_WIN_GET_WHEEL_DELTA_WPARAM(wParam)  /     GRD_WIN_WHEEL_DELTA;
			f32 float_delta = f32(-GRD_WIN_GET_WHEEL_DELTA_WPARAM(wParam)) / f32(GRD_WIN_WHEEL_DELTA);

			// WM_MOUSEWHEEL has screen relative coords, unlike WM_LBUTTONDOWN for example, which has client relative coords.
			GRD_WIN_POINT p = { .x = GRD_WIN_GET_X_LPARAM(lParam), .y = GRD_WIN_GET_Y_LPARAM(lParam) };
			ScreenToClient(window->hwnd, &p);

			GrdPointer pointer = {
				.id = grd_map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
				.kind = GrdPointerKind::Mouse,
				.position = grd_convert_windows_coords(window->hwnd, p),
				.buttons = grd_wparam_to_pointer_buttons(wParam),
			};

			GrdPointerEvent* event = grd_make_event<GrdPointerEvent>();
			event->pointer = pointer;
			event->action = GrdPointerAction::Scroll;
			event->scroll_delta = { 0, float_delta };
			event->is_scroll_in_lines = true;
			grd_push_windows_window_event(window, event);
		}
		break;

		case GRD_WIN_WM_MOUSEMOVE: {
			// https://developer.blender.org/rBd5c59913de95b6b6952088f175a8393bef376d27
			// This link helped to correct the initial code.
			
			GRD_WIN_POINT p = { .x = GRD_WIN_GET_X_LPARAM(lParam), .y = GRD_WIN_GET_Y_LPARAM(lParam) };
			ClientToScreen(window->hwnd, &p);

			GRD_WIN_MOUSEMOVEPOINT current_point = {
				.x    = p.x,
				.y    = p.y,
				.time = grd_bitcast<GRD_WIN_DWORD>(GetMessageTime()),
			};

			GRD_WIN_MOUSEMOVEPOINT points[65]; // 64 + 1 (+1 for the current_point)

			int count = GetMouseMovePointsEx(sizeof(GRD_WIN_MOUSEMOVEPOINT), &current_point, points, 64, GRD_WIN_GMMP_USE_DISPLAY_POINTS);
			int i = 0;
			while (i < count) {
				if (points[i].x > 32767) points[i].x -= 65536;
				if (points[i].y > 32767) points[i].y -= 65536;

				auto point_time = grd_get_windows_time_from_start(points[i].time);
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
				GRD_WIN_POINT p = { .x = points[i].x, .y = points[i].y };
				ScreenToClient(window->hwnd, &p);
				GrdPointer pointer = {
					.id       = grd_map_os_mouse_to_pointer_id(&window->os_pointer_mapper),
					.kind     = GrdPointerKind::Mouse,
					.position = grd_convert_windows_coords(window->hwnd, p),
					.buttons  = grd_wparam_to_pointer_buttons(wParam),
				};

				GrdPointerEvent* event = grd_make_event<GrdPointerEvent>();
				event->pointer = pointer;
				event->action = GrdPointerAction::Move;
				grd_push_windows_window_event(window, event, points[i].time);
			}
			window->last_wmmousemove_time = GetMessageTime();
		}
		break;

		case GRD_WIN_WM_LBUTTONDOWN:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Down, GRD_POINTER_BUTTON_MOUSE_LEFT);
			break;
		case GRD_WIN_WM_RBUTTONDOWN:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Down, GRD_POINTER_BUTTON_MOUSE_RIGHT);
			break;
		case GRD_WIN_WM_MBUTTONDOWN:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Down, GRD_POINTER_BUTTON_MOUSE_MIDDLE);
			break;
		case GRD_WIN_WM_LBUTTONUP:
		case GRD_WIN_WM_NCLBUTTONUP:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Up, GRD_POINTER_BUTTON_MOUSE_LEFT);
			break;
		case GRD_WIN_WM_RBUTTONUP:
		case GRD_WIN_WM_NCRBUTTONUP:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Up, GRD_POINTER_BUTTON_MOUSE_RIGHT);
			break;
		case GRD_WIN_WM_MBUTTONUP:
		case GRD_WIN_WM_NCMBUTTONUP:
			grd_push_windows_mouse_button_event(window, wParam, lParam, GrdPointerAction::Up, GRD_POINTER_BUTTON_MOUSE_MIDDLE);
			break;

		case GRD_WIN_WM_DESTROY:
		case GRD_WIN_WM_CLOSE: {
			auto event = grd_make_event<GrdWindowCloseEvent>();
			grd_push_windows_window_event(window, event);
		}
		break;
	}

	return DefWindowProcW(h, m, wParam, lParam);
}

GRD_DEDUP GrdArray<GrdEvent*> grd_os_read_window_events(GrdWindow* window) {
	auto x = grd_reflect_cast<GrdWindowsWindow>(window);
	if (!x) {
		return {};
	}
	GrdScopedRestore(x->wnd_proc);
	x->wnd_proc = grd_read_event_wnd_proc;

	GRD_WIN_MSG msg;
	while (PeekMessageW(&msg, x->hwnd, 0, 0, GRD_WIN_PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	auto result = x->events;
	x->events = { .allocator = result.allocator };
	return result;
}

GRD_DEF grd_os_close_window(GrdWindow* window) {
	auto x = grd_reflect_cast<GrdWindowsWindow>(window);
	if (!x) {
		return;
	}
	for (auto event: x->events) {
		event->free();
	}
	x->os_pointer_mapper.free();
	DestroyWindow(x->hwnd);
}
