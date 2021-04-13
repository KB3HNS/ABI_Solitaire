//  The pyrimid helper class
//  We use a Cards class object for code simplicity

#include "stdafx.h"
#include "pcard.h"
#include "cards.h"
#include "cardtools.h"

PCard::PCard() {
	card = NULL;
	n1 = NULL;
	n2 = NULL;
	p1 = NULL;
	p2 = NULL;
	selected = false;
	return;
}

PCard::PCard(short cardnum) {
	card = NULL;
	n1 = NULL;
	n2 = NULL;
	p1 = NULL;
	p2 = NULL;
	selected = false;
	setVal(cardnum);
	return;
}

PCard::PCard(short cardnum, PCard *prev1, PCard *prev2, PCard *next1, PCard *next2) {
	card = NULL;
	n1 = NULL;
	n2 = NULL;
	p1 = NULL;
	p2 = NULL;
	selected = false;
	setVal(cardnum);
	setNext1(next1);
	setNext2(next2);
	setPrev1(prev1);
	setPrev2(prev2);
	return;
}

PCard* PCard::next1() {

	return n1;
}

PCard* PCard::next2() {

	return n2;
}

PCard* PCard::prev1() {

	return p1;
}

PCard* PCard::prev2() {

	return p2;
}

PCard* PCard::get() {

	return this;
}

void PCard::setNext1(PCard *item) {

	n1 = item;
	if (n1 != NULL)
		n1->setPrev1(this);
	return;
}

void PCard::setNext2(PCard *item) {

	n2 = item;
	if (n2 != NULL)
		n2->setPrev2(this);

	return;
}

void PCard::setPrev1(PCard *item) {

	p1 = item;
	return;
}

void PCard::setPrev2(PCard *item) {

	p2 = item;
	return;
}

void PCard::setVal(short val) {
	if (card != NULL)
		delete card;

	card = new Cards(NULL, val, false);
	return;
}

void PCard::select() {

	selected = !selected;
	return;
}

void PCard::unselect() {

	selected = false;
	return;
}


short PCard::getVal() {

	return card->getVal();
}

short PCard::getSuit() {

	return card->getSuit();
}

short PCard::getNum() {

	return card->getNum();
}

short PCard::getCol() {

	return card->getCol();
}

bool PCard::isSelected() {

	return selected;
}

void PCard::draw(KXY xy, HWND hWnd, KXY ca) {

	drawCard(hWnd, xy.x, xy.y, card->getNum(), 0, selected);
	xy.y += ca.y / 2;
	xy.x -= ca.x * 11 / 20;

	if (n1 != NULL)
		n1->draw(xy, hWnd, ca);

	xy.x += ca.x * 11 / 10;
	if (n2 != NULL)
		n2->draw(xy, hWnd, ca);

	return;
}

PCard::~PCard() {
	
	if (card != NULL)
		delete card;

	if (p1 != NULL)
		p1->setNext1(NULL);

	if (p2 != NULL)
		p2->setNext2(NULL);

	if (n1 != NULL)
		n1->setPrev1(NULL);

	if (n2 != NULL)
		n2->setPrev2(NULL);

	return;
}
