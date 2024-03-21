#pragma once

#include "../string.h"
#include "../scoped.h"
#include "../math/vector.h"
#include "../error.h"
#include "event.h"

struct WindowParams {
	UnicodeString title;
	Vector2i      initial_size = { 1280, 720 };
	Vector2i      min_size = { 100, 100 };
	bool          borderless = false;
};

struct Window {
	Type*        type;
	WindowParams params;
};

struct WindowEvent: Event {
	f64 time_from_system_start = 0;
};
REFLECT(WindowEvent) {
	BASE_TYPE(Event);
	MEMBER(time_from_system_start);
}

struct FocusEvent: WindowEvent {
	bool have_focus = false;
};
REFLECT(FocusEvent) {
	BASE_TYPE(WindowEvent);
	MEMBER(have_focus);
}

struct CharEvent: WindowEvent {
	char32_t character;
};
REFLECT(CharEvent) {
	BASE_TYPE(WindowEvent);
	MEMBER(character);
}

enum class KeyAction {
	Unknown = 0,
	Up      = 1,
	Down    = 2,
	Hold    = 3,
};
REFLECT(KeyAction) {
	ENUM_VALUE(Unknown);
	ENUM_VALUE(Up);
	ENUM_VALUE(Down);
	ENUM_VALUE(Hold);
}

struct KeyEvent: WindowEvent {
	KeyAction action;
	Key       key;
	u32       os_key_code;
};
REFLECT(KeyEvent) {
	BASE_TYPE(WindowEvent);
	MEMBER(action);
	MEMBER(key);
	MEMBER(os_key_code);
}

struct WindowCloseEvent: WindowEvent {

};
REFLECT(WindowCloseEvent) {
	BASE_TYPE(WindowEvent);
}
