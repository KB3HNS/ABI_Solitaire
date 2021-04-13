//  This is the Streets and Alleys Rule set
//  Here, we draw our board, handel input, and verify moves.

#include "game.h"
#include "stdafx.h"
#include "debug.h"
#include "cardtools.h"
#include "sanda.h"
#include <time.h>
#include <math.h>
#include "colordefs.h"
#include "winteaser.h" 

//using namespace Std;

StreetsAndAlleys::StreetsAndAlleys(HWND hWind, int info) {
	int i, j, k, n;

	//dprintf("Sucessfully called StreetsAndAlleys Constructor! hWind = %i\n", (int)hWind);
	Init(hWind, info);
	
	//  clear the board
	for (i = 0; i < 5; i++)
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				for (n = 0; n < 3; n++)
					table[i][j][k][n] = 0;

	startCard = 0;
	back = NULL;
	
	place.columb = 0;
	place.row = 0;
	place.depth = 0;
	place.region = 0;
	won = true;

	return;
}

void StreetsAndAlleys::deal() {
	int i, j, k, n;
	CARDDIM cardDim = getCardAttributes();
	UNDO *del = back;
	//  clear the board
	for (i = 0; i < 5; i++)
		for (j = 0; j < 3; j++)
			for (k = 0; k < 4; k++)
				for (n = 0; n < 3; n++)
					table[i][j][k][n] = 0;

	n = 0;  //  Reset n which is our "card in deck" counter.
	
	//  Main board (5rows x3columbs x3deep) (zone 1)
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				table[i][j][k][0] = deck[n];
				n++;
			}
		}
	}
	
	//  Free Board (3columbs x3deep) (zone 2)
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 2; j++) {
			table[i][0][j][1] = deck[n];
			n++;
		}
	}

	// n should = 51
	startCard = deck[n];
	table[0][0][0][2] = startCard;  //  Place Board (2columbs x2rows x1deep (last one))
								  //  (zone 3)
	
	//  Deal is effectively our init function.
	//  We need to initialize our key variables here.
	score = 1;
	height = (int)(cardDim.Height * 1.30);  //  rows will want to have a 30% margin
									//  we put this here so it is not recalculated 
									//  1000+ times
	width = cardDim.Width;
	
	selected.columb = 0;
	selected.row = 0;
	selected.depth = 0;
	selected.region = -1;

	loc.columb = 0;
	loc.row = 0;
	loc.columb = 0;
	loc.region = 0;
	
	place.columb = 0;
	place.row = 0;
	place.depth = 3;
	place.region = 0;

	won = false;
	//  Contain a possible memory hole when running multiple games
	while (back != NULL) {
		back = back->previous;
		delete del;
		del = back;
	}

	return;
}

void StreetsAndAlleys::leftClick(int X, int Y) {
	int region = getRegion(X, Y);

	LOCATION card;
	TABLELOC temp;

	switch (region) {
		case 0:
			card = getRegion1(X, Y);
			break;

		case 1:
			card = getRegion2(X, Y);
			break;
			
		case 2:
			card = getRegion3(X, Y);
			break;
	}

	/*eraseCard(hWnd, 800, 600, 0);
	if (card.isValid) {
		drawCard(hWnd, 800, 600, table[card.columb][card.row][card.depth][region],
			0, false);
	}*/

	if (!card.isValid)
		return;

	if (selected.region == -1) {
		selectCard(card, region);
		return;

	} else if (card.columb == selected.columb && card.row == selected.row &&
		card.depth == selected.depth && region == selected.region) {
		temp.columb = card.columb;
		temp.row = card.row;
		temp.depth = card.depth;
		temp.region = region;

		selected.region = -1;
		placeCard(temp);
		return;
	}
	
	
	//  We have a valid selected card and space
	if (moveIsValid(card, region))
		makeMove(card, region);

	return;
}

void StreetsAndAlleys::rightClick(int X, int Y) {

	int region = getRegion(X, Y);
	LOCATION card;
	if (won)  //  don't do anything if the game has been won
		return;
	
	switch (region) {
		case 0:
			card = getRegion1(X, Y);
			break;

		case 1:
			card = getRegion2(X, Y);
			break;
			
		case 2:
			card = getRegion3(X, Y);
			break;
	}

	loc.columb = card.columb;
	loc.row = card.row;
	loc.depth = card.depth;
	loc.region = region;

	if (card.isValid) 
		placeCard(loc);

	return;
}

