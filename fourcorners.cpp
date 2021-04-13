//  Four Corners Game Rule Set
//  Four Corners is the third game in the Aunt Annie's Tour
//  That I am going to write.

#include "stdafx.h"
#include "cards.h"
#include "fourcorners.h"
#include "debug.h"
#include "cardtools.h"
#include "winteaser.h"
#include "colordefs.h"
#include <time.h>
#include <math.h>

FourCorners::FourCorners(HWND hWind, int info) {
	int i;
	Init(hWind, info);

	for (int j = 0; j < 3; j++)
		for (i = 0; i < 3; i++)
			table[i][j] = NULL;

	for (i = 0; i < 4; i++)
		goal[i] = NULL;

	for (i = 0; i < 5; i++)
		play[i] = NULL;

	startcard = 0;
	score = 0;
	draw = NULL;
	dis = NULL;
	to = NULL;
	Undo = NULL;
	sel.loc = 0;
	sel.num = 0;
	sel.d = 0;
	kbd.loc = 0;
	kbd.num = 0;
	kbd.d = 0;
	rdn = false;

	//  Manipulate pointers for ease of access
	goal[0] = table[0];
	goal[1] = table[0] + 2;
	goal[2] = table[2];
	goal[3] = table[2] + 2;

	play[0] = table[0] + 1;
	for (int i = 0; i < 3; i++)
		play[i + 1] = table[1] + i;
	play[4] = table[2] + 1;

	ca = getCardAttributes();
	return;
}

void FourCorners::leftClick(int X, int Y) {
	FCLOC fcl = getFCL(X, Y);
	FCUNDO *tu = Undo;
	Cards *t;
	KXY xy;

	if (score == 52)
		return;

	if (sel.loc != 0 && (fcl.loc > 1 || to != NULL)) {
			calcMove(fcl);
			return;
	}
	
	//dprintf("Left Click at %i, %i.  Location = %i, num = %i", X, Y, fcl.loc, fcl.num);
	switch (fcl.loc) {
		case 1:
			if (to == NULL && draw != NULL) {
				Undo = new FCUNDO;
				Undo->loc1 = 1;
				Undo->loc2 = 1;
				Undo->num1 = 0;
				Undo->num2 = 0;
				Undo->prev = tu;
				to = draw->pop();
				if (to == draw)
					draw = NULL;
				drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
				return;
			} 
			
			if (to == NULL)
				return;

			sel.loc = 1;
			sel.num = 0;
			sel.d = 0;
			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, true);
			break;

		case 2:
			if (dis == NULL)
				return;

			sel.loc = 2;
			sel.num = 0;
			sel.d = 0;
			t = dis->last();
			xy = getXY(fcl);
			drawCard(hWnd, xy.x, xy.y, t->getNum(), 0, true);
			break;

		case 3:
			t = *play[fcl.num];
			if (t == NULL)
				return;

			t = t->last();
			xy = getXY(fcl);
			sel.loc = 3;
			sel.num = fcl.num;
			sel.d = fcl.d;
			drawCard(hWnd, xy.x, xy.y, t->getNum(), 0, true);
			break;

	}  //  switch (fcl.loc)

	return;
}

void FourCorners::rightClick(int X, int Y) {
	Cards *tmp;

	if (X > ca.Width / 2 && X < ca.Width + ca.Width  / 2 && Y > ca.Height / 2 && 
		Y < ca.Height + ca.Height / 2 && score < 52 && to == NULL && draw != NULL) {
			rdn = true;
			tmp = draw->last();
			drawCard(hWnd, ca.Width, ca.Height, tmp->getNum(), 0, false);
	}
	
	return;
}

void FourCorners::rightUp() {

	if (!rdn)
		return;

	drawCard(hWnd, ca.Width, ca.Height, LASTCARD, 0, false);
	rdn = false;

	return;
}

