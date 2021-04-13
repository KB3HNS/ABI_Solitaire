//  The calculation game code core.
//  Calculation is the second game in the Aunt Annie's tour.
//  Well, I don't think that it's the second game,
//  but it's the second one that I will write.

#include "stdafx.h"
#include "cards.h"
#include "calculation.h"
#include "debug.h"
#include "cardtools.h"
#include "winteaser.h"
#include "colordefs.h"
#include <time.h>
#include <math.h>

Calculation::Calculation(HWND hWind, int info) {

	Init(hWind, info);
	for (int i = 0; i < 4; i++) {
		board[i] = NULL;
		top[i] = NULL;
	}
	to = NULL;
	pile = NULL;
	Undo = NULL;
	score = 0;
	sel.loc = 0;
	sel.row = 0;
	sel.depth = 0;
	rm.loc = 0;
	rm.row = 0;
	rm.depth = 0;
	kbd.loc = 0;
	kbd.row = 0;
	kbd.depth = 0;
	ca = getCardAttributes();

	return;
}

void Calculation::leftClick(int X, int Y) {

	KXY xy;
	Cards *tmp;
	CALLOC cl = getCL(X, Y);
	CALUNDO *tu;

	if (score == 52)
		return;

	if (sel.loc != 0) {
		if (to == NULL && pile != NULL && cl.loc == 1) {
			to = pile->pop();
			if (to == pile)
				pile = NULL;
			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
			tu = Undo;
			Undo = new CALUNDO;
			Undo->loc1 = 1;
			Undo->loc2 = 1;
			Undo->row1 = 0;
			Undo->row2 = 0;
			Undo->prev = tu;
		} else
			calcMove(cl);

		return;
	}  //  if (sel.loc != 0)

	switch (cl.loc) {
		case 1:
			if (to == NULL && pile != NULL) {
				to = pile->pop();
				if (to == pile)
					pile = NULL;
				drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
				tu = Undo;
				Undo = new CALUNDO;
				Undo->loc1 = 1;
				Undo->loc2 = 1;
				Undo->row1 = 0;
				Undo->row2 = 0;
				Undo->prev = tu;
				return;
			}
			
			if (to == NULL)
				return;
			
			sel.loc = 1;
			sel.row = 0;
			sel.depth = 0;
			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, true);
			break;

		case 2:
			tmp = board[cl.row];
			if (tmp == NULL)
				return;

			sel.loc = 2;
			sel.row = cl.row;
			sel.depth = tmp->getNumElements();
			tmp = tmp->last();
			xy = getXY(sel);
			drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, true);
			break;

		case 0:
		case 3:
		default:
			return;
	}  //  switch (cl.loc)

	return;
}

void Calculation::rightClick(int X, int Y) {
	CALLOC loc = getCL(X, Y);
	KXY xy;
	Cards *tmp;
	int cnum;
	bool t = false;

	if (score == 52)
		loc.loc = 0;

	dprintf("Right click at %i,%i loc = %i, row = %i, depth = %i", 
		X, Y, loc.loc, loc.row, loc.depth);

	switch (loc.loc) {
		case 1:
			cnum = E;
			if (pile != NULL) {
				tmp = pile->last();
				cnum = tmp->getNum();
			}

			if (to != NULL)
				cnum = to->getNum();

			drawCard(hWnd, ca.Width, ca.Height, cnum, 0, sel.loc == 1);
			break;

		case 2:
			xy = getXY(loc);
			t = sel.loc == 2 && loc.row == sel.row && loc.depth == sel.depth;
			tmp = board[loc.row];
			cnum = E;

			if (tmp != NULL) {
				tmp = tmp->getElement(loc.depth - 1);
				cnum = tmp->getNum();
			}

			drawCard(hWnd, xy.x, xy.y, cnum, 0, t);
			break;

		default:
			return;
	}  //  switch (loc.loc)

	rm.loc = loc.loc;
	rm.row = loc.row;
	rm.depth = loc.depth;
	return;
}

