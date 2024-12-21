#pragma once

#include "../grd_reflect.h"
#include "../grd_optional.h"

// Keys should be serialized by names.
enum class GrdKey: u32 {
	Unknown = 0,
	Number0 = '0',
	Number1 = '1',
	Number2 = '2',
	Number3 = '3',
	Number4 = '4',
	Number5 = '5',
	Number6 = '6',
	Number7 = '7',
	Number8 = '8',
	Number9 = '9',
	A = 'A',
	B = 'B',
	C = 'C',
	D = 'D',
	E = 'E',
	F = 'F',
	G = 'G',
	H = 'H',
	I = 'I',
	J = 'J',
	K = 'K',
	L = 'L',
	M = 'M',
	N = 'N',
	O = 'O',
	P = 'P',
	Q = 'Q',
	R = 'R',
	S = 'S',
	T = 'T',
	U = 'U',
	V = 'V',
	W = 'W',
	X = 'X',
	Y = 'Y',
	Z = 'Z',

	Semicolon = ';',
	Comma = ',',
	Dot = '.',
	Slash = '/',
	Tilde = '~',

	LeftSquareBracket = '[',
	RightSquareBracket = ']',
	Backslash = '\\',
	Quote = '\'',

	ASCII_RANGE_START = 0xff,

	MouseWheelUp,
	MouseWheelDown,	

	UpArrow,
	DownArrow,
	LeftArrow,
	RightArrow,

	AnyShift,
	AnyControl,
	AnyAlt,

	LeftShift,
	RightShift,
	LeftControl,
	RightControl,
	LeftAlt,
	RightAlt,

	Option,
	Fn,

	Backspace,
	Delete,
	ForwardDelete,

	Insert,
	Enter,
	Escape,
	Tab,
	Space,

	LMB,
	RMB,
	MMB,

	Mouse4,
	Mouse5,
	Mouse6,
	Mouse7,
	Mouse8,
	Mouse9,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,

	PageUp,
	PageDown,

	Plus,
	Minus,

	CtrlBreak,
	Clear,
	Pause,
	CapsLock, // Name: Caps Lock

	KanaOrHangul, // @TODO: Distinguish using OS Key name api.

	Junja,
	Final,
	HanjaOrKanji,

	Convert,
	NonConvert, // Name: Don't convert
	Accept,
	ModeChange, // Name: Mode change

	End,
	Home,

	Select,
	Print,
	Execute,
	PrintScreen, // Name: Print Screen
	Help,

	LeftWindows,
	RightWindows,
	Command,
	Menu,

	Sleep,

	Numpad0,
	Numpad1,
	Numpad2,
	Numpad3,
	Numpad4,
	Numpad5,
	Numpad6,
	Numpad7,
	Numpad8,
	Numpad9,

	NumpadMultiply,
	Separator,
	NumpadSlash,

	NumpadDot,
	NumpadPlus,
	NumpadMinus,
	NumpadEnter, // @TODO: detect this on windows. https://microsoft.public.vc.mfc.narkive.com/4NhbZpGJ/numpad-enter-vs-keyboad-enter-different-virtual-key-codes
	NumpadEqual,

	Numlock,
	ScrollLock, // Name: Scroll Lock

	Equal,

	Dictionary,
	UnregisterWord,
	RegisterWord,
	LeftOYAYUBI,
	RightOYAYUBI,

	BrowserBack,
	BrowserForward,
	BrowserRefresh,
	BrowserStop,
	BrowserSearch,
	BrowserFavorites,
	BrowserHome,

	VolumeMute,
	VolumeDown,
	VolumeUp,
	MediaNextTrack,
	MediaPreviousTrack,
	MediaStop,
	MediaPlayPause,
	LaunchMail,
	LaunchMediaSelect,
	LaunchApp1,
	LaunchApp2,

	AX,
	ZeroZero,

	Process,

	Attn,
	CrSel,
	ExSel,
	EraseEOF,

