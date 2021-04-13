#ifndef DUMMY_H
#define DUMMY_H

#include "game.h"

//  This is a dummy class for the game.  It does absolutely nothing
//  but prevent operations on null pointers.

class Dummy : public Solitaire {

public:
	
	Dummy(HWND hWind, int info = 0);

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

};

#endif
