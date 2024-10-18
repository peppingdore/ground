#pragma once

#include "../reflect.h"
#include "../optional.h"

// Keys should be serialized by names.
enum class Key: u32 {
	Unknown = 0,
	Number_0 = '0',
	Number_1 = '1',
	Number_2 = '2',
	Number_3 = '3',
	Number_4 = '4',
	Number_5 = '5',
	Number_6 = '6',
	Number_7 = '7',
	Number_8 = '8',
	Number_9 = '9',
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

	Left_Square_Bracket = '[',
	Right_Square_Bracket = ']',
	Backslash = '\\',
	Quote = '\'',

	ASCII_RANGE_START = 0xff,

	Mouse_Wheel_Up,
	Mouse_Wheel_Down,	

	Up_Arrow,
	Down_Arrow,
	Left_Arrow,
	Right_Arrow,

	Any_Shift,
	Any_Control,
	Any_Alt,

	Left_Shift,
	Right_Shift,
	Left_Control,
	Right_Control,
	Left_Alt,
	Right_Alt,

	Option,
	Fn,

	Backspace,
	Delete,
	Forward_Delete,

	Insert,
	Enter,
	Escape,
	Tab,
	Space,

	LMB,
	RMB,
	MMB,

	Mouse_4,
	Mouse_5,
	Mouse_6,
	Mouse_7,
	Mouse_8,
	Mouse_9,

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

	Page_Up,
	Page_Down,

	Plus,
	Minus,

	Ctrl_Break,
	Clear,
	Pause,
	Caps_Lock, // Name: Caps Lock

	Kana_Or_Hangul, // @TODO: Distinguish using OS Key name api.

	Junja,
	Final,
	Hanja_Or_Kanji,

	Convert,
	Non_Convert, // Name: Don't convert
	Accept,
	Mode_Change, // Name: Mode change

	End,
	Home,

	Select,
	Print,
	Execute,
	Print_Screen, // Name: Print Screen
	Help,

	Left_Windows,
	Right_Windows,
	Command,
	Menu,

	Sleep,

	Numpad_0,
	Numpad_1,
	Numpad_2,
	Numpad_3,
	Numpad_4,
	Numpad_5,
	Numpad_6,
	Numpad_7,
	Numpad_8,
	Numpad_9,

	Numpad_Multiply,
	Separator,
	Numpad_Slash,

	Numpad_Dot,
	Numpad_Plus,
	Numpad_Minus,
	Numpad_Enter, // @TODO: detect this on windows. https://microsoft.public.vc.mfc.narkive.com/4NhbZpGJ/numpad-enter-vs-keyboad-enter-different-virtual-key-codes
	Numpad_Equal,

	Numlock,
	Scroll_Lock, // Name: Scroll Lock

	Equal,

	Dictionary,
	Unregister_Word,
	Register_Word,
	Left_OYAYUBI,
	Right_OYAYUBI,

	Browser_Back,
	Browser_Forward,
	Browser_Refresh,
	Browser_Stop,
	Browser_Search,
	Browser_Favorites,
	Browser_Home,

	Volume_Mute,
	Volume_Down,
	Volume_Up,
	Media_Next_Track,
	Media_Previous_Track,
	Media_Stop,
	Media_Play_Pause,
	Launch_Mail,
	Launch_Media_Select,
	Launch_App1,
	Launch_App2,

	AX,
	Zero_Zero,

	Process,

