//  See header file for why this class exists.

#include "stdafx.h"
#include "dbgdummy.h"
#include "debug.h"
#include "cardtools.h"

//using namespace Gdiplus;

dbgDummy::dbgDummy(HWND hWind, int info) {
	
	exInit(hWnd, info);
	deal();
	
	return;
}

void dbgDummy::leftClick(int X, int Y) {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	xPos = X;
	yPos = Y;
	selected = false;
	
	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::rightClick(int X, int Y) {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	xPos = X;
	yPos = Y;
	selected = true;

	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::rightUp() {
	return;
}

int dbgDummy::load(const char *filename) {
	UNUSED(filename);
	return TRUE;
}

int dbgDummy::save(const char *filename) {
	UNUSED(filename);
	return TRUE;
}

void dbgDummy::redrawBoard() {
	clearBoard(hWnd);
	return;
}

void dbgDummy::undo() {
	return;
}

char* dbgDummy::gameName() {
	return "dbgDummy";
}

void dbgDummy::left() {
	eraseCard(hWnd, xPos, yPos, rot, zoom);	rot--;
	
	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::right() {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	rot++;
	
	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::up() {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	zoom += .01;

	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::down() {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	zoom -= .01;

	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::enter() {
	eraseCard(hWnd, xPos, yPos, rot, zoom);
	card++;

	if(card > LASTCARD)
		card = 1;

	drawCard(hWnd, xPos, yPos, card, rot, selected, zoom);

	return;
}

void dbgDummy::deal() {
	card = 1;
	rot = 0;
	xPos = 0;
	yPos = 0;
	zoom = 1;
	selected = false;

	return;
}

void dbgDummy::exInit(HWND hWnd, int info) {
	Init(hWnd, info);
	return;
}
