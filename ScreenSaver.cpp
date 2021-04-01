// Minimal Screensaver
// ---------------------
// (c) 1998 Lucian Wischik. You may do whatever you want with this code, without restriction.
//							  
// This code is a basic minimal screensaver. It does not use SCRNSAVE.LIB or any other library:
// absolutely all the code is in this source file. 
// The saver works perfectly fine under '95 and NT. It respects the Plus! configuration settings
// (password delay, mouse movement threshold). It can handle passwords correctly.
// It makes all the correct calls in the correct places, including calls that are undocumented
// by Microsoft.
// This code accompanies the guide 'How To Write a 32bit Screen Saver'. All documentation for this
// code is in that guide. It can be found at http://classic.physiol.cam.ac.uk/scr/
//
// Notes:
// 0. All of this is a real hassle. If you used my ScrPlus library then it would all be much much
// easier. And you'd get lots of extra features, like a standard 'Plus!' style configuration dialog
// with preview in it, and proper handling of hot corners under NT as well as '95, and a high
// performance multimedia timer, and lots of examples including some that use full-screen DirectDraw.
// http://classic.physiol.cam.ac.uk/scr/
// If you have C++Builder then you should use my ScrPlus/C++Builder library which has an expert
// and components for easily generating screensavers.
// If you remain blind to the joys of using ScrPlus and are willing to waste time programming it
// all yourself, then read on...
// 1. Having a 'DEBUG' flag, with diagnostic output, is ABSOLUTELY ESSENTIAL. I can guarantee
// that if you develop a screensaver without diagnostic output for every single message that
// you handle, your screensaver will crash and you won't know why.
// 2. If you also wanted to write a configuration dialog that was able to set the standard Plus!
// options, you'd need to use two additional calls: SystemAgentDetect, and ScreenSaverChanged.
// They are documented in my 'how to write a 32bit screensaver' technical guide.
// 3. NT and '95 handle passwords differently. Under NT, the saver must terminate and then the
// verify-password dialog comes up. If the user fails, then the screensaver is launched again from
// scratch. Under '95, the password dialog comes up while the saver is running.
// 4. You should probably use WM_TIMER messages for your animation, rather than idle-processing.
// By using WM_TIMER messages your animation will keep going even when (under '95) the password
// dialog is up.
// 5. Changing the saver to allow interraction is easy. All you have to do is figure out which
// messages (keyboard, mouse, ...) will be used by you and stop them from closing the window.
// 6. Changing the saver to implement your own password routine is easy under '95: all you have
// to do is change the VerifyPassword routine. Under NT it's not really possible.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>

#pragma hdrstop

#include "resource.h"
#include "SaverSettings.h"
#include "Password.h"
#include "Debug.h"
#include "Dialogs.h"

//******************************************************************************************

#define TIMERID			1000
#define UPDATE_TIME		10

#define SIGN_EXTEND(w)	((((int)(w)) << 16) >> 16)

//******************************************************************************************

enum TScrMode { smNone, smConfig, smPassword, smPreview, smSaver};

HINSTANCE hInstance = NULL;
HWND hScrWindow = NULL;
TScrMode ScrMode = smNone;
SaverSettings* ss = NULL;

//******************************************************************************************

#if DEBUG
void Debug(char *c) { OutputDebugString(c); OutputDebugString("\n"); }
#else
void Debug(char *) {}
#endif

//******************************************************************************************

