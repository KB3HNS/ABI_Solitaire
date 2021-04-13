//  The pyrimid game code
//  Pyrimid will also require a special class
//  for handling the game board cards

#include "stdafx.h"
#include "cards.h"
#include "pyrimid.h"
#include "debug.h"
#include "cardtools.h"
#include "colordefs.h"
#include "winteaser.h"
#include "pcard.h"
#include <time.h>
#include <math.h>

Pyrimid::Pyrimid(HWND hWind, int info) {
	CARDDIM cx;

	Init(hWind, info);
	ps.loc = 0;
	pile = NULL;
	draw = NULL;
	to = NULL;
	root = NULL;
	score = 0;
	pundo = NULL;

	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			pyr[i][j] = NULL;

	cx = getCardAttributes();
	ca.x = cx.Width;
	ca.y = cx.Height;

	rtxy.x = ca.x * 5;
	rtxy.y = ca.y;

	kbd.loc = 1;
	kbd.xc = 0;
	kbd.yc = 0;
	won = false;

	return;
}

void Pyrimid::leftClick(int X, int Y) {
	PYSELECT pys = getPS(X, Y);
	PYUNDO *move;
	KXY xy;
	Cards *card;
	PCard *pcard, *p1, *p2;
	int cnum;

	if (pys.loc == 0 || won)
		return;

	if (ps.loc != 0) {
		calcMove(pys);
		if (score == 52)
			winTeaser();

		return;
	}

	switch (pys.loc) {
		case 1:
			if (to == NULL && draw != NULL) {
				to = draw->pop();
				if (to == draw)
					draw = NULL;

				drawCard(hWnd, ca.x, ca.y, to->getNum(), 0, false);
				move = new PYUNDO;
				move->loc1 = 1;
				move->loc2 = 0;
				move->val1 = to->getNum();
				move->val2 = 255;
				move->xc1 = 0;
				move->xc2 = 0;
				move->yc1 = 0;
				move->yc2 = 0;
				move->prev = pundo;
				pundo = move;
				return;
			}
			if (to == NULL)
				return;

			if (to->getVal() == KING) {
				move = new PYUNDO;
				move->loc1 = 1;
				move->loc2 = 0;
				move->val1 = to->getNum();
				move->val2 = 0;
				move->xc1 = 0;
				move->xc2 = 0;
				move->yc1 = 0;
				move->yc2 = 0;
				move->prev = pundo;
				pundo = move;
				
				delete to;
				to = NULL;
				score++;
				bprintf(hWnd, ca.x / 2, ca.y * 2, BWHITE, "Score: %i", score);

				cnum = E;
				if (draw != NULL)
					cnum = LASTCARD;

				drawCard(hWnd, ca.x, ca.y, cnum, 0, false);
			}  else {
				ps.loc = 1;
				ps.xc = 0;
				ps.yc = 0;
				drawCard(hWnd, ca.x, ca.y, to->getNum(), 0, true);
			}  //  if (to->getVal() == KING) 

			break;

		case 2:
			if (pile == NULL)
				return;
			
			card = pile->last();
			ps.loc = 2;
			ps.xc = 0;
			ps.yc = 0;
			xy = getXY(pys);
			drawCard(hWnd, xy.x, xy.y, card->getNum(), 0, true);
			break;

		case 3:
			pcard = pyr[pys.yc][pys.xc];
			if (pcard == NULL)
				return;

			if (pcard->next1() != NULL || pcard->next2() != NULL)
				return;

			if (pcard->getVal() == KING) {
				move = new PYUNDO;
				move->loc1 = 3;
				move->loc2 = 0;
				move->val1 = pcard->getNum();
				move->val2 = 0;
				move->xc1 = pys.xc;
				move->xc2 = 0;
				move->yc1 = pys.yc;
				move->yc2 = 0;
				move->prev = pundo;
				pundo = move;
				score++;

				p1 = pcard->prev1();
				p2 = pcard->prev2();
				xy = getXY(pys);
				eraseCard(hWnd, xy.x, xy.y, 0);
				pyr[pys.yc][pys.xc] = NULL;
				if (pys.yc == 0)
					root = NULL;

				delete pcard;

				xy.y -= ca.y / 2;
				xy.x -= ca.x * 11 / 20;
				if (p2 != NULL)
					p2->draw(xy, hWnd, ca);

				xy.x += ca.x * 11 / 10;
				if (p1 != NULL)
					p1->draw(xy, hWnd, ca);
				
				if (root == NULL)
					drawCard(hWnd, rtxy.x, rtxy.y, E, 0, false);

			} else {
				ps.loc = 3;
				ps.xc = pys.xc;
				ps.yc = pys.yc;
				xy = getXY(pys);
				pcard->select();
				drawCard(hWnd, xy.x, xy.y, pcard->getNum(), 0, true);
			}  //  if (pcard->getVal() == KING)
			break;

		default:
			return;

	}  //  switch (pys.loc)
	
	if (score == 52)
		winTeaser();

	return;
}

