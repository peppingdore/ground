#pragma once

#include "../reflection.h"
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
// REFLECT(Key) {

// 	ENUM_VALUE(Mouse_Wheel_Up);
// 		TAG("Mouse Wheel Up"_b);
// 	ENUM_VALUE(Mouse_Wheel_Down);
// 		TAG("Mouse Wheel Down"_b);

// 	ENUM_VALUE(Up_Arrow);
// 		TAG("Up Arrow"_b);
// 	ENUM_VALUE(Down_Arrow);
// 		TAG("Down Arrow"_b);
// 	ENUM_VALUE(Left_Arrow);
// 		TAG("Left Arrow"_b);
// 	ENUM_VALUE(Right_Arrow);
// 		TAG("Right Arrow"_b);

// 	ENUM_VALUE(Any_Shift);
// 		TAG("Shift"_b);
// 	ENUM_VALUE(Any_Control);
// 		TAG("Ctrl"_b);
// 	ENUM_VALUE(Any_Alt);
// 		TAG("Alt"_b);

// 	ENUM_VALUE(Left_Shift);
// 		TAG("Left Shift"_b);
// 	ENUM_VALUE(Left_Control);
// 		TAG("Left Ctrl"_b);
// 	ENUM_VALUE(Left_Alt);
// 		TAG("Left Alt"_b);

// 	ENUM_VALUE(Right_Shift);
// 		TAG("Right Shift"_b);
// 	ENUM_VALUE(Right_Control);
// 		TAG("Right Ctrl"_b);
// 	ENUM_VALUE(Right_Alt);
// 		TAG("Right Alt"_b);


// 	ENUM_VALUE(Option);
// 	ENUM_VALUE(Fn);

// 	ENUM_VALUE(Backspace);
// 	ENUM_VALUE(Delete);
// 	ENUM_VALUE(Forward_Delete);

// 	ENUM_VALUE(Insert);
// 	ENUM_VALUE(Enter);
// 	ENUM_VALUE(Escape);
// 	ENUM_VALUE(Tab);
// 	ENUM_VALUE(Space);

// 	ENUM_VALUE(LMB);
// 	ENUM_VALUE(RMB);
// 	ENUM_VALUE(MMB);

// 	ENUM_VALUE(Mouse_4);
// 		TAG("Mouse 4"_b);
// 	ENUM_VALUE(Mouse_5);
// 		TAG("Mouse 5"_b);
// 	ENUM_VALUE(Mouse_6);
// 		TAG("Mouse 6"_b);
// 	ENUM_VALUE(Mouse_7);
// 		TAG("Mouse 7"_b);
// 	ENUM_VALUE(Mouse_8);
// 		TAG("Mouse 8"_b);
// 	ENUM_VALUE(Mouse_9);
// 		TAG("Mouse 9"_b);

// 	ENUM_VALUE(F1);
// 	ENUM_VALUE(F2);
// 	ENUM_VALUE(F3);
// 	ENUM_VALUE(F4);
// 	ENUM_VALUE(F5);
// 	ENUM_VALUE(F6);
// 	ENUM_VALUE(F7);
// 	ENUM_VALUE(F8);
// 	ENUM_VALUE(F9);
// 	ENUM_VALUE(F10);
// 	ENUM_VALUE(F11);
// 	ENUM_VALUE(F12);
// 	ENUM_VALUE(F13);
// 	ENUM_VALUE(F14);
// 	ENUM_VALUE(F15);
// 	ENUM_VALUE(F16);
// 	ENUM_VALUE(F17);
// 	ENUM_VALUE(F18);
// 	ENUM_VALUE(F19);
// 	ENUM_VALUE(F20);
// 	ENUM_VALUE(F21);
// 	ENUM_VALUE(F22);
// 	ENUM_VALUE(F23);
// 	ENUM_VALUE(F24);

// 	ENUM_VALUE(Page_Up);   TAG("Page Up"_b);
// 	ENUM_VALUE(Page_Down); TAG("Page Down"_b);

