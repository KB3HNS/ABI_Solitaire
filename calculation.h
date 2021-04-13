//  Header informaiton for the Calculation game rule set.

#ifndef CALCULATION_H
#define CALCULATION_H

#include "game.h"
#include "cards.h"
#include "cardtools.h"

struct CALLOC {
	short loc;
	short row;
	int depth;
};

struct CALUNDO {
	short loc1;
	short loc2;
	short row1;
	short row2;
	CALUNDO *prev;
};

class Calculation : public Solitaire {
public:
	Calculation(HWND hWind, int info = 0);  //  The standard constructor
	~Calculation();

		//  Required functions
	void leftClick(int X, int Y);
	void rightClick(int X, int Y);
	void rightUp();
	int load(const char *filename);
	int save(const char *filename);
	void redrawBoard();
	void undo();
	char* gameName();
	void left();
	void right();
	void up();
	void down();
	void enter();

	//  Functions we will need
	void shuffel();  //  Some games may need their own shuffel function
	void delInst();  //  all destruction code should be here
	void deal();
	void dblClick(int X, int Y);
	int winWork(void* datum);

private:
	KXY getXY(CALLOC loc);
	CALLOC getCL(int x, int y)	;
	void calcMove(CALLOC cl);

	int score;
	Cards *pile, *board[4], *top[4], *to;
	CARDDIM ca;
	CALLOC sel, rm, kbd;
	CALUNDO *Undo;
};

#endif
