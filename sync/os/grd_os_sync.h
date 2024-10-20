#pragma once

#include "../../grd_base.h"

#if GRD_OS_WINDOWS
	#include "grd_win_sync.h"
#else
	#include "grd_posix_sync.h"
#endif