void Pyrimid::rightClick(int X, int Y) {
	PYSELECT pys = getPS(X, Y);
	int cnum = 0;
	Cards *temp;
	PCard *t2;
	KXY xy;
	bool s;
	
	if (won)
		return;

	rdn.loc = 0;
	rdn.xc = pys.xc;
	rdn.yc = pys.yc;

	switch (pys.loc) {
		case 1:
			temp = to;
			if (temp == NULL)
				temp = draw;

			if (temp == NULL)
				return;

			temp = temp->last();
			cnum = temp->getNum();
			xy = getXY(pys);
			rdn.loc = 1;
			s = (ps.loc == 1 && to != NULL);
			break;

		case 2:
			temp = pile;
			if (temp == NULL)
				return;

			temp = temp->last();
			cnum = temp->getNum();
			xy = getXY(pys);
			rdn.loc = 2;
			s = (ps.loc == 2);
			break;

		case 3:
			t2 = pyr[pys.yc][pys.xc];
			if (t2 == NULL)
				return;

			cnum = t2->getNum();
			xy = getXY(pys);
			rdn.loc = 3;
			dprintf("Right click at %i,%i loc = 3, xc = %i, yc = %i, num = %i; xy = %i,%i",
				X, Y, pys.xc, pys.yc, cnum, xy.x, xy.y);
			s = (ps.loc == 3 && ps.xc == pys.xc && ps.yc == pys.yc);
			break;

		default:
			return;
	} // switch (pys.loc)

	drawCard(hWnd, xy.x, xy.y, cnum, 0, s);

	return;
}

void Pyrimid::rightUp() {
	KXY xy;
	PCard *p;

	if (won)
		return;

	switch (rdn.loc) {
		case 1:
			rdn.loc = 0;
			if (to != NULL)
				return;

			rdn.loc = 1;
			xy = getXY(rdn);
			drawCard(hWnd, xy.x, xy.y, LASTCARD, 0, ps.loc == 1);
			break;

		case 3:
			xy = getXY(rdn);
			p = pyr[rdn.yc][rdn.xc];
			p->draw(xy, hWnd, ca);
			/*if (ps.loc == 3) {
				xy = getXY(ps);
				p = pyr[ps.yc][ps.xc];
				drawCard(hWnd, xy.x, xy.y, p->getNum(), 0, true);
			}*/
		
		default:
			UNUSED(xy);

	}  //  switch (rdn.loc)
	
	rdn.loc = 0;
	
	return;
}

int Pyrimid::load(const char *filename) {
	PYUNDO *del = NULL, *tmp = NULL;
	int i, j, cnum;
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
	
	for (i = 0; i < 7; i++)
		for (j = 0; j < 7; j++) {
			cnum =fgetc(file);
			if (cnum != 0)
				pyr[i][j] = new PCard(cnum);
	}  //  for (i = 0; i < 7; i++)

	fgetc(file);
	cnum = fgetc(file);
	for (i = 0; i < cnum; i++)
		if (draw == NULL)
			draw = new Cards(NULL, fgetc(file), false);
		else
			draw->push(new Cards(NULL, fgetc(file), false));
	

	fgetc(file);
	cnum = fgetc(file);
	if (cnum != 0)
		to = new Cards(NULL, cnum, false);

	fgetc(file);
	cnum = fgetc(file);
	for (i = 0; i < cnum; i++)
		if (pile == NULL)
			pile = new Cards(NULL, fgetc(file), false);
		else
			pile->push(new Cards(NULL, fgetc(file), false));

	fgetc(file);

	i = fgetc(file);
	if (i != 0)
		to = new Cards(NULL, i, false);

	fgetc(file);

	cnum = fgetc(file);
	for (i = 0; i < cnum; i++) {
		tmp = new PYUNDO;
		fgetc(file);
		tmp->loc1 = fgetc(file);
		tmp->loc2 = fgetc(file);
		tmp->val1 = fgetc(file);
		tmp->val2 = fgetc(file);
		tmp->xc1 = fgetc(file);
		tmp->xc2 = fgetc(file);
		tmp->yc1 = fgetc(file);
		tmp->yc2 = fgetc(file);
		tmp->prev = del;
		del = tmp;
	}

	while (del != NULL) {
		tmp = new PYUNDO;
		tmp->loc1 = del->loc1;
		tmp->loc2 = del->loc2;
		tmp->val1 = del->val1;
		tmp->val2 = del->val2;
		tmp->xc1 = del->xc1;
		tmp->xc2 = del->xc2;
		tmp->yc1 = del->yc1;
		tmp->yc2 = del->yc2;
		tmp->prev = pundo;
		pundo = tmp;
		tmp = del;
		del = del->prev;
		delete tmp;
	}

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

	//  Now we have game data...  explode it.
	root = pyr[0][0];
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < i + 1; j++) {
			if (pyr[i][j] != NULL) {
				pyr[i][j]->setNext1(pyr[i+1][j]);
				pyr[i][j]->setNext2(pyr[i + 1][j + 1]);
			}
		}
	}

	redrawBoard();
	fclose(file);
	return TRUE;
}

