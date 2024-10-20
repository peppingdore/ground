#pragma once

#include "../grd_reflect.h"
#include "key.h"

struct Event {
	Type* type = NULL;
};
GRD_REFLECT(Event) {
	GRD_MEMBER(type); GRD_TAG(GrdRealTypeMember{});
}

template <typename T>
T* grd_make_event() {
	auto event = grd_make<T>();
	event->type = grd_reflect_type_of<T>();
	return event;
}
