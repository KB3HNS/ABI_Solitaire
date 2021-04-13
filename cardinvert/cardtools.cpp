#include "stdafx.h"
#include "colordefs.h"
#include "cardtools.h"
#include "debug.h"
#include <math.h>
#include "colordefs.h"

using namespace Gdiplus;

#define PI 3.1415926535897932384626433832795

//  Internal functions
int RadtoDeg(double rad);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

//  Card images are global to this module
Image *card[56];

//  Initializer function.  Generates card images, reads a few key values.
//  This function will be developed as the card placement API evolves (sp?).
void initCards() {
	wchar_t filename[15];
	int i;
	
	for (i = 0; i < 56; i++) {
		swprintf(filename, L"bitmap/%i.bmp", i+1);
		card[i] = new Image(filename);
	}

	return;
}

//  drawCard Draws card at mouse location when left button is clicked (once?)
void drawCard(HWND hWnd, int xPos, int yPos, int cardNum, int rotation, bool selected) {
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
	c += yPos;
	d += xPos;
	skewer[0] = Point(d, c);

	c = (int)((-cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (sin(rad) * a));
	c += yPos;
	d += xPos;
	skewer[1] = Point(d, c);

	c = (int)((cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (-sin(rad) * a));
	c += yPos;
	d += xPos;
	skewer[2] = Point(d, c);

	//dprintf("Called void dwawCard! hWnd = %i\n", (int)hWnd);
	if (selected) {
		colorMap[0].oldColor = Color(255, 0, 0, 0);
		colorMap[0].newColor = Color(255, 255, 255, 255);
		colorMap[1].oldColor = Color(255, 255, 255, 255);
		colorMap[1].newColor = Color(255, 0, 0, 0);
		imageAttributes.SetRemapTable(2, colorMap, ColorAdjustTypeBitmap);
		
		graphics.DrawImage(card[cardNum], skewer, 3, 0, 0, card[cardNum]->GetWidth(),
			card[cardNum]->GetHeight(), UnitPixel, &imageAttributes);
		
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
void eraseCard (HWND hWnd, int cx, int cy, int rotation, int cardNum) {
	HDC hdc = GetDC(hWnd);
	HGDIOBJ hbr = CreateSolidBrush(RGB(0,128,0));
	HGDIOBJ hbrOld;
	HGDIOBJ hPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
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
	c += cy;
	d += cx;
	skewer[0].x = d;
	skewer[0].y = c;

	c = (int)((-cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (sin(rad) * a));
	c += cy;
	d += cx;
	skewer[1].x = d;
	skewer[1].y = c;

	c = (int)((cos(rad) * a) + (sin(rad) * b));
	d = (int)((cos(rad) * b) + (-sin(rad) * a));
	c += cy;
	d += cx;
	skewer[2].x = d;
	skewer[2].y = c;

	c = (int)((cos(rad) * a) + (-sin(rad) * b));
	d = (int)((-cos(rad) * b) + (-sin(rad) * a));
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
	HBRUSH hbr = CreateSolidBrush(RGB(0,128,0));

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
	//char *outString;
	char outString[1000];

	for (i = 0; i < 1000; i++)
		outString[i] = 0;

	va_start(marker, output);
	count = vprintf(output, marker);
	if (count > 1000)
		return -1;

//	outString = (char*)malloc((size_t)count);
	//outString = new char(count);

	vsprintf(outString, output, marker);
	len = (int)strlen(outString);
	hfnt = GetStockObject(ANSI_VAR_FONT);

	if (hOldFont = SelectObject(hdc, hfnt)) {
		//  White on green theme
		SetBkColor(hdc, RGB(0, 128, 0));
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
	//free(outString);

	return count;  //  For Compatibility
}

void closeCards() {
	int i;
	
	//  Very Dangerous.  Do not call unless init has already been called.
	for (i = 0; i < 56; i++)
		delete card[i];
	
	return;
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

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}
