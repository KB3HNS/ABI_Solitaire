#include "stdafx.h"
#include "game.h"
#include "winteaser.h"
#include "cardtools.h"
#include "colordefs.h"
#include <process.h>
#include <time.h>

//  Win Game Software...  *VERY* OS dependant as it relies on threads.

//  Global storage area...  most of the "worker" stuff is actually a function of the
//  parent class... an S&A object.
//StreetsAndAlleys *game;

//  Handel a pop up menu asking to play again.
//  Processes the result as well
int Win(HWND hWnd, Solitaire *ref, const char *text, const char *caption) {
	int ret;

	//clearBoard(hWnd);
	//bprintf(hWnd, 100, 500, BWHITE, "Congraduations, You Won!!!");
	MessageBeep(-1);

	ret = MessageBox(hWnd, text, caption, MB_OKCANCEL);
	
	if (ret == IDOK) {
		ref->shuffel();
		ref->deal();
		ref->redrawBoard();
	}
	
	return ret;
}

void SpawnThread(void *pObject) {
	
	Solitaire *game;
    void **pOb = (void**)pObject;

	//  since we are in a new thread, we have to re-initialize the random number
	//  generator  -- this may be a windows thing.
	srand((unsigned)time( NULL ));
	game = (Solitaire*)pOb[0];
	if (game != NULL) 
		game->winWork(pOb[1]);
	
	return;   // thread completed successfully

}

long int* threadInit(Solitaire *cur, void *datum) {
	static void *data[2];

	data[0] = (void*)cur;
	data[1] = datum;

	return (long int*)_beginthread(SpawnThread, 0, (void*)data);
}
