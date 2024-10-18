#pragma once

#include "../../grd_base.h"

#if OS_WINDOWS
	#include "win_thread.h"
#else
	#include "posix_thread.h"
#endif
 