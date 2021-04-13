// This is the WAR rule set.  Same as sanda but a different game.
//  This one will have A LOT of windows specific code in it.  Eventually, I will
//  make it a bit less OS dependant.

#include "stdafx.h"
#include "cardtools.h"
#include "war.h"
#include "debug.h"
#include "colordefs.h"
#include "winteaser.h"
#include "resource.h"  //  for the menu keys

//using namespace Gdiplus;

War::War(HWND hWind, int info) {

	Init(hWind, info);
	int i;
	nplayers = 2; //  expandability option

	for (i = 0; i < nplayers; i++) {
		play[i] = NULL;
		dis[i] = NULL;
	}
	statusUndo(false);

	return;
}

void War::deal() {

	int i, j, v;
	PILE *cur[2];
	CARDDIM dim;

	Cleanup();

	for (j = 0; j  < nplayers; j++) {
		cur[j] = NULL;
		dis[j] = NULL;
		play[j] = NULL;
		for (i = 0; i < 52; i+=nplayers) {
			cur[j] = push(new PILE, j, true);
			v = deck[i + j];
			cur[j]->card = v;
			cur[j]->value = (v - 1) % 13;
			if (cur[j]->value == 0)
				cur[j]->value = 13;
			cur[j]->value++;
			if (i == 0)
				play[j] = cur[j];
		}
	}
	
	dim = getCardAttributes();
	height = dim.Height;
	width = dim.Width;
	
	return;
}

void War::leftClick(int X, int Y) {
	PILE *t = last(ME, true);
	XY1 xy;
	const int h = height / 2, w = width / 2;
	if (won)
		return;
	
	if (t != NULL)
		xy = getXY(ME, t->sequence, true);
	else
		xy = getXY(ME, 0, true);

	if (X > xy.x - w && X < xy.x + w && Y > xy.y - h && Y < xy.y + h)
		handelClick();
	
	return;
} 
 
void War::rightClick(int X, int Y) {
	int region = getRegion(X, Y), n;
	PILE *t;
	XY1 xy;

	if (won)
		return;
	
	switch (region) {
		case 1:
			rmouse.player = YOU;
			rmouse.isPlay = true;
			break;
			
		case 2:
			rmouse.player = YOU;
			rmouse.isPlay = false;
			break;

		case 3:
			rmouse.player = ME;
			rmouse.isPlay = true;
			break;

		case 4:
			rmouse.player = ME;
			rmouse.isPlay = false;
			break;

		case -1:
		default:
			rmouse.player = -1;
			rmouse.isPlay = false;
			rmouse.seq = 0;
			return;
	}

	if(!findSeq(&rmouse, X, Y))
		return;

	//  We have a valid location..  now to display it.
	t = get(rmouse.player, rmouse.seq, rmouse.isPlay);
	
	if (t == NULL) { //  this shouldn't happen.. but it could
		rmouse.isPlay = false;
		rmouse.player = -1;
		rmouse.seq = 0;
		return;
	}

	n = t->card;
	if (rmouse.player == YOU && rmouse.isPlay)
		n = LASTCARD;

	xy = getXY(rmouse.player, t->sequence, rmouse.isPlay);
	drawCard(hWnd, xy.x, xy.y, n, 0, false);

	return;
}

void War::rightUp() {
	PILE *t;
	XY1 xy;
	int n;
	const int inc = width / 10;

	if (rmouse.player == -1)
		return;

	t = get(rmouse.player, rmouse.seq, rmouse.isPlay);
	if (t == NULL) {
		rmouse.isPlay = false;
		rmouse.player = -1;
		rmouse.seq = 0;
		return;
	}

	xy = getXY(rmouse.player, t->sequence, rmouse.isPlay);
	while (t != NULL) {
		n = t->card;
		if (rmouse.isPlay && (t->next != NULL || rmouse.player == YOU))
			n = LASTCARD;

		drawCard(hWnd, xy.x, xy.y, n, 0, false);
		xy.x += inc;
		t = t->next;
	}

	rmouse.isPlay = false;
	rmouse.player = -1;
	rmouse.seq = 0;

	return;
}

