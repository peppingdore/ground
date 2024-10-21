#pragma once

#include "../grd_reflect.h"
#include "grd_key.h"

struct GrdEvent {
	GrdType* type = NULL;
};
GRD_REFLECT(GrdEvent) {
	GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
}

template <typename T>
T* grd_make_event() {
	auto event = grd_make<T>();
	event->type = grd_reflect_type_of<T>();
	return event;
}
