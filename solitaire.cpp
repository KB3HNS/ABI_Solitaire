// solitaire.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "solitaire.h"
#include "debug.h"
#include "cardtools.h"
#include "game.h"
#include "sanda.h"
#include "dummy.h"
#include "dbgdummy.h"
#include "war.h"
#include "klondike2.h"
#include "pyrimid.h"
#include "calculation.h"
#include "fourcorners.h"
#include <htmlhelp.h>
#include <assert.h>

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL				getGameType(HWND hWnd, int nCmdShow, LPTSTR cmdLine);
void				runHelp(HWND hWnd);

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
int gameType;
Solitaire *game;

//using namespace Gdiplus;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	HWND hWnd;
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SOLITAIRE, szWindowClass, MAX_LOADSTRING);
	
	MyRegisterClass(hInstance);

	// Perform application initialization:
	hWnd = InitInstance (hInstance, nCmdShow);
	
	if (hWnd == NULL)
		return FALSE;
	
	if (getGameType(hWnd, nCmdShow, lpCmdLine) == NULL)
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SOLITAIRE);

	// Main message loop:
	dprintf("Solitaire version %i.%i build %i...  Entering MAIN loop."
		, VMAJOR, VMINOR, VBUILD);
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	closeCards();

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	HBRUSH hbrush = CreateSolidBrush(BGCOLOR);

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SOLITAIRE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= hbrush;
	wcex.lpszMenuName	= (LPCTSTR)IDC_SOLITAIRE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, "Solitaire", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   
   if (!hWnd)
   {
      return NULL;
   }

   //  Create our Streets and Alleys game object
   //  ToDo:  Add other game handlers.
   debugInit(hInstance, nCmdShow, szWindowClass);
   initCards(hWnd);

   gameType = 0;
   game = static_cast<Solitaire*>(new DUMMY(hWnd, gameType));

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	int xPos, yPos; //  added for mouse information
	PAINTSTRUCT ps;
	HDC hdc;
	OPENFILENAME file;
	int worked = TRUE;
	static char filename[300];

   	switch (message) 
	{

	case WM_LBUTTONDBLCLK:
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
		game->dblClick(xPos, yPos);
		game->Hit();
		break;
	
	case WM_LBUTTONDOWN:

		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
		game->leftClick(xPos, yPos);
		game->Hit();

		break;
		
	case WM_RBUTTONDOWN:

		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);
		game->rightClick(xPos, yPos);
		game->Hit();

		break;

	case WM_RBUTTONUP:
		game->rightUp();
		break;


	case WM_KEYDOWN:

		game->Hit();
		switch(wParam) {

			case VK_LEFT:
				game->left();
//				dprintf("left");
				break;

			case VK_RIGHT:
				game->right();
//				dprintf("right");
				break;

			case VK_UP:
				game->up();
//				dprintf("up");
				break;
				
			case VK_DOWN:
				game->down();
//				dprintf("down");
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}

	case WM_CHAR:

		game->Hit();
		switch (wParam) {
			case 'n':
			case 'N':
				game->shuffel();
				game->deal();
				game->redrawBoard();
				break;

			case 0x08: //  Backspace
				game->undo();
				break;
		
			case 0x0D:
				game->enter();
				break;

			default:
				if (!game->keyPress((char)wParam)) 
					/*char text[50];
					sprintf(text, "Button Pressed: %c", (char)wParam);
					MessageBox(hWnd, text, NULL, MB_OKCANCEL);*/
					return DefWindowProc(hWnd, message, wParam, lParam);
				
		}
		break;
		
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		game->Hit();

		// Parse the menu selections:
		switch (wmId)
		{
			case IDM_ABOUT:
				DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;

			case ID_NEW_STREETSANDALLEYS:
				if (gameType != 1) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}
					gameType = 1;
					game = static_cast<Solitaire*>(new StreetsAndAlleys(hWnd, gameType));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

				}
			
				game->shuffel();
				game->deal();
				game->redrawBoard();

				break;

			case ID_GAME_UNDO:
				game->undo();
				break;

			case ID_GAME_RESTART:
				game->deal();
				game->redrawBoard();
				break;

			case ID_NEW_WAR:
				//  Unimplimented
				//dprintf("ID_NEW_WAR is Unimplimented\n");
				if (gameType != 2) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}

					gameType = 2;
					game = static_cast<Solitaire*>(new War(hWnd, gameType));
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					gameType = 2;
				}
				game->shuffel();
				game->deal();
				game->redrawBoard();
				break;
			
			case ID_NEW_KLONDIKEII:
				if (gameType != 3) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}
					gameType = 3;
					game = static_cast<Solitaire*>(new Klondike2(hWnd, gameType));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

				}
			
				game->shuffel();
				game->deal();
				game->redrawBoard();

				break;

			case ID_NEW_PYRIMID:
				if (gameType != 4) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}
					gameType = 4;
					game = static_cast<Solitaire*>(new Pyrimid(hWnd, gameType));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

				}
			
				game->shuffel();
				game->deal();
				game->redrawBoard();

				break;

			case ID_NEW_CALCULATION:
				if (gameType != 5) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}
					gameType = 5;
					game = static_cast<Solitaire*>(new Calculation(hWnd, gameType));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

				}
			
				game->shuffel();
				game->deal();
				game->redrawBoard();

				break;
			
			case ID_NEW_FOURCORNERS:
				if (gameType != 6) {
					if (game != NULL) {
						game->delInst();
						delete game;
					}
					gameType = 6;
					game = static_cast<Solitaire*>(new FourCorners(hWnd, gameType));
					ZeroMemory(filename, 300); //  Erase file name when switching classes
					SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

				}
			
				game->shuffel();
				game->deal();
				game->redrawBoard();

				break;

			case ID_GAME_SHUFFEL:
				game->shuffel();
				game->deal();
				game->redrawBoard();
				break;

			case ID_HELP_HELPTOPICS:
				//  Unimplimented
				runHelp(hWnd);
				//dprintf("ID_HELP_HELPTOPICS is Unimplimented\n");
				break;

			case ID_FILE_SAVE:
				if (filename[0] != 0) {
					game->save(filename);
					return 0;
				}
				//  Otherwise we save to new file name
			case ID_FILE_S:
				ZeroMemory(&file, sizeof(file));

				file.lStructSize = sizeof(file);
				file.hwndOwner = hWnd;
				//file.hInstance = hInst;
				file.lpstrFilter = "Solitaire Data Files\0*.SOL;\0All Files\0*.*\0";
				file.lpstrCustomFilter = NULL;
				file.nMaxCustFilter = 0;
				file.nFilterIndex = 1;
				file.lpstrFile = filename;
				file.nMaxFile = 256;
				file.lpstrInitialDir = NULL;
				file.lpstrFileTitle = NULL;
				file.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP;
				file.lpstrDefExt = "SOL";
				//file.pvReserved = NULL;
				//file.dwReserved = 0;
				//file.FlagsEx = 0;

				if (GetSaveFileName(&file))
					worked = game->save(filename);
				else
					ZeroMemory(filename, 300);

				dprintf(filename);

				if (worked != TRUE) {
					char text[25];
					dprintf("Save bailed with value %i", worked);
					sprintf(text, "Save Generated error #%i", worked);
					MessageBox(hWnd, text, NULL, MB_OKCANCEL);
				}
				
				break;

			case ID_FILE_OPEN:
				ZeroMemory(&file, sizeof(file));
				//ZeroMemory(filename, 300);
				
				file.lStructSize = sizeof(file);
				file.hwndOwner = hWnd;
				//file.hInstance = hInst;
				file.lpstrFilter = "Solitaire Data Files\0*.SOL;\0All Files\0*.*\0";
				file.lpstrCustomFilter = NULL;
				file.nMaxCustFilter = 0;
				file.nFilterIndex = 1;
				file.lpstrFile = filename;
				file.nMaxFile = 256;
				file.lpstrInitialDir = NULL;
				file.lpstrFileTitle = NULL;
				file.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | 
					OFN_PATHMUSTEXIST;
				file.lpstrDefExt = "SOL";
				//file.pvReserved = NULL;
				//file.dwReserved = 0;
				//file.FlagsEx = 0;
				
				if (GetOpenFileName(&file))
					if (gameType == 0)
						getGameType(hWnd, 0, filename);
					else
						worked = game->load(filename);
				else
					ZeroMemory(filename, 300);
				
				dprintf(filename);

				if (worked != TRUE) {
					char text[255];
					dprintf("Open bailed with value %i", worked);
					switch (worked) {
						case FALSE:
							sprintf(text, "Could not open file: %s",
								filename + file.nFileOffset);
							break;

						case INVAL_GAME_TYPE:
						case EOF:
							sprintf(text, "File: %s Contains invalid game information.",
								filename + file.nFileOffset);
							break;

						default:
                            sprintf(text, "Open Generated error #%i", worked);
					}
					MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
				}

				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

