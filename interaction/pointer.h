#pragma once

#include "../reflect.h"
#include "../math/vector.h"
#include "../hash_map.h"
#include "event.h"
#include "window_interface.h"

enum class PointerAction: s32 {
	Down   = 0,
	Up     = 1,
	Move   = 2,
	Cancel = 3,
	Scroll = 4,	
};
REFLECT(PointerAction) {
	ENUM_VALUE(Down);
	ENUM_VALUE(Up);
	ENUM_VALUE(Move);
	ENUM_VALUE(Cancel);
	ENUM_VALUE(Scroll);
}

enum class PointerKind: s32 {
	Mouse = 0,
	Pen   = 1,
	Touch = 2,
};
REFLECT(PointerKind) {
	ENUM_VALUE(Mouse);
	ENUM_VALUE(Pen);
	ENUM_VALUE(Touch);
}

enum PointerButtonFlags: u64 {
	POINTER_BUTTON_NONE         = 0,
	POINTER_BUTTON_MOUSE_LEFT   = 1 << 0,
	POINTER_BUTTON_MOUSE_RIGHT  = 1 << 1,
	POINTER_BUTTON_MOUSE_MIDDLE = 1 << 2,
	POINTER_BUTTON_MOUSE_4      = 1 << 3,
	POINTER_BUTTON_MOUSE_5      = 1 << 4,
};
REFLECT(PointerButtonFlags) {
	ENUM_VALUE(POINTER_BUTTON_NONE);
	ENUM_VALUE(POINTER_BUTTON_MOUSE_LEFT);
	ENUM_VALUE(POINTER_BUTTON_MOUSE_RIGHT);
	ENUM_VALUE(POINTER_BUTTON_MOUSE_MIDDLE);
	ENUM_VALUE(POINTER_BUTTON_MOUSE_4);
	ENUM_VALUE(POINTER_BUTTON_MOUSE_5);
}

struct Pointer {
	u64                id;
	PointerKind        kind;
	Vector2            position;
	PointerButtonFlags buttons;
};
REFLECT(Pointer) {
	MEMBER(kind);
	MEMBER(id);
	MEMBER(position);
	MEMBER(buttons);
}

struct PointerEvent: WindowEvent {
	PointerAction      action;
	Pointer            pointer;
	PointerButtonFlags changed_buttons = POINTER_BUTTON_NONE;
	Vector2            scroll_delta = { 0, 0 };
	bool               is_scroll_in_lines = false;
};
REFLECT(PointerEvent) {
	BASE_TYPE(WindowEvent);
	MEMBER(action);
	MEMBER(pointer);
	MEMBER(changed_buttons);
	MEMBER(scroll_delta);
	MEMBER(is_scroll_in_lines);
}

struct OsPointerIdMapper {
	HashMap<u64, u64> map;
	s64               next_pointer_id = 1;

	void free() {
		map.free();
	}
};

u64 map_os_pointer_id(OsPointerIdMapper* mapper, u64 os_id) {
	auto* found = mapper->map.get(os_id);
	if (found) {
		return *found;
	}
	if (mapper->next_pointer_id + 1 == 0) {
		mapper->next_pointer_id = 1;
	}
	auto result = mapper->next_pointer_id++;
	mapper->map.put(os_id, result);
	return result;
}

u64 map_os_mouse_to_pointer_id(OsPointerIdMapper* mapper) {
	return 0;
}
