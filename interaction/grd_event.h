#pragma once

#include "../grd_reflect.h"
#include "grd_key.h"

struct GrdEvent {
	GrdType*     type = NULL;
	GrdFreeList* free_list = NULL;

	void free() {
		grd_free_list_free(free_list);
		GrdFree(this);
	}
};
GRD_REFLECT(GrdEvent) {
	GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
	GRD_MEMBER(free_list);
}

template <typename T>
GRD_DEF grd_make_event() -> T* {
	auto event = grd_make<T>();
	event->type = grd_reflect_type_of<T>();
	return event;
}