void Calculation::rightUp() {
	KXY xy;
	Cards *tmp;
	int cnum;

	if (rm.loc == 0 || score == 52)
		return;

	if (rm.loc == 1 && to == NULL) {
		drawCard(hWnd, ca.Width, ca.Height, LASTCARD, 0, false);
		rm.loc = 0;
		return;
	}

	if (rm.depth == 0) {
		rm.loc = 0;
		return;
	}

	tmp = board[rm.row]->getElement(rm.depth - 1);
	xy = getXY(rm);
	while (tmp != NULL) {
		cnum = tmp->getNum();
		drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
		xy.y += ca.Height / 5;
		tmp = tmp->Next();
	}

	xy.x -= ca.Height / 5;
	if (sel.loc == 2 && sel.depth == rm.depth && sel.row == rm.row)
		drawCard(hWnd, xy.x, xy.y, cnum, 0, true);

	rm.loc = 0;
	return;
}

int Calculation::load(const char *filename) {
	CALUNDO *del = NULL, *tmp = NULL;
	int i, j;
	char name[256];
	FILE *file = fopen(filename, "rb");

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
	delInst();

	for(i = 0; i < 48; i++)
		deck[i] = fgetc(file);

	fgetc(file);
	for (i = 0; i < 4; i++) {
		j = fgetc(file);
		for (int k = 0; k < j; k++) {
			if (board[i] == NULL)
				board[i] = new Cards(NULL, fgetc(file), false);
			else
				board[i]->push(new Cards(NULL, fgetc(file), false));
		}

		fgetc(file);
		j = fgetc(file);
		for (int k = 0; k < j; k++) {
			if (top[i] == NULL)
				top[i] = new Cards(NULL, fgetc(file), false);
			else
				top[i]->push(new Cards(NULL, fgetc(file), false));
		}

		fgetc(file);
	}  //  for (i = 0; i < 4; i++)

	i = fgetc(file);
	if (i != 0)
		to = new Cards(NULL, i, false);

	fgetc(file);

	j = fgetc(file);
	for (i = 0; i < j; i++) {
		if (pile == NULL)
			pile = new Cards(NULL, fgetc(file), false);
		else
			pile->push(new Cards(NULL, fgetc(file), false));
	}

	fgetc(file);
	
	j = fgetc(file);
	for (i = 0; i < j; i++) {
		tmp = new CALUNDO;
		tmp->loc1 = fgetc(file);
		tmp->loc2 = fgetc(file);
		tmp->row1 = fgetc(file);
		tmp->row2 = fgetc(file);
		fgetc(file);
		tmp->prev = del;
		del = tmp;
	}

	for (i = 0; i < j; i++) {
		tmp = new CALUNDO;
		tmp->loc1 = del->loc1;
		tmp->loc2 = del->loc2;
		tmp->row1 = del->row1;
		tmp->row2 = del->row2;
		tmp->prev = Undo;
		Undo = tmp;
		tmp = del->prev;
		delete del;
		del = tmp;
	}
	
	score = fgetc(file);
	i = fgetc(file);
	if (i != 255) {
		//  Something terribly wrong happened
		shuffel();
		delInst();
		deal();
		redrawBoard();
		fclose(file);
		return i;
	}

	redrawBoard();
	fclose(file);
	return TRUE;
}

int Calculation::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i;
	Cards *cur;
	CALUNDO *tu = Undo;

	if (file == NULL)
		return FALSE;

	if (score == 52) {
		fclose(file);
		return EOF;
	}

	//  Generic solitaire header
	fputc(datum, file);

	//  Calculation Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	48; i++)
		fputc(deck[i], file);

	fputc(0xFF, file);

	for (i = 0; i < 4; i++) {
		cur = board[i];
		if (cur == NULL)
			fputc(0, file);
		else
			fputc(cur->getNumElements(), file);

		while (cur != NULL) {
			fputc(cur->getNum(), file);
			cur = cur->Next();
		}

		fputc(0xFF, file);
		cur = top[i];
		if (cur == NULL)
			fputc(0, file);
		else
			fputc(cur->getNumElements(), file);

		while (cur != NULL) {
			fputc(cur->getNum(), file);
			cur = cur->Next();
		}

		fputc(0xFF, file);

	}  //  for (i = 0; i < 4; i++)

	cur = pile;

	if (cur == NULL)
		fputc(0, file);
	else
		fputc(cur->getNumElements(), file);

	while (cur != NULL) {
		fputc(cur->getNum(), file);
		cur = cur->Next();
	}

	fputc(0xFF, file);
	if (to != NULL)
		fputc(to->getNum(), file);
	else
		fputc(0, file);
	
	fputc(0xFF, file);

	i = 0;
	while (tu != NULL) {
		tu = tu->prev;
		i++;
	}
	fputc(i, file);

	tu = Undo;
	while (tu != NULL) {
		fputc(tu->loc1, file);
		fputc(tu->loc2, file);
		fputc(tu->row1, file);
		fputc(tu->row2, file);
		fputc(0xFF, file);
		tu = tu->prev;
	}

	fputc(score, file);
	fputc(0xFF, file);

	i = fclose(file);
	if (i != 0)
		return i;

	return TRUE;
}