int Pyrimid::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i, j, cnum;
	Cards *cur;
	PCard *pc;
	PYUNDO *u = pundo;

	if (file == NULL)
		return FALSE;

	if (score == 52) {
		fclose(file);
		return EOF;
	}

	//  Generic solitaire header
	fputc(datum, file);

	//  Pyrimid Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	52; i++)
		fputc(deck[i], file);

	fputc(0xFF, file);

	//  put main board information
	for (i = 0; i < 7; i++)
		for (j = 0; j < 7; j++) {
			cnum = 0;
			pc = pyr[i][j];
			if (pc != NULL)
				cnum = pc->getNum();
			fputc(cnum, file);
	}  //  for (i = 0; i < 7; i++)

	fputc(0xFF, file);
	cnum = 0;
	if (draw != NULL)
		cnum = draw->getNumElements();
	
	fputc(cnum, file);
	cur = draw;
	while (cur != NULL) {
		fputc(cur->getNum(), file);
		cur = cur->Next();
	}

	fputc(0xFF, file);
	cnum = 0;
	if (to != NULL)
		cnum = to->getNum();

	fputc(cnum, file);
	fputc(0xFF, file);
	
	cnum = 0;
	if (pile != NULL)
		cnum = pile->getNumElements();

	fputc(cnum, file);
	cur = pile;
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
	while (u != NULL) {
		u = u->prev;
		i++;
	}
	fputc(i, file);

	u = pundo;
	while (u != NULL) {
		fputc(0xFF, file);
		fputc(u->loc1, file);
		fputc(u->loc2, file);
		fputc(u->val1, file);
		fputc(u->val2, file);
		fputc(u->xc1, file);
		fputc(u->xc2, file);
		fputc(u->yc1, file);
		fputc(u->yc2, file);
		u = u->prev;
	}

	fputc(0xFF, file);
	fputc(score, file);
	//  Final marker
	fputc(0xFF, file);

	i = fclose(file);
	if (i != 0)
		return i;

	return TRUE;
}

void Pyrimid::redrawBoard() {
	Cards *t;
	PCard *p;
	KXY xy;

	if (won)
		return;

	clearBoard(hWnd);

	if (root != NULL)
		root->draw(rtxy, hWnd, ca);
	else
		drawCard(hWnd, rtxy.x, rtxy.y, E, 0, false);

	if (draw != NULL)
		drawCard(hWnd, ca.x, ca.y, LASTCARD, 0, false);
	else
		drawCard(hWnd, ca.x, ca.y, E, 0, false);

	if (to != NULL)
		drawCard(hWnd, ca.x, ca.y, to->getNum(), 0, ps.loc == 1);

	if (pile != NULL) {
		t = pile->last();
		drawCard(hWnd, ca.x * 2 + ca.x / 10, ca.y, t->getNum(), 0, ps.loc == 2);
	} else
		drawCard(hWnd, ca.x * 2 + ca.x / 10, ca.y, E, 0, false);

	if (ps.loc == 3) {
		xy = getXY(ps);
		p = pyr[ps.yc][ps.xc];
		drawCard(hWnd, xy.x, xy.y, p->getNum(), 0, true);
	}
	
	bprintf(hWnd, ca.x / 2, ca.y * 2, BWHITE, "Score: %i", score);

	return;
}

