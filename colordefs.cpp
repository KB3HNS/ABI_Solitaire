#include "stdafx.h"
#include "colordefs.h"

//  A quick and dirty way to translate colors..  I'm starting out using the 
COLORREF Basic(int Color) {
	//  Remember 0x00bbggrr
	switch (Color) {
		case BLACK:  //0
			return RGB(0,0,0);
			break;

		case BLUE:  //1
			return RGB(0,0,128);
			break;

		case GREEN:  //2
			return RGB(0,128,0);
			break;

		case RED:  //3
			return RGB(128,0,0);
			break;

		case CYAN:  //4
			return RGB(0,128,128);
			break;

		case MAGENTA: //5
			return RGB(128,0,128);
			break;

		case GREY:  //6
			return RGB(64,64,64);
			break;

		case WHITE:  //7
			return RGB(128,128,128);
			break;

		case BBLUE: //8
			return RGB(0,0,255);
			break;

		case BGREEN: //9
			return RGB(0,255,0);
			break;

		case BRED:  //10
			return RGB(255,0,0);
			break;

		case LCYAN:  //11
			return RGB(0,255,255);
			break;

		case LMAGENTA:  //12
			return RGB(255,0,255);
			break;

		case YELLOW:  //13
			return RGB(255,255,0);
			break;

		case ORANGE:  //14
			return RGB(255,128,0);
			break;

		case BWHITE:  //15
			return RGB(255,255,255);
			break;

		default:
			return RGB(0,0,0);  //black is the default
	}
	//we've already returned
}