int War::load(const char *filename) {
	FILE *file = fopen(filename, "rb");
	int i, n;
	PILE *t1;
	char name[256];
	CARDDIM dim;  //  used in case we have loaded our war object with load as opposed to deal

	ZeroMemory(name, 256);
	if (file == NULL)
		return FALSE;
	
	//  datum int
	i = fgetc(file);
	if (i == EOF) {
		fclose(file);
		return i;
	}

	// name length
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
	Cleanup();
	dim = getCardAttributes();
	height = dim.Height;
	width = dim.Width;

	for(i = 0; i < 52; i++)
		deck[i] = fgetc(file);

	fgetc(file);  // nplayers assumed to be 2

	for (i = 0; i < nplayers; i++) {
		//  Play pile
		while ((n = fgetc(file)) != 255) {
			t1 = push(new PILE, i, true);
			t1->card = n;
			t1->value = (n - 1) % 13; 
			if (t1->value == 0)
				t1->value = 13;
			t1->value++;
		}
		
		//  Play pile
		while ((n = fgetc(file)) != 255) {
			t1 = push(new PILE, i, false);
			t1->card = n;
			t1->value = (n - 1) % 13; 
			if (t1->value == 0)
				t1->value = 13;
			t1->value++;
		}
	}

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

int War::save(const char *filename) {
	FILE *file = fopen(filename, "wb");
	int i;
	PILE *t;

	if (file == NULL)
		return FALSE;

	if (won) {
		fclose(file);
		return EOF;
	}
	
	//  Generic solitaire header
	fputc(datum, file);

	//  Streets and Alleys Specific Header
	fputc((int)strlen(gameName()) + 1, file);
	fputs(gameName(), file);
	
	for (i = 0; i <	52; i++)
		fputc(deck[i], file);

	fputc(nplayers, file);

	for (i = 0; i < nplayers; i++) {
		t = play[i];
		while (t != NULL) {
			fputc(t->card, file);
			t = t->next;
		}
		fputc(255, file);
		
		t = dis[i];
		while(t != NULL) {
			fputc(t->card, file);
			t = t->next;
		}
		fputc(255, file);
	}
	fputc(255, file); //  close out the file

	i = fclose(file);
    if (i != 0)
		return i;

	return TRUE;
}

void War::redrawBoard() {
	int x = width * 2, y = height, inc = width / 10;
	PILE *loc1 = play[YOU], *loc2 = play[ME];

	clearBoard(hWnd);
	if (won)
		return;

	drawCard(hWnd, x, y, E, 0, false);  // Draw the "E" card when there are no cards
	while (loc1 != NULL) {
		drawCard(hWnd, x, y, LASTCARD, 0, false);
		loc1 = loc1->next;
		x += inc;
	}
	
	x = width * 9;
	drawCard(hWnd, x, y, E, 0, false);
	loc1 = dis[YOU];
	while (loc1 != NULL) {
		drawCard(hWnd, x, y, loc1->card, 0, false);
		loc1 = loc1->next;
		x += inc;
	}

	y = height * 6;
	x = width * 9;
	drawCard(hWnd, x, y, E, 0, false);
	loc1 = dis[ME];
	while (loc1 != NULL) {
		drawCard(hWnd, x, y, loc1->card, 0, false);
		loc1 = loc1->next;
		x += inc;
	}

	x = width * 2;
	drawCard(hWnd, x, y, E, 0, false);
	if (loc2 == NULL)
		return;

	while (loc2->next != NULL) {
		drawCard(hWnd, x, y, LASTCARD, 0, false);
		loc2 = loc2->next;
		x += inc;
	}

	drawCard(hWnd, x, y, loc2->card, 0, false);

	return;
}

void War::undo() {
	//  There is no UNDO in war!
	return;
}

char* War::gameName() {
	return "War!";
}

void War::left() {
	down();
	return;
}

void War::right() {
	down();
	return;
}

void War::up() {
	down();
	return;
}

void War::down() {
	PILE *me = last(ME, true);
	if (me != NULL) {
		XY1 xy = getXY(ME, me->sequence, true);
		putMouse(xy.x, xy.y, hWnd);
	}

	return;
}

void War::enter() {
	PILE *me = last(ME, true);
	if (me != NULL) {
		XY1 xy = getXY(ME, me->sequence, true);
		putMouse(xy.x, xy.y, hWnd);
	}
	if (won)
		return;

	handelClick();

	return;
}

void War::Cleanup() {
	PILE *temp;
	int i;

	for (i = 0; i < nplayers; i++) {
		temp = play[i];
		while (temp != NULL) {
			play[i] = play[i]->next;
            delete temp;
			temp = play[i];
		}
		
		temp = dis[i];
		while (temp != NULL) {
			dis[i] = dis[i]->next;
			delete temp;
			temp = dis[i];
		}
	}
	won = false;

	rmouse.player = -1;
	rmouse.seq = 0;
	rmouse.isPlay = false;

	return;
}

PILE* War::push(PILE *card, int player, bool isPlay) {
	PILE *cur = last(player, isPlay), *temp = NULL;
	int num = 0;
	
	if (cur == NULL && isPlay)
		play[player] = card;
	else if (cur == NULL && !isPlay)
		dis[player] = card;
	else {
		temp = cur;
		cur->next = card;
		num = cur->sequence;
	}

	num++;
	card->sequence = num;
	card->last = temp;
	card->next = NULL;

	return card;
}

PILE* War::pop(int player, bool isPlay) {
	PILE *cur = last(player, isPlay);
	XY1 xy;
	int n;

	if (cur == NULL)	
		return NULL;
	
	xy = getXY(player, cur->sequence, isPlay);
	eraseCard(hWnd, xy.x, xy.y, 0);
	if (cur->sequence == 1)
		drawCard(hWnd, xy.x, xy.y, E, 0, false);

	cur = cur->last;
	
	if(cur == NULL && isPlay)
		play[player] = NULL;

	if (cur == NULL && !isPlay)
		dis[player] = NULL;
	
	if (cur == NULL)
		return NULL;

	if (player == ME || !isPlay)
		n = cur->card;
	else
		n = LASTCARD;

	xy = getXY(player, cur->sequence, isPlay);
	drawCard(hWnd, xy.x, xy.y, n, 0, false);
	
	cur->next = NULL;
	return cur;
}

void War::recycle(int player) {
	PILE *temp = play[player];
	play[player] = dis[player];
	dis[player] = temp;

	//grecycle(player);  //  moved to it's own sub for expandabity purposes
	return;
}

PILE* War::last(int player, bool isPlay) {
	PILE *cur;

	if (isPlay)
		cur = play[player];
	else
		cur = dis[player];

	if (cur == NULL)
		return NULL;

	while (cur->next != NULL)
		cur = cur->next;

	return cur;
}

PILE* War::click(int player) {
	PILE *p = last(player, true);
	static int d = player;  // so data doesn't go out of scope
	void *data = (void*)(&d);

	if (p == NULL) {
		grecycle(player);
		p = last(player, true);
		if (p == NULL)
			winTeaser(data);
	}
	pop(player, true);
	
	return p;
}

XY1 War::getXY(int player, int seq, bool isPlay) {
	int x = width * 2, y = height, inc = width / 10;
	int i;
	XY1 ret;

	if (player == ME)
		y = height * 6;

	if (!isPlay)
		x = width * 9;

	for (i = 1; i < seq; i++)   //  sequences are 1 base
		x += inc;
	
	ret.x = x;
	ret.y = y;

	return ret;
}

void War::handelClick() {
	PILE *me, *you;
	XY1 xy1, xy2;

	me = click(ME);
	you = click(YOU);
	if (you == NULL || me == NULL)
		return;

	xy1 = getXY(YOU, you->sequence, true);
	xy1.y += height * 2;
	xy1.x += width;
	drawCard(hWnd, xy1.x, xy1.y, you->card, 0, false);
	
	xy2 = getXY(ME, me->sequence, true);
	xy2.y -= height * 2;
	xy2.x += width;
	drawCard(hWnd, xy2.x, xy2.y, me->card, 0, false);

	//dprintf("Comparing: PC = %i, Player = %i", you->value, me->value);

	Sleep(1000);
	needdraw = false;

	if (you->value > me->value)
		winner(YOU, you, me);
	
	if(you->value < me->value)
		winner(ME, me, you);

	if(you->value == me->value)
		winner(war(you, me, xy1, xy2), me, you);

	if (needdraw) 
		redrawBoard();
	else {
		eraseCard(hWnd, xy1.x, xy1.y, 0);
		eraseCard(hWnd, xy2.x, xy2.y, 0);
	}

	return;
}

void War::winner(int player, PILE *a, PILE *b){
	
	//  Note: for expandability purposes, the caller is responsibile for
	//  cleaning up the board.
	PILE *t1;
	XY1 xy;
	while (a != NULL) {
		t1 = a->next;
		push(a, player, false);
		xy = getXY(player, a->sequence, false);
		drawCard(hWnd, xy.x, xy.y, a->card, 0, false);
		a = t1;
	}

	while (b != NULL) {
		t1 = b->next;
		push(b, player, false);
		xy = getXY(player, b->sequence, false);
		drawCard(hWnd, xy.x, xy.y, b->card, 0, false);
		b = t1;
	}
	//dprintf("Player %i is the winner!", player);
	return;
}

int War::war(PILE *you, PILE* me, XY1 Yxy, XY1 Mxy) {
	const int inc = height / 10;
	PILE *t1, *t2 = you, *t3, *t4 = me, *t5 = you, *t6 = me;
	int i, winner = -1;
	XY1 xy[4];
	
	xy[0].x = Yxy.x;
	xy[0].y = Yxy.y;
	xy[1].x = Yxy.x;
	xy[1].y = Yxy.y;
	xy[2].x = Mxy.x;
	xy[2].y = Mxy.y;
	xy[3].x = Mxy.x;
	xy[3].y = Mxy.y;

	for (i = 0; i < 3; i++) {
		//  You
		t1 = last(YOU, true);
		if (t1 == NULL) {
			grecycle(YOU);
			t1 = last(YOU, true);
			needdraw = true;
		}
		
		if (t1 != NULL) {
			pop(YOU, true);
			t5 = t1;
			t2->next = t1;
			t1->last = t2;
			t2 = t2->next;
			xy[1].y -= inc;
			drawCard(hWnd, xy[1].x, xy[1].y, LASTCARD, 0, false);
		} else 
			t1 = t5;
		
		//  Me
		t3 = last(ME, true);
		if (t3 == NULL) {
			grecycle(ME);
			t3 = last(ME, true);
			needdraw = true;
		}
		
		if (t3 != NULL) {
			pop(ME, true);
			t6 = t3;
			t4->next = t3;
			t3->last = t4;
			t4 = t4->next;
			xy[3].y += inc;
			drawCard(hWnd, xy[3].x, xy[3].y, LASTCARD, 0, false);
		} else {
			t3 = t6;
		}

	} // For

	//  t1, t3 = current card (bottom), t2, t4 = t1, t3
	drawCard(hWnd, xy[1].x, xy[1].y, t1->card, 0, false);
	drawCard(hWnd, xy[3].x, xy[3].y, t3->card, 0, false);
	Sleep(1000);

	if (t1->value > t3->value)
		winner = YOU;

	if (t1->value < t3->value)
		winner = ME;

	if (t1->value == t3->value)
		winner = war(t1, t3, xy[1], xy[3]);

	while (xy[1].y != xy[0].y) {
		eraseCard(hWnd, xy[1].x, xy[1].y, 0);
		xy[1].y += inc;
	}

	while (xy[3].y != xy[2].y) {
		eraseCard(hWnd, xy[3].x, xy[3].y, 0);
		xy[3].y -= inc;
	}

	return winner;
}

void War::grecycle(int player) {
	PILE *t;
	XY1 xy;
	int n = LASTCARD;

	t = dis[player];
	while (t != NULL) {
		xy = getXY(player, t->sequence, false);
		eraseCard(hWnd, xy.x, xy.y, 0);
		t = t->next;
	}
	
	//  Draw "E" card on empty piles
	xy = getXY(player, 0, true);
	drawCard(hWnd, xy.x, xy.y, E, 0, false);
	xy = getXY(player, 0, false);
	drawCard(hWnd, xy.x, xy.y, E, 0, false);

	recycle(player);
	t = play[player];
	if (t == NULL)
		return;

	while (t->next != NULL) {
		xy = getXY(player, t->sequence, true);
		drawCard(hWnd, xy.x, xy.y, LASTCARD, 0, false);
		t = t->next;
	}
	
	if (player == ME)
		n = t->card;
	
	xy = getXY(player, t->sequence, true);
	drawCard(hWnd, xy.x, xy.y, n, 0, false);
	
	return;
}

int War::winWork(void* datum) {
	XY1 xy[52], vxy[52];
	const int gz = -1;
	int vz[52], z[52], v[52], vt[52], t[52];
	int *p = (int*)datum;
	int pl = *p;
	PILE *t1;
	int i = 0, j = 0, k;
	CARDDIM tmp = getBoardDims(hWnd);
	int h = tmp.Height, w = tmp.Width;
	bool nodraw[52], done;

	//  p currently = the LOSER..  we need the winner..  assume it is the other player
	pl = (pl == YOU);
	
	Sleep(30);  //  See SandA::winWork for why I did this
	for (k = 0; k < 52; k++) {
		vz[k] = 0;
		z[k] = 0;
		v[k] = 0;
		vxy[k].x = 0;
		vxy[k].y = 0;
		vt[k] = 0;
		t[k] = 0;
		nodraw[k] = false;
	}
	hit = false;

	t1 = play[pl];
	while (t1 != NULL) {
		xy[i] = getXY(pl, t1->sequence, true);
		v[i] = LASTCARD;
		i++;
		t1 = t1->next;
	}
	
	j = i;
	t1 = dis[pl];
	while (t1 != NULL) {
		xy[i] = getXY(pl, t1->sequence, false);
		i++;
		v[i] = t1->card;
		t1 = t1->next;
	}
	
	Cleanup();  //  we no longer want to have valid game information.
	won = true;

	//  Shrink cards
	while (z[0] > -100) {
		Sleep(60);
		clearBoard(hWnd);
		for (k = 0; k <= i; k++)
			drawCard(hWnd, xy[k].x, xy[k].y, v[k], 0, false, axis2zoom(z[0]));

		vz[0] += gz;
		z[0] += vz[0];
	}	
	z[0] = 0;

	for (k = 0; k < 52; k++) {
		v[k] = k + 1;
		vz[k] = (rand() % 6) + 5;
		xy[k].x = (rand() % (w / 3)) + (w / 3);
		xy[k].y = (rand() % (h / 3)) + (h / 3);
		vxy[k].x = (rand() % 51) - 25;
		vxy[k].y = (rand() % 51) - 25;
		vt[k] = (rand() % 31) - 15;
		t[k] = rand() % 360;
	}
	
	clearBoard(hWnd);
	bprintf(hWnd, w / 3, h / 3, BWHITE, "Presenting the 52 card pickup...");
	Sleep(1000);

	//  Scatter cards
	done = false;
	while (!done) {
		Sleep(60);
		done = true;
		//  Draw
		clearBoard(hWnd);
		for (i = 0; i < 52; i++)
			drawCard(hWnd, xy[i].x, xy[i].y, v[i], t[i], false, axis2zoom(z[i]));
		
		//  Calculate
		for (i = 0; i < 52; i++) {
			if (z[i] >= 0) {
				xy[i].y += vxy[i].y;
				xy[i].x += vxy[i].x;
				t[i] += vt[i];
				vz[i] += gz;
				z[i] += vz[i];
			} else {
				z[i] = 0;
				nodraw[i] = true;
			}
			done &= nodraw[i];
		}
	} //wend

	//  Pick Up Cards:
	
	for(i = 0; i < 52; i++) {
		Sleep(150);
		eraseCard(hWnd, xy[i].x, xy[i].y, t[i], axis2zoom(z[i]));
		for (k = i + 1; k < 52; k++)
			drawCard(hWnd, xy[k].x, xy[k].y, v[k], t[k], false, axis2zoom(z[k]));
	}

	return Win(hWnd, static_cast<Solitaire*>(this));
}

int War::getRegion(int X, int Y) {
	//  Return 1, 2, 3, 4 or -1
	//  1 = Opponent play pile, 2 = Opponent discard pile
	//  3 = Player play pile, 4 = Player discard pile
	//  assumptions:  the Y range is constant for a player;
	//		the X range is constant for play / discard
	int region = -1;
	const int h = height / 2, w = width / 2;
	XY1 xy1 = getXY(YOU, 0, true), xy2 = getXY(YOU, 0, false);
	XY1 xy3 = getXY(ME, 0, true);

	if (X < xy1.x - w)
		return region;
	
	if (Y > xy1.y - h && Y < xy1.y + h)
		region = 1;

	if (Y > xy3.y - h && Y < xy3.y + h)
		region = 3;

	if (region == -1)
		return region;

	if (X > xy2.x - w)
		region++;

	return region;
}

bool War::findSeq(LOC *loc, int X, int Y) {
	//  Note, loc MUST contain valid player and isPlay information
	//  This function returns weather or not it still contains valid information
	//  Assume that the Y value is already within the card's Y values
	//  And that X's values are sane
	//  Basically this works if getRegion has already been called
	XY1 xyf = getXY(loc->player, 0, loc->isPlay), xyl;
	PILE *l = last(loc->player, loc->isPlay);
	int x = X - xyf.x, seq = 1;
	const int w = width / 2, inc = width / 10;

	if (l == NULL) {
		loc->isPlay = false;
		loc->player = -1;
		loc->seq = 0;
		return false;
	}
    
	xyl = getXY(loc->player, l->sequence, loc->isPlay);
	if (X > xyl.x + w || X < xyf.x - w) {
		loc->isPlay = false;
		loc->player = -1;
		loc->seq = 0;
		return false;
	}
	
	//  we have a valid location
	x += w;
	
	seq = x / inc;
	seq++;

	if (seq > l->sequence)
		seq = l->sequence;

	loc->seq = seq;
	return true;
}

PILE* War::get(int player, int sequence, bool isPlay) {
	PILE *ret;

	if (isPlay)
		ret = play[player];
	else
		ret = dis[player];

	if (ret == NULL)
		return ret;

	while (ret != NULL) {
		if (ret->sequence == sequence)
			break;
		ret = ret->next;
	}

	return ret;
}

BOOL War::statusUndo(bool status) {
	//  sets the status of the undo menu command.
	//  windows specific code is here.
	//  Returns the status that undo has been set to
	HMENU hMenu = GetMenu(hWnd);
	UINT uEnable = MF_GRAYED;

	if (status)
		uEnable = MF_ENABLED;

	return EnableMenuItem(hMenu, ID_GAME_UNDO, uEnable);
}

void War::delInst() {
	Cleanup();
	statusUndo(true);
	return;
}

War::~War() {
	Cleanup();
	statusUndo(true);

	return;
}
