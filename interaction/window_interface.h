#pragma once

#include "../grd_string.h"
#include "../grd_scoped.h"
#include "../math/grd_vector.h"
#include "../grd_error.h"
#include "event.h"

struct WindowParams {
	GrdUnicodeString title;
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
GRD_REFLECT(WindowEvent) {
	GRD_BASE_TYPE(Event);
	GRD_MEMBER(time_from_system_start);
}

struct FocusEvent: WindowEvent {
	bool have_focus = false;
};
GRD_REFLECT(FocusEvent) {
	GRD_BASE_TYPE(WindowEvent);
	GRD_MEMBER(have_focus);
}

struct CharEvent: WindowEvent {
	char32_t character;
};
GRD_REFLECT(CharEvent) {
	GRD_BASE_TYPE(WindowEvent);
	GRD_MEMBER(character);
}

enum class KeyAction {
	Unknown = 0,
	Up      = 1,
	Down    = 2,
	Hold    = 3,
};
GRD_REFLECT(KeyAction) {
	GRD_ENUM_VALUE(Unknown);
	GRD_ENUM_VALUE(Up);
	GRD_ENUM_VALUE(Down);
	GRD_ENUM_VALUE(Hold);
}

struct KeyEvent: WindowEvent {
	KeyAction action;
	Key       key;
	u32       os_key_code;
};
GRD_REFLECT(KeyEvent) {
	GRD_BASE_TYPE(WindowEvent);
	GRD_MEMBER(action);
	GRD_MEMBER(key);
	GRD_MEMBER(os_key_code);
}

struct WindowCloseEvent: WindowEvent {

};
GRD_REFLECT(WindowCloseEvent) {
	GRD_BASE_TYPE(WindowEvent);
}