int StreetsAndAlleys::load(const char *filename) {
	FILE *file = fopen(filename, "rb");
	int i, j, k, region;
	UNDO *del = NULL, *temp = NULL;
	char name[256];

	ZeroMemory(name, 256);
	if (file == NULL)
		return FALSE;
	
	i = fgetc(file);
	if (i == EOF) {
		fclose(file);
		return i;
	}

	i = fgetc(file);
	if (i == EOF) {
		fclose(file);
		return i;
	}

	if (fgets(name, i, file) == NULL) {
		fclose(file);
		return EOF;
	}

	if (strcmp(name, gameName()) != 0) {
		fclose(file);
		return INVAL_GAME_TYPE;
	}

	//  There should not be any errors after this as we have validated the file
	//  First, clear our game
	deal();

	for(i = 0; i < 52; i++)
		deck[i] = fgetc(file);

	for (region = 0; region < 3; region++)
		for (i = 0; i < 5; i++)
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					table[i][j][k][region] = fgetc(file);


	
	j = fgetc(file); //  number of undo structures
	for (i = 0; i < j; i++) {
		del = new UNDO;
		del->source.columb = fgetc(file);
		del->source.row = fgetc(file);
		del->source.depth = fgetc(file);
		del->source.region = fgetc(file);
		del->dest.columb = fgetc(file);
		del->dest.row = fgetc(file);
		del->dest.depth = fgetc(file);
		del->dest.region = fgetc(file);
		
		del->previous = temp;
		temp = del;
	}
	
	temp = NULL;
	back = NULL;
	//  Now we need to swap the structure
	while (del != NULL) {
	back = new UNDO;
	back->previous = temp;
	back->source.columb = del->source.columb;
	back->source.row = del->source.row;
	back->source.depth = del->source.depth;
	back->source.region = del->source.region;
	back->dest.columb = del->dest.columb;
	back->dest.row = del->dest.row;
	back->dest.depth = del->dest.depth;
	back->dest.region = del->dest.region;
	temp = del;
	del = del->previous;
	delete temp;
	temp = back;
	}

	startCard = fgetc(file);
	score = fgetc(file);

	i = fgetc(file);
	if (i != 255) {
		//  Something terribly wrong happened
		shuffel();
		deal();
		redrawBoard();
		fclose(file);
		return i;
	}

	redrawBoard();
	fclose(file);
	return TRUE;
}

int StreetsAndAlleys::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i, j, k, region;
	UNDO *del = back;
	if (won) {  //  don't do anything if the game has been won
		fclose(file);	
		return EOF;
	}
	if (file == NULL)
		return FALSE;

	//  Generic solitaire header
	fputc(datum, file);

	//  Streets and Alleys Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	52; i++)
		fputc(deck[i], file);

	for (region = 0; region < 3; region++)
		for (i = 0; i < 5; i++)
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					fputc(table[i][j][k][region], file);

	i = 0;
	while (del != NULL) {
		del = del->previous;
		i++;
	}
	fputc(i, file);  //  Number of undo structures
	
	del = back;
	while (del != NULL) {
		fputc(del->source.columb, file);
		fputc(del->source.row, file);
		fputc(del->source.depth, file);
		fputc(del->source.region, file);
		fputc(del->dest.columb, file);
		fputc(del->dest.row, file);
		fputc(del->dest.depth, file);
		fputc(del->dest.region, file);

		del = del->previous;
	}

	fputc(startCard, file);
	fputc(score, file);

	fputc(255, file);  //  Last data to close the file

	i = fclose(file);
    if (i != 0)
		return i;

	return TRUE;
}

void StreetsAndAlleys::redrawBoard() {

	int i, j, k, x, y;
	TABLELOC card;

	if (won)  //  don't do anything if the game has been won
		return;

	clearBoard(hWnd);

	card.region = 0;
	// Main Board
	for (i = 0; i < 5; i++) {
		card.columb = i;
		for (j = 0; j < 3; j++) {
			card.row = j;
			for (k = 0; k < 3; k++) {
				card.depth = k;
				placeCard(card);
			}
		}
	}

	//  Free Board
	card.region = 1;
	card.row = 0;
	for (i = 0; i < 3; i++) {
		card.columb = i;
		for (j = 0; j < 3; j++) {
			card.depth = j;
			placeCard(card);
		}
	}

	//  Place Board
	card.region = 2;
	card.depth = 0;
	for (i = 0; i < 2; i++) {
		card.columb = i;
		for (j = 0; j < 2; j++) {
			card.row = j;
			placeCard(card);
	
		}
	}

	//  Show Start Card, draw board text
	if (startCard != 0) {
		x = width * 8;
		y = (int)(height * 3.7);
		bprintf(hWnd, x, y, BWHITE, "Start Card:");
		y += height / 3;
		bprintf(hWnd, x, y, BWHITE, "Score: %i.", score);
		x += 100;
		y = height * 4;
		drawCard(hWnd, x, y, startCard, 0, false);
	}

	return;
}

