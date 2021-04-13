#ifndef DBGDUMMY_H
#define DBGDUMMY_H

#include "game.h"

//  Dummy class with some usefull debuging routines

class dbgDummy : public Solitaire {

public:
	
	dbgDummy(HWND hWind, int info = 0);

	virtual void leftClick(int X, int Y);  
	virtual void rightClick(int X, int Y);
	virtual void rightUp();
	virtual int load(const char *filename);
	virtual int save(const char *filename);
	virtual void redrawBoard();
	virtual void undo();
	virtual char* gameName();
	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void enter();
	virtual void deal();
	virtual void exInit(HWND hWnd, int info);

private:
	int rot, xPos, yPos, card;
	double zoom;
	bool selected;
};

#endif