	Zoom,
	Play,

/*
 * Virtual Keys, Standard Set
 * 0x07 : reserved
 * 0x0A - 0x0B : reserved
 * 0x0E - 0x0F : unassigned
 * 0x16 : unassigned
 * 0x1A : unassigned
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 * 0x5E : reserved
 * 0x88 - 0x8F : UI navigation
 * 0x97 - 0x9F : unassigned
 * 0xB8 - 0xB9 : reserved
 * 0xC1 - 0xC2 : reserved
 * 0xC3 - 0xDA : Gamepad input
// @TODO: do we support this ones?
#define VK_GAMEPAD_A                         0xC3 // reserved
#define VK_GAMEPAD_B                         0xC4 // reserved
#define VK_GAMEPAD_X                         0xC5 // reserved
#define VK_GAMEPAD_Y                         0xC6 // reserved
#define VK_GAMEPAD_RIGHT_SHOULDER            0xC7 // reserved
#define VK_GAMEPAD_LEFT_SHOULDER             0xC8 // reserved
#define VK_GAMEPAD_LEFT_TRIGGER              0xC9 // reserved
#define VK_GAMEPAD_RIGHT_TRIGGER             0xCA // reserved
#define VK_GAMEPAD_DPAD_UP                   0xCB // reserved
#define VK_GAMEPAD_DPAD_DOWN                 0xCC // reserved
#define VK_GAMEPAD_DPAD_LEFT                 0xCD // reserved
#define VK_GAMEPAD_DPAD_RIGHT                0xCE // reserved
#define VK_GAMEPAD_MENU                      0xCF // reserved
#define VK_GAMEPAD_VIEW                      0xD0 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA // reserved
#endif  _WIN32_WINNT >= 0x0604 

 * 0xE0 : reserved
 * Various extended or enhanced keyboards
 * 0xE8 : unassigned
 * Nokia/Ericsson definitions
 
 // Ignore this keys???.
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5
*/
};


// // @TODO: add missing keys.
// GRD_REFLECT(Key) {

// 	GRD_ENUM_VALUE(MouseWheelUp);
// 		GRD_TAG("Mouse Wheel Up"_b);
// 	GRD_ENUM_VALUE(MouseWheelDown);
// 		GRD_TAG("Mouse Wheel Down"_b);

// 	GRD_ENUM_VALUE(UpArrow);
// 		GRD_TAG("Up Arrow"_b);
// 	GRD_ENUM_VALUE(DownArrow);
// 		GRD_TAG("Down Arrow"_b);
// 	GRD_ENUM_VALUE(LeftArrow);
// 		GRD_TAG("Left Arrow"_b);
// 	GRD_ENUM_VALUE(RightArrow);
// 		GRD_TAG("Right Arrow"_b);

// 	GRD_ENUM_VALUE(AnyShift);
// 		GRD_TAG("Shift"_b);
// 	GRD_ENUM_VALUE(AnyControl);
// 		GRD_TAG("Ctrl"_b);
// 	GRD_ENUM_VALUE(AnyAlt);
// 		GRD_TAG("Alt"_b);

// 	GRD_ENUM_VALUE(LeftShift);
// 		GRD_TAG("Left Shift"_b);
// 	GRD_ENUM_VALUE(LeftControl);
// 		GRD_TAG("Left Ctrl"_b);
// 	GRD_ENUM_VALUE(LeftAlt);
// 		GRD_TAG("Left Alt"_b);

// 	GRD_ENUM_VALUE(RightShift);
// 		GRD_TAG("Right Shift"_b);
// 	GRD_ENUM_VALUE(RightControl);
// 		GRD_TAG("Right Ctrl"_b);
// 	GRD_ENUM_VALUE(RightAlt);
// 		GRD_TAG("Right Alt"_b);


// 	GRD_ENUM_VALUE(Option);
// 	GRD_ENUM_VALUE(Fn);

// 	GRD_ENUM_VALUE(Backspace);
// 	GRD_ENUM_VALUE(Delete);
// 	GRD_ENUM_VALUE(ForwardDelete);

// 	GRD_ENUM_VALUE(Insert);
// 	GRD_ENUM_VALUE(Enter);
// 	GRD_ENUM_VALUE(Escape);
// 	GRD_ENUM_VALUE(Tab);
// 	GRD_ENUM_VALUE(Space);

// 	GRD_ENUM_VALUE(LMB);
// 	GRD_ENUM_VALUE(RMB);
// 	GRD_ENUM_VALUE(MMB);

