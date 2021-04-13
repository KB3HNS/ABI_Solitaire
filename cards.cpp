//  A simple usable card linked list handler
//  Required for klondike and all newer linked-list based setups.
//  Some day I will probably migrate war to this as well

#include "stdafx.h"
#include "cards.h"
#include "debug.h"
#include <assert.h>

Cards::Cards(Cards *prev, short val, bool acehi) {

	next = NULL;
	aceishi = acehi;
	setVal(val);
	setPrev(prev);
	return;
}

Cards::Cards() {

	previous = NULL;
	next = NULL;
	value = 0;
	aceishi = false;
	return;
}

void Cards::setVal(short val) {
	//  We will get 2 - 14 for 2 - 10, J, Q, K, A for acehi
	//  or we will get 1 - 13 for A, 2 - 10, J, Q, K for acelo
	//  15 denotes a joker
	value = -1;
	sequ = -1;
	suit = -1;
	color = -1;

	if (val == 0)
		return;

	value = val;
	val--;
	suit = val / 13;
	sequ = val % 13;
	
	color = suit % 2;
	suit++;
	color++;
	sequ++;

	if (sequ == 1 && aceishi)
		sequ = ACEHI;

	if (val == 53) {
		//  black joker
		sequ = JOKER;
		suit = WILD;
		color = BCARD;
	}

	if (val == 54) {
		//  red joker
		sequ = JOKER;
		suit = WILD;
		color = RCARD;
	}

	if (val == 55) {
		//  The 'E' Card
		sequ = ECARD;
		suit = ECARD;
		color = ECARD;
	}

	return;
}

short Cards::getVal() {

	return sequ;
}

short Cards::getSuit() {

	return suit;
}

short Cards::getNum() {

	return value;
}

short Cards::getCol() {

	return color;
}

int Cards::getNumElements() {
	int n = 0;
	Cards *cur = this;
	
	while (cur != NULL) {
		cur = cur->Next();
		n++;
	}
	//dprintf("Called Cards::getNumElememts();  I returned: %i.  My value: %i", n, value);
	return n;
}

Cards* Cards::Prev() {

	return previous;
}

Cards* Cards::Next() {

	return next;
}

Cards* Cards::seq(int num) {
	Cards *now = this;

	for (int i = 0; i < num; i++) {
		if (now->Next() == NULL) {
			now = NULL;
			break;
		}

		now = now->Next();
	}

	return now;
}

Cards* Cards::first() {
	Cards *cur = this;
	
	while (cur->Prev() != NULL)
		cur = cur->Prev();
	
	return cur;
}

Cards* Cards::last() {
	Cards *cur = this;

	while (cur->Next() != NULL)
		cur = cur->Next();

	return cur;
}

Cards* Cards::getElement(int num) {
	Cards *cur = this;

	for (int i = 0; i < num; i++) {
		cur = cur->Next();
		if (cur == NULL)
			return NULL;
	}

	return cur;
}

void Cards::setNext(Cards *nxt) {

	next = nxt;
	return;
}

void Cards::setPrev(Cards *prev) {

	previous = prev;
	if (previous != NULL)
		previous->setNext(this);
	
	return;
}

Cards* Cards::pop() {
	Cards *cur = last();
	Cards *l = cur->Prev();

	if (l != NULL)
		l->setNext(NULL);

	cur->setPrev(NULL);

	return cur;
}

Cards* Cards::push(Cards *p) {
	Cards *cur = last();

	assert(p);
	p->setPrev(cur);

	return p;
}

Cards::~Cards() {

	if (previous != NULL)
		previous->setNext(NULL);

	return;
}
