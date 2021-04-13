#ifndef KLONDIKE2_H
#define KLONDIKE2_H

#include "game.h"
#include "war.h"
#include "cards.h"

struct KUNDO {
	KUNDO *prev;
	int toJ;  // section: turn over, play, win
	int toL;  // line (1 - 7)
	int toP;  // depth
	int fromJ;
	int fromL;
	//  The depth from will ALWAYS be the last item
};

struct KSELECT {
	int lval;
	int pval;
	bool selected;
};

struct KREGION {
	int jval;
	int lval;
	int pval;
};

class Klondike2 : public Solitaire {
public:
	Klondike2(HWND hWind, int info = 0);
	~Klondike2();

	void leftClick(int X, int Y);  
	void rightClick(int X, int Y);
	void rightUp();
	int load(const char *filename);
	int save(const char *filename);
	void redrawBoard();
	void undo();
	char* gameName();
	void left();
	void right();
	void up();
	void down();
	void enter();
	void deal();
	void delInst();  //  all destruction code should be here
	void Hit();
	bool keyPress(char key);
	int winWork(void* datum);
	void dblClick(int X, int Y);

private:
	KREGION getKR(int mx, int my);
	void Region3(KREGION *loc, int mx, int my);
	void Region12(KREGION *loc, int mx, int my);
	void CheckSelect(KREGION *loc, int mx, int my);
	KXY getXY(KREGION kreg);
	void calcMove(KREGION lm);
	void rKbd();
	bool totop(Cards *cur, Cards *sl, KREGION lm);
	bool topile(Cards *cur, Cards *sl, KREGION lm);
	void runAnim();

	Cards *top[7], *bot[7], *pile[4];
	KUNDO *Undo;
	int ht, wd, score;
	KSELECT sel;
	KREGION rmouse, kbd;
	bool rdn;

};

#endif