// 	GRD_ENUM_VALUE(Mouse4);
// 		GRD_TAG("Mouse 4"_b);
// 	GRD_ENUM_VALUE(Mouse5);
// 		GRD_TAG("Mouse 5"_b);
// 	GRD_ENUM_VALUE(Mouse6);
// 		GRD_TAG("Mouse 6"_b);
// 	GRD_ENUM_VALUE(Mouse7);
// 		GRD_TAG("Mouse 7"_b);
// 	GRD_ENUM_VALUE(Mouse8);
// 		GRD_TAG("Mouse 8"_b);
// 	GRD_ENUM_VALUE(Mouse9);
// 		GRD_TAG("Mouse 9"_b);

// 	GRD_ENUM_VALUE(F1);
// 	GRD_ENUM_VALUE(F2);
// 	GRD_ENUM_VALUE(F3);
// 	GRD_ENUM_VALUE(F4);
// 	GRD_ENUM_VALUE(F5);
// 	GRD_ENUM_VALUE(F6);
// 	GRD_ENUM_VALUE(F7);
// 	GRD_ENUM_VALUE(F8);
// 	GRD_ENUM_VALUE(F9);
// 	GRD_ENUM_VALUE(F10);
// 	GRD_ENUM_VALUE(F11);
// 	GRD_ENUM_VALUE(F12);
// 	GRD_ENUM_VALUE(F13);
// 	GRD_ENUM_VALUE(F14);
// 	GRD_ENUM_VALUE(F15);
// 	GRD_ENUM_VALUE(F16);
// 	GRD_ENUM_VALUE(F17);
// 	GRD_ENUM_VALUE(F18);
// 	GRD_ENUM_VALUE(F19);
// 	GRD_ENUM_VALUE(F20);
// 	GRD_ENUM_VALUE(F21);
// 	GRD_ENUM_VALUE(F22);
// 	GRD_ENUM_VALUE(F23);
// 	GRD_ENUM_VALUE(F24);

// 	GRD_ENUM_VALUE(PageUp);   GRD_TAG("Page Up"_b);
// 	GRD_ENUM_VALUE(PageDown); GRD_TAG("Page Down"_b);

// 	GRD_ENUM_VALUE(Number0); GRD_TAG("0"_b);
// 	GRD_ENUM_VALUE(Number1); GRD_TAG("1"_b);
// 	GRD_ENUM_VALUE(Number2); GRD_TAG("2"_b);
// 	GRD_ENUM_VALUE(Number3); GRD_TAG("3"_b);
// 	GRD_ENUM_VALUE(Number4); GRD_TAG("4"_b);
// 	GRD_ENUM_VALUE(Number5); GRD_TAG("5"_b);
// 	GRD_ENUM_VALUE(Number6); GRD_TAG("6"_b);
// 	GRD_ENUM_VALUE(Number7); GRD_TAG("7"_b);
// 	GRD_ENUM_VALUE(Number8); GRD_TAG("8"_b);
// 	GRD_ENUM_VALUE(Number9); GRD_TAG("9"_b);

// 	// Imagine I did a typo here..
// 	GRD_ENUM_VALUE(A); GRD_TAG("A"_b);
// 	GRD_ENUM_VALUE(B); GRD_TAG("B"_b);
// 	GRD_ENUM_VALUE(C); GRD_TAG("C"_b);
// 	GRD_ENUM_VALUE(D); GRD_TAG("D"_b);
// 	GRD_ENUM_VALUE(E); GRD_TAG("E"_b);
// 	GRD_ENUM_VALUE(F); GRD_TAG("F"_b);
// 	GRD_ENUM_VALUE(G); GRD_TAG("G"_b);
// 	GRD_ENUM_VALUE(H); GRD_TAG("H"_b);
// 	GRD_ENUM_VALUE(I); GRD_TAG("I"_b);
// 	GRD_ENUM_VALUE(K); GRD_TAG("K"_b);
// 	GRD_ENUM_VALUE(L); GRD_TAG("L"_b);
// 	GRD_ENUM_VALUE(M); GRD_TAG("M"_b);
// 	GRD_ENUM_VALUE(N); GRD_TAG("N"_b);
// 	GRD_ENUM_VALUE(O); GRD_TAG("O"_b);
// 	GRD_ENUM_VALUE(P); GRD_TAG("P"_b);
// 	GRD_ENUM_VALUE(Q); GRD_TAG("Q"_b);
// 	GRD_ENUM_VALUE(R); GRD_TAG("R"_b);
// 	GRD_ENUM_VALUE(S); GRD_TAG("S"_b);
// 	GRD_ENUM_VALUE(T); GRD_TAG("T"_b);
// 	GRD_ENUM_VALUE(V); GRD_TAG("V"_b);
// 	GRD_ENUM_VALUE(X); GRD_TAG("X"_b);
// 	GRD_ENUM_VALUE(Y); GRD_TAG("Y"_b);
// 	GRD_ENUM_VALUE(Z); GRD_TAG("Z"_b);

