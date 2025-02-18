#pragma once

#include "../grd_string.h"
#include "../grd_scoped.h"
#include "../math/grd_vector.h"
#include "../grd_error.h"
#include "../grd_log.h"
#include "grd_event.h"

struct GrdWindowParams {
	GrdUnicodeString title;
	GrdVector2i      initial_size = { 1280, 720 };
	GrdVector2i      min_size = { 100, 100 };
	bool             borderless = false;
};

struct GrdWindow {
	GrdType*        type;
	GrdWindowParams params;
	GrdThreadId     owner_thread_id;
};

struct GrdWindowEvent: GrdEvent {
	f64 time_from_system_start = 0;
};
GRD_REFLECT(GrdWindowEvent) {
	GRD_BASE_TYPE(GrdEvent);
	GRD_MEMBER(time_from_system_start);
}

struct GrdFocusEvent: GrdWindowEvent {
	bool have_focus = false;
};
GRD_REFLECT(GrdFocusEvent) {
	GRD_BASE_TYPE(GrdWindowEvent);
	GRD_MEMBER(have_focus);
}

struct GrdCharEvent: GrdWindowEvent {
	char32_t character;
};
GRD_REFLECT(GrdCharEvent) {
	GRD_BASE_TYPE(GrdWindowEvent);
	GRD_MEMBER(character);
}

enum class GrdKeyAction {
	Unknown = 0,
	Up      = 1,
	Down    = 2,
	Hold    = 3,
};
GRD_REFLECT(GrdKeyAction) {
	GRD_ENUM_VALUE(Unknown);
	GRD_ENUM_VALUE(Up);
	GRD_ENUM_VALUE(Down);
	GRD_ENUM_VALUE(Hold);
}

struct GrdKeyEvent: GrdWindowEvent {
	GrdKeyAction action;
	GrdKey       key;
	u32          os_key_code;
};
GRD_REFLECT(GrdKeyEvent) {
	GRD_BASE_TYPE(GrdWindowEvent);
	GRD_MEMBER(action);
	GRD_MEMBER(key);
	GRD_MEMBER(os_key_code);
}

struct GrdWindowCloseEvent: GrdWindowEvent {

};
GRD_REFLECT(GrdWindowCloseEvent) {
	GRD_BASE_TYPE(GrdWindowEvent);
}

GRD_DEF grd_init_window(GrdWindow* window, GrdWindowParams params, GrdType* tp) {
	window->params = params;
	window->owner_thread_id = grd_current_thread_id();
	window->type = tp;
}

GRD_DEF grd_os_read_window_events(GrdWindow* window) -> GrdArray<GrdEvent*>;

GRD_DEF grd_read_window_events(GrdWindow* window) -> GrdArray<GrdEvent*> {
	if (window->owner_thread_id != grd_current_thread_id()) {
		GrdLogWithInfo({ .level = GrdLogLevel::Warning }, "grd_read_window_events(), is called from a different thread than the window owner.");
		return {};
	}
	return grd_os_read_window_events(window);
}