void StreetsAndAlleys::undo() {
	UNDO *thisMove = back;
	TABLELOC temp;

	int i, j, k, i1, j1, k1, region, region1, card, x, y;
	if (won)  //  don't do anything if the game has been won
		return;

	if (selected.region != -1) {
		temp.columb = selected.columb;
		temp.row = selected.row;
		temp.depth = selected.depth;
		temp.region = selected.region;
		selected.region = -1;
		placeCard(temp);
	}

	if (back == NULL)
		return;

	back = back->previous;
	i = thisMove->source.columb;
	j = thisMove->source.row;
	k = thisMove->source.depth;
	region = thisMove->source.region;
	i1 = thisMove->dest.columb;
	j1 = thisMove->dest.row;
	k1 = thisMove->dest.depth;
	region1 = thisMove->dest.region;

	removeCard(thisMove->dest);
	
	card = table[i1][j1][k1][region1];
	table[i][j][k][region] = card;
	card--;

	placeCard(thisMove->source);
	table[i1][j1][k1][region1] = 0; 

	if (k1 != 0) {
		temp.columb = i1;
		temp.row = j1;
		temp.depth = k1 - 1;
		temp.region = region1;
		placeCard(temp);
	}

	if (region1 == 2) {
		score--;
		x = width * 8;
		y = (int)(height * 3.7);
		y += height / 3;
		bprintf(hWnd, x, y, BWHITE, "Score: %i.", score);
		
		
		if (card % 13 == 0)
			card += 13;

		card--;

		/*bprintf(hWnd, 0, 0, BWHITE,
			"card = %i, startcard r13 = %i, card r13 = %i, i = %i, j = %i", 
			card, (startCard - 1) % 13, (card + 1) % 13, i1, j1);*/

		if ((card + 1) % 13 != (startCard  - 1) % 13)
			table[i1][j1][0][2] = card + 1;
		placeCard(thisMove->dest);
	}

	delete thisMove;

	return;
}

int StreetsAndAlleys::getRegion(int X, int Y) {

	//  What Board region are we in?
	//  Based on maximum card locations
	if (Y > (int)(3.3 * height))  //  Average between 2.75 and 4
		return 1;
	if (X > (int)(10.3 * width))  //  Average between 9.5 and 11
		return 2;

	return 0;  //  Main board is everything else

}

LOCATION StreetsAndAlleys::getRegion1(int X, int Y) {
	LOCATION card;
	const int lheight = (int)(height / 1.3);
	int x, y;

	card.isValid = false;
	card.row = 1;	
	card.columb = 0;

	if (Y < (int)(1.25 * height))
		card.row = 0;

	if (Y > (int)(2.25 * height))
		card.row = 2;

	// 1.5 2.8
	if (X > (int)(2.15 * width))
		card.columb = 1;

	//3.5 4.8
	if (X > (int)(4.15 * width))
		card.columb = 2;
	
	//5.5 6.8
	if (X > (int)(6.15 * width))
		card.columb = 3;

	if (X > (int)(8.15 * width))
		card.columb = 4;

	//bprintf(hWnd, 0, 0, BWHITE,
	//	"Click in Region 1 Subregion %i, %i", card.row, card.columb);
	
	x = width / 3;
	x += width * 2 * card.columb;
	x += width / 2;

	y = height * card.row;
	y += (int)(height * .75);

	card.depth = 0;
	if ( isValid(x, y, X, Y, 345) ) 
		card.isValid = true;
		

	//  base of row is now determined, now we check for selecting additional cards
	if (table[card.columb][card.row][1][0] != 0) {

		x = width * 2 / 3;
		x += width * 2 * (card.columb);
		
		y -= lheight / 2;
		//bprintf(hWnd, x, y, BWHITE, "Processing...");
		if (X > x && X < x + width && Y > y && Y < y + lheight) {
			card.depth = 1;
			card.isValid = true;
			//bprintf(hWnd, x, y, BWHITE,
			//	"Click in Region 1 Subregion %i, %i, depth = 1",
			//	card.row, card.columb);
		
		}
		y += lheight / 2;
		
	}

	if (table[card.columb][card.row][2][0] != 0) {
		x = width * 2 * (card.columb);
		x += (int)(width * 1.5);
		
		if ( isValid(x, y, X, Y, 15) ) {
			card.depth = 2;
			card.isValid = true;
		
			//bprintf(hWnd, x, y, BWHITE,
			//	"Click in Region 1 Subregion %i, %i, depth = 2",
			//	card.row, card.columb);
					
		}

	}

	return card;
}

