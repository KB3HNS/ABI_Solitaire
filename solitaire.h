#pragma once

#include "resource.h"
#include "debug.h"

#if (DEBUG == TRUE)
	#define DUMMY(x, y) dbgDummy(x, y)
#else
	#define DUMMY(x, y) Dummy(x, y)
#endif