LRESULT CALLBACK
SaverWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bRoll = FALSE;
	static BOOL bMovement = FALSE;
	static POINT anchor;

	switch (msg)
	{
		case WM_CREATE:
		{
			Debug("WM_CREATE...");
			ss->hwnd = hwnd;
			GetCursorPos(&(ss->InitCursorPos));
			ss->InitTime = GetTickCount();
			ss->idTimer = SetTimer(hwnd, TIMERID, UPDATE_TIME, NULL);
			if (!InitGraphicEnv(ss->hwnd, &ss->hdc, &ss->hglrc))
				return FALSE;
			InitState();
			InitTexture(ss->hwnd, ss->hdc);
			InitDisplayLists();
		}
		break;

		case WM_TIMER:
		{
			UpdateTime();
			InvalidateRect(hwnd, NULL, FALSE);
		}
		break;

		case WM_ACTIVATE:
		case WM_ACTIVATEAPP:
		case WM_NCACTIVATE:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive && LOWORD(wParam)==WA_INACTIVE && !DEBUG)
			{
				Debug("WM_ACTIVATE: about to inactive window, so sending close");
				ss->CloseSaverWindow();
			}
		}
		break;

		case WM_SETCURSOR:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive && !DEBUG)
				SetCursor(NULL);
			else
				SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		break;

		case WM_KEYDOWN:
		case WM_MBUTTONDOWN:
			ss->CloseSaverWindow();
		break;
		
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive)
			{
				if (ss->state.bUseMouse)
				{
					RECT rc;
					bMovement = TRUE;
					GetClientRect(hwnd, &rc);
					GetCursorPos(&anchor);
					ScreenToClient(hwnd, &anchor);
					if (msg == WM_RBUTTONDOWN) bRoll = TRUE;
					else bRoll = FALSE;
					SetCapture(hwnd);
				}
				else	// Not using mouse so we can try to close the screensaver
				{
					Debug("WM_BUTTONDOWN: sending close");
					ss->CloseSaverWindow();
				}
			}
		}
		break;
		
		case WM_LBUTTONUP:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive)
			{
				bMovement = FALSE;
				bRoll = FALSE;
				ReleaseCapture();
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive)
			{
				if (ss->state.bUseMouse)
				{
					ResetOrientationGraphicEnv();
				}
				else	// Not using mouse so we can try to close the screensaver
				{
					Debug("WM_BUTTONDOWN: sending close");
					ss->CloseSaverWindow();
				}
			}
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (ScrMode==smSaver && !ss->IsDialogActive)
			{
				if (ss->state.bUseMouse)
				{
					if (bMovement & ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)) )
					{
						POINT mouse;
						mouse.x = SIGN_EXTEND(LOWORD(lParam));
						mouse.y = SIGN_EXTEND(HIWORD(lParam));
						MotionOfGraphicEnv(&anchor, mouse, bRoll);
					}
				}
				else	// Not using mouse so we can try to close the screensaver
				{
					POINT pt;
					GetCursorPos(&pt);
					int dx = pt.x-ss->InitCursorPos.x;
					if (dx<0) dx=-dx;
					int dy=pt.y-ss->InitCursorPos.y;
					if (dy<0) dy=-dy;
					if (dx>(int)ss->MouseThreshold || dy>(int)ss->MouseThreshold)
					{
						Debug("WM_MOUSEMOVE: gone beyond threshold, sending close");
						ss->CloseSaverWindow();
					}
				}
			}
		}
		break;

		case (WM_SYSCOMMAND):
		{
			if (ScrMode == smSaver)
			{
				if (wParam == SC_SCREENSAVE)
				{
					Debug("WM_SYSCOMMAND: gobbling up a SC_SCREENSAVE to stop a new saver from running.");
					return FALSE;
				}
				if (wParam == SC_CLOSE && !DEBUG)
				{
					Debug("WM_SYSCOMMAND: gobbling up a SC_CLOSE");
					return FALSE;
				}
			}
		}
		break;

		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			if (width && height)
			{
				Reshape(ss->hdc, width, height);
			}
		}
		break;

		case WM_PAINT:
		{   
			RECT r;
			PAINTSTRUCT ps;

			// Retrieve the coordinates of the smallest rectangle that completely encloses the 
			// update region of the specified window (win)
			if (GetUpdateRect(ss->hwnd, &r, FALSE))
			{   
				BeginPaint(ss->hwnd, &ps);
					Display(ss->hdc);
				EndPaint(ss->hwnd, &ps);
			}	  	
		}
		break;

		case WM_CLOSE:
		{
			if (ScrMode==smSaver && ss->ReallyClose && !ss->IsDialogActive)
			{
				Debug("WM_CLOSE: maybe we need a password");
				BOOL CanClose = TRUE;
				if (GetTickCount()-ss->InitTime > 1000*ss->PasswordDelay)
				{
					ss->StartDialog();
					CanClose = VerifyPassword(hwnd);
					ss->EndDialog();
				}
				if (CanClose)
				{
					Debug("WM_CLOSE: doing a DestroyWindow");
					DestroyWindow(hwnd);
				}
				else
				{
					Debug("WM_CLOSE: but failed password, so doing nothing");
				}
			}
			if (ScrMode == smSaver)
				return FALSE; // so that DefWindowProc doesn't get called, because it would just DestroyWindow
		}
		break;

		case WM_DESTROY:
		{
			if (ss->idTimer != 0)
				KillTimer(hwnd, ss->idTimer);
			DestroyGraphicEnv(ss->hwnd, ss->hdc, ss->hglrc);
			ss->idTimer = 0;
			Debug("POSTQUITMESSAGE from WM_DESTROY!!");
			PostQuitMessage(0);
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//******************************************************************************************

void
DoSaver(HWND hparwnd)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = SaverWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "ScrClass";
	RegisterClass(&wc);
	if (ScrMode == smPreview)
	{
		RECT rc;
		GetWindowRect(hparwnd, &rc);
		int cx = rc.right-rc.left, cy = rc.bottom-rc.top;  
		hScrWindow = CreateWindowEx(
			0, "ScrClass", "SaverPreview",
			WS_CHILD | WS_VISIBLE,
			0, 0, cx, cy,
			hparwnd, NULL, hInstance, NULL
		);
	}
	else
	{
		int cx = GetSystemMetrics(SM_CXSCREEN), cy = GetSystemMetrics(SM_CYSCREEN);
		DWORD exstyle, style;
		if (DEBUG)
		{
			cx = cx/3;
			cy = cy/3;
			exstyle = 0;
			style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		}
		else
		{
			exstyle = WS_EX_TOPMOST;
			style = WS_POPUP | WS_VISIBLE;
		}
		hScrWindow = CreateWindowEx(
			exstyle, "ScrClass", "SaverWindow",
			style, 0, 0, cx, cy,
			NULL, NULL, hInstance, NULL
		);
	}
	if (hScrWindow == NULL) return;

	UINT oldval;
	if (ScrMode == smSaver)
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &oldval, 0);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (ScrMode == smSaver)
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 0, &oldval, 0);

	return;
}

