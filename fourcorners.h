//  Four Corners Game Class Header

#ifndef FOURCORNERS_H
#define FOURCORNERS_H

#include "game.h"
#include "cards.h"
#include "cardtools.h"

struct FCLOC {
	short loc;
	short num;
	unsigned short d;
};

struct FCUNDO {
	short loc1;
	short num1;
	short loc2;
	short num2;
	FCUNDO *prev;
};

class FourCorners : public Solitaire {
public:
	FourCorners(HWND hWind, int info = 0);  //  The standard constructor
	~FourCorners();

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
	void dblClick(int X, int Y);
	int winWork(void* datum);


private:
	FCLOC getFCL(int X, int Y);
	KXY FourCorners::getXY(FCLOC fl);
	void calcMove(FCLOC fl);
	Cards* getCard(FCLOC fl);
	void Move2(FCLOC fl);
	void Move3(FCLOC fl);
	void Move4(FCLOC fl);
	unsigned short getD(short loc, short num);
	void fromMove1(Cards *t1);

	Cards *draw, *dis, *to;
	Cards *table[3][3];
	Cards **play[5], **goal[4];
	int startcard, score;
	CARDDIM ca;
	FCLOC sel, kbd;
	bool rdn;
	FCUNDO *Undo;
};

#endif