// 	ENUM_VALUE(Number_0); TAG("0"_b);
// 	ENUM_VALUE(Number_1); TAG("1"_b);
// 	ENUM_VALUE(Number_2); TAG("2"_b);
// 	ENUM_VALUE(Number_3); TAG("3"_b);
// 	ENUM_VALUE(Number_4); TAG("4"_b);
// 	ENUM_VALUE(Number_5); TAG("5"_b);
// 	ENUM_VALUE(Number_6); TAG("6"_b);
// 	ENUM_VALUE(Number_7); TAG("7"_b);
// 	ENUM_VALUE(Number_8); TAG("8"_b);
// 	ENUM_VALUE(Number_9); TAG("9"_b);

// 	// Imagine I did a typo here..
// 	ENUM_VALUE(A); TAG("A"_b);
// 	ENUM_VALUE(B); TAG("B"_b);
// 	ENUM_VALUE(C); TAG("C"_b);
// 	ENUM_VALUE(D); TAG("D"_b);
// 	ENUM_VALUE(E); TAG("E"_b);
// 	ENUM_VALUE(F); TAG("F"_b);
// 	ENUM_VALUE(G); TAG("G"_b);
// 	ENUM_VALUE(H); TAG("H"_b);
// 	ENUM_VALUE(I); TAG("I"_b);
// 	ENUM_VALUE(K); TAG("K"_b);
// 	ENUM_VALUE(L); TAG("L"_b);
// 	ENUM_VALUE(M); TAG("M"_b);
// 	ENUM_VALUE(N); TAG("N"_b);
// 	ENUM_VALUE(O); TAG("O"_b);
// 	ENUM_VALUE(P); TAG("P"_b);
// 	ENUM_VALUE(Q); TAG("Q"_b);
// 	ENUM_VALUE(R); TAG("R"_b);
// 	ENUM_VALUE(S); TAG("S"_b);
// 	ENUM_VALUE(T); TAG("T"_b);
// 	ENUM_VALUE(V); TAG("V"_b);
// 	ENUM_VALUE(X); TAG("X"_b);
// 	ENUM_VALUE(Y); TAG("Y"_b);
// 	ENUM_VALUE(Z); TAG("Z"_b);

// 	ENUM_VALUE(Semicolon); TAG(";"_b);
// 	ENUM_VALUE(Comma);     TAG(","_b);
// 	ENUM_VALUE(Dot);       TAG("."_b);
// 	ENUM_VALUE(Slash);     TAG("/"_b);
// 	ENUM_VALUE(Tilde);     TAG("~"_b);

// 	ENUM_VALUE(Left_Square_Bracket);  TAG("["_b);
// 	ENUM_VALUE(Right_Square_Bracket); TAG("]"_b);
// 	ENUM_VALUE(Backslash);            TAG("\\"_b);
// 	ENUM_VALUE(Quote);                TAG("\'"_b);


// 	ENUM_VALUE(Plus);  TAG("+"_b);
// 	ENUM_VALUE(Minus); TAG("-"_b);

// 	ENUM_VALUE(Ctrl_Break); TAG("Ctrl+Break"_b);
// 	ENUM_VALUE(Clear);
// 	ENUM_VALUE(Pause);
// 	ENUM_VALUE(Caps_Lock); TAG("Caps Lock"_b);

// 	ENUM_VALUE(Kana_Or_Hangul); // @TODO: Distinguish using OS Key name api.

// 	ENUM_VALUE(Junja);
// 	ENUM_VALUE(Final);
// 	ENUM_VALUE(Hanja_Or_Kanji);  // @TODO: Distinguish using OS Key name api.

// 	ENUM_VALUE(Convert);
// 	ENUM_VALUE(Non_Convert); TAG("Don't convert"_b);
// 	ENUM_VALUE(Accept);
// 	ENUM_VALUE(Mode_Change); TAG("Mode change"_b);

// 	ENUM_VALUE(End);
// 	ENUM_VALUE(Home);

// 	ENUM_VALUE(Select);
// 	ENUM_VALUE(Print);
// 	ENUM_VALUE(Execute);
// 	ENUM_VALUE(Print_Screen); TAG("Print Screen"_b);
// 	ENUM_VALUE(Help);