void Pyrimid::undo() {
	KXY xy;
	PYSELECT pys1, pys2;
	short cnum1, cnum2;
	int i, j;
	Cards *c;
	PCard *pc, *pc1, *pc2;
	PYUNDO *move;

	if (pundo == NULL || won)
		return;

	if (ps.loc != 0) {
		//  always unselect when undoing
		xy = getXY(ps);
		leftClick(xy.x, xy.y);
	}

	if (pundo->val2 == 255) {
		//  Internal move
		if (pundo->loc2 == 2) {
			to = pile->pop();
			if (to == pile)
				pile = NULL;


			drawCard(hWnd, ca.x, ca.y, to->getNum(), 0, false);
			pys1.loc = 2;
			pys1.xc = 0;
			pys1.yc = 0;

			xy = getXY(pys1);
			cnum1 = E;
			if (pile != NULL) {
				c = pile->last();
				cnum1 = c->getNum();
			}

			drawCard(hWnd, xy.x, xy.y, cnum1, 0, false);
		}  else  {
			if (draw == NULL)
				draw = to;
			else
				draw->push(to);

			to = NULL;

			drawCard(hWnd, ca.x, ca.y, LASTCARD, 0, false);
						
		}  //  if (pundo->loc2 == 2)

	}  else {
		//  match move
		pys1.loc = pundo->loc1;
		pys2.loc = pundo->loc2;
		pys1.xc = pundo->xc1;
		pys2.xc = pundo->xc2;
		pys1.yc = pundo->yc1;
		pys2.yc = pundo->yc2;
		cnum1 = pundo->val1;
		cnum2 = pundo->val2;
		
		//  Done in reverse order as the origional selected object will always be 
		//  at the bottom, the second item might be above it.  
		switch (pys2.loc) {
			case 1:
				to = new Cards(NULL, cnum1, false);
				drawCard(hWnd, ca.x, ca.y, cnum2, 0, false);
				break;

			case 2:
				if (pile == NULL)
					pile = new Cards(NULL, cnum2, false);
				else
					pile->push(new Cards(NULL, cnum2, false));

				xy = getXY(pys2);
				drawCard(hWnd, xy.x, xy.y, cnum2, 0, false);
				break;

			case 3:
				pc = new PCard(cnum2);
				pyr[pys2.yc][pys2.xc] = pc;
				xy = getXY(pys2);
				i = pys2.yc - 1;
				j = pys2.xc;
				pc1 = NULL;
				pc2 = NULL;

				if (i >= 0) {
					pc1 = pyr[i][j];
					if (j > 0)
						pc2 = pyr[i][j - 1];
				}

				if (pc1 != NULL)
					pc1->setNext1(pc);

				if (pc2 != NULL)
					pc2->setNext2(pc);

				pc->draw(xy, hWnd, ca);
				break;

			default:
				score++;
		}  //  switch (pys2.loc)

		switch (pys1.loc) {
			case 1:
				to = new Cards(NULL, cnum1, false);
				drawCard(hWnd, ca.x, ca.y, cnum1, 0, false);
				break;

			case 2:
				if (pile == NULL)
					pile = new Cards(NULL, cnum1, false);
				else
					pile->push(new Cards(NULL, cnum1, false));

				xy = getXY(pys1);
				drawCard(hWnd, xy.x, xy.y, cnum1, 0, false);
				break;

			case 3:
				pc = new PCard(cnum1);
				pyr[pys1.yc][pys1.xc] = pc;
				xy = getXY(pys1);
				i = pys1.yc - 1;
				j = pys1.xc;
				pc1 = NULL;
				pc2 = NULL;

				if (i >= 0) {
					pc1 = pyr[i][j];
					if (j > 0)
						pc2 = pyr[i][j - 1];
				}

				if (pc1 != NULL)
					pc1->setNext1(pc);

				if (pc2 != NULL)
					pc2->setNext2(pc);

				pc->draw(xy, hWnd, ca);
				break;

			default:
				score++;
		}  //  switch (pys1.loc)
		
		score -= 2;
		bprintf(hWnd, ca.x / 2, ca.y * 2, BWHITE, "Score:          ");
		bprintf(hWnd, ca.x / 2, ca.y * 2, BWHITE, "Score: %i", score);

	}//  if (pundo->val2 == 255)

	move = pundo;
	pundo = pundo->prev;
	delete move;

	return;
}

char* Pyrimid::gameName() {

	return "Pyrimid";
}

