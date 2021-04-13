#ifndef PCARD_H
#define PCARD_H

//  The Pyrimid Card class
//  Used in the pyrimid card game to help a few redundant routines

#include "stdafx.h"
#include "cards.h"

class PCard {
public:
	PCard();
	PCard(short cardnum);
	PCard(short cardnum, PCard *prev1, PCard *prev2, PCard *next1, PCard *next2);
	~PCard();

	PCard* next1();
	PCard* next2();
	PCard* prev1();
	PCard* prev2();
	PCard* get();

	void setNext1(PCard *item);
	void setNext2(PCard *item);
	void setPrev1(PCard *item);
	void setPrev2(PCard *item);
	void setVal(short val);

	void select();
	void unselect();

	short getVal();
	short getSuit();
	short getNum();
	short getCol();
	bool isSelected();

	void draw(KXY xy, HWND hWnd, KXY ca);

private:
	PCard *n1, *n2, *p1, *p2;
	Cards *card;
	bool selected;

};

#endif
