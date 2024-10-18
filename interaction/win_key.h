#pragma once

#include "key.h"

GRD_BUILD_RUN("params.add_lib('user32.lib')");

// stolen from: https://stackoverflow.com/questions/15966642/how-do-you-tell-lshift-apart-from-rshift-in-wm-keydown-events
WPARAM map_windows_left_and_right_keys(WPARAM wParam, LPARAM lParam) {
	UINT   scancode = (lParam & 0x00ff0000) >> 16;
	int    extended = (lParam & 0x01000000) != 0;
	switch (wParam) {
		case VK_SHIFT:   return MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX);
		case VK_CONTROL: return extended ? VK_RCONTROL : VK_LCONTROL;
		case VK_MENU:    return extended ? VK_RMENU : VK_LMENU;
	}
	return wParam;
}

Key map_windows_key(WPARAM wParam, LPARAM lParam) {
	wParam = map_windows_left_and_right_keys(wParam, lParam);

	#define KEY_BINDING(from, to) case from: return Key:: to;
	switch (wParam) {
		KEY_BINDING(VK_UP,    Up_Arrow);
		KEY_BINDING(VK_DOWN,  Down_Arrow);
		KEY_BINDING(VK_LEFT,  Left_Arrow);
		KEY_BINDING(VK_RIGHT, Right_Arrow);

	#if 0
		KEY_BINDING(VK_SHIFT,    Any_Shift);
		KEY_BINDING(VK_CONTROL,  Any_Control);
		KEY_BINDING(VK_MENU,     Any_Alt);
	#endif

		KEY_BINDING(VK_LSHIFT, Left_Shift);
		KEY_BINDING(VK_RSHIFT, Right_Shift);

		KEY_BINDING(VK_LMENU, Left_Alt);
		KEY_BINDING(VK_RMENU, Right_Alt);

		KEY_BINDING(VK_LCONTROL, Left_Control);
		KEY_BINDING(VK_RCONTROL, Right_Control);

		KEY_BINDING(VK_BACK,   Backspace);
		KEY_BINDING(VK_DELETE, Delete);
		KEY_BINDING(VK_INSERT, Insert);
		KEY_BINDING(VK_RETURN, Enter);
		KEY_BINDING(VK_ESCAPE, Escape);
		KEY_BINDING(VK_TAB,    Tab);
		KEY_BINDING(VK_SPACE,  Space);

		KEY_BINDING(VK_LBUTTON, LMB);
		KEY_BINDING(VK_RBUTTON, RMB);
		KEY_BINDING(VK_MBUTTON, MMB);
		KEY_BINDING(VK_XBUTTON1, Mouse_4);
		KEY_BINDING(VK_XBUTTON2, Mouse_5);

		KEY_BINDING(VK_F1,  F1);
		KEY_BINDING(VK_F2,  F2);
		KEY_BINDING(VK_F3,  F3);
		KEY_BINDING(VK_F4,  F4);
		KEY_BINDING(VK_F5,  F5);
		KEY_BINDING(VK_F6,  F6);
		KEY_BINDING(VK_F7,  F7);
		KEY_BINDING(VK_F8,  F8);
		KEY_BINDING(VK_F9,  F9);
		KEY_BINDING(VK_F10, F10);
		KEY_BINDING(VK_F11, F11);
		KEY_BINDING(VK_F12, F12);
		KEY_BINDING(VK_F13, F13);
		KEY_BINDING(VK_F14, F14);
		KEY_BINDING(VK_F15, F15);
		KEY_BINDING(VK_F16, F16);
		KEY_BINDING(VK_F17, F17);
		KEY_BINDING(VK_F18, F18);
		KEY_BINDING(VK_F19, F19);
		KEY_BINDING(VK_F20, F20);
		KEY_BINDING(VK_F21, F21);
		KEY_BINDING(VK_F22, F22);
		KEY_BINDING(VK_F23, F23);
		KEY_BINDING(VK_F24, F24);
		
		KEY_BINDING(VK_PRIOR, Page_Up);
		KEY_BINDING(VK_NEXT,  Page_Down);

		KEY_BINDING(VK_ADD,      Numpad_Plus);
		KEY_BINDING(VK_SUBTRACT, Numpad_Minus);

		KEY_BINDING(VK_OEM_PLUS,  Plus);
		KEY_BINDING(VK_OEM_MINUS, Minus);

		KEY_BINDING(VK_CANCEL, Ctrl_Break);
		KEY_BINDING(VK_CLEAR, Clear);
		KEY_BINDING(VK_PAUSE, Pause);
		KEY_BINDING(VK_CAPITAL, Caps_Lock);
		
		KEY_BINDING(VK_KANA,   Kana_Or_Hangul);

		KEY_BINDING(VK_JUNJA, Junja);
		KEY_BINDING(VK_FINAL, Final);
		KEY_BINDING(VK_HANJA, Hanja_Or_Kanji);

		KEY_BINDING(VK_CONVERT,    Convert);
		KEY_BINDING(VK_NONCONVERT, Non_Convert);
		KEY_BINDING(VK_ACCEPT,     Accept);
		KEY_BINDING(VK_MODECHANGE, Mode_Change);

		KEY_BINDING(VK_END, End);
		KEY_BINDING(VK_HOME, Home);

		KEY_BINDING(VK_SELECT, Select);
		KEY_BINDING(VK_PRINT, Print);
		KEY_BINDING(VK_EXECUTE, Execute);
		KEY_BINDING(VK_SNAPSHOT, Print_Screen);
		KEY_BINDING(VK_HELP, Help);

		KEY_BINDING(VK_LWIN, Left_Windows);
		KEY_BINDING(VK_RWIN, Right_Windows);
		KEY_BINDING(VK_APPS, Menu);

		KEY_BINDING(VK_SLEEP, Sleep);

		KEY_BINDING(VK_NUMPAD0, Numpad_0);
		KEY_BINDING(VK_NUMPAD1, Numpad_1);
		KEY_BINDING(VK_NUMPAD2, Numpad_2);
		KEY_BINDING(VK_NUMPAD3, Numpad_3);
		KEY_BINDING(VK_NUMPAD4, Numpad_4);
		KEY_BINDING(VK_NUMPAD5, Numpad_5);
		KEY_BINDING(VK_NUMPAD6, Numpad_6);
		KEY_BINDING(VK_NUMPAD7, Numpad_7);
		KEY_BINDING(VK_NUMPAD8, Numpad_8);
		KEY_BINDING(VK_NUMPAD9, Numpad_9);

		KEY_BINDING(VK_MULTIPLY,  Numpad_Multiply);
		KEY_BINDING(VK_SEPARATOR, Separator);
		KEY_BINDING(VK_DECIMAL,   Numpad_Dot);
		KEY_BINDING(VK_DIVIDE,    Numpad_Slash);

		KEY_BINDING(VK_NUMLOCK,  Numlock);
		KEY_BINDING(VK_SCROLL,   Scroll_Lock);

		KEY_BINDING(VK_OEM_NEC_EQUAL, Numpad_Equal);

		// KEY_BINDING(VK_OEM_FJ_JISHO,   Dictionary);
		KEY_BINDING(VK_OEM_FJ_MASSHOU, Unregister_Word);
		KEY_BINDING(VK_OEM_FJ_TOUROKU, Register_Word);
		KEY_BINDING(VK_OEM_FJ_LOYA,    Left_OYAYUBI);
		KEY_BINDING(VK_OEM_FJ_ROYA,    Right_OYAYUBI);

		KEY_BINDING(VK_BROWSER_BACK,      Browser_Back);
		KEY_BINDING(VK_BROWSER_FORWARD,   Browser_Forward);
		KEY_BINDING(VK_BROWSER_REFRESH,   Browser_Refresh);
		KEY_BINDING(VK_BROWSER_STOP,      Browser_Stop);
		KEY_BINDING(VK_BROWSER_SEARCH,    Browser_Search);
		KEY_BINDING(VK_BROWSER_FAVORITES, Browser_Favorites);
		KEY_BINDING(VK_BROWSER_HOME,      Browser_Home);

		KEY_BINDING(VK_VOLUME_MUTE, Volume_Mute);
		KEY_BINDING(VK_VOLUME_DOWN, Volume_Down);
		KEY_BINDING(VK_VOLUME_UP,   Volume_Up);
		KEY_BINDING(VK_MEDIA_NEXT_TRACK, Media_Next_Track);
		KEY_BINDING(VK_MEDIA_PREV_TRACK, Media_Previous_Track);
		KEY_BINDING(VK_MEDIA_STOP,       Media_Stop);
		KEY_BINDING(VK_MEDIA_PLAY_PAUSE, Media_Play_Pause);
		KEY_BINDING(VK_LAUNCH_MAIL,         Launch_Mail);
		KEY_BINDING(VK_LAUNCH_MEDIA_SELECT, Launch_Media_Select);
		KEY_BINDING(VK_LAUNCH_APP1,         Launch_App1);
		KEY_BINDING(VK_LAUNCH_APP2,         Launch_App2);

		KEY_BINDING(VK_OEM_1,      Semicolon);
		KEY_BINDING(VK_OEM_COMMA,  Comma);
		KEY_BINDING(VK_OEM_PERIOD, Dot);
		KEY_BINDING(VK_OEM_2,      Slash);
		KEY_BINDING(VK_OEM_3,      Tilde);

		KEY_BINDING(VK_OEM_4, Left_Square_Bracket);
		KEY_BINDING(VK_OEM_5, Backslash);
		KEY_BINDING(VK_OEM_6, Right_Square_Bracket);
		KEY_BINDING(VK_OEM_7, Quote);
		// KEY_BINDING(VK_OEM_8, @TODO XXXX ignore this key ??);

		KEY_BINDING(VK_OEM_AX, AX);
		// KEY_BINDING(VK_OEM_102, @TODO XXX ); "<>" or "\|" on RT 102-key kbd.
		KEY_BINDING(VK_ICO_HELP, Help);
		KEY_BINDING(VK_ICO_00, Zero_Zero);

		KEY_BINDING(VK_PROCESSKEY, Process);
		
		KEY_BINDING(VK_ICO_CLEAR, Clear);

		KEY_BINDING(VK_ATTN,  Attn);
		KEY_BINDING(VK_CRSEL, CrSel);
		KEY_BINDING(VK_EXSEL, ExSel);
		KEY_BINDING(VK_EREOF, Erase_EOF);

		KEY_BINDING(VK_PLAY, Play);
		KEY_BINDING(VK_ZOOM, Zoom);

		KEY_BINDING(VK_OEM_CLEAR, Clear);

		// Character keys will be bound here
		default: 
			return (Key) wParam;
	}
	#undef KEY_BINDING
}
