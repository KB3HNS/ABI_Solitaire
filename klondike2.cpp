// Klondike II game code.
#include "stdafx.h"
#include "game.h"
#include "klondike2.h"
#include "debug.h"
#include "cardtools.h"
#include "colordefs.h"
#include "winteaser.h"
#include <time.h>

Klondike2::Klondike2(HWND hWind, int info) {

	CARDDIM c;
	Init(hWind, info);

	for (int i = 0; i < 7; i++) {
		top[i] = NULL;
		bot[i] = NULL;
		if (i < 4)
			pile[i] = NULL;
	}

	Undo = NULL;
	rdn = false;
	rmouse.jval = -1;
	sel.selected = false;
	score = 0;

	c = getCardAttributes();
	ht = c.Height;
	wd = c.Width;

	return;
}

void Klondike2::leftClick(int X, int Y) {
	
	Cards *cur;
	KXY xy;
	const int inc2 = ht / 5;
	KREGION flip, lm = getKR(X, Y);
	KUNDO *mv;
	
	//dprintf("Left mouse click on %i, %i, %i at %i, %i", lm.jval, lm.lval, lm.pval, X, Y);

	if (score == 52)
		return;

	if (lm.jval == -1)
		return;

	if (lm.jval == 1) {
		//  Flip code
		cur = top[lm.lval];
		if (cur != NULL)
			return;

		cur = bot[lm.lval];
		if (cur == NULL)
			return;

		top[lm.lval] = cur->pop();
		if (top[lm.lval] == cur)
			bot[lm.lval] = NULL;
		
		mv = new KUNDO;
		mv->prev = Undo;
		Undo = mv;
		Undo->fromJ = 1;
		Undo->fromL = lm.lval;
		Undo->toJ = 2;
		Undo->toL = lm.lval;
		Undo->toP = 0;

		flip.jval = 2;
		flip.lval = lm.lval;
		flip.pval = 0;
		xy = getXY(flip);
		drawCard(hWnd, xy.x, xy.y, top[lm.lval]->getNum(), 0, false);

	}  //  if (lm.jval == 1)

	if (!sel.selected) {
		if (lm.jval != 2)
			return;  //  You can only select a card that is up and not in the winner's pile
	

		cur = top[lm.lval];
		if (cur == NULL)
			return;

		sel.lval = lm.lval;
		sel.pval = lm.pval;
		sel.selected = true;
		cur = cur->getElement(lm.pval);

		xy = getXY(lm);

		drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, true);
	
	}  else if (lm.jval == 2 && lm.lval == sel.lval && lm.pval == sel.pval) {
		sel.selected = false;
		xy = getXY(lm);
		cur = top[lm.lval];
		cur = cur->getElement(lm.pval);
		
		while (cur != NULL) {
			drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, false);
			cur = cur->Next();
			xy.y += inc2;
		}

	} else
		calcMove(lm);
	//  if (!sel.selected)
	
	if (score == 52)
		winTeaser();

	return;
}

void Klondike2::rightClick(int X, int Y) {
	Cards *rck;
	int cnum;
	KXY xy;
	KREGION rm = getKR(X, Y);

	if (score == 52)
		return;

	rmouse.jval = rm.jval;
	rmouse.lval = rm.lval;
	rmouse.pval = rm.pval;

	//dprintf("Right mouse click on %i, %i, %i at %i, %i", rm.jval, rm.lval, rm.pval, X, Y);
	rdn = true;
	switch (rm.jval) {
		case 1:
			rck = bot[rm.lval];
			rck = rck->last();
			break;

		case 2:
			rck = top[rm.lval];
			rck = rck->getElement(rm.pval);
			break;

		case 3:
			rck = pile[rm.lval];
			rck = rck->getElement(rm.pval);
			break;

		case -1:
			return;
	}

	cnum = E;
	if (rck != NULL)
		cnum = rck->getNum();

	if (cnum != E && rm.jval == 1)
		cnum = LASTCARD;

	xy = getXY(rm);

	drawCard(hWnd, xy.x, xy.y, cnum, 0, false);

	return;
}

void Klondike2::rightUp() {
	Cards *cur;
	KXY xy;
	KREGION kreg;
	int inc1 = ht / 20, inc2 = ht / 5;

	rdn = false;

	if (score == 52)
		return;


	if (rmouse.jval == -1 || rmouse.jval == 3)
		return;

	xy = getXY(rmouse);
	if (rmouse.jval == 1 && bot[rmouse.lval] != NULL) {
		cur = bot[rmouse.lval];
		cur = cur->getElement(rmouse.pval);
		cur = cur->Next();
		xy.y += inc1;
		while (cur != NULL) {
			drawCard(hWnd, xy.x, xy.y, LASTCARD, 0, false);
			cur = cur->Next();
			xy.y += inc1;
		}  //  while (cur != NULL)
		rmouse.jval = 2;
		rmouse.pval = 0;
		xy = getXY(rmouse);
	}  //  if (rmouse.jval == 1)

	cur = top[rmouse.lval];
	cur = cur->getElement(rmouse.pval);
	while (cur != NULL) {
		drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, false);
		cur = cur->Next();
		xy.y += inc2;
	}

	if (sel.selected && sel.lval == rmouse.lval) {
		kreg.jval = 2;
		kreg.lval = sel.lval;
		kreg.pval = sel.pval;
		cur = top[sel.lval];
		cur = cur->getElement(sel.pval);
		xy = getXY(kreg);
		drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, true);
	}
		
	rmouse.jval = -1;

	return;
}

