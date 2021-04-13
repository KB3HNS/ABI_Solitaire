//  Contains code for the empty Solitaire class
//  Most functions are overided by the inhereting class
//  This just provides templates and other interesting things
//  for development purposes.

#include "stdafx.h"
#include <time.h>
#include "colordefs.h"
#include "debug.h"
#include "game.h"
#include "cardtools.h"
#include "winteaser.h"

//using namespace Gdiplus;

Solitaire::Solitaire() {
	//  Bad constructor -- do not use
	//  The system will use this one anyways so it is up to the inheriter
	//  to call the correct one use:
	//  Solitaire::Solitaire(hWind);

	//dprintf("Unsucessfully called Solitaire Constructor!\n");
	return;
}
Solitaire::Solitaire(HWND hWind, int info) {
	//  Initialize the Random Number Generator and exit.
	//  It is up to the caller to shuffel [and deal].
	//  Also, we will clear the card array, and capture our
	//  window handel.
	//  Any inheriter will be required to perform these functions.
	//  Shuffel no longer required for constructor -- moved to deal function

	//   I think that this is platform dependant!!!
	Init(hWind, info);  //   Functionality moved to an Init function for easier child access
	//  I think that this is a VC.net thing

	//dprintf("Sucessfully called Solitaire Constructor! hWind = %i\n", (int)hWind);
	return;
}

void Solitaire::shuffel() {
	//  Impliment a standard 52 card shuffel.
	
	int i, n;
	
	//  clear out the deck  --  moved from constructor
		for (i = 0; i < 54; i++)
		deck[i] = 0;
	
	//  Start n out so first card is random
	n = rand() % 52;
	
	for (i = 1; i < 53; i++) {  //  Bug?  No progress bar - we want as little
								//  windows specific code here as possible
		while (deck[n] != 0)
			n = rand() % 52;
	
		deck[n] = i;	
	}

	return;
}

void Solitaire::deal() {
	return;  //  Inheriter will use the existing deck structure
}

int Solitaire::winWork(void* datum) {
	//  Boaring, but works for debug purposes
	UNUSED(datum);
	dprintf("Congradulations, you won!!!\n");
	bprintf(hWnd, 100, 100, BWHITE, "Congraduations, You Won!!!");
	
	return Win(hWnd, this);
}

long int* Solitaire::winTeaser(void* datum) {
	hit = false;
	
	return threadInit(this, datum);
}

void Solitaire::Init(HWND hWind, int info) {

	srand((unsigned)time( NULL ));

	hWnd = hWind;
	datum = info;
	return;
}

bool Solitaire::keyPress(char key) {
	UNUSED(key);  //  Do nothing, tell the caller that we did so
	return false;
}

void Solitaire::Hit() {
	//  A function that would be able to handel an input to kick off a screen saver
	//  Overridden if needed.

	hit = true;
	return;
}

bool Solitaire::getHit() {
	return hit;
}

void Solitaire::dblClick(int X, int Y) {

	leftClick(X, Y);
	return;
}

void Solitaire::delInst() {
	//  Only called if no one else needed it... hopefully
	return;
}

Solitaire::~Solitaire() { return; }  // empty destructor -- code moved to delInst
