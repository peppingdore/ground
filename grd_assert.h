#pragma once

#include "grd_panic.h"

#define grd_assert(expr) if (!(expr)) grd_panic("Assertion failed: " #expr )
