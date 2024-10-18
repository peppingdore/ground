#pragma once

#include "../../grd_base.h"

#if OS_WINDOWS
	#include "win_sync.h"
#else
	#include "posix_sync.h"
#endif