int FourCorners::load(const char *filename) {
	FCUNDO *del = NULL, *tmp = NULL;
	int i, j, k;
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

	for(i = 0; i < 52; i++)
		deck[i] = fgetc(file);

	fgetc(file);

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			k = fgetc(file);
			for (int n = 0; n < k; n++)
				if (table[i][j] == NULL)
					table[i][j] = new Cards(NULL, fgetc(file), false);
				else
					table[i][j]->push(new Cards(NULL, fgetc(file), false));

			fgetc(file);
		}  //  for (j = 0; j < 3; j++)

	i = fgetc(file);
	for (j = 0; j < i; j++)
		if (draw == NULL)
			draw = new Cards(NULL, fgetc(file), false);
		else
			draw->push(new Cards(NULL, fgetc(file), false));

	fgetc(file);

	i = fgetc(file);
	for (j = 0; j < i; j++)
		if (dis == NULL)
			dis = new Cards(NULL, fgetc(file), false);
		else
			dis->push(new Cards(NULL, fgetc(file), false));

	fgetc(file);

	i = fgetc(file);
	if (i != 0)
		to = new Cards(NULL, i, false);

	fgetc(file);

	j = fgetc(file);
	for (i = 0; i < j; i++) {
		tmp = new FCUNDO;
		tmp->loc1 = fgetc(file);
		tmp->loc2 = fgetc(file);
		tmp->num1 = fgetc(file);
		tmp->num2 = fgetc(file);
		tmp->prev = del;
		del = tmp;
		fgetc(file);
	}

	for (i = 0; i < j; i++) {
		tmp = new FCUNDO;
		tmp->loc1 = del->loc1;
		tmp->loc2 = del->loc2;
		tmp->num1 = del->num1;
		tmp->num2 = del->num2;
		tmp->prev = Undo;
		Undo = tmp;
		tmp = del->prev;
		delete del;
		del = tmp;
	}

	startcard = fgetc(file);
	fgetc(file);

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

int FourCorners::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i, j;
	Cards *cur;
	FCUNDO *tu = Undo;

	if (file == NULL)
		return FALSE;

	if (score == 52) {
		fclose(file);
		return EOF;
	}

	//  Generic solitaire header
	fputc(datum, file);

	//  Four Corners Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	52; i++)
		fputc(deck[i], file);

	fputc(0xFF, file);

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			cur = table[i][j];
			if (cur != NULL)
				fputc(cur->getNumElements(), file);
			else
				fputc(0, file);

			while (cur != NULL) {
				fputc(cur->getNum(), file);
				cur = cur->Next();
			}
			fputc(0xFF, file);
		}  //  for (j = 0; j < 3; j++)

	cur = draw;
	if (cur != NULL)
		fputc(cur->getNumElements(), file);
	else
		fputc(0, file);

	while (cur != NULL) {
		fputc(cur->getNum(), file);
		cur = cur->Next();
	}
	fputc(0xFF, file);
	
	cur = dis;
	if (cur != NULL)
		fputc(cur->getNumElements(), file);
	else
		fputc(0, file);

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
		fputc(tu->num1, file);
		fputc(tu->num2, file);
		fputc(0xFF, file);
		tu = tu->prev;
	}

	fputc(startcard, file);
	fputc(0xFF, file);
	fputc(score, file);	
	//  Final marker
	fputc(0xFF, file);

	i = fclose(file);
	if (i != 0)
		return i;

	return TRUE;
}

void FourCorners::redrawBoard() {
	short i, j, cnum;
	Cards *t;
	KXY xy;

	if (score == 52)
		return;

	clearBoard(hWnd);
	
	cnum = LASTCARD;
	if (draw == NULL)
		cnum = E;

	if (to != NULL)
		cnum = to->getNum();

	drawCard(hWnd, ca.Width, ca.Height, cnum, 0, sel.loc == 1);

	cnum = E;
	if (dis != NULL) {
		t = dis->last();
		cnum = t->getNum();
	}
	xy.x = ca.Width * 2;
	xy.y = ca.Height;
	xy.x += ca.Width / 10;
	drawCard(hWnd, xy.x, xy.y, cnum, 0, sel.loc == 2);
	
	xy.y = ca.Height;
	for (i = 0; i < 3; i++) {
		xy.x = ca.Width * 4;

		for (j = 0; j < 3; j++) {
			t = table[i][j];
			cnum = E;
			if (t != NULL) {
				t = t->last();
				cnum = t->getNum();
			}
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			xy.x += ca.Width + (ca.Width / 10);
		}  //  for (j = 0; j < 3; j++)
		xy.y += ca.Height + (ca.Height / 10);
	}  //  for (i = 0; i < 3; i++)
		
	bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score: %i", score);

	bprintf(hWnd, ca.Width / 2, ca.Height * 3, BWHITE, "Start Card: ");
	drawCard(hWnd, ca.Width * 2, ca.Height * 3, startcard, 0, false);

	if (sel.loc == 3) {
		t = *play[sel.num];
		xy = getXY(sel);
		t = t->last();
		drawCard(hWnd, xy.x, xy.y, t->getNum(), 0, true);
	}			
			
	return;
}