void Calculation::redrawBoard() {

	Cards *tmp;
	int cnum;
	KXY xy;

	if (score == 52)
		return;

	clearBoard(hWnd);

	cnum = E;
	if (pile != NULL)
		cnum = LASTCARD;

	if (to != NULL)
		cnum = to->getNum();

	drawCard(hWnd, ca.Width, ca.Height, cnum, 0, /*sel.loc == 1*/false);

	xy.x = ca.Width * 3;
	xy.y = ca.Height;
	for (int i = 0; i < 4; i++) {
		tmp = top[i]->last();
		drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, false);
		xy.x += ca.Width;
		xy.x += ca.Width / 10;
	}

	xy.x = ca.Width * 3;
	for (int i = 0; i < 4; i++) {
		xy.y = ca.Height * 2;
		xy.y += ca.Height / 10;
		drawCard(hWnd, xy.x, xy.y, E, 0, false);
		tmp = board[i];
		while (tmp != NULL) {
			drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, false);
			xy.y += ca.Height / 5;
			tmp = tmp->Next();
		}

		xy.x += ca.Width;
		xy.x += ca.Width / 10;
	}  //  for (int i = 0; i < 4; i++)

	xy.x = ca.Width / 2;
	xy.y = ca.Height * 2;
	bprintf(hWnd, xy.x, xy.y, BWHITE, "Score: %i", score);

	if (sel.loc == 2) {
		xy = getXY(sel);
		tmp = board[sel.row]->last();
		drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, true);
	}

	return;
}

void Calculation::undo() {
	Cards *tmp;
	KXY xy;
	CALLOC cl;
	int cnum;
	CALUNDO *tu = Undo;

	if (Undo == NULL || score == 52)
		return;
	
	if (sel.loc != 0) {
		xy = getXY(sel);
		leftClick(xy.x, xy.y);
	}

	switch (Undo->loc2) {
		case 1:
			if (pile == NULL)
				pile = to;
			else
				pile->push(to);

			to = NULL;
			drawCard(hWnd, ca.Width, ca.Height, LASTCARD, 0, false);
			break;

		case 2:
			cl.loc = 2;
			cl.row = Undo->row2;
			cl.depth = board[Undo->row2]->getNumElements();
			tmp = board[Undo->row2]->last();
			tmp = tmp->Prev();
			xy = getXY(cl);
			eraseCard(hWnd, xy.x, xy.y, 0);

			to = board[Undo->row2]->pop();
			if (board[Undo->row2] == to)
				board[Undo->row2] = NULL;
			
			cl.depth--;
			cnum = E;
			if (tmp != NULL)
				cnum = tmp->getNum();

			xy = getXY(cl);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
			break;

		case 3:
			tmp = top[Undo->row2]->pop();
			if (top[Undo->row2] == tmp)
				top[Undo->row2] = NULL;

			switch (Undo->loc1) {
				case 1:
					to = tmp;
					drawCard(hWnd, ca.Width, ca.Height, tmp->getNum(), 0, false);
					break;
					
				case 2:
					if (board[Undo->row1] == NULL)
						board[Undo->row1] = tmp;
					else
						board[Undo->row1]->push(tmp);

					cl.loc = 2;
					cl.row = Undo->row1;
					cl.depth = board[Undo->row1]->getNumElements();
					xy = getXY(cl);
					drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, false);
					break;
			}  //  switch (Undo->loc1)

			cl.loc = 3;
			cl.row = Undo->row2;
			cl.depth = 0;
			xy = getXY(cl);
			cnum = E;
			if (top[Undo->row2] != NULL) {
				tmp = top[Undo->row2]->last();
				cnum = tmp->getNum();
			}
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);

			bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score:          ");
			bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score: %i", --score); 
			break;
	}  //  switch (Undo->loc2)

	Undo = Undo->prev;
	delete tu;
	return;
}