#if (DEBUG == TRUE)
		if (gameType == 0) {
			dbgDummy *temp = static_cast<dbgDummy*>(game);
			temp->exInit(hWnd, gameType);
		}
#endif


		// TODO: Add any drawing code here...
		//if (game != NULL)
		game->redrawBoard();
		game->Hit();

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	case WM_CREATE:
		ZeroMemory(filename, 300);
		break;
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char ver[100];

	switch (message)
	{
	case WM_INITDIALOG:
		sprintf(ver, "Solitaire Version %i.%i, Build %i", VMAJOR, VMINOR, VBUILD);
		SetDlgItemText(hDlg, IDC_Version, ver);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL getGameType(HWND hWnd, int nCmdShow, LPTSTR cmdLine) {
	//  An important note:  There should be no spaces except between switches and
	//  the file name.  Also, the filename MUST be the last argument.
	//  Priority:  filename, gametype, mouse input.
	
	char *cmd = static_cast<char*>(cmdLine);
	char *filename = NULL, *switches = cmd, text[255];
	int i = 0, ret = IDOK, last = 0;
	FILE *file;
	bool load = false;

	dprintf("Command Line: %s, nCmdShow = %i", cmdLine, nCmdShow);
	
	while ((switches = strstr(switches, "/")) != 0) {
		//  we have a switch..  process it.
		switches++;
		switch (*switches) {

			case 'g':
			case 'G':
				switch (*(switches + 2)) {
					case 's':
					case 'S':
						gameType = 1;
						break;

					case 'w':
					case 'W':
						gameType = 2;
						break;

					case 'k':
					case 'K':
						gameType = 3;
						break;
					
					case 'p':
					case 'P':
						gameType = 4;
						break;

					case 'c':
					case 'C':
						gameType = 5;
						break;

					case 'f':
					case 'F':
						gameType = 6;
						break;

					default:
						sprintf(text, "Error: %s - Game Type not recognized.", switches);
						ret = MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
				}
				break;

			case 'h':
			case 'H':
			case '?':
				sprintf(text,
					"Usage:  solitaire.exe [options] [filename]\n\t/h - help\n\t/g=gametype");
				ret = MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
				break;

			case 'f':
			case 'F':
				filename = switches + 2;
				break;

			default:
				sprintf(text, "Error:  Bad command arguments: %s", switches);
				ret = MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
		}

	}// wend	
	//  The file name must be everything left.. including spaces

	if (switches == NULL)
		switches = cmd;
	else 
		switches = strstr(switches, " ") + 1;  //  find the NEXT space after arguments
	
	if (filename == NULL)
		filename = switches;
	
	if (filename[0] != 0) {
		file = fopen(filename, "rb");
		if (file != NULL) {
			i = fgetc(file);
		} else {
			i = EOF;
		}
		fclose(file);
		load = true;

		switch (i) {
			case 1:
				gameType = 1;
				break;
			
			case 2:
				gameType = 2;
				break;

			case 3:
				gameType = 3;
				break;

			case 4:
				gameType = 4;
				break;

			case 5:
				gameType = 5;
				break;

			case 6:
				gameType = 6;
				break;

			default:
				sprintf(text, "Could not load file: %s", filename);
				ret = MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
				load = false;
		}
	} //if (filename[0] != 0)

	switch (gameType) {
	case 1:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new StreetsAndAlleys(hWnd, gameType));
		break;

	case 2:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new War(hWnd, gameType));
		//  Unimplimented
		break;

	case 3:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new Klondike2(hWnd, gameType));
		break;

	case 4:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new Pyrimid(hWnd, gameType));
		break;

	case 5:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new Calculation(hWnd, gameType));
		break;
		
	case 6:
		game->delInst();
		delete game;
		game = static_cast<Solitaire*>(new FourCorners(hWnd, gameType));
		break;

	case 0:
	default:  //  Should never happen... :-)