// 	ENUM_VALUE(Left_Windows);  TAG("Left Windows"_b);
// 	ENUM_VALUE(Right_Windows); TAG("Right Windows"_b);
// 	ENUM_VALUE(Menu);

// 	ENUM_VALUE(Sleep);

// 	ENUM_VALUE(Numpad_0); TAG("Num 0"_b);
// 	ENUM_VALUE(Numpad_1); TAG("Num 1"_b);
// 	ENUM_VALUE(Numpad_2); TAG("Num 2"_b);
// 	ENUM_VALUE(Numpad_3); TAG("Num 3"_b);
// 	ENUM_VALUE(Numpad_4); TAG("Num 4"_b);
// 	ENUM_VALUE(Numpad_5); TAG("Num 5"_b);
// 	ENUM_VALUE(Numpad_6); TAG("Num 6"_b);
// 	ENUM_VALUE(Numpad_7); TAG("Num 7"_b);
// 	ENUM_VALUE(Numpad_8); TAG("Num 8"_b);
// 	ENUM_VALUE(Numpad_9); TAG("Num 9"_b);

// 	ENUM_VALUE(Numpad_Multiply); TAG("*"_b);
// 	ENUM_VALUE(Separator);
// 	ENUM_VALUE(Numpad_Slash); TAG("Num /"_b);

// 	ENUM_VALUE(Numpad_Dot); TAG("Num ."_b);

// 	ENUM_VALUE(Numlock);     TAG("Num Lock"_b);
// 	ENUM_VALUE(Scroll_Lock); TAG("Scroll Lock"_b);

// 	ENUM_VALUE(Equal); TAG("="_b);

// 	ENUM_VALUE(Dictionary);
// 	ENUM_VALUE(Unregister_Word); TAG("Unregister Word"_b);
// 	ENUM_VALUE(Register_Word);   TAG("Register Word"_b);
// 	ENUM_VALUE(Left_OYAYUBI);    TAG("Left OYAYUBI"_b);
// 	ENUM_VALUE(Right_OYAYUBI);   TAG("Right OYAYUBI"_b);

// 	ENUM_VALUE(Browser_Back);     TAG("Browser Back"_b);
// 	ENUM_VALUE(Browser_Forward);  TAG("Browser Forward"_b);
// 	ENUM_VALUE(Browser_Refresh);  TAG("Browser Refresh"_b);
// 	ENUM_VALUE(Browser_Stop);     TAG("Browser Stop"_b);
// 	ENUM_VALUE(Browser_Search);   TAG("Browser Search"_b);
// 	ENUM_VALUE(Browser_Favorites);TAG("Browser Favorites"_b);
// 	ENUM_VALUE(Browser_Home);     TAG("Browser Home"_b);

// 	ENUM_VALUE(Volume_Mute);          TAG("Mute"_b);
// 	ENUM_VALUE(Volume_Down);          TAG("Volume Down"_b);
// 	ENUM_VALUE(Volume_Up);            TAG("Volume Up"_b);
// 	ENUM_VALUE(Media_Next_Track);     TAG("Next Track"_b);
// 	ENUM_VALUE(Media_Previous_Track); TAG("Previous Track"_b);
// 	ENUM_VALUE(Media_Stop);           TAG("Stop"_b);
// 	ENUM_VALUE(Media_Play_Pause);     TAG("Play/Pause"_b);
// 	ENUM_VALUE(Launch_Mail);          TAG("Mail"_b);
// 	ENUM_VALUE(Launch_Media_Select);  TAG("Media select"_b);
// 	ENUM_VALUE(Launch_App1);          TAG("App 1"_b);
// 	ENUM_VALUE(Launch_App2);          TAG("App 2"_b);

// 	ENUM_VALUE(AX);
// 	ENUM_VALUE(Zero_Zero); TAG("00"_b);

// 	ENUM_VALUE(Process);

// 	ENUM_VALUE(Attn);
// 	ENUM_VALUE(CrSel);
// 	ENUM_VALUE(ExSel);
// 	ENUM_VALUE(Erase_EOF); TAG("Erase EOF"_b);

// 	ENUM_VALUE(Zoom);
// 	ENUM_VALUE(Play);
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