void FourCorners::undo() {
	Cards *t, *t1;
	int cnum;
	FCLOC fl;
	FCUNDO *tu = Undo;
	KXY xy;

	if (score == 52 || Undo == NULL)
		return;
	
	if (sel.loc != 0) {
		xy = getXY(sel);
		leftClick(xy.x, xy.y);
	}

	switch (Undo->loc2) {
		case 1:
			if (draw == NULL)
				draw = to;
			else
				draw->push(to);

			to = NULL;
			drawCard(hWnd, ca.Width, ca.Height, LASTCARD, 0, false);
			break;

		case 2:
			to = dis->pop();
			if (to == dis)
				dis = NULL;

			cnum = E;
			if (dis != NULL) {
				t = dis->last();
				cnum = t->getNum();
			}

			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
			fl.loc = 2;
			fl.num = 0;
			fl.d = 0;
			xy = getXY(fl);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;

		case 3:
			t = *play[Undo->num2];
			t1 = t->pop();
			if (t == t1) {
				*play[Undo->num2] = NULL;
				cnum = E;
			} else {
				t = t->last();
				cnum = t->getNum();
			}

			fl.loc = 3;
			fl.num = Undo->num2;
			fl.d = getD(3, Undo->num2);
			xy = getXY(fl);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			fromMove1(t1);
			break;

		case 4:
			t = *goal[Undo->num2];
			t1 = t->pop();
			if (t == t1) {
				*goal[Undo->num2] = NULL;
				cnum = E;
			} else {
				t = t->last();
				cnum = t->getNum();
			}

			fl.loc = 4;
			fl.num = Undo->num2;
			fl.d = getD(4, Undo->num2);
			xy = getXY(fl);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			fromMove1(t1);

			bprintf(hWnd, ca.Width / 2, ca.Height * 2, GREEN, "Score: %i", score--);
			bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score: %i", score);
			break;
	}  //  switch (Undo->loc2)

	Undo = Undo->prev;
	delete tu;

	return;
}

char* FourCorners::gameName() {

	return "Four Corners";
}

void FourCorners::left() {
	KXY xy;
	short j, k;

	if (score == 52)
		return;

	switch (kbd.loc) {
		case 1:
			kbd.loc = 4;
			kbd.num = 1;
			j = 2;
			k = 0;
			break;

		case 2:
			kbd.loc = 1;
			kbd.num = 0;
			j = 0;
			k = 0;
			break;

		case 3:
		case 4:
			j = kbd.d % 0x10;
			k = kbd.d / 0x10;
			j--;
			if (j < 0) {
				j = 0;
				k = 0;
				kbd.loc = 2;
				kbd.num = 0;
				break;
			}

			kbd.num--;
			if (k == 1)
				break;

			if (kbd.num == 2) {
				kbd.num = 4;
				kbd.loc = 3;
				break;
			}

			if (kbd.num == 3) {
				kbd.num = 2;
				kbd.loc = 4;
				break;
			}
			
			if (kbd.num < 0) {
				kbd.num = 0;
				kbd.loc = 4;
				break;
			}

			kbd.loc = 3;
	}  //  switch (kbd.loc)
	
	kbd.d = (0x10 * k) + j;
	
	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);
	return;
}

