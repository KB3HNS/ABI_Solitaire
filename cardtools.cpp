#include "stdafx.h"
#include "colordefs.h"
#include "cardtools.h"
#include "debug.h"
#include <math.h>
#include <assert.h>
#include <gdiplus.h>

using namespace Gdiplus;

//  Card images are global to this module
Image *card[112];
ULONG_PTR gdiplusToken;  //  GDI+ Init / Shutdown Tolken

struct SORT {
	SORT *prev;
	int val;
	SORT *next;
};


//  Initializer function.  Generates card images, reads a few key values.
//  This function will be developed as the card placement API evolves (sp?).
//  GDIinit has also now been moved here.

void initCards(HWND hWnd) {
	HKEY key, software, parent;
	DWORD lpdwD;
	SECURITY_ATTRIBUTES sa;
	//  GDI+ Init
	GdiplusStartupInput gdiplusStartupInput;
	OPENFILENAME file;

	wchar_t filename[MAX_LOADSTRING + 16];
	char data[MAX_LOADSTRING + 1];
	int len = MAX_LOADSTRING;
	int i;
	
	ZeroMemory(data, MAX_LOADSTRING + 1);
	ZeroMemory(filename, (MAX_LOADSTRING + 16) * 2);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//  Create parent
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, NULL, REG_OPTION_NON_VOLATILE,
		 KEY_WRITE, &sa, &software, &lpdwD);

	RegCreateKeyEx(software, "ABI", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
		&sa, &parent, &lpdwD);

	RegCreateKeyEx(parent, "Solitaire", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
		&sa, &key, &lpdwD);


	if (lpdwD == REG_CREATED_NEW_KEY) {
		ZeroMemory(&file, sizeof(file));
		//ZeroMemory(filename, 300);
		strcpy(data, "1.bmp");
		file.lStructSize = sizeof(file);
		file.hwndOwner = hWnd;
		//file.hInstance = hInst;
		file.lpstrFilter = "Solitaire card image\0 1.bmp\0";
		file.lpstrCustomFilter = NULL;
		file.nMaxCustFilter = 0;
		file.nFilterIndex = 1;
		file.lpstrFile = data;
		file.nMaxFile = MAX_LOADSTRING;
		file.lpstrInitialDir = "bitmaps";
		file.lpstrFileTitle = NULL;
		file.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		file.lpstrDefExt = "bmp";
				
		while (!GetOpenFileName(&file));

		for (int i = file.nFileOffset; i < MAX_LOADSTRING + 1; i++)
			data[i] = 0;

		RegSetValueEx(key, "bitmaps location", 0, REG_SZ, (BYTE*)data, 1);
		/*MessageBox(hWnd,
			"Remember to set\n\tHKEY_LOCAL_MACHINE/SOFTWARE/ABI/solitaire/bitmaps location\n\tto the location of the card files!", 
			"New Registry Entry", MB_OKCANCEL);*/
	}

	RegCloseKey(key);
	RegCloseKey(software);
	RegCloseKey(parent);

	ZeroMemory(data, MAX_LOADSTRING + 1);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &software);

	RegOpenKeyEx(software, "ABI", 0, KEY_READ, &parent);

	RegOpenKeyEx(parent, "Solitaire", 0, KEY_READ, &key);
	
	RegQueryValueEx(key, "bitmaps location", NULL, NULL, (BYTE*)data, (LPDWORD)&len);
	
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	for (i = 0; i < 111; i++) {
		swprintf(filename, L"%S/%i.bmp", data, i+1);
		card[i] = new Image(filename);
	}

	RegCloseKey(key);
	RegCloseKey(software);
	RegCloseKey(parent);
	return;
}

