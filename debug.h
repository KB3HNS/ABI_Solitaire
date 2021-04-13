#ifndef DEBUG_H
#define DEBUG_H

#include "stdafx.h"

#ifndef _DEBUG
	#define DEBUG FALSE  //change for a non-debug build
#else
	#define DEBUG TRUE
#endif

//  Function Definitions
ATOM debugInit(HINSTANCE hInstance, int nCmdShow, 
			   TCHAR szWindowClass[MAX_LOADSTRING]);

_CRTIMP int __cdecl dprintf(char * , ...); // compatible with printf syntax

#endif