	Attn,
	CrSel,
	ExSel,
	Erase_EOF,

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

// 	GRD_ENUM_VALUE(Mouse_Wheel_Up);
// 		GRD_TAG("Mouse Wheel Up"_b);
// 	GRD_ENUM_VALUE(Mouse_Wheel_Down);
// 		GRD_TAG("Mouse Wheel Down"_b);

// 	GRD_ENUM_VALUE(Up_Arrow);
// 		GRD_TAG("Up Arrow"_b);
// 	GRD_ENUM_VALUE(Down_Arrow);
// 		GRD_TAG("Down Arrow"_b);
// 	GRD_ENUM_VALUE(Left_Arrow);
// 		GRD_TAG("Left Arrow"_b);
// 	GRD_ENUM_VALUE(Right_Arrow);
// 		GRD_TAG("Right Arrow"_b);

// 	GRD_ENUM_VALUE(Any_Shift);
// 		GRD_TAG("Shift"_b);
// 	GRD_ENUM_VALUE(Any_Control);
// 		GRD_TAG("Ctrl"_b);
// 	GRD_ENUM_VALUE(Any_Alt);
// 		GRD_TAG("Alt"_b);

// 	GRD_ENUM_VALUE(Left_Shift);
// 		GRD_TAG("Left Shift"_b);
// 	GRD_ENUM_VALUE(Left_Control);
// 		GRD_TAG("Left Ctrl"_b);
// 	GRD_ENUM_VALUE(Left_Alt);
// 		GRD_TAG("Left Alt"_b);

// 	GRD_ENUM_VALUE(Right_Shift);
// 		GRD_TAG("Right Shift"_b);
// 	GRD_ENUM_VALUE(Right_Control);
// 		GRD_TAG("Right Ctrl"_b);
// 	GRD_ENUM_VALUE(Right_Alt);
// 		GRD_TAG("Right Alt"_b);


// 	GRD_ENUM_VALUE(Option);
// 	GRD_ENUM_VALUE(Fn);

// 	GRD_ENUM_VALUE(Backspace);
// 	GRD_ENUM_VALUE(Delete);
// 	GRD_ENUM_VALUE(Forward_Delete);

// 	GRD_ENUM_VALUE(Insert);
// 	GRD_ENUM_VALUE(Enter);
// 	GRD_ENUM_VALUE(Escape);
// 	GRD_ENUM_VALUE(Tab);
// 	GRD_ENUM_VALUE(Space);

// 	GRD_ENUM_VALUE(LMB);
// 	GRD_ENUM_VALUE(RMB);
// 	GRD_ENUM_VALUE(MMB);

// 	GRD_ENUM_VALUE(Mouse_4);
// 		GRD_TAG("Mouse 4"_b);
// 	GRD_ENUM_VALUE(Mouse_5);
// 		GRD_TAG("Mouse 5"_b);
// 	GRD_ENUM_VALUE(Mouse_6);
// 		GRD_TAG("Mouse 6"_b);
// 	GRD_ENUM_VALUE(Mouse_7);
// 		GRD_TAG("Mouse 7"_b);
// 	GRD_ENUM_VALUE(Mouse_8);
// 		GRD_TAG("Mouse 8"_b);
// 	GRD_ENUM_VALUE(Mouse_9);
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

// 	GRD_ENUM_VALUE(Page_Up);   GRD_TAG("Page Up"_b);
// 	GRD_ENUM_VALUE(Page_Down); GRD_TAG("Page Down"_b);

// 	GRD_ENUM_VALUE(Number_0); GRD_TAG("0"_b);
// 	GRD_ENUM_VALUE(Number_1); GRD_TAG("1"_b);
// 	GRD_ENUM_VALUE(Number_2); GRD_TAG("2"_b);
// 	GRD_ENUM_VALUE(Number_3); GRD_TAG("3"_b);
// 	GRD_ENUM_VALUE(Number_4); GRD_TAG("4"_b);
// 	GRD_ENUM_VALUE(Number_5); GRD_TAG("5"_b);
// 	GRD_ENUM_VALUE(Number_6); GRD_TAG("6"_b);
// 	GRD_ENUM_VALUE(Number_7); GRD_TAG("7"_b);
// 	GRD_ENUM_VALUE(Number_8); GRD_TAG("8"_b);
// 	GRD_ENUM_VALUE(Number_9); GRD_TAG("9"_b);

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

// 	GRD_ENUM_VALUE(Left_Square_Bracket);  GRD_TAG("["_b);
// 	GRD_ENUM_VALUE(Right_Square_Bracket); GRD_TAG("]"_b);
// 	GRD_ENUM_VALUE(Backslash);            GRD_TAG("\\"_b);
// 	GRD_ENUM_VALUE(Quote);                GRD_TAG("\'"_b);


// 	GRD_ENUM_VALUE(Plus);  GRD_TAG("+"_b);
// 	GRD_ENUM_VALUE(Minus); GRD_TAG("-"_b);

// 	GRD_ENUM_VALUE(Ctrl_Break); GRD_TAG("Ctrl+Break"_b);
// 	GRD_ENUM_VALUE(Clear);
// 	GRD_ENUM_VALUE(Pause);
// 	GRD_ENUM_VALUE(Caps_Lock); GRD_TAG("Caps Lock"_b);

// 	GRD_ENUM_VALUE(Kana_Or_Hangul); // @TODO: Distinguish using OS Key name api.

// 	GRD_ENUM_VALUE(Junja);
// 	GRD_ENUM_VALUE(Final);
// 	GRD_ENUM_VALUE(Hanja_Or_Kanji);  // @TODO: Distinguish using OS Key name api.

// 	GRD_ENUM_VALUE(Convert);
// 	GRD_ENUM_VALUE(Non_Convert); GRD_TAG("Don't convert"_b);
// 	GRD_ENUM_VALUE(Accept);
// 	GRD_ENUM_VALUE(Mode_Change); GRD_TAG("Mode change"_b);

// 	GRD_ENUM_VALUE(End);
// 	GRD_ENUM_VALUE(Home);

// 	GRD_ENUM_VALUE(Select);
// 	GRD_ENUM_VALUE(Print);
// 	GRD_ENUM_VALUE(Execute);
// 	GRD_ENUM_VALUE(Print_Screen); GRD_TAG("Print Screen"_b);
// 	GRD_ENUM_VALUE(Help);

// 	GRD_ENUM_VALUE(Left_Windows);  GRD_TAG("Left Windows"_b);
// 	GRD_ENUM_VALUE(Right_Windows); GRD_TAG("Right Windows"_b);
// 	GRD_ENUM_VALUE(Menu);

// 	GRD_ENUM_VALUE(Sleep);

// 	GRD_ENUM_VALUE(Numpad_0); GRD_TAG("Num 0"_b);
// 	GRD_ENUM_VALUE(Numpad_1); GRD_TAG("Num 1"_b);
// 	GRD_ENUM_VALUE(Numpad_2); GRD_TAG("Num 2"_b);
// 	GRD_ENUM_VALUE(Numpad_3); GRD_TAG("Num 3"_b);
// 	GRD_ENUM_VALUE(Numpad_4); GRD_TAG("Num 4"_b);
// 	GRD_ENUM_VALUE(Numpad_5); GRD_TAG("Num 5"_b);
// 	GRD_ENUM_VALUE(Numpad_6); GRD_TAG("Num 6"_b);
// 	GRD_ENUM_VALUE(Numpad_7); GRD_TAG("Num 7"_b);
// 	GRD_ENUM_VALUE(Numpad_8); GRD_TAG("Num 8"_b);
// 	GRD_ENUM_VALUE(Numpad_9); GRD_TAG("Num 9"_b);

// 	GRD_ENUM_VALUE(Numpad_Multiply); GRD_TAG("*"_b);
// 	GRD_ENUM_VALUE(Separator);
// 	GRD_ENUM_VALUE(Numpad_Slash); GRD_TAG("Num /"_b);

// 	GRD_ENUM_VALUE(Numpad_Dot); GRD_TAG("Num ."_b);

// 	GRD_ENUM_VALUE(Numlock);     GRD_TAG("Num Lock"_b);
// 	GRD_ENUM_VALUE(Scroll_Lock); GRD_TAG("Scroll Lock"_b);

// 	GRD_ENUM_VALUE(Equal); GRD_TAG("="_b);

// 	GRD_ENUM_VALUE(Dictionary);
// 	GRD_ENUM_VALUE(Unregister_Word); GRD_TAG("Unregister Word"_b);
// 	GRD_ENUM_VALUE(Register_Word);   GRD_TAG("Register Word"_b);
// 	GRD_ENUM_VALUE(Left_OYAYUBI);    GRD_TAG("Left OYAYUBI"_b);
// 	GRD_ENUM_VALUE(Right_OYAYUBI);   GRD_TAG("Right OYAYUBI"_b);

// 	GRD_ENUM_VALUE(Browser_Back);     GRD_TAG("Browser Back"_b);
// 	GRD_ENUM_VALUE(Browser_Forward);  GRD_TAG("Browser Forward"_b);
// 	GRD_ENUM_VALUE(Browser_Refresh);  GRD_TAG("Browser Refresh"_b);
// 	GRD_ENUM_VALUE(Browser_Stop);     GRD_TAG("Browser Stop"_b);
// 	GRD_ENUM_VALUE(Browser_Search);   GRD_TAG("Browser Search"_b);
// 	GRD_ENUM_VALUE(Browser_Favorites);GRD_TAG("Browser Favorites"_b);
// 	GRD_ENUM_VALUE(Browser_Home);     GRD_TAG("Browser Home"_b);

// 	GRD_ENUM_VALUE(Volume_Mute);          GRD_TAG("Mute"_b);
// 	GRD_ENUM_VALUE(Volume_Down);          GRD_TAG("Volume Down"_b);
// 	GRD_ENUM_VALUE(Volume_Up);            GRD_TAG("Volume Up"_b);
// 	GRD_ENUM_VALUE(Media_Next_Track);     GRD_TAG("Next Track"_b);
// 	GRD_ENUM_VALUE(Media_Previous_Track); GRD_TAG("Previous Track"_b);
// 	GRD_ENUM_VALUE(Media_Stop);           GRD_TAG("Stop"_b);
// 	GRD_ENUM_VALUE(Media_Play_Pause);     GRD_TAG("Play/Pause"_b);
// 	GRD_ENUM_VALUE(Launch_Mail);          GRD_TAG("Mail"_b);
// 	GRD_ENUM_VALUE(Launch_Media_Select);  GRD_TAG("Media select"_b);
// 	GRD_ENUM_VALUE(Launch_App1);          GRD_TAG("App 1"_b);
// 	GRD_ENUM_VALUE(Launch_App2);          GRD_TAG("App 2"_b);

// 	GRD_ENUM_VALUE(AX);
// 	GRD_ENUM_VALUE(Zero_Zero); GRD_TAG("00"_b);

// 	GRD_ENUM_VALUE(Process);

// 	GRD_ENUM_VALUE(Attn);
// 	GRD_ENUM_VALUE(CrSel);
// 	GRD_ENUM_VALUE(ExSel);
// 	GRD_ENUM_VALUE(Erase_EOF); GRD_TAG("Erase EOF"_b);

// 	GRD_ENUM_VALUE(Zoom);
// 	GRD_ENUM_VALUE(Play);
// }

Optional<Key> map_mouse_button_index(int index) {
	switch (index) {
		case 0:  return Key::LMB;
		case 1:  return Key::RMB;
		case 2:  return Key::MMB;
		case 3:  return Key::Mouse_4;
		case 4:  return Key::Mouse_5;
		case 5:  return Key::Mouse_6;
		case 6:  return Key::Mouse_7;
		case 7:  return Key::Mouse_8;
		case 8:  return Key::Mouse_9;
	}
	return {};
}