int Klondike2::load(const char *filename) {

	Cards *cur;
	KUNDO *del = NULL, *tmp = NULL;
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

	for(i = 0; i < 52; i++)
		deck[i] = fgetc(file);

	fgetc(file);
	for (i = 0; i < 4; i++) {
		j = fgetc(file);
		cur = NULL;
		pile[i] = NULL;
		for (int k = 0; k < j; k++) {
			cur = new Cards(cur, fgetc(file));
			if (k == 0)
				pile[i] = cur;
		}
		fgetc(file);
	}  //  for (i = 0; i < 4; i++)

	for (i = 0; i < 7; i++) {
		j = fgetc(file);
		cur = NULL;
		bot[i] = NULL;
		for (int k = 0; k < j; k++) {
			cur = new Cards(cur, fgetc(file));
			if (k == 0)
				bot[i] = cur;
		}
		fgetc(file);

		j = fgetc(file);
		cur = NULL;
		top[i] = NULL;
		for (int k = 0; k < j; k++) {
			cur = new Cards(cur, fgetc(file));
			if (k == 0)
				top[i] = cur;
		}
		fgetc(file);
	}  //  for (i = 0; i < 7; i++)

	j = fgetc(file);
	for (i = 0; i < j; i++) {
		tmp = new KUNDO;
		tmp->fromJ = fgetc(file);
		tmp->fromL = fgetc(file);
		tmp->toJ = fgetc(file);
		tmp->toL = fgetc(file);
		tmp->toP = fgetc(file);
		fgetc(file);
		if (tmp->toP == 255)
			tmp->toP = -1;
		tmp->prev = del;
		del = tmp;
	}

	//  now... swap the structure:
	Undo = NULL;
	for (i = 0; i < j; i++) {
		tmp = new KUNDO;
		tmp->fromJ = del->fromJ;
		tmp->fromL = del->fromL;
		tmp->toJ = del->toJ;
		tmp->toL = del->toL;
		tmp->toP = del->toP;
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

int Klondike2::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i;
	Cards *cur;
	KUNDO *u = Undo;

	if (file == NULL)
		return FALSE;

	if (score == 52) {
		fclose(file);
		return EOF;
	}

	//  Generic solitaire header
	fputc(datum, file);

	//  Klondike 2 Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	52; i++)
		fputc(deck[i], file);

	fputc(0xFF, file);
	//  Put the piles first:
	for (i = 0; i < 4; i++) {
		cur = pile[i];
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

	//  Board data
	for (i = 0; i < 7; i++) {
		cur = bot[i];
		if (cur == NULL)
			fputc(0, file);
		else
			fputc(cur->getNumElements(), file);

		while (cur != NULL)  {
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
	}  //  for (i = 0; i < 7; i++)

	//  Save undo data
	i = 0;
	while (u != NULL) {
		u = u->prev;
		i++;
	}

	fputc(i, file);
	u = Undo;
	while (u != NULL) {
		i = u->toP;
		if (i == -1)
			i = 255;  //  downsizing 4 bytes to 1
		fputc(u->fromJ, file);
		fputc(u->fromL, file);
		fputc(u->toJ, file);
		fputc(u->toL, file);
		fputc(i, file);
		fputc(0xFF, file);
		u = u->prev;
	}

	fputc(score, file);
	//  Final marker
	fputc(0xFF, file);

	i = fclose(file);
	if (i != 0)
		return i;

	return TRUE;
}

void Klondike2::redrawBoard() {
	
	Cards *cur;
	KXY xy;
	KREGION kreg;

	int x = wd, y = ht * 5 / 8, inc1 = ht / 20, inc2 = ht / 5, incx = wd * 5 / 4;

	if (score == 52)
		return;

	clearBoard(hWnd);

	for (int i = 0; i < 4; i++)  {
		if (pile[i] == NULL)
			drawCard(hWnd, x, y, E, 0, false);
		else {
			cur = pile[i]->last();
			drawCard(hWnd, x, y, cur->getNum(), 0, false);
		}
		x += incx * 2;
	}

	for (int i = 0; i < 7; i++) {
        x = wd;
		y = ht * 7 / 4;
		x += i * incx;

		cur = bot[i];
		drawCard(hWnd, x, y, E, 0, false);	
		while (cur != NULL) {
			drawCard(hWnd, x, y, LASTCARD, 0, false);
			y += inc1;
			cur = cur->Next();
		}

		cur = top[i];
		while (cur != NULL) {
			drawCard(hWnd, x, y, cur->getNum(), 0, false);
			y += inc2;
			cur = cur->Next();
		}
	}  // for (int i = 0; i < 7; i++)

	if (sel.selected) {
		cur = top[sel.lval];
		cur = cur->getElement(sel.pval);

		kreg.jval = 2;
		kreg.lval = sel.lval;
		kreg.pval = sel.pval;

		xy = getXY(kreg);

		drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, true);
		
	}  //  if (sel.selected)
	
	return;
}

void Klondike2::undo() {

	//  Hmm.. first some objects.
	Cards *cur, *sl;
	KUNDO *now = Undo;
	KREGION kreg, slreg;
	KXY xy;
	int n;
	const int inc2 = ht / 5;

	if (score == 52)
		return;

	if (Undo == NULL)
		return;

	if (sel.selected) {
		//  We want to unselect any cards to avoid bugs.
		kreg.jval = 2;
		kreg.lval = sel.lval;
		kreg.pval = sel.pval;
		xy = getXY(kreg);
		leftClick(xy.x, xy.y);
	}

	//  first try to figure out what type of move we are trying to undo
	//  type 1:  a standerd move
	//  type 2:  move an item to the top
	//  type 3:  flipping a card.  
	//  This has NOTHING to do with jval
	//  Now..  Enough talk. Let's do it.

	//  type 3
	Undo = Undo->prev;
	if (now->fromJ == 1) {
		kreg.lval = now->fromL;
		kreg.jval = 2;
		kreg.pval = 0;
		xy = getXY(kreg);
		drawCard(hWnd, xy.x, xy.y, LASTCARD, 0, false);
		if (bot[kreg.lval] == NULL)
			bot[kreg.lval] = top[kreg.lval];
		else
			bot[kreg.lval]->push(top[kreg.lval]);

		top[kreg.lval] = NULL;
		delete now;
		return;	
	}

	//  type 2
	if (now->toJ == 3) {
		kreg.jval = 3;
		kreg.lval = now->toL;
		kreg.pval = 0;
		xy = getXY(kreg);

		cur = pile[kreg.lval];
		cur = cur->pop();

		if (cur == pile[kreg.lval]) {
			pile[kreg.lval] = NULL;
			drawCard(hWnd, xy.x, xy.y, E, 0, false);
		} else {
			sl = pile[kreg.lval];
			sl = sl->last();
			drawCard(hWnd, xy.x, xy.y, sl->getNum(), 0, false);
		}
			
		sl = top[now->fromL];
		if (sl == NULL)
			top[now->fromL] = cur;
		else
			sl->push(cur);

		sl = top[now->fromL];
		kreg.jval = 2;
		kreg.lval = now->fromL;
		kreg.pval = sl->getNumElements() - 1;
		xy = getXY(kreg);
		drawCard(hWnd, xy.x, xy.y, cur->getNum(), 0, false);
		score--;

		delete now;
		return;
	}

	//  default:  type 1
	now->toP++;
	kreg.jval = 2;
	kreg.lval = now->toL;
	kreg.pval = now->toP;

	xy = getXY(kreg);
	cur = top[kreg.lval];
	cur = cur->getElement(kreg.pval);
	sl = cur;
	while (sl != NULL) {
		eraseCard(hWnd, xy.x, xy.y, 0);
		sl = sl->Next();
		xy.y += inc2;
	}

		cur = top[kreg.lval];
	if (kreg.pval == 0)
		top[kreg.lval] = NULL;
	else {
		cur = cur->getElement(now->toP);
		sl = cur->Prev();
		cur->setPrev(NULL);

		if (sl != NULL)
			sl->setNext(NULL);
	}  //  if (kreg.pval == 0)

	kreg.pval--;
	if (sl != NULL) {
		xy = getXY(kreg);
		drawCard(hWnd, xy.x, xy.y, sl->getNum(), 0, false);
	} else {
		kreg.jval = 1;
		sl = bot[kreg.lval];
		n = E;
		kreg.pval = 0;
		
		if (sl != NULL) {
			kreg.pval = sl->getNumElements() - 1;
			n = LASTCARD;
		}
		
		xy = getXY(kreg);
		drawCard(hWnd, xy.x, xy.y, n, 0, false);
	}  //  if (cur != NULL)

	slreg.jval = 2;
	slreg.lval = now->fromL;
	slreg.pval = 0;
	sl = top[slreg.lval];
	
	if (sl == NULL)
		top[slreg.lval] = cur;
	else {
		kreg.pval = sl->getNumElements() - 1;
		sl->push(cur);
	}

	rmouse.jval = 2;
	rmouse.lval = slreg.lval;
	rmouse.pval = slreg.pval;
	rightUp();
	
	delete now;
	return;
}

char* Klondike2::gameName() {

	return "Klondike 2";
}

void Klondike2::left() {

	Cards *cur;
	KXY xy;

	if (score == 52)
		return;

	kbd.lval--;
	kbd.pval = 0;
	if (kbd.lval < 0)
		if (kbd.jval == 2) {
			kbd.jval = 3;
			kbd.lval = 3;
		} else {
			kbd.jval = 2;
			kbd.lval = 6;
		}

	//  if (kbd.lval < 0)


	if (kbd.jval == 2) {
		cur = top[kbd.lval];
		if (cur != NULL)
			kbd.pval = cur->getNumElements() - 1;
	}

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Klondike2::right() {
	Cards *cur;
	KXY xy;
	int max = (kbd.jval == 3 ? 3 : 6);

	if (score == 52)
		return;

	kbd.lval++;
	kbd.pval = 0;

	if (kbd.lval > max) {
		if (kbd.jval == 2)
			kbd.jval = 3;
		else
			kbd.jval = 2;

		kbd.lval = 0;
	}  //  if (kbd.lval > max)

	
	if (kbd.jval == 2) {
		cur = top[kbd.lval];
		if (cur != NULL)
			kbd.pval = cur->getNumElements() - 1;
	}

	xy = getXY(kbd);
	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Klondike2::up() {

	Cards *cur;
	KXY xy;
	int inc2 = ht / 5;

	kbd.pval--;
	if (kbd.pval < 0)
		kbd.pval = 0;

	xy = getXY(kbd);

	if (kbd.jval == 2) {
		//  work with me as I step through this...
		cur = top[kbd.lval];
		if (cur != NULL)
			cur = cur->getElement(kbd.pval);
		
		if (cur != NULL)
			cur = cur->Next();

		if (cur != NULL) {
			//  There is a next element so we have to compensate for the fact that
			//  only the top 20% of the card is visible
			xy.y -= ht / 2;
			xy.y += inc2 / 2;
		}
	}  //  if (kbd.jval == 2)

	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Klondike2::down() {

	Cards *cur;
	KXY xy;
	int inc2 = ht / 5;

	if (score == 52)
		return;

	kbd.pval++;
	if (kbd.jval == 3)
		kbd.pval = 0;
	else {
		cur = top[kbd.lval];
		if (cur != NULL) {
			if (cur->getElement(kbd.pval) == NULL)
				kbd.pval--;
		} else
			kbd.pval = 0;
		//  if (cur != NULL)

	}  //  if (kbd.jval == 3)
	xy = getXY(kbd);

	if (kbd.jval == 2) {
		//  work with me as I step through this...
		cur = top[kbd.lval];
		if (cur != NULL)
			cur = cur->getElement(kbd.pval);
		
		if (cur != NULL)
			cur = cur->Next();

		if (cur != NULL) {
			//  There is a next element so we have to compensate for the fact that
			//  only the top 20% of the card is visible
			xy.y -= ht / 2;
			xy.y += inc2 / 2;
		}
	}  //  if (kbd.jval == 2)

	putMouse(xy.x, xy.y, hWnd);

	return;
}

void Klondike2::enter() {
	
	Cards *cur;
	KXY xy;
	int inc2 = ht / 5;

	if (score == 52)
		return;

	xy = getXY(kbd);

	if (kbd.jval == 2) {
		//  work with me as I step through this...
		cur = top[kbd.lval];
		if (cur != NULL)
			cur = cur->getElement(kbd.pval);
		
		if (cur != NULL)
			cur = cur->Next();

		if (cur != NULL) {
			//  There is a next element so we have to compensate for the fact that
			//  only the top 20% of the card is visible
			xy.y -= ht / 2;
			xy.y += inc2 / 2;
		}
	}  //  if (kbd.jval == 2)

	leftClick(xy.x, xy.y);

	return;
}

void Klondike2::deal() {
	Cards *a[7], *b[7];
	int min = 0, max = 6, cur = 0;
	bool dir = true;

	//  First, wipe away the board and history
	delInst();
	sel.lval = 0;
	sel.pval = 0;
	sel.selected = false;
	score = 0;

	kbd.jval = 3;
	kbd.lval = 0;
	kbd.pval = 0;

	rmouse.jval = -1;
	rmouse.lval = -1;
	rmouse.pval = -1;

	for (int i = 0; i < 7; i++) {
		a[i] = NULL;
		b[i] = NULL;
		top[i] = NULL;
		bot[i] = NULL;
		if (i < 4)
			pile[i] = NULL;
	}

	while (min != max) {
		for (int i = min; i != max; i++) {
			b[i] = new Cards(a[i], deck[cur]);
			cur++;
			
			a[i] = b[i];
			
			if (max == 6)  // first row
				bot[i] = b[i];

		}  //  for (int i = min; i != max, i++)
		if (dir) 
			max--;
		else
			min++;
		
		dir = !dir;
	}  //  while (min != max)

	min = 0;
	max = 7;
	dir = true;
	
	for (int i = 0; i < 7; i++) {
		a[i] = new Cards(NULL, deck[cur]);
		cur++;
		top[i] = a[i];
	}

	while (min != max && cur < 52) {
		for (int i = min; i != max && cur < 52; i++) {
			b[i] = new Cards(a[i], deck[cur]);
			cur++;

			a[i] = b[i];
			
		}  //  for (int i = min; i != max, i++)
		if (dir) 
			max--;
		else
			min++;
		
		dir = !dir;
	}  //  while (min != max && cur < 52)

	return;
}

void Klondike2::delInst() {
	Cards *cur, *next, **root;
	int max;
	KUNDO *now = Undo;

	//  I think that this is the fastest way to do this
	//  unconditionally blow away the board
	for (int j = 1; j < 4; j++) {
		switch (j) {
			case 1:
				root = top;
				max = 7;
				break;

			case 2:
				root = bot;
				max = 7;
				break;

			case 3:
				root = pile;
				max = 4;
				break;
		} // switch (j)

		for (int i = 0; i < max; i++) {
			cur = root[i];
			while (cur != NULL) {
				cur->setPrev(NULL);
				next = cur->Next();
				delete cur;
				cur = next;
			}  // while (cur != NULL)

		} // for (int i = 0; i < max; i++)

	} // for (int j = 1; j < 3; j++)

	while (now != NULL) {
		Undo = now->prev;
		delete now;
		now = Undo;
	}

	for (int i = 0; i < 7; i++) {
		top[i] = NULL;
		bot[i] = NULL;
		if (i < 4)
			pile[i] = NULL;
	}

	sel.selected = false;
	sel.lval = -1;
	sel.pval = -1;
	rmouse.jval = -1;
	rmouse.lval = -1;
	rmouse.pval = -1;
	rdn = false;
	kbd.jval = 3;
	kbd.lval = 0;
	kbd.pval = 0;
	score = 0;

	return;
}

KREGION Klondike2::getKR(int mx, int my) {
	KREGION regi;

	regi.jval = -1;

	if (my < ht * 9 / 8)
		Region3(&regi, mx, my);
	else if (my > ht * 5 / 4)
		Region12(&regi, mx, my);
	else
		return regi;

	return regi;
}

void Klondike2::Region3(KREGION *loc, int mx, int my) {
	Cards *crd;
	int lx = wd / 2, dx = wd * 5 / 2, n = 0;

	loc->lval = -1;
	if (my < ht / 8)
		return;

	while (lx < mx) {
		if (mx > lx && mx < lx + wd)
			loc->lval = n;
		lx += dx;
		n++;
	}

	if (loc->lval > 3)
		loc->lval = -1;

	if (loc->lval != -1) {
		loc->jval = 3;
		crd = pile[loc->lval];
		if (crd != NULL)
			loc->pval = crd->getNumElements() - 1;
		else 
			loc->pval = 0;
	}
	
	return;
}

void Klondike2::Region12(KREGION *loc, int mx, int my) {
	Cards *crd;
	int lx = wd / 2, dx = wd * 5 / 4, n = 0, inc1 = ht / 20, inc2 = ht / 5, h = ht * 5 / 4;

	loc->lval = -1;

	while (lx < mx) {
		if (mx > lx && mx < lx + wd)
			loc->lval = n;
		lx += dx;
		n++;
	}

	if (loc->lval > 6)
		loc->lval = -1;

	if (loc->lval == -1)
		return;

	//  At this point, the mouse x coordinate is within one of the columbs
	//  and the Y value is within stack range.
	crd = bot[loc->lval];
/*	if (crd == NULL && my < ht * 9 / 4) {
		loc->jval = 2;
		loc->pval = 0;
		return;
    }*/
	
	loc->pval = 0;  // start with a zero base so we are one behind
	//  as we will end as one ahead
	h += inc1;
	
	while (my > h && crd != NULL) {
		crd = crd->Next();
		loc->pval++;
		h += inc1;
	}

	if (crd != NULL) {
		loc->jval = 1;
		return;
	}

	crd = top[loc->lval];
	h += inc2;
	h -= inc1;
	if (crd == NULL && my < h + ht) {
		loc->jval = 1;
		if (bot[loc->lval] == NULL)
			loc->jval = 2;  // special case

		return;
	}

	if (crd == NULL)
		return;

	loc->pval = 0;
	while (crd->Next() != NULL && my > h) {
		crd = crd->Next();
		loc->pval++;
		h += inc2;
	}

	if (crd->Next() != NULL) {
		loc->jval = 2;
		CheckSelect(loc, mx, my);
		return;
	}

	h -= inc2;
	if (my > h + ht)
		return;

	loc->jval = 2;
	CheckSelect(loc, mx, my);

	return;
}

void Klondike2::CheckSelect(KREGION *loc, int mx, int my) {
	KXY selxy;
	KREGION kreg;
	const int x2 = wd / 2, y2 = ht / 2;

	if (loc->jval == -1 || !sel.selected)
		return;
	
	kreg.jval = 2;
	kreg.lval = sel.lval;
	kreg.pval = sel.pval;

	selxy = getXY(kreg);

	if (mx > selxy.x - x2 && mx < selxy.x + x2 && my > selxy.y - y2 && my < selxy.y + y2) {
		loc->jval = 2;
		loc->lval = sel.lval;
		loc->pval = sel.pval;
	}

	return;
}

KXY Klondike2::getXY(KREGION kreg) {
	KXY kxy;
	int inc1 = ht / 20, inc2 = ht / 5, incx = wd * 5 / 4;
	Cards *cur;

	kxy.x = -1;
	kxy.y = -1;
	switch (kreg.jval) {
		case 1:
			kxy.x = wd;
			kxy.x += kreg.lval * incx;
			kxy.y = ht * 7 / 4;
			kxy.y += inc1 * kreg.pval;
			return kxy;
			break;

		case 2:
			kxy.x = wd;
			kxy.x += kreg.lval * incx;
			kxy.y = ht * 7 / 4;
			//  This execution continues after the switch statement
			break;

		case 3:
			kxy.y = ht * 5 / 8;
			kxy.x = wd;
			kxy.x += kreg.lval * incx * 2;
			return kxy;
			break;

		default:
			return kxy;
	} // switch (kreg.jval)

	//  Continuing affset calculation for region 2 cards:
	cur = bot[kreg.lval];

	if (cur != NULL)
		kxy.y += cur->getNumElements() * inc1;
	
	kxy.y += kreg.pval * inc2;
	//  Whew.. that was easier than I thought it would be :-)

	return kxy;
}

void Klondike2::calcMove(KREGION lm) {

	Cards *cur, *sl;
	int a, b;  // integer storage
	bool worked;
	KUNDO *n;

	const char color[] = "-BR";
	const char card[] = "-A234567890JQK";

	sl = top[sel.lval];
	sl = sl->getElement(sel.pval);

	switch (lm.jval) {
		case 2:
			cur = top[lm.lval];
			lm.pval = -1;  //  If there are no cards where lm is referencing
							//  This is used for the Undo structure

			if (cur != NULL) {
				lm.pval = cur->getNumElements() - 1;
				cur = cur->last();
			}

			worked = totop(cur, sl, lm);

			break;

		case 3:
			cur = pile[lm.lval];
			if (cur != NULL)
				cur = cur->last();
			if (sl->Next() != NULL)
				return;  //  you can not place a card onto the finish pile
							//  unless it is the last card on the stack
			worked = topile(cur, sl, lm);

			if (worked)
				score++;

			break;

		//  you can't place anything on a turned over card
		default:
			return;  //  exit hastily if lm is invalid

	}
	
	a = 0;
	b = 0;
	if (cur != NULL) {
		a = cur->getCol();
		b = cur->getVal();
	}

	/*dprintf("Trying to move a %c %c at 2,%i,%i onto a %c %c at %i,%i,%i: %svalid",
		color[sl->getCol()], card[sl->getVal()], sel.lval, sel.pval, 
		color[a], card[b], lm.jval, lm.lval, lm.pval, (worked ? "" : "in"));*/

	if (worked) {
		n = new KUNDO;
		n->prev = Undo;
		Undo = n;
		n->toJ = lm.jval;
		n->toL = lm.lval;
		n->toP = lm.pval;
		n->fromJ = 2;
		n->fromL = sel.lval;
	}

	return;
}

bool Klondike2::totop(Cards *cur, Cards *sl, KREGION lm) {
	KXY xy;
	Cards *a;
	int val;
	const int inc2 = ht / 5;
	KREGION c;

	if (bot[lm.lval] == NULL && sl->getVal() == KING && top[lm.lval] == NULL) {
		a = sl;
		c.jval = 2;
		c.lval = sel.lval;
		c.pval = sel.pval;
		xy = getXY(c);

		while (a != NULL) {
			eraseCard(hWnd, xy.x, xy.y, 0);
			a = a->Next();
			xy.y += inc2;
		}

		
		if (sl->Prev() == NULL)
			top[sel.lval] = NULL;
		else {
			a = sl->Prev();
			a->setNext(NULL);
			sl->setPrev(NULL);
		}

		top[lm.lval] = sl;
		sel.selected = false;

		rmouse.jval = 1;
		rmouse.lval = sel.lval;
		rmouse.pval = 0;
		xy = getXY(rmouse);
		drawCard(hWnd, xy.x, xy.y, (bot[sel.lval] == NULL ? E : LASTCARD), 0, false);
		rightUp();

		rmouse.jval = 2;
		rmouse.lval = lm.lval;
		rmouse.pval = 0;
		rightUp();

		return true;
	}  //  if (bot[lm.lval] == NULL && sl->getVal == KING && top[lm.lval] == NULL)

	//  Get rid of a few things that are illegal
	if (sl->getVal() == KING)  //  king can only go on an empty row
		return false;

	if (top[lm.lval] == NULL)  //  only a king can go on an empty row ... see above
		return false;

	if (lm.lval == sel.lval)
		return false;  //  you can't move to the same row
	
	val = cur->getVal();
	val--;

	if (sl->getVal() != val || sl->getSuit() == cur->getSuit())
		return false;

	//  Ok, the suits don't match and the dest card is one higher than
	//  the source card.  This IS a valid move.

	a = sl;
	c.jval = 2;
	c.lval = sel.lval;
	c.pval = sel.pval;

	xy = getXY(c);
	while (a != NULL) {
		eraseCard(hWnd, xy.x, xy.y, 0);
		a = a->Next();
		xy.y += inc2;
	}
	
	if (sl->Prev() == NULL)
		top[sel.lval] = NULL;
	else {
		a = sl->Prev();
		a->setNext(NULL);
		sl->setPrev(NULL);
	}

	cur->push(sl);
	sel.selected = false;

	rmouse.jval = 1;
	rmouse.lval = sel.lval;
	rmouse.pval = 0;
	xy = getXY(rmouse);
	drawCard(hWnd, xy.x, xy.y, (bot[sel.lval] == NULL ? E : LASTCARD), 0, false);
	rightUp();

	rmouse.jval = 2;
	rmouse.lval = lm.lval;
	rmouse.pval = lm.pval;
	rightUp();

	return true;
}

bool Klondike2::topile(Cards *cur, Cards *sl, KREGION lm) {
	//  We already know that sl is the LAST card in the pile
	//  and that cur, sl, sel, and lm are good
	KXY xy;

	//  some place holders
	Cards *a, *b;
	int val;
	KREGION c;

	//  get this situation out of the way first
	if (cur == NULL) {
		if (sl->getVal() != ACELO)
			return false;

		//  we are trying to move an ACE onto an empty pile
		//  first do it...
		a = top[sel.lval];
		pile[lm.lval] = a->pop();
		if (pile[lm.lval] == a)
			top[sel.lval] = NULL;

		//  ... then show it.
		c.jval = 2;
		c.lval = sel.lval;
		c.pval = sel.pval;
		xy = getXY(c);
		eraseCard(hWnd, xy.x, xy.y, 0);
		xy = getXY(lm);
		drawCard(hWnd, xy.x, xy.y, sl->getNum(), 0, false);
		//  yes, this IS cheating....  but I really don't feel like rewriting the code
		sel.selected = false;
		rmouse.jval = 1;
		rmouse.lval = sel.lval;
		rmouse.pval = 0;
		xy = getXY(rmouse);
		drawCard(hWnd, xy.x, xy.y, (bot[sel.lval] == NULL ? E : LASTCARD), 0, false);
		//  redraw the stack
		rightUp();

		return true;
	}  //  if (cur == NULL)

	val = cur->getVal();
	val++;
	if (sl->getVal() != val || cur->getSuit() != sl->getSuit())
		return false;

	//  first do it....
	a = top[sel.lval];
	b = a->pop();
	if (b == a)
		top[sel.lval] = NULL;

	a = pile[lm.lval];
	a->push(b);

	//  then show it;
	c.jval = 2;
	c.lval = sel.lval;
	c.pval = sel.pval;
	xy = getXY(c);
	eraseCard(hWnd, xy.x, xy.y, 0);
	xy = getXY(lm);
	drawCard(hWnd, xy.x, xy.y, sl->getNum(), 0, false);

	sel.selected = false;
	rmouse.jval = 1;
	rmouse.lval = sel.lval;
	rmouse.pval = 0;
	xy = getXY(rmouse);
	drawCard(hWnd, xy.x, xy.y, (bot[sel.lval] == NULL ? E : LASTCARD), 0, false);
	rightUp();

	return true;
}


void Klondike2::Hit() {

	//  First call the solitaire hit routine to manage status
	Solitaire::Hit();

	//  now run our code.
	if (!rdn)
		rightUp();

	return;
}
bool Klondike2::keyPress(char key) {

	switch (key) {
		case ' ':
			rKbd();
			break;

		default:
			return false;
	}  //  switch (key)
	
	return true;
}

void Klondike2::rKbd() {

	Cards *cur;
	KXY xy;
	int inc2 = ht / 5;

	xy = getXY(kbd);

	if (kbd.jval == 2) {
		//  work with me as I step through this...
		cur = top[kbd.lval];
		if (cur != NULL)
			cur = cur->getElement(kbd.pval);
		
		if (cur != NULL)
			cur = cur->Next();

		if (cur != NULL) {
			//  There is a next element so we have to compensate for the fact that
			//  only the top 20% of the card is visible
			xy.y -= ht / 2;
			xy.y += inc2 / 2;
		}
	}  //  if (kbd.jval == 2)

	rightClick(xy.x, xy.y);

	return;
}

int Klondike2::winWork(void* datum) {

	UNUSED(datum);

	runAnim();

	clearBoard(hWnd);

	return Win(hWnd, this);
}

void Klondike2::runAnim() {
	Cards *cur[4];
	KREGION kreg[4];
	KXY xy[4];
	int xv, yv, i, mx, my;

	const int dx = wd / 4, sx = dx / 2, g = 2, w2 = wd / 2;
	CARDDIM cd = getBoardDims(hWnd);
	
	mx = cd.Width;
	my = cd.Height;

	dprintf("Win running:  Screen dims: 0x0, %i,%i", mx, my);

	Sleep(30);
	clearBoard(hWnd);
	for (i = 0; i < 4; i++) {
		cur[i] = pile[i];
		kreg[i].jval = 3;
		kreg[i].lval = i;
		kreg[i].pval = 13;
		cur[i] = cur[i]->last();
		xy[i] = getXY(kreg[i]);
		drawCard(hWnd, xy[i].x, xy[i].y, cur[i]->getNum(), 0, false);
	}

	hit = false;
	while (cur[3] != NULL) {

		for (i = 0; i < 4; i++) {
			yv = rand() % 20;
			yv *= -1;
			xv = rand() % dx;
			xv -= sx;
			if (xv == 0)
				xv++;  //   The card MUST move in the X direction

			xy[i] = getXY(kreg[i]);
			while (xy[i].x > -w2 && xy[i].x < mx + w2) {
				xy[i].x += xv;
				yv += g;
				xy[i].y += yv;
				if (xy[i].y > my - ht / 2) {
					yv = yv * -9 / 10;
					xy[i].y = my - ht / 2;
					if (yv > -3)
						xy[i].x = -w2;
					//dprintf("Boing! new, spring coeff = %f", t);
					//t = float(yv) * t;
					//yv = int(t);
				}
				
				drawCard(hWnd, xy[i].x, xy[i].y, cur[i]->getNum(), 0, false);
				Sleep(17);  //  roughly 60fps
				if (hit)
					return;
			}  //  while (xy[i].x > 0 && xy[i].x < mx)
			cur[i] = cur[i]->Prev();
		}  //  for (i = 0; i < 4; i++)
	}  //  while (cur[i] != NULL)

	return;
}

void Klondike2::dblClick(int X, int Y) {

	Cards *c, *c1 = NULL;
	KREGION kreg = getKR(X, Y), tmp;
	KXY xy;
	int cnum, i = 0;

	if (kreg.jval == 1) {
		leftClick(X, Y);
		return;
	}

	if (kreg.jval != 2 || score == 52)
		return;


	if (sel.selected) {
		tmp.jval = 2;
		tmp.lval = sel.lval;
		tmp.pval = sel.pval;
		if (kreg.lval != sel.lval || kreg.pval != sel.pval)
			return;
		
		xy = getXY(tmp);
		leftClick(xy.x, xy.y);
	}

	dprintf("dblClick called!");

	c = top[kreg.lval];
	if (c == NULL)
		return;

	kreg.pval = c->getNumElements();
	c = c->last();
	cnum = c->getVal();
	xy = getXY(kreg);
	if (cnum == ACELO)  {
		leftClick(xy.x, xy.y);

		c1 = pile[0];
		i = 1;

		while (c1 != NULL) {
			c1 = pile[i];
			i++;
		}

		tmp.jval = 3;
		tmp.lval = i - 1;
		tmp.pval = 0;
		xy = getXY(tmp);
		leftClick(xy.x, xy.y);
		return;
	}  //  if (cnum == ACELO)

	cnum = c->getNum();
	for (i = 0; i < 4; i++) {
		c1 = pile[i];
		
		if (c1 == NULL)
			continue;

		c1 = c1->last();
		if (c1->getNum() + 1 == cnum)
			break;
	}

	if (i == 4)
		return;

	leftClick(xy.x, xy.y);

	tmp.jval = 3;
	tmp.lval = i;
	tmp.pval = 0;
	xy = getXY(tmp);
	leftClick(xy.x, xy.y);

	return;
}

Klondike2::~Klondike2() {
	delInst();
	return;
}