//  drawCard Draws card at mouse location when left button is clicked (once?)
void drawCard(HWND hWnd, int xPos, int yPos, int cardNum, int rotation,
			  bool selected, double zoom) {
	HDC hdc = GetDC(hWnd);
	Graphics graphics(hdc);
	Point skewer[3];
	int a, b, c, d;  //  Integer storage
	double rad;
	ImageAttributes imageAttributes;
	ColorMap colorMap[2];

	cardNum--;  //  We will be receiving a card from 1 to 55, convert it to our array

	if (cardNum == -1)
		return;  //  Do not draw anything if the card = 0
		//cardNum = 54;

	rad = DegtoRad(rotation + 360);
	a = card[cardNum]->GetHeight() / 2;
	b = card[cardNum]->GetWidth() / 2;
	c = (int)((-cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += yPos;
	d += xPos;
	skewer[0] = Point(d, c);

	c = (int)((-cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += yPos;
	d += xPos;
	skewer[1] = Point(d, c);

	c = (int)((cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (-sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += yPos;
	d += xPos;
	skewer[2] = Point(d, c);

	//dprintf("Called void dwawCard! hWnd = %i\n", (int)hWnd);
	//cardNum += 55;
	if (selected) {
		colorMap[0].oldColor = Color(255, 0, 0, 0);
		colorMap[0].newColor = Color(255, 255, 255, 255);
		colorMap[1].oldColor = Color(255, 255, 255, 255);
		colorMap[1].newColor = Color(255, 0, 0, 0);
		imageAttributes.SetRemapTable(2, colorMap, ColorAdjustTypeBitmap);
		
		if (graphics.DrawImage(card[cardNum], skewer, 3, 0, 0, card[cardNum]->GetWidth(),
			card[cardNum]->GetHeight(), UnitPixel, &imageAttributes) != 0) {
				/*wchar_t filename[16];
				int i = cardNum + 55;
				Image *temp;
				
				//if (i == 56)
				//	i = 560;
				swprintf(filename, L"bitmap/%i.bmp", i);
				temp = new Image(filename);
				
				assert(card[cardNum]);
				assert(temp);
				graphics.DrawImage(temp, skewer, 3);
				delete temp;*/
				graphics.DrawImage(card[cardNum + 55], skewer, 3);
			}

	} else {
		graphics.DrawImage(card[cardNum], skewer, 3);
	}

	ReleaseDC(hWnd, hdc);

	return;	
}

double DegtoRad(int deg) {
	double rad = deg * PI / 180;
	return rad;
}

int RadtoDeg(double rad) {
	double deg = rad * 180 / PI;
	return (int)deg;
}

//  Erases a previously drawn card.
//  Note!  This will erase much more than the card at the moment.
//  TODO:  Incorporate code that makes erase more selective.  --  Done.
void eraseCard (HWND hWnd, int cx, int cy, int rotation, double zoom, int cardNum) {
	HDC hdc = GetDC(hWnd);
	HGDIOBJ hbr = CreateSolidBrush(BGCOLOR);
	HGDIOBJ hbrOld;
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, BGCOLOR);
	HGDIOBJ hPenOld; 
	POINT skewer[4];
	int a, b, c, d, lastFill;  //  Integer storage
	double rad;
	const int vertex = 4;

	cardNum--;  //  We will be receiving a card from 1 to 55, convert it to our array

	if (cardNum == -1)
		cardNum = 0;  //  Use the default card if cardNum = 0
	
	rad = DegtoRad(rotation + 360);
	a = card[cardNum]->GetHeight() / 2;
	b = card[cardNum]->GetWidth() / 2;
	c = (int)((-cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += cy;
	d += cx;
	skewer[0].x = d;
	skewer[0].y = c;

	c = (int)((-cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += cy;
	d += cx;
	skewer[1].x = d;
	skewer[1].y = c;

	c = (int)((cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (-sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += cy;
	d += cx;
	skewer[2].x = d;
	skewer[2].y = c;

	c = (int)((cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (-sin(rad) * a));
	c = (int)(c * zoom);
	d = (int)(d * zoom);
	c += cy;
	d += cx;
	skewer[3].x = d;
	skewer[3].y = c;

	hPenOld = SelectObject(hdc, hPen);
	hbrOld = SelectObject(hdc, hbr);
	lastFill = SetPolyFillMode(hdc, ALTERNATE);

	PolyPolygon(hdc, skewer, &vertex, 1);
		
	SelectObject(hdc, hPenOld); 
	DeleteObject(hPen);
	SelectObject(hdc, hbrOld); 
	DeleteObject(hbr);
	SetPolyFillMode(hdc, lastFill);
	ReleaseDC(hWnd, hdc);

	return;	
}

CARDDIM getCardAttributes(int cardNum) {
	CARDDIM dim;

	dim.Height = card[cardNum]->GetHeight();
	dim.Width = card[cardNum]->GetWidth();

	return dim;
}

void clearBoard(HWND hWnd) {
	HDC hdc = GetDC(hWnd);
	RECT boardArea;
	HBRUSH hbr = CreateSolidBrush(BGCOLOR);

	boardArea.top = 0;
	boardArea.left = 0;
	boardArea.bottom = GetDeviceCaps(hdc, VERTRES);
	boardArea.right = GetDeviceCaps(hdc, HORZRES);

	FillRect(hdc, &boardArea, hbr);

	ReleaseDC(hWnd, hdc);

	return;	
}

_CRTIMP int __cdecl bprintf(HWND hWnd, int x, int y, int Color, char *output, ...) {
	va_list marker;
	int count, len, i;
	HDC hdc = GetDC(hWnd);
	HGDIOBJ hfnt, hOldFont;
	char *outString;
	//char outString[1000];

	/*for (i = 0; i < 1000; i++)
		outString[i] = 0;*/

	va_start(marker, output);
	count = vprintf(output, marker) + 1;
	/*if (count > 1000)
		return -1;*/

	outString = (char*)malloc((size_t)count);
	//outString = new char(count);

	vsprintf(outString, output, marker);
	len = (int)strlen(outString);
	hfnt = GetStockObject(ANSI_VAR_FONT);

	if (hOldFont = SelectObject(hdc, hfnt)) {
		//  White on green theme
		SetBkColor(hdc, BGCOLOR);
		SetTextColor(hdc, Basic(Color));
		TextOut(hdc, x, y, outString, len);
		SelectObject(hdc, hOldFont);
	}

	va_end(marker);
	ReleaseDC(hWnd, hdc);
	DeleteObject(hfnt);

	for (i = 0; i < count; i++)
		outString[i] = 0;

	//  Bug?  Memory Hole!!!
	//delete outString;
	free(outString);

	return count - 1;  //  For Compatibility
}

void putMouse(int x, int y, HWND hWnd) {
	POINT pt;
	
//	GetCursorPos(&useless);
//	ScreenToClient(hWnd, &useless);

	pt.x = x;
	pt.y = y;
	ClientToScreen(hWnd, &pt); 
	SetCursorPos(pt.x, pt.y);
	return;
}

double axis2zoom(int zoom) {
	double expn = (double)zoom / 20;
	return pow((double)2, expn);
}


void hsort(int *source, int *dest, int count, bool down) {
	SORT *root, *local = NULL, *base, *temp, *temp1;
	int i, n;
	bool place;

	root = new SORT;
	root->next = NULL;
	root->prev = NULL;
	root->val = *source;
	base = root;

	for (i = 1; i < count; i++) {
		n = source[i];
		local = base;
		place = false;
		if ((n < local->val && !down) || (n > local->val && down)) {
			while (local->next != NULL && !place) {
				local = local->next;
				if ((n > local->val && !down) || (n < local->val && down)) { // insert
					temp = local->next;
					temp1 = local;
					local->next = new SORT;
					local = local->next;
					local->prev = temp1;
					local->next = temp;
					local->val = n;
					place = true;
				}
			}
			if (!place) { // append
				local->next = new SORT;
				temp = local;
				local = local->next;
				local->prev = temp;
				local->next = NULL;
				local->val = n;
				place = true;
			}
		} else {  // prepend
			base->prev = new SORT;
			temp = base;
			base = base->prev;
			base->prev = NULL;
			base->val = n;
			base->next = temp;
			place = true;
		}  //endif (n > local->val)
	} // for

	local = base;
	for (i = 0; i < count; i++) {
		dest[i] = local->val;
		temp = local;
		local = local->next;
		delete temp;
	}
	
	return;
}

CARDDIM getBoardDims(HWND hWnd) {
	CARDDIM ret;
	RECT hRect;

	GetClientRect(hWnd, &hRect);
	ret.Height = hRect.bottom;
	ret.Width = hRect.right;

	return ret;
}

void closeCards() {
	//  assume that cards are valid already
	int i;
	for (i = 0; i < 111; i++) 
		delete card[i];

	GdiplusShutdown(gdiplusToken);
	return;
}

