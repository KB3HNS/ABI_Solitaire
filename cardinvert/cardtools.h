//  Utilities for manipulating cards
#ifndef CARDTOOLS_H
#define CARDTOOLS_H

#include "stdafx.h"

struct CARDDIM {
	int Height;
	int Width;
};

void drawCard(HWND hWnd, int xPos, int yPos, int cardNum, int rotation, bool selected);
void initCards();
void closeCards();
void eraseCard (HWND hWnd, int cx, int cy, int rotation, int cardNum = 0);
double DegtoRad(int deg);  //  This IS required for the Solitare games
CARDDIM getCardAttributes(int cardNum = 0);
void clearBoard(HWND hWnd);
_CRTIMP int __cdecl bprintf(HWND hWnd, int x, int y, int Color, char * , ...);
	// compatible with printf syntax
void putMouse(int x, int y, HWND hWnd);


#endif
