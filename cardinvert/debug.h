#ifndef DEBUG_H
#define DEBUG_H

#include "stdafx.h"

#define DEBUG TRUE  //change for a non-debug build

//  Function Definitions
ATOM debugInit(HINSTANCE hInstance, int nCmdShow, 
			   TCHAR szWindowClass[MAX_LOADSTRING]);

_CRTIMP int __cdecl dprintf(char * , ...); // compatible with printf syntax

#endif