char* Calculation::gameName() {

	return "Calculation";
}

void Calculation::left() {
	KXY xy;
	Cards *tmp;

	if (score == 52)
		return;

	kbd.row--;
	if (kbd.row < 0) {
		kbd.row = 0;
		switch (kbd.loc) {
			case 1:
				kbd.loc = 2;
				kbd.row = 3;
				break;

			case 2:
			case 3:
				kbd.loc = 1;
				kbd.row = 0;
				kbd.depth = 0;
				break;
		}  //  switch (kbd.loc)

	}  //  if (kbd.row < 0)

	if (kbd.loc == 2) {
		tmp = board[kbd.row];
		kbd.depth = 0;
		if (tmp != NULL)
			kbd.depth = tmp->getNumElements();

	}  //  if (kbd.loc == 2)

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Calculation::right() {
	KXY xy;
	Cards *tmp;

	if (score == 52)
		return;

	kbd.row++;
	if (kbd.loc == 1)
		kbd.row = 4;

	if (kbd.row > 3) {
		kbd.row = 0;
		switch (kbd.loc) {
			case 1:
				kbd.loc = 2;
				kbd.row = 0;
				break;

			case 2:
			case 3:
				kbd.loc = 1;
				kbd.row = 0;
				kbd.depth = 0;
				break;
		}  //  switch (kbd.loc)

	}  //  if (kbd.row < 0)

	if (kbd.loc == 2) {
		tmp = board[kbd.row];
		kbd.depth = 0;
		if (tmp != NULL)
			kbd.depth = tmp->getNumElements();

	}  //  if (kbd.loc == 2)

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Calculation::up() {
	KXY xy;

	if (score == 52)
		return;

	if (kbd.loc == 2) {
		kbd.loc = 3;
		kbd.depth = 0;
	}

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Calculation::down() {
	KXY xy;
	Cards *tmp;

	if (score == 52)
		return;

	if (kbd.loc == 3)
		kbd.loc = 2;

	if (kbd.loc == 2) {
		tmp = board[kbd.row];
		kbd.depth = 0;
		if (tmp != NULL)
			kbd.depth = tmp->getNumElements();

	}  //  if (kbd.loc == 2)

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Calculation::enter() {
	KXY xy;
	Cards *tmp;

	if (score == 52)
		return;

	if (kbd.loc == 2) {
		tmp = board[kbd.row];
		kbd.depth = 0;
		if (tmp != NULL)
			kbd.depth = tmp->getNumElements();

	}  //  if (kbd.loc == 2)

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);
	leftClick(xy.x, xy.y);

	return;
}

void Calculation::shuffel() {

	//  We need to take out the A, 2, 3, 4 of progressing suits
	int i, n;
	
	//  clear out the deck  --  moved from constructor
	for (i = 0; i < 54; i++)
		deck[i] = 0;
	
	//  Start n out so first card is random
	n = rand() % 48;
	
	for (i = 1; i < 53; i++) {  
								
		if (i % 14 == 1)
			i++;

		while (deck[n] != 0)
			n = rand() % 48;
	
		deck[n] = i;	
	}

	return;
}

void Calculation::deal() {

	int i;

	delInst();
	for (i = 0; i < 4; i++)
		top[i] = new Cards(NULL, i * 14 + 1, false);

	pile = new Cards(NULL, deck[0], false);
	for (i = 1; i < 48; i++)
		pile->push(new Cards(NULL, deck[i], false));
    
	score = 4;

	return;
}

void Calculation::dblClick(int X, int Y) {
	Cards *tmp, *dst;
	KXY xy;
	CALLOC loc = getCL(X, Y);
	int cnum, t, i;
	bool n = loc.loc == 1 && to != NULL;
	
	if (loc.loc != 2 && !n) {
		leftClick(X, Y);
		return;
	}

	if (loc.loc == 2) {
		tmp = board[loc.row];
		if (tmp == NULL) {
			leftClick(X, Y);
			return;
		}
		loc.depth = tmp->getNumElements();
		tmp = tmp->last();
	}  else
		tmp = to;
		//  if (kbd.loc == 2)

	if (sel.loc == loc.loc && sel.row == loc.row) {
		xy = getXY(sel);
		leftClick(X, Y);
	} 
	
	if (sel.loc != 0) {
		leftClick(X, Y);
		return;
	}

	t = tmp->getVal();
	for (i = 0; i < 4; i++) {
		dst = top[i]->last();
		cnum = dst->getVal() + i + 1;
		cnum %= 13;
		if (cnum == t)
			break;
	}

	dprintf("Got to doubleClick! t = %i; i = %i", t, i);

	if (i > 3) {
		leftClick(X, Y);
		return;
	}

	xy = getXY(loc);
	leftClick(xy.x, xy.y);

	loc.loc = 3;
	loc.row = i;
	loc.depth = 0;
	xy = getXY(loc);
	leftClick(xy.x, xy.y);

	return;
}

int Calculation::winWork(void* datum) {
	CARDDIM sa = getBoardDims(hWnd);
	KXY xy, c;
	int cnum = 52, bnum = 0, t = -26, dt = 1, disp = LASTCARD;
	bool up = false;
	double dx, dy, r;

	UNUSED(datum);
	Sleep(30);
	clearBoard(hWnd);
	hit = false;

	c.y = sa.Height;
	c.x = sa.Width / 2;
	r = sa.Height * 3 / 4;
	
	while (!hit && cnum > 0) {
		xy.x = c.x;
		xy.y = c.y;
		dx = r * sin(DegtoRad(t));
		dy = r * cos(DegtoRad(t));
		xy.x += int(dx);
		xy.x += t * 2;
		xy.y -= int(dy);

		if (up) {
			disp = cnum;
			cnum--;
		} else
			bnum++;

		if (bnum > 52) {
			up = true;
			dt = -1;
			bnum = 0;
		}
		
		//dprintf("Displaying card %i at %i,%i rot = %i", disp, xy.x, xy.y, t);
		drawCard(hWnd, xy.x, xy.y, disp, t, false);
		t += dt;
		Sleep(60);

	}  //  while (!hit && cnum < 53)

	Sleep(60);

	return Win(hWnd, this);
}

KXY Calculation::getXY(CALLOC loc) {
	KXY xy;
	int t = ca.Width;

	xy.x = ca.Width;
	xy.y = ca.Height;
	if (loc.loc == 1)
		return xy;

	xy.y = ca.Height;
	xy.x = ca.Width * 3;
	t += ca.Width / 10;
	xy.x += t * loc.row;
	if (loc.loc == 3)
		return xy;

	xy.y = ca.Height * 2;
	xy.y += ca.Height / 10;

	loc.depth--;
	if (loc.depth < 0)
		loc.depth = 0;

	xy.y += loc.depth * ca.Height / 5;

	return xy;
}

CALLOC Calculation::getCL(int x, int y) {
	CALLOC cl;
	KXY xy;
	int t;
	Cards *tmp;

	cl.loc = 0;
	cl.depth = 0;
	cl.row = 0;

	if (x > ca.Width / 2 && x < ca.Width + ca.Width / 2 &&
		y > ca.Height / 2 && y < ca.Height + ca.Height / 2) {
			cl.loc = 1;
			return cl;
		}
	
	xy.x = ca.Width * 3;
	xy.x -= ca.Width / 2;
	t = 0;
	cl.row = 4;

	while (xy.x < ca.Width * 8) {
		if (x > xy.x && x < xy.x + ca.Width)
			cl.row = t;
		t++;
		xy.x += ca.Width;
		xy.x += ca.Width / 10;
	}

	if (cl.row > 3)
		return cl;

	if (y > ca.Height / 2 && y < ca.Height + ca.Height / 2) {
		cl.loc = 3;
		return cl;
	}

	xy.y = ca.Height * 2;
	xy.y += ca.Height / 10;
	xy.y -= ca.Height / 2;

	if (y < xy.y)
		return cl;

	tmp = board[cl.row];
	if (tmp == NULL && y < xy.y + ca.Height) {
		cl.loc = 2;
		return cl;
	}

	while (tmp != NULL && y > xy.y) {
		tmp = tmp->Next();
		xy.y += ca.Height / 5;
		cl.depth++;
	}

	if (tmp != NULL) {
		cl.loc = 2;
		return cl;
	}

	xy.y -= ca.Height / 5;
	if (y > xy.y + ca.Height)
		return cl;

	cl.loc = 2;   
	return cl;
}

void Calculation::calcMove(CALLOC cl) {
	KXY xy;
	Cards *tmp;
	CALUNDO *tu = Undo;
	int cnum, val1, val2;

	if (sel.loc == 1 && cl.loc == 2) {
		if (board[cl.row] == NULL)
			board[cl.row] = to;
		else
			board[cl.row]->push(to);

		sel.loc = 0;
		Undo = new CALUNDO;
		Undo->loc1 = 1;
		Undo->loc2 = 2;
		Undo->row1 = 0;
		Undo->row2 = cl.row;
		Undo->prev = tu;

		cl.depth = board[cl.row]->getNumElements();
		xy = getXY(cl);
		drawCard(hWnd, xy.x, xy.y, to->getNum(), 0, false);

		cnum = E;
		if (pile != NULL)
			cnum = LASTCARD;

		drawCard(hWnd, ca.Width, ca.Height, cnum, 0, false);
		to = NULL;
		return;
	}  //  if (sel.loc == 1 && cl.loc == 2)

	if (sel.row == cl.row && sel.loc == cl.loc) {
		xy = getXY(cl);
		switch(cl.loc) {
			case 1:
				tmp = to;
				break;

			case 2:
				tmp = board[cl.row]->last();
				break;
		}  //  switch(cl.loc)
		
		drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, false);
		sel.loc = 0;
	}

	if (cl.loc != 3)
		return;

	switch (sel.loc) {
		case 1:
			tmp = to;
			break;

		case 2:
			tmp = board[sel.row]->last();
			break;
	}  //  switch (sel.loc)
	val1 = tmp->getVal();
	tmp = top[cl.row]->last();
	val2 = tmp->getVal() + cl.row;
	val2++;
	val1 %= 13;
	val2 %= 13;

	if (val1 != val2 || tmp->getVal() == KING)
		return;

	switch (sel.loc) {
		case 1:
			tmp = to;
			to = NULL;
			break;

		case 2:
			tmp = board[sel.row]->pop();
			if (board[sel.row] == tmp)
				board[sel.row] = NULL;

			break;
	}  //  switch (sel.loc)

	Undo = new CALUNDO;
	Undo->loc1 = sel.loc;
	Undo->loc2 = 3;
	Undo->row1 = sel.row;
	Undo->row2 = cl.row;
	Undo->prev = tu;

	top[cl.row]->push(tmp);
	xy = getXY(cl);
	drawCard(hWnd, xy.x, xy.y, tmp->getNum(), 0, false);

	switch (sel.loc) {
		case 1:
			cnum = E;
			if (pile != NULL)
				cnum = LASTCARD;
			
			drawCard(hWnd, ca.Width, ca.Height, cnum, 0, false);
			break;

		case 2:
			xy = getXY(sel);
			eraseCard(hWnd, xy.x, xy.y, 0);
			sel.depth--;
			xy = getXY(sel);
			tmp = tmp->Prev();
	
			cnum = E;
			tmp = board[sel.row];	
			if (tmp != NULL) {
				tmp = tmp->last();
				cnum = tmp->getNum();
			}

			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;
	}  //  switch (sel.loc)

	score++;
	bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score: %i", score);
	sel.loc = 0;
	if (score == 52)
		winTeaser();

	return;
}

void Calculation::delInst() {

	Cards *tmp;
	CALUNDO *tu;

	for (int i = 0; i < 4; i++) {
		while (board[i] != NULL) {
			tmp = board[i];
			board[i] = tmp->Next();
			if (board[i] != NULL)
				board[i]->setPrev(NULL);

			delete tmp;
		}

		while (top[i] != NULL) {
			tmp = top[i];
			top[i] = tmp->Next();
			if (top[i] != NULL)
				top[i]->setPrev(NULL);

			delete tmp;
		}

	}  //  for (int i = 0; i < 4; i++)

	while (pile != NULL) {
		tmp = pile;
		pile = tmp->Next();
		if (pile != NULL)
			pile->setPrev(NULL);

		delete tmp;
	}

	while (Undo != NULL) {
		tu = Undo;
		Undo = tu->prev;
		delete tu;
	}

	if (to != NULL)
		delete to;
	to = NULL;

	score = 0;
	sel.loc = 0;
	sel.row = 0;
	sel.depth = 0;
	rm.loc = 0;
	rm.row = 0;
	rm.depth = 0;
	kbd.loc = 1;
	kbd.row = 0;
	kbd.depth = 0;
	return;
}

Calculation::~Calculation() {
	delInst();
	return;
}
