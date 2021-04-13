//  Utilities for manipulating cards
#ifndef CARDTOOLS_H
#define CARDTOOLS_H

#include "stdafx.h"

#define LASTCARD 111

#ifndef BGCOLOR
	#define BGCOLOR RGB(0, 128, 0)
#endif

#ifndef PI
	#define PI 3.1415926535897932384626433832795
#endif

#define E 55

struct CARDDIM {
	int Height;
	int Width;
};

void drawCard(HWND hWnd, int xPos, int yPos, int cardNum, int rotation,
			  bool selected, double zoom = 1);
void initCards(HWND hWnd);
void eraseCard (HWND hWnd, int cx, int cy, int rotation, double zoom = 1, int cardNum = 0);
double DegtoRad(int deg);  //  This IS required for the Solitare games
CARDDIM getCardAttributes(int cardNum = 0);
void clearBoard(HWND hWnd);

// compatible with printf syntax
_CRTIMP int __cdecl bprintf(HWND hWnd, int x, int y, int Color, char * , ...);

void putMouse(int x, int y, HWND hWnd);
double axis2zoom(int zoom);

//  Sorts integers in ascending / descending order
void hsort(int *source, int *dest, int count, bool down = false);
CARDDIM getBoardDims(HWND hWnd);
int RadtoDeg(double rad);
void closeCards();

#endif
