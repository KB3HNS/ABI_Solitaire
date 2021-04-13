/*  
 *  debug.cpp
 *  Creates a second window that prints out debug information
 *  similar to printf utility.  This routine will need
 *  significant development.
 */

#include "stdafx.h"
#include "debug.h"
#include "resource.h"

//  Internal Use Only
LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 

#if (DEBUG == TRUE) //  Debug Code
//  Global variables:
HWND hWnd;
int line;

LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM debugInit(HINSTANCE hInstance, int nCmdShow, 
			   TCHAR szWindowClass[MAX_LOADSTRING]) {

	WNDCLASSEX wcex;
	HWND dWnd;
	ATOM retClassX;
	HBRUSH hbrush = CreateSolidBrush(RGB(0,0,0));

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_SAVEBITS;
	wcex.lpfnWndProc	= (WNDPROC)WndProcDebug;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_CARDINVERT);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= hbrush;
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.lpszClassName	= "Debug Output";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	retClassX = RegisterClassEx(&wcex);
	
	dWnd = CreateWindow("Debug Output", "Debug Output",
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL,
		50,	50, 500, 300, NULL, NULL, hInstance, NULL);

	ShowWindow(dWnd, nCmdShow);
	UpdateWindow(dWnd);
	
	hWnd = dWnd;  //  Store our window handel so we draw here and not on the main window.
	line = 0;  //  Reset global line location
	return retClassX;
}

/*
 *  Compatible with printf syntax.  We use an sprintf to concatenate the arguments
 *  and then output the text to the debug screen.  We try to keep track of where the
 *  Text goes, but this is still somewhat buggy.
 */
_CRTIMP int __cdecl dprintf(char *output, ...) {

	va_list marker;
	int count, len;
	HDC hdc = GetDC(hWnd);
	HGDIOBJ hfnt, hOldFont;
	char *outString;

	va_start(marker, output);
	count = vprintf(output, marker);
	outString = (char*)malloc((size_t)count);
	
	vsprintf(outString, output, marker);
	len = (int)strlen(outString);
	hfnt = GetStockObject(ANSI_VAR_FONT);

	if (hOldFont = SelectObject(hdc, hfnt)) {
		//  White on black theme
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));
		TextOut(hdc, 0, line, outString, len);
		SelectObject(hdc, hOldFont);
	}

	line += 15;

	DeleteObject(hfnt);
	ReleaseDC(hWnd, hdc);
//	free((void*)outString);
	va_end(marker);

	return count;  //  For Compatibility
}

#else  // Non Debug Code

LRESULT CALLBACK WndProcDebug(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);  //  Just return handel to the OS
}

ATOM debugInit(HINSTANCE hInstance, int nCmdShow, 
			   TCHAR szWindowClass[MAX_LOADSTRING]) {

	return NULL;  //  No Class will be registered, hopefully will throw an exception
				//  I don't think that I use the return anywhere anyways.
				//  It's more for compatibility than anything else.
}

/*
 *  Compatible with printf syntax.  We use an sprintf to concatenate the arguments
 *  and then output the text to the debug screen.  We try to keep track of where the
 *  Text goes, but this is still somewhat buggy.
 */
_CRTIMP int __cdecl dprintf(char *output, ...) {

	va_list marker;
	int count;

	va_start(marker, output);
	count = vprintf(output, marker);
	return count;  //  For Compatibility
	//  Even though we do nothing, we still have to return the byte count.
}

#endif