// 	GRD_ENUM_VALUE(Semicolon); GRD_TAG(";"_b);
// 	GRD_ENUM_VALUE(Comma);     GRD_TAG(","_b);
// 	GRD_ENUM_VALUE(Dot);       GRD_TAG("."_b);
// 	GRD_ENUM_VALUE(Slash);     GRD_TAG("/"_b);
// 	GRD_ENUM_VALUE(Tilde);     GRD_TAG("~"_b);

// 	GRD_ENUM_VALUE(LeftSquareBracket);  GRD_TAG("["_b);
// 	GRD_ENUM_VALUE(RightSquareBracket); GRD_TAG("]"_b);
// 	GRD_ENUM_VALUE(Backslash);            GRD_TAG("\\"_b);
// 	GRD_ENUM_VALUE(Quote);                GRD_TAG("\'"_b);


// 	GRD_ENUM_VALUE(Plus);  GRD_TAG("+"_b);
// 	GRD_ENUM_VALUE(Minus); GRD_TAG("-"_b);

// 	GRD_ENUM_VALUE(CtrlBreak); GRD_TAG("Ctrl+Break"_b);
// 	GRD_ENUM_VALUE(Clear);
// 	GRD_ENUM_VALUE(Pause);
// 	GRD_ENUM_VALUE(CapsLock); GRD_TAG("Caps Lock"_b);

// 	GRD_ENUM_VALUE(KanaOrHangul); // @TODO: Distinguish using OS Key name api.

// 	GRD_ENUM_VALUE(Junja);
// 	GRD_ENUM_VALUE(Final);
// 	GRD_ENUM_VALUE(HanjaOrKanji);  // @TODO: Distinguish using OS Key name api.

// 	GRD_ENUM_VALUE(Convert);
// 	GRD_ENUM_VALUE(NonConvert); GRD_TAG("Don't convert"_b);
// 	GRD_ENUM_VALUE(Accept);
// 	GRD_ENUM_VALUE(ModeChange); GRD_TAG("Mode change"_b);

// 	GRD_ENUM_VALUE(End);
// 	GRD_ENUM_VALUE(Home);

// 	GRD_ENUM_VALUE(Select);
// 	GRD_ENUM_VALUE(Print);
// 	GRD_ENUM_VALUE(Execute);
// 	GRD_ENUM_VALUE(PrintScreen); GRD_TAG("Print Screen"_b);
// 	GRD_ENUM_VALUE(Help);

// 	GRD_ENUM_VALUE(LeftWindows);  GRD_TAG("Left Windows"_b);
// 	GRD_ENUM_VALUE(RightWindows); GRD_TAG("Right Windows"_b);
// 	GRD_ENUM_VALUE(Menu);

// 	GRD_ENUM_VALUE(Sleep);

// 	GRD_ENUM_VALUE(Numpad0); GRD_TAG("Num 0"_b);
// 	GRD_ENUM_VALUE(Numpad1); GRD_TAG("Num 1"_b);
// 	GRD_ENUM_VALUE(Numpad2); GRD_TAG("Num 2"_b);
// 	GRD_ENUM_VALUE(Numpad3); GRD_TAG("Num 3"_b);
// 	GRD_ENUM_VALUE(Numpad4); GRD_TAG("Num 4"_b);
// 	GRD_ENUM_VALUE(Numpad5); GRD_TAG("Num 5"_b);
// 	GRD_ENUM_VALUE(Numpad6); GRD_TAG("Num 6"_b);
// 	GRD_ENUM_VALUE(Numpad7); GRD_TAG("Num 7"_b);
// 	GRD_ENUM_VALUE(Numpad8); GRD_TAG("Num 8"_b);
// 	GRD_ENUM_VALUE(Numpad9); GRD_TAG("Num 9"_b);

