//  This is the Game Definition Header.
//  All solitare games are inhereted from this class.

#ifndef GAME_H
#define GAME_H

#define INVAL_GAME_TYPE -2

#include "stdafx.h"

class Solitaire {
public:
	
	Solitaire();  //   Bad Constructor
	Solitaire(HWND hWind, int info = 0);
	~Solitaire();
	
	virtual void shuffel();  //  Some games may need their own shuffel function
	virtual void deal();
	virtual bool keyPress(char);
	virtual void Hit();
	virtual bool getHit();
	virtual long int* winTeaser(void* datum = NULL);
	virtual int winWork(void* datum);
	virtual void dblClick(int X, int Y);
	virtual void delInst();  //  all destruction code should be here
	
	//  These functions are pure virtual and MUST be implimented by the inheriter
	virtual void leftClick(int X, int Y) = 0;  
	virtual void rightClick(int X, int Y) = 0;
	virtual void rightUp() = 0;
	virtual int load(const char *filename) = 0;
	virtual int save(const char *filename) = 0;
	virtual void redrawBoard() = 0;
	virtual void undo() = 0;
	virtual char* gameName() = 0;
	virtual void left() = 0;
	virtual void right() = 0;
	virtual void up() = 0;
	virtual void down() = 0;
	virtual void enter() = 0;

protected:
	void Init(HWND hWind, int info = 0);
	int deck[55];  //  Allow the use of expanded cards
	int datum;  // A generic integer that the program can pass to us to help identify
				//  the file format.
	HWND hWnd;  //  We have to keep track of our window so we know where to put cards
	bool hit;
};

#endif