void Pyrimid::left() {

	KXY xy;
	if (won)
		return;

	valKBD();

	switch (kbd.loc) {
		case 1:
			kbd.loc = 3;
			kbd.xc = 6;
			kbd.yc = 6;
			valKBD();
            break;

		case 2:
			kbd.loc = 1;
			break;

		case 3:
			kbd.xc--;
			if (kbd.xc < 0) {
				kbd.yc = 0;
				kbd.xc = 0;
				kbd.loc = 2;
			} else 
				valKBD();

			break;

	}  //  switch (kbd.loc)

	xy = getXY(kbd);
	xy.y -= ca.y / 6;
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Pyrimid::right() {
	KXY xy;

	if (won)
		return;

	valKBD();

	switch (kbd.loc) {
		case 1:
			kbd.loc = 2;
			break;

		case 2:
			kbd.loc = 3;
			kbd.xc = 0;
			kbd.yc = 6;
			valKBD();
			break;

		case 3:
			kbd.xc++;
			if (kbd.xc > kbd.yc) {
				kbd.loc = 1;
				kbd.xc = 0;
				kbd.yc = 0;
			} else 
				valKBD();
	
			break;
	}  //  switch (kbd.loc)

	xy = getXY(kbd);
	xy.y -= ca.y / 6;
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Pyrimid::up() {

	int i, j;
	PCard *pc;
	KXY xy;

	if (won)
		return;
	
	if (kbd.loc != 3) {
		xy = getXY(kbd);
		xy.y -= ca.y / 6;
		putMouse(xy.x, xy.y, hWnd);
		return;
	}

	valKBD();
	i = kbd.yc;
	j = kbd.xc;

	i--;
	if (i < 0)
		i = 0;

	if (j > i)
		j = i;

	pc = pyr[i][j];
	if (pc == NULL)
		j++;

	kbd.xc = j;
	kbd.yc = i;
	xy = getXY(kbd);
	xy.y -= ca.y / 6;
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Pyrimid::down() {
	
	int i, j;
	PCard *pc;
	KXY xy;
	bool t = false;

	if (won)
		return;

	if (kbd.loc < 3) {
		kbd.loc = 3;
		kbd.yc = 6;
		kbd.xc = 0;
		valKBD();
		xy = getXY(kbd);
		xy.y -= ca.y / 6;
		putMouse(xy.x, xy.y, hWnd);
		return;
	}

	valKBD();
	i = kbd.yc;
	j = kbd.xc;

	i++;
	if (i > 6)
		i--;

	pc = pyr[i][j];

	while (pc == NULL && j <= i) {
		t = true;
		pc = pyr[i][j];
		j++;
	}

	if (pc != NULL && t)
		j--;
	
	t = false;
	while (pc == NULL && j >= 0) {
		pc = pyr[i][j];
		j--;
		t = true;
	}

	if (t)
		j++;

	if (pc != NULL) {
		kbd.xc = j;
		kbd.yc = i;
	}

	xy = getXY(kbd);
	xy.y -= ca.y / 6;
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Pyrimid::enter() {

	KXY xy;
	int i, j;

	if (won)
		return;

	valKBD();

	xy = getXY(kbd);
	xy.y -= ca.y / 6;
	putMouse(xy.x, xy.y, hWnd);

	leftClick(xy.x, xy.y);	
	i = kbd.yc;
	j = kbd.xc;
	valKBD();

	if (kbd.xc != j || kbd.yc != i) {
		kbd.xc = j;
		kbd.yc = i;
		down();
	}

	return;
}

void Pyrimid::deal() {
	int c = 0;

	delInst();

	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < i + 1; j++) {
			pyr[i][j] = new PCard(deck[c]);
			c++;
		}
	}

	draw = new Cards(NULL, deck[c], false);
	c++;

	while (c < 52) {
		draw->push(new Cards(NULL, deck[c], false));
		c++;
	}
	
	root = pyr[0][0];
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < i + 1; j++) {
			pyr[i][j]->setNext1(pyr[i+1][j]);
			pyr[i][j]->setNext2(pyr[i + 1][j + 1]);
		}
	}

	return;
}

