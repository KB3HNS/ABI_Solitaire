#ifndef SANDA_H
#define SANDA_H

#include "game.h"

struct LOCATION {
	int row;
	int columb;
	int depth;
	bool isValid;
};

struct TABLELOC {
	int row;
	int columb;
	int depth;
	int region;
};

struct XY {
	int x;
	int y;
	int rot;
};

struct UNDO {
	TABLELOC source;
	TABLELOC dest;
	UNDO *previous;
};

class StreetsAndAlleys : public Solitaire {
public:

	StreetsAndAlleys(HWND hWind, int info = 0);
	~StreetsAndAlleys();

	void deal();
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
	int winWork(void* datum);
	void delInst();
	void dblClick(int X, int Y);

private:
	//  Private functions are for performing internal calculations and should
	//  NEVER be called outside of the class code.
	int getRegion(int X, int Y);
	LOCATION getRegion1(int X, int Y);
	LOCATION getRegion2(int X, int Y);
	LOCATION getRegion3(int X, int Y);
	bool isValid(int x, int y, int X, int Y, int rotation);
	void placeCard(TABLELOC card);
	void selectCard(LOCATION card, int region);
	bool moveIsValid(LOCATION card, int region);
	void makeMove(LOCATION card, int region);
	void removeCard(TABLELOC card);
	XY getXY(TABLELOC card);

	int table[5][3][4][3];  //  columbs x rows x deep x zone
	int startCard, score, height, width;
	TABLELOC loc, selected, place;
	UNDO *back;
	bool won;
};

#endif