/****************************************************************************/
//
// Creates the Property Sheets - Replaces calls to DialogBox
//
int
CreatePropertySheet(HWND hwndOwner)
{
	PROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = hInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = GeneralDialogProc;
	psp[0].pszTitle = "General";
	psp[0].lParam = 0;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = hInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_GRAPHIC_CONFIG);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = GraphicConfigDialogProc;
	psp[1].pszTitle = "OpenGL";
	psp[1].lParam = 0;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE;
	psp[2].hInstance = hInstance;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_BEHAVIOR);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = BehaviorDialogProc;
	psp[2].pszTitle = "Behavior";
	psp[2].lParam = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USEICONID;
	psh.hwndParent = hwndOwner;
	psh.hInstance = hInstance;
	psh.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
	psh.pszCaption = (LPSTR) "Screen Saver Properties";
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 1;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;

	return (PropertySheet(&psh));
}

//******************************************************************************************
// This routine is for using ScrPrev. It's so that you can start the saver
// with the command line /p scrprev and it runs itself in a preview window.
// You must first copy ScrPrev somewhere in your search path.

HWND 
CheckForScrprev()
{
	HWND hwnd = FindWindow("Scrprev", NULL); // looks for the Scrprev class
	if (hwnd == NULL) // try to load it
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(si);
		si.lpReserved = NULL;
		si.lpTitle = NULL;
		si.dwFlags = 0;
		si.cbReserved2 = 0;
		si.lpReserved2 = 0;
		si.lpDesktop = 0;
		BOOL cres = CreateProcess(
				NULL, "Scrprev", 0, 0, FALSE, 
				CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE,
				0, 0, &si, &pi
		);
		if (!cres) 
		{
			Debug("Error creating scrprev process");
			return NULL;
		}
		DWORD wres = WaitForInputIdle(pi.hProcess, 2000);
		if (wres == WAIT_TIMEOUT)
		{
			Debug("Scrprev never becomes idle");
			return NULL;
		}
		if (wres == 0xFFFFFFFF)
		{
			Debug("ScrPrev, misc error after ScrPrev execution");
			return NULL;
		}
		hwnd = FindWindow("Scrprev", NULL);
	}
	if (hwnd == NULL)
	{
		Debug("Unable to find Scrprev window");
		return NULL;
	}
	::SetForegroundWindow(hwnd);
	hwnd = GetWindow(hwnd, GW_CHILD);
	if (hwnd == NULL)
	{
		Debug("Couldn't find Scrprev child");
		return NULL;
	}
	return hwnd;
}