void FourCorners::right() {
	KXY xy;
	short j, k;

	if (score == 52)
		return;

	switch (kbd.loc) {
		case 1:
			kbd.loc = 2;
			kbd.num = 0;
			j = 0;
			k = 0;
			break;

		case 2:
			kbd.loc = 4;
			kbd.num = 0;
			j = 0;
			k = 0;
			break;

		case 3:
		case 4:
			j = kbd.d % 0x10;
			k = kbd.d / 0x10;
			j++;
			if (j > 2) {
				j = 0;
				k = 0;
				kbd.loc = 1;
				kbd.num = 0;
				break;
			}

			kbd.num++;
			if (k == 1)
				break;

			if (kbd.num == 1 && kbd.loc == 3) {
				kbd.loc++;
				break;
			}

			if (kbd.num == 1) {
				kbd.loc--;
				break;
			}

			if (kbd.num == 3) {
				kbd.loc = 3;
				kbd.num = 4;
				break;
			}

			kbd.num = 3;
			kbd.loc = 4;
	}  //  switch (kbd.loc)

	kbd.d = (0x10 * k) + j;
	
	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);
	return;
}

void FourCorners::up() {
	KXY xy;
	short j, k;

	if (score == 52 || kbd.loc < 3)
		return;

	j = kbd.d % 0x10;
	k = kbd.d / 0x10;
	k--;
	
	switch (k) {
		case 0:
			kbd.loc = 4;
			if (j == 1) {
				kbd.loc = 3;
				kbd.num = 0;
				break;
			}

			kbd.num = 0;
			if (j == 2)
				kbd.num = 1;
			break;

		case 1:
			kbd.loc = 3;
			kbd.num = j + 1;
			break;

		default:
			k = 0;
	}  //  switch (k)

	kbd.d = (0x10 * k) + j;
	
	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);
	return;
}

void FourCorners::down() {
	KXY xy;
	short j, k;

	if (score == 52 || kbd.loc < 3)
		return;

	j = kbd.d % 0x10;
	k = kbd.d / 0x10;
	k++;
	
	switch (k) {
		case 1:
			kbd.loc = 3;
			kbd.num = j + 1;
			break;

		case 2:
			kbd.loc = 4;
			if (j == 1) {
				kbd.loc = 3;
				kbd.num = 4;
				break;
			}

			kbd.num = 2;
			if (j == 2)
				kbd.num = 3;
			break;

		default:
			k = 2;
	}  //  switch (k)

	kbd.d = (0x10 * k) + j;
	
	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);
	return;
}

void FourCorners::enter() {
	KXY xy = getXY(kbd);
	
	if (score == 52)
		return;
	
	putMouse(xy.x, xy.y, hWnd);
	leftClick(xy.x, xy.y);

	return;
}

void FourCorners::deal() {
	//  How to deal....
	register int n;
	delInst();

	for (n = 0; n < 5; n++)
		*play[n] = new Cards(NULL, deck[n], false);

	startcard = deck[n];
	*goal[0] = new Cards(NULL, startcard, false);

	draw = new Cards(NULL, deck[++n], false);
	while (++n < 52)
		draw->push(new Cards(NULL, deck[n], false));
	
	score = 1;
	return;
}

void FourCorners::dblClick(int X, int Y) {
	FCLOC fl = getFCL(X, Y);
	int cnum, cnum2, i;
	Cards *t, *t2;

	if (score == 52)
		return;

	if (fl.loc == 0)
		return;

	if (fl.loc == 1 && to == NULL) {
		leftClick(X, Y);
		return;
	}

	t = getCard(fl);
	if (t == NULL) {
		leftClick(X, Y);
		return;
	}

	if (fl.loc == sel.loc && fl.num == sel.num)
		leftClick(X, Y);

	if (sel.loc != 0) {
		leftClick(X, Y);
		return;
	}

	cnum = t->getNum();
	for (i = 0; i < 4; i++) {
		t2 = *goal[i];
		if (t2 == NULL && cnum % 13 == startcard % 13)
			break;

		if (t2 == NULL)
			continue;

		t2 = t2->last();
		cnum2 = t2->getNum() + 1;
		if (cnum2 % 13 == 1)
			cnum2 -= 13;

		if (cnum2 == cnum)
			break;
	}

	if (i == 4) {
		leftClick(X, Y);
		return;
	}

	leftClick(X, Y);
	fl.loc = 4;
	fl.num = i;
	fl.d = getD(4, i);
	Move4(fl);

	return;
}

