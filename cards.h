#ifndef CARDS_H
#define CARDS_H

#define SPADES 1
#define DIAMONDS 2
#define CLUBS 3
#define HEARTS 4
#define WILD 5

#define JACK 11
#define QUEEN 12
#define KING 13
#define ACEHI 14
#define ACELO 1
#define JOKER 15
#define ECARD -1

#define BCARD 1
#define RCARD 2

struct KXY {
	int x;
	int y;
};

class Cards {
public:
	Cards(Cards *prev, short val = 0, bool acehi = false);
	Cards();
	~Cards();

	void setVal(short val);
	short getVal();
	short getSuit();
	short getNum();
	short getCol();
	int getNumElements();

	Cards* Prev();
	Cards* Next();
	Cards* seq(int num);
	Cards* first();
	Cards* last();
	Cards* getElement(int num);
	Cards* pop();
	Cards* push(Cards *p);
	void setNext(Cards *nxt);
	void setPrev(Cards *prev);

private:
	short value, sequ, suit, color;
	Cards *next, *previous;
	bool aceishi;
};

#endif
