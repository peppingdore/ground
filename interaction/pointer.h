#pragma once

#include "../grd_reflect.h"
#include "../math/grd_vector.h"
#include "../grd_hash_map.h"
#include "grd_event.h"
#include "grd_window_interface.h"

enum class GrdPointerAction: s32 {
	Down   = 0,
	Up     = 1,
	Move   = 2,
	Cancel = 3,
	Scroll = 4,	
};
GRD_REFLECT(GrdPointerAction) {
	GRD_ENUM_VALUE(Down);
	GRD_ENUM_VALUE(Up);
	GRD_ENUM_VALUE(Move);
	GRD_ENUM_VALUE(Cancel);
	GRD_ENUM_VALUE(Scroll);
}

enum class GrdPointerKind: s32 {
	Mouse = 0,
	Pen   = 1,
	Touch = 2,
};
GRD_REFLECT(GrdPointerKind) {
	GRD_ENUM_VALUE(Mouse);
	GRD_ENUM_VALUE(Pen);
	GRD_ENUM_VALUE(Touch);
}

enum GrdPointerButtonFlags: u64 {
	GRD_POINTER_BUTTON_NONE         = 0,
	GRD_POINTER_BUTTON_MOUSE_LEFT   = 1 << 0,
	GRD_POINTER_BUTTON_MOUSE_RIGHT  = 1 << 1,
	GRD_POINTER_BUTTON_MOUSE_MIDDLE = 1 << 2,
	GRD_POINTER_BUTTON_MOUSE_4      = 1 << 3,
	GRD_POINTER_BUTTON_MOUSE_5      = 1 << 4,
};
GRD_REFLECT(GrdPointerButtonFlags) {
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_NONE);
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_MOUSE_LEFT);
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_MOUSE_RIGHT);
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_MOUSE_MIDDLE);
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_MOUSE_4);
	GRD_ENUM_VALUE(GRD_POINTER_BUTTON_MOUSE_5);
}

struct GrdPointer {
	u64                   id;
	GrdPointerKind        kind;
	GrdVector2            position;
	GrdPointerButtonFlags buttons;
};
GRD_REFLECT(GrdPointer) {
	GRD_MEMBER(kind);
	GRD_MEMBER(id);
	GRD_MEMBER(position);
	GRD_MEMBER(buttons);
}

struct GrdPointerEvent: GrdWindowEvent {
	GrdPointerAction      action;
	GrdPointer            pointer;
	GrdPointerButtonFlags changed_buttons = GRD_POINTER_BUTTON_NONE;
	GrdVector2            scroll_delta = { 0, 0 };
	bool                  is_scroll_in_lines = false;
};
GRD_REFLECT(GrdPointerEvent) {
	GRD_BASE_TYPE(GrdWindowEvent);
	GRD_MEMBER(action);
	GRD_MEMBER(pointer);
	GRD_MEMBER(changed_buttons);
	GRD_MEMBER(scroll_delta);
	GRD_MEMBER(is_scroll_in_lines);
}

struct GrdOsPointerIdMapper {
	GrdHashMap<u64, u64> map;
	s64                  next_pointer_id = 1;

	void free() {
		map.free();
	}
};

GRD_DEDUP u64 grd_map_os_pointer_id(GrdOsPointerIdMapper* mapper, u64 os_id) {
	auto* found = grd_get(&mapper->map, os_id);
	if (found) {
		return *found;
	}
	if (mapper->next_pointer_id + 1 == 0) {
		mapper->next_pointer_id = 1;
	}
	auto result = mapper->next_pointer_id++;
	grd_put(&mapper->map, os_id, result);
	return result;
}

GRD_DEDUP u64 grd_map_os_mouse_to_pointer_id(GrdOsPointerIdMapper* mapper) {
	return 0;
}