int FourCorners::winWork(void* datum) {
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

FCLOC FourCorners::getFCL(int X, int Y) {
	FCLOC fcl;
	unsigned short i;
	short j = -1, k = -1;
	KXY xy;

	fcl.loc = 0;
	fcl.num = 0;

	if (X > ca.Width / 2 && X < ca.Width + ca.Width / 2 && Y > ca.Height / 2 && 
		Y < ca.Height + ca.Height / 2) {
			fcl.loc = 1;
			return fcl;
		}

	xy.x = ca.Width * 2;
	xy.x += ca.Width / 10;
	xy.x -= ca.Width / 2;
	xy.y = ca.Height / 2;

	if (X > xy.x && X < xy.x + ca.Width && Y > xy.y && Y < xy.y + ca.Height) {
		fcl.loc = 2;
		return fcl;
	}

	xy.y = ca.Height;
	xy.y -= ca.Height / 2;
	xy.x = ca.Width * 4;
	xy.x -= ca.Width / 2;

	for (i = 0; i < 3; i++) {
		if (X > xy.x && X < xy.x + ca.Width)
			j = i;
		
		xy.x += ca.Width;
		xy.x += ca.Width / 10;
	}

	for (i = 0; i < 3; i++) {
		if (Y > xy.y && Y < xy.y + ca.Height)
			k = i;
			
		xy.y += ca.Height;
		xy.y += ca.Height / 10;
	}
	
	//dprintf("j = %i, j = %i", j, k);
	if (k < 0 || j < 0)
		return fcl;

	//  we have a valid row / columb

	i = (0x10 * k) + j;
	fcl.d = i;

	switch (i) {
		case 0x00:
			fcl.loc = 4;
			fcl.num = 0;
			break;

		case 0x01:
			fcl.loc = 3;
			fcl.num = 0;
			break;

		case 0x02:
			fcl.loc = 4;
			fcl.num = 1;
			break;

		case 0x10:
		case 0x11:
		case 0x12:
			fcl.loc = 3;
			fcl.num = j + 1;
			break;

		case 0x20:
			fcl.loc = 4;
			fcl.num = 2;
			break;

		case 0x21:
			fcl.loc = 3;
			fcl.num = 4;
			break;

		case 0x22:
			fcl.loc = 4;
			fcl.num = 3;
			break;

		default:
			fcl.d = 0;
	}  //  switch (i)

	return fcl;
}

KXY FourCorners::getXY(FCLOC fl) {
	short j, k;
	const int dx = ca.Width + ca.Width / 10, dy = ca.Height + ca.Height / 10;
	KXY xy;

	switch (fl.loc) {
		case 1:
			xy.x = ca.Width;
			xy.y = ca.Height;
			break;

		case 2:
			xy.x = ca.Width + dx;
			xy.y = ca.Height;
			break;

		case 3:
		case 4:
			j = fl.d % 0x10;
			k = fl.d / 0x10;
			xy.y = ca.Height;
			xy.x = ca.Width * 4;
			xy.x += dx * j;
			xy.y += dy * k;
			break;

		default:
			xy.x = 0;
			xy.y = 0;
	}  //  switch (fl.loc)
	
	return xy;
}

void FourCorners::calcMove(FCLOC fl) {
	Cards *t;
	KXY xy;

	if (sel.loc == fl.loc && sel.num == fl.num) {
		t = getCard(sel);
		xy = getXY(sel);
		drawCard(hWnd, xy.x, xy.y, t->getNum(), 0, false);
		sel.loc = 0;
		return;
	}

	switch (fl.loc) {
		case 2:
			Move2(fl);
			break;

		case 3:
			Move3(fl);
			break;

		case 4:
			Move4(fl);
			break;
	}  //  switch (fl.loc)
	return;
}

Cards* FourCorners::getCard(FCLOC fl) {
	Cards *t = NULL;

	switch (fl.loc) {
		case 1:
			if (draw != NULL)
				t = draw;

			if (to != NULL)
				t = to;

			break;

		case 2:
			t = dis;
			break;

		case 3:
			t = *play[fl.num];
			break;

		case 4:
			t = *goal[fl.num];
			break;
	}  //  switch (fl.loc)

	if (t != NULL)
		t = t->last();
			
	return t;
}

void FourCorners::Move2(FCLOC fl) {
	KXY xy;
	FCUNDO *tu = Undo;
	int cnum = E;
	Cards *t;

	if (sel.loc != 1)
		return;

	if (dis == NULL)
		dis = to;
	else
		dis->push(to);

	xy = getXY(fl);
	drawCard(hWnd, xy.x, xy.y, to->getNum(), 0, false);
	Undo = new FCUNDO;
	Undo->loc1 = 1;
	Undo->loc2 = 2;
	Undo->num1 = 0;
	Undo->num2 = 0;
	Undo->prev = tu;
	to = NULL;

	switch (sel.loc) {
		case 1:
			if (draw != NULL)
				cnum = LASTCARD;

			break;

		case 3:
			t = *play[sel.num];
			if (t == NULL)
				break;

			t = t->last();
			cnum = t->getNum();
			break;
	}  //  switch (sel.loc)

	xy = getXY(sel);
	drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
	sel.loc = 0;

	return;
}

void FourCorners::Move3(FCLOC fl) {
	Cards *t, *t1, *t2;
	FCUNDO *tu = Undo;
	KXY xy;
	short cnum, cnum1, cnum2;
	
	t1 = getCard(sel);
	t2 = getCard(fl);
	cnum1 = t1->getVal();

	if (t2 != NULL) {
		//  Eliminate anything that can go wrong....
		cnum2 = t2->getVal();
		if (--cnum2 == 0)
			cnum2 = KING;
		
		if (cnum1 != cnum2)
			return;
	}

	//  We have a valid move.
	switch (sel.loc) {
		case 1:
			to = NULL;
			cnum = E;
			if (draw != NULL)
				cnum = LASTCARD;

			drawCard(hWnd, ca.Width, ca.Height, cnum, 0, false);
			break;

		case 2:
			t1 = dis->pop();
			if (t1 == dis)
				dis = NULL;

			cnum = E;
			if (dis != NULL) {
				t = dis->last();
				cnum = t->getNum();
			}

			xy = getXY(sel);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;

		case 3:
			t = *play[sel.num];
			t1 = t->pop();
			t = t->last();	
			if (t1 == t) {
				*play[sel.num] = NULL;
				cnum = E;
			} else
				cnum = t->getNum();
			
			xy = getXY(sel);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;

		default:
			return;
	}  //  switch (sel.loc)

	if (*play[fl.num] == NULL)
		*play[fl.num] = t1;
	else {
		t = *play[fl.num];
		t->push(t1);
	}
	xy = getXY(fl);
	drawCard(hWnd, xy.x, xy.y, t1->getNum(), 0, false);
	Undo = new FCUNDO;
	Undo->loc1 = sel.loc;
	Undo->loc2 = 3;
	Undo->num1 = sel.num;
	Undo->num2 = fl.num;
	Undo->prev = tu;

	sel.loc = 0;
	return;
}

void FourCorners::Move4(FCLOC fl) {
	Cards *t, *t1, *t2;
	FCUNDO *tu = Undo;
	KXY xy;
	short cnum, cnum1, cnum2;

	t1 = getCard(sel);
	t2 = getCard(fl);
	cnum1 = t1->getNum();
	cnum2 = startcard % 13;
	if (t2 != NULL)
		cnum2 = t2->getNum();
	
	if (++cnum2 % 13 == 1)
		cnum2 -= 13;
	
	if (cnum1 % 13 != startcard % 13 && t2 == NULL)
		return;
	
	if (cnum1 != cnum2 && t2 != NULL)
		return;

	switch (sel.loc) {
		case 1:
			to = NULL;
			cnum = E;
			if (draw != NULL)
				cnum = LASTCARD;
			drawCard(hWnd, ca.Width, ca.Height, cnum, 0, false);
			break;

		case 2:
			t1 = dis->pop();
			if (t1 == dis)
				dis = NULL;

			cnum = E;
			if (dis != NULL) {
				t = dis->last();
				cnum = t->getNum();
			}
			xy = getXY(sel);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;

		case 3:
			t1 = *play[sel.num];
			t1 = t1->pop();
			if (t1 == *play[sel.num])
				*play[sel.num] = NULL;

			cnum = E;
			if (*play[sel.num] != NULL) {
				t = *play[sel.num];
				t = t->last();
				cnum = t->getNum();
			}
			xy = getXY(sel);
			drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
			break;

		default:
			return;
	}  //  switch (sel.loc)

	t = *goal[fl.num];
	if (t == NULL)
		*goal[fl.num] = t1;
	else
		t->push(t1);

	xy = getXY(fl);
	drawCard(hWnd, xy.x, xy.y, t1->getNum(), 0, false);
	bprintf(hWnd, ca.Width / 2, ca.Height * 2, BWHITE, "Score: %i", ++score);
	
	Undo = new FCUNDO;
	Undo->loc1 = sel.loc;
	Undo->loc2 = 4;
	Undo->num1 = sel.num;
	Undo->num2 = fl.num;
	Undo->prev = tu;
	sel.loc = 0;

	if (score == 52)
		winTeaser();

	return;

}

unsigned short FourCorners::getD(short loc, short num) {
	unsigned short d, i;
	short j, k;
	//  j is x, k is y

	i = loc * 0x10 + num;

	switch (i) {
		case 0x40:
			j = 0;
			k = 0;
			break;

		case 0x30:
			j = 1;
			k = 0;
			break;

		case 0x41:
			j = 2;
			k = 0;
			break;

		case 0x31:
		case 0x32:
		case 0x33:
			k = 1;
			j = i & 0x0F;
			j--;
			break;

		case 0x42:
			j = 0;
			k = 2;
			break;

		case 0x34:
			j = 1;
			k = 2;
			break;

		case 0x43:
			j = 2;
			k = 2;
			break;

		default:
			j = 0;
			k = 0;
	}  //  switch (i)

	d = (0x10 * k) + j;
	return d;
}

void FourCorners::fromMove1(Cards *t1) {
	Cards *t;
	FCLOC fl;
	FCUNDO *tu = Undo;
	KXY xy;

	switch (Undo->loc1) {
		case 1:
			to = t1;
			drawCard(hWnd, ca.Width, ca.Height, to->getNum(), 0, false);
			break;

		case 2:
			if (dis == NULL)
				dis = t1;
			else
				dis->push(t1);

			fl.loc = 2;
			fl.num = 0;
			fl.d = 0;
			xy = getXY(fl);
			drawCard(hWnd, xy.x, xy.y, t1->getNum(), 0, false);
			break;

		case 3:
			t = *play[Undo->num1];
			if (t == NULL)
				*play[Undo->num1] = t1;
			else
				t->push(t1);

			fl.loc = 3;
			fl.num = Undo->num1;
			fl.d = getD(3, Undo->num1);
			xy = getXY(fl);
			drawCard(hWnd, xy.x, xy.y, t1->getNum(), 0, false);
			break;
	}  //  switch (Undo->loc1)

	return;
}

void FourCorners::delInst() {
	Cards *t;
	FCUNDO *tu = Undo;

	while (draw != NULL) {
		t = draw;
		draw = draw->Next();
		if (draw != NULL)
			draw->setPrev(NULL);

		delete t;
	}

	while (dis != NULL) {
		t = dis;
		dis = dis->Next();
		if (dis != NULL)
			dis->setPrev(NULL);

		delete t;
	}

	while (Undo != NULL) {
		tu = Undo->prev;;
		delete Undo;
		Undo = tu;
	}

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			while (table[i][j] != NULL) {
				t = table[i][j];
				table[i][j] = table[i][j]->Next();
				if (table[i][j] != NULL)
					table[i][j]->setPrev(NULL);

				delete t;
	}
	
	if (to != NULL)
		delete to;
	to = NULL;

	startcard = 0;
	score = 0;
	sel.loc = 0;
	sel.num = 0;
	sel.d = 0;
	rdn = false;
	kbd.loc = 1;
	kbd.num = 0;
	kbd.d = 0;

	return;
}

FourCorners::~FourCorners() {
	delInst();
	return;
}
