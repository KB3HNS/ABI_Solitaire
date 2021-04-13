#ifndef PYRIMID_H
#define PYRIMID_H

//  The pyrimid game class definition
//  Yet another game in the solitaire system

#include "stdafx.h"
#include "game.h"
#include "cards.h"
#include "pcard.h"

struct PYSELECT {
	int loc;
	int xc;
	int yc;
};

struct PYUNDO {
	PYUNDO *prev;
	short loc1;
	short loc2;
	short xc1;
	short xc2;
	short yc1;
	short yc2;
	short val1;
	short val2;
};

class Pyrimid : public Solitaire {
public:
	Pyrimid(HWND hWind, int info = 0);  //  The standard constructor
	~Pyrimid();

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
	void delInst();  //  all destruction code should be here
	void deal();
	int winWork(void* datum);


private:
	PYSELECT getPS(int x, int y);
	KXY getXY(PYSELECT pys);
	void calcMove(PYSELECT pys);
	void valKBD();
	void runAnim();

	PCard *root;
	PCard *pyr[7][7];
	Cards *draw, *pile, *to;
	KXY ca, rtxy;
	PYSELECT ps, rdn, kbd;
	int score;
	PYUNDO *pundo;
	bool won;
};

#endif
