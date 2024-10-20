#pragma once

#include "../../grd_base.h"

#if GRD_OS_WINDOWS
	#include "grd_win_thread.h"
#else
	#include "grd_posix_thread.h"
#endif
 