//		game = static_cast<Solitaire*>(new DUMMY(hWnd, gameType));
		//  Nothing should change
		UNUSED(game);
	}

	if (load) {
		if (game->load(filename) != TRUE) {
			sprintf(text, "Could not load file: %s", filename);
			ret = MessageBox(hWnd, (LPCSTR)text, NULL, MB_OKCANCEL);
			gameType = 0;
			game->delInst();
			delete game;
			game = static_cast<Solitaire*>(new DUMMY(hWnd, gameType));
		}
	}

	if (gameType != 0)
		SetWindowText(hWnd, (LPCTSTR)(game->gameName()));

	if (ret != IDOK) 
		return FALSE;

	return TRUE;
}

void runHelp(HWND hWnd) {
	HKEY key, software, parent;
	char filename[MAX_LOADSTRING + 16];
	char data[MAX_LOADSTRING + 1];
	char *loc;
	int len = MAX_LOADSTRING;

	ZeroMemory(data, MAX_LOADSTRING + 1);
	ZeroMemory(filename, MAX_LOADSTRING + 16);

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &software);

	RegOpenKeyEx(software, "ABI", 0, KEY_READ, &parent);

	RegOpenKeyEx(parent, "Solitaire", 0, KEY_READ, &key);
	
	RegQueryValueEx(key, "bitmaps location", NULL, NULL, (BYTE*)data, (LPDWORD)&len);

	loc = strstr(data, "Bitmap");
	assert(loc);

	while (loc < data + MAX_LOADSTRING + 1) {
		*loc = 0;
		loc++;
	}
	
	sprintf(filename, "%shelp\\solitaire.chm", data);

	HtmlHelp(hWnd, filename, HH_DISPLAY_TOC, 0);

	RegCloseKey(key);
	RegCloseKey(software);
	RegCloseKey(parent);

	return;
}