LOCATION StreetsAndAlleys::getRegion2(int X, int Y) {
	LOCATION card;
	const int lheight = (int)(height / 1.3);
	int region = 2;
	int x, y;
	
	card.isValid = false;
	card.row = 0;

	if (X < (int)(width * 3.15))
		region = 1;

	if (X > (int)(width * 5.15))
		region = 3;

	card.columb = region - 1;

	x = width / 3;
	x += width * 2 * (region - 1);
	x += (int)(width * 1.5);
	y = height * 4;	
	
	card.depth = 0;
	if ( isValid(x, y, X, Y, 345) ) 
		card.isValid = true;
	

	//  base of row is now determined, now we check for selecting additional cards
	if (table[card.columb][0][1][1] != 0) {
		x = width * 2 / 3;
		x += width * 2 * (region - 1);
		x += (int)(width * 1.5);
		x -= width / 2;
		y = height * 4;	
		y -= lheight / 2;

			
		if (X > x && X < x + width && Y > y && Y < y + lheight) {
			card.depth = 1;
			card.isValid = true;
		
			//bprintf(hWnd, x, y, BWHITE,
			//	"Click in Region 2 Subregion %i, card height = %i, card width = %i, depth = 1",
			//	region, lheight, width);

		}

	}

	if (table[card.columb][0][2][1] != 0) {
		x = width * 2 * (region - 1);
		x += (int)(width * 2.5);
		y = height * 4;	
		if ( isValid(x, y, X, Y, 15) ) {
			card.depth = 2;
			card.isValid = true;
		
			//bprintf(hWnd, x, y, BWHITE,
			//	"Click in Region 2 Subregion %i, card height = %i, card width = %i, depth = 2",
			//	region, lheight, width);
					
		}

	}

	return card;
}

LOCATION StreetsAndAlleys::getRegion3(int X, int Y) {
	LOCATION card;
	CARDDIM cardDim = getCardAttributes();
	const int cheight = cardDim.Height / 2;

	card.depth = 0;
	card.isValid = false;
	
	if (X > (int)(width * 10.5) && X < (int)(width * 11.5)) {
		card.columb = 0;
		//bprintf(hWnd, 0, 0, BWHITE, "card.columb = 1");
		if (Y > (int)(height * .75 - cheight) && Y < (int)(height * .75 + cheight)) {
			card.row = 0;
			card.isValid = true;
			//bprintf(hWnd, 0, 15, BWHITE, "card.row = 1");
		}
	
		if (Y > (int)(height * 2.25 - cheight) && Y < (int)(height * 2.25 + cheight)) {
			card.row = 1;
			card.isValid = true;
			//bprintf(hWnd, 0, 15, BWHITE, "card.row = 2");
		}
	}

	if (X > (int)(width * 12.5) && X < (int)(width * 13.5)) {
		card.columb = 1;
		//bprintf(hWnd, 0, 0, BWHITE, "card.columb = 2");
		if (Y > (int)(height * .75 - cheight) && Y < (int)(height * .75 + cheight)) {
			card.row = 0;
			card.isValid = true;
			//bprintf(hWnd, 0, 15, BWHITE, "card.row = 1");
		}
	
		if (Y > (int)(height * 2.25 - cheight) && Y < (int)(height * 2.25 + cheight)) {
			card.row = 1;
			card.isValid = true;
			//bprintf(hWnd, 0, 15, BWHITE, "card.row = 2");
		}
	}

	return card;
}