//******************************************************************************************

int WINAPI
WinMain(HINSTANCE h, HINSTANCE, LPSTR, int)
{
	hInstance = h;
	HWND hwnd = NULL;
	INITCOMMONCONTROLSEX initCntrls;

	char *c = GetCommandLine();
	if (*c == '\"') 
	{
		c++; 
		while (*c!=0 && *c!='\"') c++;
	} 
	else 
	{
		while (*c!=0 && *c!=' ') c++;
	}
	if (*c != 0) c++;
	while (*c == ' ') c++;

	if (*c == 0) 
	{
		ScrMode = smConfig;
		hwnd = NULL;
	}
	else
	{ 
		if (*c=='-' || *c=='/') c++;
		if (*c=='p' || *c=='P' || *c=='l' || *c=='L')
		{
			c++; 
			while (*c==' ' || *c==':') c++;
			if ( (strcmp(c, "scrprev") == 0) || (strcmp(c, "ScrPrev") == 0) || 
				 (strcmp(c, "SCRPREV") == 0) ) 
				hwnd = CheckForScrprev();
			else hwnd = (HWND)atoi(c);
			ScrMode = smPreview;
		}
		else if (*c=='s' || *c=='S') 
		{ 
			ScrMode = smSaver; 
		}
		else if (*c=='c' || *c=='C') 
		{
			c++; 
			while (*c==' ' || *c==':') c++; 
			if (*c == 0) hwnd = GetForegroundWindow(); 
			else hwnd = (HWND)atoi(c); 
			ScrMode = smConfig;
		}
		else if (*c=='a' || *c=='A') 
		{
			c++; 
			while (*c==' ' || *c==':') c++; 
			hwnd = (HWND)atoi(c); 
			ScrMode = smPassword;
		}
	}

	initCntrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	initCntrls.dwICC  = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&initCntrls);

	// We create a global SaverSettings here, for convenience. 
	// It will get used by the config dialog and by the saver as it runs.

	ss = new SaverSettings();
	ss->ReadGeneralRegistry(); 
	ss->ReadGraphicConfigRegistry();
	ss->ReadBehaviorRegistry();
  
	if (ScrMode == smPassword) ChangePassword(hwnd);
	if (ScrMode == smConfig)
	{
		CreatePropertySheet(hwnd);
	}
	if (ScrMode == smSaver || ScrMode == smPreview) DoSaver(hwnd);

	delete ss;
	return 0;
}

//******************************************************************************************