void Pyrimid::delInst() {
	
	Cards *t = pile, *q;
	PYUNDO *u2;

	ps.loc = 0;

	while (t != NULL) {
		q = t;
		t = t->Next();
		if (t != NULL)
			t->setPrev(NULL);

		delete q;
	}
	pile = NULL;

	t = draw;
	while (t != NULL) {
		q = t;
		t = t->Next();
		if (t != NULL)
			t->setPrev(NULL);

		delete q;
	}
	draw = NULL;

	if (to != NULL) {
		delete to;
		to = NULL;
	}

/*	pyr[0][0] = root;
	
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < i + 1; j++)
			if (pyr[i][j] != NULL) {
				pyr[i + 1][j] = pyr[i][j]->next1();
				pyr[i + 1][j + 1] = pyr[i][j]->next2();
			}*/

	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			if (pyr[i][j] != NULL)
				delete pyr[i][j];
    
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			pyr[i][j] = NULL;

	root = NULL;
	score = 0;
	kbd.loc = 1;
	kbd.xc = 0;
	kbd.yc = 0;

	while (pundo != NULL) {
		u2 = pundo->prev;
		delete pundo;
		pundo = u2;
	}
	won = false;

	return;
}

PYSELECT Pyrimid::getPS(int x, int y) {
	PYSELECT psel;
	KXY kxy;
	int ly = 0, lx = 0;

	psel.loc = 0;
	psel.xc = 0;
	psel.yc = 0;
	kxy.x = ca.x;
	kxy.y = ca.y;
	

	if (x > kxy.x / 2 && x < kxy.x + (ca.x / 2) && y > kxy.y / 2 && y < kxy.y + (kxy.y / 2))
		psel.loc = 1;

	kxy.x += ca.x + ca.x / 10;
	if (x > kxy.x - (ca.x / 2) && x < kxy.x + (ca.x / 2) && y > kxy.y / 2 && y < kxy.y + (kxy.y / 2))
		psel.loc = 2;

	if (psel.loc != 0)
		return psel;

	kxy.x = rtxy.x;
	kxy.x -= ca.x / 2;
	kxy.y = rtxy.y;
	kxy.y -= ca.y / 2;

	y -= kxy.y;
	kxy.y = 0;

	if (y < 0)
		return psel;

	while (ly < y) {
		ly += ca.y / 2;
		psel.yc++;
	}
	
	psel.yc--;
	if (psel.yc > 7)
		return psel;

	lx = rtxy.x;
	lx -= ca.x / 2;
	lx -= psel.yc * ca.x * 11 / 20;
	psel.xc = 0;

	if (psel.yc < 7) {
		while (lx < x && psel.loc == 0) {
			if (x > lx && x < lx + ca.x && pyr[psel.yc][psel.xc] != NULL)
				psel.loc = 3;
		
			psel.xc++;
			lx += ca.x * 11 / 10;

			if (psel.xc > 7) {
				psel.loc = 0;
				return psel;
			}

		}  //  while (lx < x && psel.loc == 0)
	}   //  if (psel.yc < 7)

	psel.xc--;
	if (psel.loc != 0 || psel.yc == 0)
		return psel;

	psel.yc--;
	lx = rtxy.x;
	lx -= ca.x / 2;
	lx -= psel.yc * ca.x * 11 / 20;
	psel.xc = 0;
	ly -= ca.y / 2;

	while (lx < x && psel.loc == 0) {
		if (x > lx && x < lx + ca.x && pyr[psel.yc][psel.xc] != NULL)
			psel.loc = 3;
		
		psel.xc++;
		lx += ca.x * 11 / 10;

		if (psel.xc > 7) {
			psel.loc = 0;
			return psel;
		}

	}  //  while (lx < x && psel.loc == 0)

	psel.xc--;
	return psel;
}

KXY Pyrimid::getXY(PYSELECT pys) {
	KXY xy;
	int sx1, sx2, sy;
	xy.x = 0;
	xy.y = 0;

	switch (pys.loc) {
		case 1:
			xy.x = ca.x;
			xy.y = ca.y;
			break;

		case 2:
			xy.x = ca.x * 2 + ca.x / 10;
			xy.y = ca.y;
			break;

		case 3:
			xy.x = rtxy.x;
			xy.y = rtxy.y;
			sy = ca.y / 2;
			xy.y += sy * pys.yc;
			sx1 = ca.x * 11 / 20;
			sx2 = ca.x * 11 / 10;
			xy.x -= pys.yc * sx1;
			xy.x += sx2 * pys.xc;
			break;

		default:
			UNUSED(pys);
	}

    return xy;
}

