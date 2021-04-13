// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

//#pragma once
#ifndef STDAFX_H
#define STDAFX_H

//  This breaks GDI+
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#define UNICODE
#define MAX_LOADSTRING 100

#define VMAJOR 1
#define VMINOR 4
#define VBUILD 7

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
//#include <gdiplus.h>
#include <stdio.h>

#ifndef  UNUSED
	#define UNUSED(x) x
#endif

#endif
