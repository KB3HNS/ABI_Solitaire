//  See header file for why this class exists.

#include "stdafx.h"
#include "dummy.h"
//using namespace Gdiplus;

Dummy::Dummy(HWND hWind, int info) {

	Init(hWnd, info);
	return;
}

void Dummy::leftClick(int X, int Y) {
	UNUSED(X);
	UNUSED(Y);
	return;
}

void Dummy::rightClick(int X, int Y) {
	UNUSED(X);
	UNUSED(Y);
	return;
}

void Dummy::rightUp() {
	return;
}

int Dummy::load(const char *filename) {
	UNUSED(filename);
	return TRUE;
}

int Dummy::save(const char *filename) {
	UNUSED(filename);
	return TRUE;
}

void Dummy::redrawBoard() {
	return;
}

void Dummy::undo() {
	return;
}

char* Dummy::gameName() {
	return "Dummy";
}

void Dummy::left() {
	return;
}

void Dummy::right() {
	return;
}

void Dummy::up() {
	return;
}

void Dummy::down() {
	return;
}

void Dummy::enter() {
	return;
}
