#ifndef WINTEASER_H
#define WINTEASER_H

#include "stdafx.h"
#include "game.h"

int Win(HWND hWnd, Solitaire *ref,
		const char *text = "Congradulations, You Won!!!\nPlay Again?", 
		const char *caption = "Winner!");
long int* threadInit(Solitaire *cur, void *datum = NULL);
void SpawnThread(void *pObject);

#endif