bool StreetsAndAlleys::isValid(int x, int y, int X, int Y, int rotation) {
	//  Determine weather or not Y is within the region bounded by the card
	//  I know that this works with +/- 15deg -- don't know about anything else

	const int lheight = (int)(height / 1.3);
	double m, m1, rad, B, B1;
	int a, b, c, d;
	
	while (rotation > 360)
		rotation-=360;

	while (rotation < 1)
		rotation += 360;

	rad = DegtoRad(rotation);
	a = lheight / 2;
	b = width / 2;
	c = (int)((-cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (sin(rad) * a));
	//  Corner 4 of card
	y += c;
	x += d;

	c = (int)((cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (-sin(rad) * a));
	//  corner 3 of card

	m = tan(rad);
	m1 = -1 / m;
	B = (c * 2) - (m * d * 2);
	B1 = (c * 2) - (m1 * d * 2);
				
	X -= x;
	Y -= y;
//	bprintf(hWnd, x, y, BWHITE, "Processing...");

	if ( Y > m * X && Y < (m * X) + B && Y > m1 * X && Y < (m1 * X) + B1
		&& rotation < 180) 
		return true;

	if ( Y > m * X && Y < (m * X) + B && Y < m1 * X && Y > (m1 * X) + B1
		&& rotation > 180) 
		return true;

	return false;
}

void StreetsAndAlleys::placeCard(TABLELOC card) {
// Main Board
	int i = card.columb;
	int j = card.row;
	int k = card.depth;
	int x, y, n = 0, value = table[i][j][k][card.region];
	bool isSelected;

	switch (card.region) {
		case 0:

			switch (card.depth) {
			
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
				
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += width / 2;

			y = height * j;
			y += (int)(height * .75);

			break;
	
		//  Free Board
		case 1:

			switch (k) {
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
			
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += (int)(width * 1.5);
	
			y = height * 4;

			break;

	
		//  Place Board
		case 2:
			x = width * 11;
			x += width * 2 * i;
			y = (int)(height * .75);
			y += (int)(height * 1.5 * j);

			break;

	}

	isSelected = (card.columb == selected.columb && card.row == selected.row &&
		card.depth == selected.depth && card.region == selected.region);
	
	if (value == 0 && (card.region == 2 || (card.region == 1 && k == 0))) {
		value = E;  //  Print the "E" card so users know where empty areas are
		isSelected = false;
	}

	drawCard(hWnd, x, y, value, n, isSelected);
	
	return;
}

void StreetsAndAlleys::rightUp() {
	
	int i;
	TABLELOC maybe;
	if (won)  //  don't do anything if the game has been won
		return;

	maybe.columb = loc.columb;
	maybe.row = loc.row;
	maybe.region = loc.region;
	
	for (i = loc.depth; i < 3; i++) {
		maybe.depth = i;
		placeCard(maybe);
	}
	
	return;
}

void StreetsAndAlleys::selectCard(LOCATION card, int region) {

	int i = card.columb, j = card.row, k = card.depth;
	
	//  Card must exist and be at the top of the stack
	if (card.isValid && table[i][j][k][region] != 0  &&
		(k + 1 == 3 || table[i][j][k + 1][region] == 0)) {
	
		if (region == 2)  //  can not select a card in the place pile
			return;
	
		selected.columb = i;
		selected.row = j;
		selected.depth = k;
		selected.region = region;
		
		//bprintf(hWnd, 0, 0, BWHITE, "Selecting: %i, %i, %i, %i", i, j, k, region);
	
		placeCard(selected);
	}

	return;
}

bool StreetsAndAlleys::moveIsValid(LOCATION card, int region) {
	int i = card.columb, j = card.row, k = card.depth;
	int i1 = selected.columb, j1 = selected.row, k1 = selected.depth;
	int cardNum = table[i1][j1][k1][selected.region] - 1;
	int series  = cardNum % 13, suit = cardNum / 13;
	int destNum = table[i][j][k][region] - 1;
	int destSeries = destNum % 13, destSuit = destNum / 13;
	int isRed = suit % 2, destIsRed = destSuit % 2;
	//  suit should be 0 - 4 for Spades, Diamonds, Clubs, Hearts
	//  series should be 0 - 13 for A, 2-9, J, Q, K 
	int a, b, c;  // Some temporary use integers
	
	/*bprintf(hWnd, 0, 0, BWHITE, 
		"Calc: Source: i = %i, j = %i, k = %i, cardNum = %i, series = %i, suit = %i, isRed = %i", 
		i1, j1, k1, cardNum, series, suit, isRed);
	
	bprintf(hWnd, 0, 15, BWHITE, 
		"Dest: i = %i, j = %i, k = %i, cardNum = %i, series = %i, suit = %i, isRed = %i", 
		i, j, k, destNum, destSeries, destSuit, destIsRed);*/
		
	
	//  First .. some sanity checks
	if (k == 2)
		return false;  //  can not place on a full stack

	if (k == 0 && region == 0 && destNum == -1)
		return false;  //  can not place on a stack in the main board
						//  that has been depleted
	
	a = destSeries - 1;
	if (a < 0)
		a = 12;
	
	b = destSeries + 1;
	if (b > 12)
		b = 0;

	c = (startCard - 1) % 13;  //  the series of the start card
						//  this only works if you assume that there are
						//  no duplicate cards... which there shouldn't be
	//bprintf(hWnd, 500, 0, BWHITE, "a = %i, b = %i, c = %i", a, b, c);

	switch (region) {
		case 0:

			if (destIsRed != isRed && series == a)
				return true;  //  Place in decending order, alternating red/black
			
			break;

		case 1:
			if (k == 0 && destNum == -1)
				return true;  //  Anything can start a new stack in the free zone

			if (destIsRed != isRed && (series == a || series == b))
				return true; //  Place in acending or decending order
							//  alternating red/black in the place zone

			break;

		case 2:
			
			if (suit == destSuit && series == b)
				return true;  //  Place in ascending order in same suit for 
								//  placing area

			if (destNum == -1 && series == c)
				return true;  //  start a new pile in the place area if the
							//  selected card is the same in the series as the
							//  start card
			break;
	}

	return false;  // if we have gotten here, it is not a legal move
}

void StreetsAndAlleys::makeMove(LOCATION card, int region) {
	int i = selected.columb, j = selected.row, k = selected.depth;
	int i1 = card.columb, j1 = card.row, k1 = card.depth;
	int cardNum = table[i][j][k][selected.region];
	int newNum = table[i1][j1][k1][region];
	int x, y;
	TABLELOC temp;
	UNDO *newMove;

	if (back != NULL) {
		 newMove = back;
		 back = new UNDO;
		 back->previous = newMove;
	} else {
		back = new UNDO;
		back->previous = NULL;
	}
	
	back->source.columb = i;
	back->source.row = j;
	back->source.depth = k;
	back->source.region = selected.region;
	
	// first erase the origin card
	removeCard(selected);
	table[i][j][k][selected.region] = 0;
	//if (k == 0 && selected.region == 1)
	//	placeCard(selected);
	

	if (k != 0) {
		temp.columb = i;
		temp.row = j;
		temp.depth = k - 1;
		temp.region = selected.region;
		placeCard(temp);
	}

	if (newNum != 0 && region != 2) 
		k1++;

	table[i1][j1][k1][region] = cardNum;
	back->dest.columb = i1;
	back->dest.row = j1;
	back->dest.depth = k1;
	back->dest.region = region;

	temp.columb = i1;
	temp.row = j1;
	temp.depth = k1;
	temp.region = region;
	placeCard(temp);

	//  save undo data

	selected.region = -1;
	selected.columb = 0;
	selected.row = 0;
	selected.depth = 0;

	if ( region == 2) {
		score++;
		x = width * 8;
		y = (int)(height * 3.7);
		y += height / 3;
		bprintf(hWnd, x, y, BWHITE, "Score: %i.", score);
	
		if (score == 52)
			winTeaser();

	}
	return;
}

void StreetsAndAlleys::removeCard(TABLELOC card) {
// Main Board
	int i = card.columb;
	int j = card.row;
	int k = card.depth;
	int x, y, n = 0;

	switch (card.region) {
		case 0:

			switch (k) {
			
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
				
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += width / 2;

			y = height * j;
			y += (int)(height * .75);

			break;
	
		//  Free Board
		case 1:

			switch (k) {
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
			
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += (int)(width * 1.5);
	
			y = height * 4;

			break;

	
		//  Place Board
		case 2:
			x = width * 11;
			x += width * 2 * i;
			y = (int)(height * .75);
			y += (int)(height * 1.5 * j);

			break;

	}

	eraseCard(hWnd, x, y, n);
	if (k == 0 && card.region == 1)
		drawCard(hWnd, x, y, E, n, false);

	return;
}

char* StreetsAndAlleys::gameName() {
	return "Streets and Alleys";
}
void StreetsAndAlleys::right() {
	XY xy;
	if (won)  //  don't do anything if the game has been won
		return;

	place.columb++;
	switch (place.region) {

		place.columb++;

		case 0:
			if (place.columb > 4) {
				place.columb = 0;
				place.row = 0;
				place.depth = 0;
				place.region = 2;
			} else {
				place.depth = 0;
				while (table[place.columb][place.row][place.depth][0] != 0
					&& place.depth < 3)
					place.depth++;

				if (place.depth > 0)
					place.depth--;

			}

			break;

		case 1:

			if (place.columb > 2) {
				place.columb = 0;
				place.row = 1;
				place.depth = 0;
				place.region = 2;
			} else {
				place.depth = 0;
				while (table[place.columb][place.row][place.depth][1] != 0 &&
					place.depth < 3)
					place.depth++;

				if (place.depth > 0)
					place.depth--;

			}

			break;

		case 2:
			if (place.columb > 1)
				place.columb = 1;
			break;
	}

	xy = getXY(place);
	putMouse(xy.x, xy.y, hWnd);

	return;
}
void StreetsAndAlleys::left() {
	XY xy;
	if (won)  //  don't do anything if the game has been won
		return;

	place.columb--;
	place.depth = 0;

	if (place.columb < 0 && place.region == 2) {
		place.region = place.row;
		if (place.region == 0) {
            place.row = 1;
			place.columb = 4;
		} else {
			place.row = 0;
			place.columb = 2;
		}
	}

	if (place.columb < 0)
		place.columb = 0;
	
	while (table[place.columb][place.row][place.depth][place.region] != 0 &&
		place.depth < 3)
		place.depth++;

	if (place.depth > 0)
		place.depth--;
	
	xy = getXY(place);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void StreetsAndAlleys::up() {
	XY xy;
	if (won)  //  don't do anything if the game has been won
		return;

	place.row--;
	place.depth = 0;
	
	if (place.row < 0 && place.region == 1) {
		place.region = 0;
		place.columb++;
		place.row = 2;
	}

	if (place.row < 0)
		place.row = 0;

	while (table[place.columb][place.row][place.depth][place.region] != 0 &&
		place.depth < 3)
		place.depth++;

	if (place.depth > 0)
		place.depth--;

	xy = getXY(place);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void StreetsAndAlleys::down() {
	XY xy;
	place.row++;
	if (won)  //  don't do anything if the game has been won
		return;

	switch (place.region) {
		case 0:
			place.depth = 0;
			if (place.row > 2) {
				int t = place.columb;
				place.row = 0;
				place.region = 1;
				place.columb = 1;
				if (t < 2)
					place.columb = 0;

				if (t > 2)
					place.columb = 2;
			}

			while (table[place.columb][place.row][place.depth][place.region] != 0 &&
				place.depth < 3)
				place.depth++;

			if (place.depth > 0)
				place.depth--;

			break;

		case 1:
			place.row = 0;
			break;

		case 2:
			if (place.row > 1)
				place.row = 1;

			break;
	}

	xy = getXY(place);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void StreetsAndAlleys::enter() {
	XY xy = getXY(place);
	if (won)  //  don't do anything if the game has been won
		return;

	leftClick(xy.x, xy.y);

	xy = getXY(place);
	putMouse(xy.x, xy.y, hWnd);

	return;

}

XY StreetsAndAlleys::getXY(TABLELOC card) {
	XY xy;
	int i = card.columb;
	int j = card.row;
	int k = card.depth;
	int x, y, n = 0;

	switch (card.region) {
		case 0:

			switch (card.depth) {
			
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
				
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += width / 2;

			y = height * j;
			y += (int)(height * .75);

			break;
	
		//  Free Board
		case 1:

			switch (k) {
				case 0:
					n = -15;
					break;
				case 1:
					n = 0;
					break;
				case 2:
					n = 15;
					break;
			}
			
			x = width / 3;
			x *= k + 1;
			x += width * 2 * i;
			x += (int)(width * 1.5);
	
			y = height * 4;

			break;

	
		//  Place Board
		case 2:
			x = width * 11;
			x += width * 2 * i;
			y = (int)(height * .75);
			y += (int)(height * 1.5 * j);

			break;

	}
	xy.x = x;
	xy.y = y;
	xy.rot = n;

	return xy;
}

int StreetsAndAlleys::winWork(void* datum) {
	//  function called as the worker thread after win and not otherwise	
	XY sxy[4], dxy[4];
	TABLELOC card[4];
	int value[4], start[4], lvalue[4];
	//  v = velocity, l = local(falling), p = permanent(place), t = angular velocity
	int i, j, vy[4], lx[4], ly[4], lz[4], vz[4], vt[4], vx[4], lz1[4];
	const int steps = 100, gy = 1, gz = -1, zmin = -100;
	float dx[4], dy[4], px[4], py[4];
	long signed wait[4];
	bool done = false, ldone[4];
	
	UNUSED(datum);
	dprintf("Thread sucessully spawned!");
	
	card[0].columb = 0;
	card[0].row = 0;
	card[0].depth = 0;
	card[0].region = 2;
	
	card[1].columb = 0;
	card[1].row = 1;
	card[1].depth = 0;
	card[1].region = 2;
	
	card[2].columb = 1;
	card[2].row = 1;
	card[2].depth = 0;
	card[2].region = 2;
	
	card[3].columb = 1;
	card[3].row = 0;
	card[3].depth = 0;
	card[3].region = 2;
	
	for (i = 0; i < 4; i++) {
		sxy[i] = getXY(card[i]);
		value[i] = table[card[i].columb][card[i].row][0][2];
		start[i] = value[i];
		lvalue[i] = value[i];
	}

	j = sxy[3].x;
	dxy[3].x = j;
	dxy[3].y = sxy[3].y;
	dxy[3].rot = 0;	
	j /= 4;

	for (i = 0; i < 3; i++) {
		dxy[i].x = j * (i + 1);
		dxy[i].y = sxy[3].y;
		dxy[i].rot = 0;
	}
	
	Sleep(30);  //allow time for threads to synchronize
	//  it is concievable that on a multiprocessor system, hit
	//  could be set to true by the same event that called win before we get here
	//  this ensures that it does not.
	
	//  Initialize all local values
	for (i = 0; i < 4; i++) {
		dx[i] = (float)(dxy[i].x - sxy[i].x) / steps;
		dy[i] = (float)(dxy[i].y - sxy[i].y) / steps;
		px[i] = (float)sxy[i].x;
		py[i] = (float)sxy[i].y;
		lx[i] = dxy[i].x;
		ly[i] = dxy[i].y;
		lz[i] = zmin;
		lz1[i] = zmin;
		vy[i] = 0;
		vz[i] = 0;		
		wait[i] = steps * i / 10;
		ldone[i] = false;
		vt[i] = 0;
		vx[i] = 0;
	}
	hit = false;
	won = true;
	clearBoard(hWnd);
	
	//  Move cards to places
	j = 0;
	while (j < steps && !hit) {
		Sleep(30);  //  limit to roughly 30 fps
		for(i = 0; i < 4; i++) {
			eraseCard(hWnd, (int)px[i], (int)py[i], 0);
			px[i] += dx[i];
			py[i] += dy[i];
			drawCard(hWnd, (int)px[i], (int)py[i], value[i], 0, false);
		}
		j++;
	} // Wend
	
	//  now set card values to 0 base:
	for (i = 0; i < 4; i++)
		value[i]--;

	//  falling animation
	while (!done && !hit) {
		Sleep(60);
		done = true;
		// erase
		for (i = 0; i < 4; i++) {
			eraseCard(hWnd, lx[i], ly[i], dxy[i].rot, axis2zoom(lz[i]));
			if (!ldone[i] && value[i] != LASTCARD)
				drawCard(hWnd, (int)px[i], (int)py[i], value[i] + 1, 0, false, 1);
			else
				eraseCard(hWnd, (int)px[i], (int)py[i], 0, 1);
		}
		
		// calculate
		for (i = 0; i < 4; i++) {
			wait[i]--;
			if (wait[i] < 0 && !ldone[i]) {
				vy[i] += gy;
				vz[i] += gz;
				dxy[i].rot += vt[i];
				lx[i] += vx[i];
				ly[i] += vy[i];
				lz[i] += vz[i];

				if (lz[i] < zmin) { //  time for a new card.
					lz[i] = 0;
					ly[i] = (int)py[i];
					lx[i] = (int)px[i];

					vy[i] = 0;
					vz[i] = (rand() % 6) + 5;
					vt[i] = rand() % 16;
					vx[i] = (rand() %11) - 5;
					
					if (value[i] == LASTCARD)
						ldone[i] = true;
					
					lvalue[i] = value[i];
					if (value[i] % 13 == 0)
						value[i] += 13;
					value[i]--;

					if (value[i] + 1 == start[i])  
						value[i] = LASTCARD;

				} //endif (lz[i] < 10)
				lz1[i] = lz[i];

			} //endif (wait[i] < 0 && !ldone[i])
			done &= ldone[i];			

		} // for
		
		// draw
		hsort(lz, lz1, 4);
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				if (lz[i] == lz1[j] && !ldone[i] && lz[i] > zmin)
					drawCard(hWnd, lx[i], ly[i], lvalue[i] + 1, dxy[i].rot,
						false, axis2zoom(lz[i]));
        
	}// wend
		
	if (hit)
		clearBoard(hWnd);
	
	return Win(hWnd, static_cast<Solitaire*>(this));
}

void StreetsAndAlleys::dblClick(int X, int Y) {

	int region = getRegion(X, Y), cnum;
	LOCATION card;
	TABLELOC tloc, dloc;
	XY xy;

	if (won)
		return;

	switch (region) {
		case 0:
			card = getRegion1(X, Y);
			break;

		case 1:
			card = getRegion2(X, Y);
			break;
			
		case 2:
			card = getRegion3(X, Y);
			break;
	}
	
	if (!card.isValid)
		return;

	if (card.columb == selected.columb && card.row == selected.row &&
		card.depth == selected.depth && region == selected.region) {
		tloc.columb = card.columb;
		tloc.row = card.row;
		tloc.depth = card.depth;
		tloc.region = region;
		selected.region = -1;
		placeCard(tloc);
	}
	
	if (selected.region != -1) {
		leftClick(X, Y);
		return;
	}

	tloc.columb = card.columb;
	tloc.depth = card.depth;
	tloc.row = card.row;
	tloc.region = region;
	dloc.columb = 0;
	dloc.row = 0;
	dloc.depth = 0;
	dloc.region = -1;
	
	cnum = table[tloc.columb][tloc.row][tloc.depth][tloc.region];

	if (cnum == 0)
		return;

	if (cnum % 13 == startCard % 13) {
		for (int i = 1; i >= 0; i--)
			for (int j = 0; j < 2; j++)
				if (table[j][i][0][2] == 0) {
					dloc.columb = j;
					dloc.row = i;
					dloc.depth = 0;
					dloc.region = 2;
					break;
				}

		selectCard(card, region);

		xy = getXY(dloc);
		leftClick(xy.x, xy.y);
	}  //  if (cnum % 13 == 1)

	cnum--;
	if (cnum % 13 < 1)
		cnum += 13;

	for (int i = 0; i < 4; i++)
		if (table[i % 2][i / 2][0][2] == cnum) {
			dloc.columb = i % 2;
			dloc.row = i / 2;
			dloc.depth = 0;
			dloc.region = 2;
		}
	
	if (dloc.region != 2)
		return;


	selectCard(card, region);

	xy = getXY(dloc);
	leftClick(xy.x, xy.y);

	return;
}

void StreetsAndAlleys::delInst() {
	UNDO *del = back;
	
	while (back != NULL) {
		back = back->previous;
		delete del;
		del = back;
	}

	return;
}

StreetsAndAlleys::~StreetsAndAlleys() {
	UNDO *del = back;
	
	while (back != NULL) {
		back = back->previous;
		delete del;
		del = back;
	}
	return;
}
