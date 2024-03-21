#pragma once

#include "../reflection.h"
#include "key.h"

struct Event {
	Type* type = NULL;
};
REFLECT(Event) {
	MEMBER(type); TAG(RealTypeMember{});
}

template <typename T>
T* make_event() {
	auto event = make<T>();
	event->type = reflect.type_of<T>();
	return event;
}
