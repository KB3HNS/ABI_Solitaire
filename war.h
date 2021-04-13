//  Ruleset for the GAME war for the solitaire backbone.

#ifndef WAR_H
#define WAR_H

#include "stdafx.h"
#include "game.h"

#define ME 1
#define YOU 0

struct PILE {
	PILE *last;
	int card;
	int sequence;
	int value;
	PILE *next;
};

struct XY1 {
	int x;
	int y;
};

struct LOC {
	int seq;
	int player;
	bool isPlay;
};

class War : public Solitaire {

public:

	War(HWND hWind, int info = 0);
	~War();

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
	BOOL statusUndo(bool status);  //  public just in case
	void delInst();

private:
	void Cleanup();
	PILE* push(PILE *card, int player, bool isPlay);
	PILE* pop(int player, bool isPlay);
	PILE* last(int player, bool isPlay);
	void recycle(int player);
	PILE* click(int player);
	XY1 getXY(int player, int seq, bool isPlay);
	void handelClick();
	void winner(int player, PILE *a, PILE *b);
	int war(PILE *you, PILE* me, XY1 Yxy, XY1 Mxy);
	void grecycle(int player);
	int getRegion(int X, int Y);
	bool findSeq(LOC *loc, int X, int Y);
	PILE* get(int player, int sequence, bool isPlay);

	PILE *play[2], *dis[2];
	int height, width, nplayers;
	bool won, needdraw;
	LOC rmouse;

};

#endif