// 	GRD_ENUM_VALUE(NumpadMultiply); GRD_TAG("*"_b);
// 	GRD_ENUM_VALUE(Separator);
// 	GRD_ENUM_VALUE(NumpadSlash); GRD_TAG("Num /"_b);

// 	GRD_ENUM_VALUE(NumpadDot); GRD_TAG("Num ."_b);

// 	GRD_ENUM_VALUE(Numlock);     GRD_TAG("Num Lock"_b);
// 	GRD_ENUM_VALUE(ScrollLock); GRD_TAG("Scroll Lock"_b);

// 	GRD_ENUM_VALUE(Equal); GRD_TAG("="_b);

// 	GRD_ENUM_VALUE(Dictionary);
// 	GRD_ENUM_VALUE(UnregisterWord); GRD_TAG("Unregister Word"_b);
// 	GRD_ENUM_VALUE(RegisterWord);   GRD_TAG("Register Word"_b);
// 	GRD_ENUM_VALUE(LeftOYAYUBI);    GRD_TAG("Left OYAYUBI"_b);
// 	GRD_ENUM_VALUE(RightOYAYUBI);   GRD_TAG("Right OYAYUBI"_b);

// 	GRD_ENUM_VALUE(BrowserBack);     GRD_TAG("Browser Back"_b);
// 	GRD_ENUM_VALUE(BrowserForward);  GRD_TAG("Browser Forward"_b);
// 	GRD_ENUM_VALUE(BrowserRefresh);  GRD_TAG("Browser Refresh"_b);
// 	GRD_ENUM_VALUE(BrowserStop);     GRD_TAG("Browser Stop"_b);
// 	GRD_ENUM_VALUE(BrowserSearch);   GRD_TAG("Browser Search"_b);
// 	GRD_ENUM_VALUE(BrowserFavorites);GRD_TAG("Browser Favorites"_b);
// 	GRD_ENUM_VALUE(BrowserHome);     GRD_TAG("Browser Home"_b);

// 	GRD_ENUM_VALUE(VolumeMute);          GRD_TAG("Mute"_b);
// 	GRD_ENUM_VALUE(VolumeDown);          GRD_TAG("Volume Down"_b);
// 	GRD_ENUM_VALUE(VolumeUp);            GRD_TAG("Volume Up"_b);
// 	GRD_ENUM_VALUE(MediaNextTrack);     GRD_TAG("Next Track"_b);
// 	GRD_ENUM_VALUE(MediaPreviousTrack); GRD_TAG("Previous Track"_b);
// 	GRD_ENUM_VALUE(MediaStop);           GRD_TAG("Stop"_b);
// 	GRD_ENUM_VALUE(MediaPlayPause);     GRD_TAG("Play/Pause"_b);
// 	GRD_ENUM_VALUE(LaunchMail);          GRD_TAG("Mail"_b);
// 	GRD_ENUM_VALUE(LaunchMediaSelect);  GRD_TAG("Media select"_b);
// 	GRD_ENUM_VALUE(LaunchApp1);          GRD_TAG("App 1"_b);
// 	GRD_ENUM_VALUE(LaunchApp2);          GRD_TAG("App 2"_b);

// 	GRD_ENUM_VALUE(AX);
// 	GRD_ENUM_VALUE(ZeroZero); GRD_TAG("00"_b);

// 	GRD_ENUM_VALUE(Process);

// 	GRD_ENUM_VALUE(Attn);
// 	GRD_ENUM_VALUE(CrSel);
// 	GRD_ENUM_VALUE(ExSel);
// 	GRD_ENUM_VALUE(EraseEOF); GRD_TAG("Erase EOF"_b);

// 	GRD_ENUM_VALUE(Zoom);
// 	GRD_ENUM_VALUE(Play);
// }

GRD_DEDUP GrdOptional<GrdKey> grd_map_mouse_button_index(int index) {
	switch (index) {
		case 0:  return GrdKey::LMB;
		case 1:  return GrdKey::RMB;
		case 2:  return GrdKey::MMB;
		case 3:  return GrdKey::Mouse4;
		case 4:  return GrdKey::Mouse5;
		case 5:  return GrdKey::Mouse6;
		case 6:  return GrdKey::Mouse7;
		case 7:  return GrdKey::Mouse8;
		case 8:  return GrdKey::Mouse9;
	}
	return {};
}
