#pragma once

#include "panic.h"

#define gnd_assert(expr) if (!(expr)) panic("Assertion failed: " #expr )