void Pyrimid::calcMove(PYSELECT pys) {
	PCard *pc, *pc1, *pc2;
	Cards *c;
	KXY xy;
	int cnum, val1, val2;
	PYUNDO *move;

	if (pys.loc == ps.loc && pys.xc == ps.xc && pys.yc == ps.yc) {
		xy = getXY(ps);
		switch (ps.loc) {
			case 1:
				cnum = to->getNum();
				break;

			case 2:
				c = pile->last();
				cnum = c->getNum();
				break;

			case 3:
				pc = pyr[ps.yc][ps.xc];
				pc->unselect();
				pc->draw(xy, hWnd, ca);
				ps.loc = 0;
				return;
				break;
		}  //  switch (ps.loc)
		drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
		ps.loc = 0;
		return;
	}  //  if (pys.loc == ps.loc && pys.xc == ps.xc && pys.yc == ps.yc)

	if (pys.loc == 1 && to == NULL && draw != NULL) {
		to = draw->pop();
		if (to == draw)
			draw = NULL;
		drawCard(hWnd, ca.x, ca.y, to->getNum(), 0, false);
		move = new PYUNDO;
		move->loc1 = 1;
		move->loc2 = 0;
		move->val1 = to->getNum();
		move->val2 = 255;
		move->xc1 = 0;
		move->xc2 = 0;
		move->yc1 = 0;
		move->yc2 = 0;
		move->prev = pundo;
		pundo = move;
		return;
	}  //  if (pys.loc == 1 && to == NULL && draw != NULL)

	if (ps.loc == 1 && pys.loc == 2) {
		val1 = to->getVal();
		val2 = 0;
		if (pile != NULL) {
			c = pile->last();
			val2 = c->getVal();
		}
//		dprintf("val1 = %i, val2 = %i, sum = %i", val1, val2, val1 + val2);

		if (val1 + val2 == KING)
			goto BK;

		if (pile == NULL)
			pile = to;
		else
			pile->push(to);

		move = new PYUNDO;
		move->loc1 = 1;
		move->loc2 = 2;
		move->val1 = to->getNum();
		move->val2 = 255;
		move->xc1 = 0;
		move->xc2 = 0;
		move->yc1 = 0;
		move->yc2 = 0;
		move->prev = pundo;
		pundo = move;
		to = NULL;

		cnum = E;
		if (draw != NULL)
			cnum = LASTCARD;

		drawCard(hWnd, ca.x, ca.y, cnum, 0, false);

		cnum = E;
		if (pile != NULL) {
			c = pile->last();
			cnum = c->getNum();
		}

		xy = getXY(pys);
		drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
		ps.loc = 0;

		return;
	}  //  if (ps.loc = 1 && pys.loc = 2)

BK:	switch (ps.loc) {
		case 1:
			val1 = to->getVal();
			break;
		
		case 2:
			c = pile->last();
			val1 = c->getVal();
			break;

		case 3:
			pc = pyr[ps.yc][ps.xc];
			val1 = pc->getVal();
			break;
	}  //  switch (ps.loc)

	switch (pys.loc) {
		case 1:
			if (to == NULL)
				return;

			val2 = to->getVal();
			break;

		case 2:
			if (pile == NULL)
				return;

			c = pile->last();
			val2 = c->getVal();
			break;

		case 3:
			pc = pyr[pys.yc][pys.xc];
			if (pc == NULL)
				return;

			pc1 = pc->next1();
			if (pc1 != NULL)
				if (!pc1->isSelected())
					return;

			pc2 = pc->next2();
			if (pc2 != NULL)
				if (!pc2->isSelected())
					return;

			val2 = pc->getVal();
			break;
	}

	if (val1 + val2 == KING) {
		move = new PYUNDO;
		move->loc1 = ps.loc;
		move->loc2 = pys.loc;
		move->xc1 = ps.xc;
		move->xc2 = pys.xc;
		move->yc1 = ps.yc;
		move->yc2 = pys.yc;
		move->prev = pundo;
		pundo = move;
		
		switch (ps.loc) {
			case 1:
				move->val1 = to->getNum();
				cnum = E;
				if (draw != NULL)
					cnum = LASTCARD;

				drawCard(hWnd, ca.x, ca.y, cnum, 0, false);
				delete to;
				to = NULL;
				break;

			case 2:
				c = pile->pop();
				if (c == pile)
					pile = NULL;

				move->val1 = c->getNum();
				delete c;

				cnum = E;
				if (pile != NULL) {
					c = pile->last();
					cnum = c->getNum();
				}

				xy = getXY(ps);
				drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
				break;

			case 3:
				pc = pyr[ps.yc][ps.xc];
				move->val1 = pc->getNum();
				pc1 = pc->prev1();
				pc2 = pc->prev2();
				xy = getXY(ps);
				eraseCard(hWnd, xy.x, xy.y, 0);
				pyr[ps.yc][ps.xc] = NULL;
				if (ps.yc == 0)
					root = NULL;

				delete pc;

				xy.y -= ca.y / 2;
				xy.x -= ca.x * 11 / 20;
				if (pc2 != NULL)
					pc2->draw(xy, hWnd, ca);

				xy.x += ca.x * 11 / 10;
				if (pc1 != NULL)
					pc1->draw(xy, hWnd, ca);

				if (root == NULL)
					drawCard(hWnd, rtxy.x, rtxy.y, E, 0, false);

				break;
		}  //  switch (ps.loc)
	
		switch (pys.loc) {
			case 1:
				move->val2 = to->getNum();
				cnum = E;
				if (draw != NULL)
					cnum = LASTCARD;

				drawCard(hWnd, ca.x, ca.y, cnum, 0, false);
				delete to;
				to = NULL;
				break;

			case 2:
				c = pile->pop();
				if (c == pile)
					pile = NULL;

				move->val2 = c->getNum();
				delete c;

				cnum = E;
				if (pile != NULL) {
					c = pile->last();
					cnum = c->getNum();
				}

				xy = getXY(pys);
				drawCard(hWnd, xy.x, xy.y, cnum, 0, false);
				break;

			case 3:
				pc = pyr[pys.yc][pys.xc];
				move->val2 = pc->getNum();
				pc1 = pc->prev1();
				pc2 = pc->prev2();
				xy = getXY(pys);
				eraseCard(hWnd, xy.x, xy.y, 0);
				pyr[pys.yc][pys.xc] = NULL;
				if (pys.yc == 0)
					root = NULL;

				delete pc;

				xy.y -= ca.y / 2;
				xy.x -= ca.x * 11 / 20;
				if (pc2 != NULL)
					pc2->draw(xy, hWnd, ca);

				xy.x += ca.x * 11 / 10;
				if (pc1 != NULL)
					pc1->draw(xy, hWnd, ca);
				
				if (root == NULL)
					drawCard(hWnd, rtxy.x, rtxy.y, E, 0, false);

				break;
		}  //  switch (pys.loc)
	
		ps.loc = 0;
		score += 2;
		bprintf(hWnd, ca.x / 2, ca.y * 2, BWHITE, "Score: %i", score);
		return;
	}  //  if (val1 + val2 == KING)

	if (ps.loc == 1 && pys.loc == 2) {
		if (pile == NULL)
			pile = to;
		else
			pile->push(to);

		move = new PYUNDO;
		move->loc1 = 1;
		move->loc2 = 2;
		move->val1 = to->getNum();
		move->val2 = 255;
		move->xc1 = 0;
		move->xc2 = 0;
		move->yc1 = 0;
		move->yc2 = 0;
		move->prev = pundo;
		pundo = move;
		to = NULL;

		cnum = E;
		if (draw != NULL)
			cnum = LASTCARD;

		drawCard(hWnd, ca.x, ca.y, cnum, 0, false);

		cnum = E;
		if (pile != NULL) {
			c = pile->last();
			cnum = c->getNum();
		}

		xy = getXY(pys);
		drawCard(hWnd, xy.x, xy.y, cnum, 0, false);

		return;
	}  //  if (ps.loc = 1 && pys.loc = 2)

	//  shouldn't get here.	
	
	return;
}

void Pyrimid::valKBD() {

	PCard *pc;
	int i, j;

	if (kbd.loc != 3)
		return;

	j = kbd.xc;
	i = kbd.yc;

	if (i > 6)
		i = 6;

	if (j > 6)
		j = 6;

	pc = pyr[kbd.yc][kbd.xc];
	while (i > 0 && pc == NULL) {
		i--;
		if (j > i)
			j = i;

		pc = pyr[i][j];
		if (pc == NULL && j > 0)
			pc = pyr[i][j - 1];

	}

	kbd.yc = i;
	kbd.xc = j;

	return;
}

int Pyrimid::winWork(void* datum) {
	
	UNUSED(datum);

	Sleep(30);
	clearBoard(hWnd);
	hit = false;
	won = true;

	runAnim();
	//clearBoard(hWnd);
	return Win(hWnd, this);
}

void Pyrimid::runAnim() {

	CARDDIM sa = getBoardDims(hWnd);
	KXY xy, c;
	int cnum = 52, bnum = 0, t = -26, dt = 1, disp = LASTCARD;
	bool up = false;
	double dx, dy, r;

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

	return;
}

Pyrimid::~Pyrimid() {

	delInst();
	return;
}
