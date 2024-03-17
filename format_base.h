#pragma once

#include "reflection.h"
#include "string.h"

struct Formatter;

using Type_Format_Type_Erased = void(Formatter*, void*, String);

void insert_custom_type_format();

REFLECTION_REFLECT_HOOK(T) {

}