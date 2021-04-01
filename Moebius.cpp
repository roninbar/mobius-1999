//==========================================================================
//
//  File: Mobius.cpp
//
//==========================================================================

#include <windows.h>
#include <stdio.h>
#include "afxres.h"
#include "resource.h"
#include "OpenGLenv.h"

#define TIMERID			1000
#define UPDATE_TIME		10

#define SIGN_EXTEND(w)	((((int)(w)) << 16) >> 16)


/****************************************************************************/

#ifndef _SIM_DLL
		int style = WS_OVERLAPPEDWINDOW; 
#else
		int style =  WS_CHILD | WS_CAPTION;         // style 
		char* str_DllName = "SimOpenGLDLL";
#endif
   
/****************************************************************************/
// Forward function declarations

static BOOL FirstInstance(HINSTANCE);
static BOOL AnyInstance(HWND&, HINSTANCE, int);
long FAR PASCAL WindowProc(HWND, UINT, WPARAM, LPARAM);

/****************************************************************************/
// Frame rate stuff
DWORD g_dwBegTime = 0;
DWORD g_dwLastTime = 0;
DWORD g_dwCurTime = 0;
DWORD g_dwFpsTime = 0;
DWORD g_dwDeltaTime = 0;
DWORD g_dwFramesRendered = 0;
DWORD g_dwFps = 0;
DWORD g_faces = 0;

/****************************************************************************/

HINSTANCE	_prev_inst	= NULL;
LPSTR		_cmdline	= NULL;
int			_cmdshow	= SW_SHOW;

int hChildLeft	= 0;
int hChildTop	= 0;
int hChildWidth	= 512;
int hChildHeight= 512;
HWND	hParwin = NULL;
HWND	hWin = NULL;			// Child application
HDC		hdc = NULL;
HGLRC	hglrc = NULL;

static char ClassName[32] = "OpenGLClass";
static char CaptionName[32] = "OpenGL Moebius Clock";

BOOL done = FALSE;			// If simulation should be finished
BOOL bQuit = FALSE;			// Set if we are quitting
BOOL bMovement = FALSE;

extern GraphicEnvState state;

/****************************************************************************/
//
// Register the window class for the application. Also perform other
// desired application initialization
//
static BOOL
FirstInstance(HINSTANCE this_inst)
{
	// Contains the window class attributes to be registered
	WNDCLASS    wc;
	// Status of the RegisteredClass call
	BOOL        rc;
	
	//
	// Fill the slots of and register the window class
	//
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD);
	wc.hInstance = this_inst;
	wc.hIcon = LoadIcon(this_inst, MAKEINTRESOURCE(IDI_ICON1));
	//wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = "OpenGLMenu";
	wc.lpszClassName = ClassName;
	rc = RegisterClass(&wc);

	// Return the initialized object
	return rc;
}

/****************************************************************************/
//
// Perform the setup required for every instance of the application -
// Creation of the window and initialization of the data.
//

static BOOL
AnyInstance(HWND& win, HINSTANCE this_inst, int cmdshow)
{
	HMENU hmenu = LoadMenu(this_inst, MAKEINTRESOURCE(IDR_MENU1));

	//
	// Create the main window
	//
	win = CreateWindow(
				ClassName,			// class
				CaptionName,		// caption 
				style,				// style 
				hChildLeft,			// init. x pos 
				hChildTop,			// init. y pos 
				hChildWidth,		// init. x size 
				hChildHeight,		// init. y size 
				hParwin,			// parent window 
				hmenu,				// menu handle 
				this_inst,			// program handle 
				NULL				// create params 
	);
	if (!win) return FALSE;

	if (!InitGraphicEnv(hWin, &hdc, &hglrc)) return FALSE;

	InitState();
	InitTexture();
	InitDisplayLists();

	SetTimer(win, TIMERID, UPDATE_TIME, NULL);
  
	//
	// Show and update the window
	//
	ShowWindow(win, cmdshow);
	UpdateWindow(win);

	return TRUE;
}

/****************************************************************************/

BOOL
InitWindow(HINSTANCE this_inst, HWND hPar, int xleft, int xright, int width, int height)
{
	hParwin = hPar;
	hChildLeft = xleft;
	hChildTop = xright;
	hChildWidth = width;
	hChildHeight = height;

	g_dwBegTime = timeGetTime();

	// If there is no previous instance set, register the window class 
	// for the application
	if (!_prev_inst)
		if (!FirstInstance(this_inst))
			return FALSE;

	// If we cannot do the remaining window setup, return an error
	if (!AnyInstance(hWin, this_inst, _cmdshow))
		return FALSE;
	
	return TRUE;
}

/****************************************************************************/

BOOL
ReInitWindow()
{
	KillTimer(hWin, TIMERID);
	DestroyGraphicEnv(hWin, hdc, hglrc);
	CloseWindow(hWin);

	return TRUE;
}

/****************************************************************************/

int
WinOpen(HINSTANCE this_inst, HWND hPar, int xleft, int xright, int width, int height)
{
	MSG msg;
  
	if (InitWindow(this_inst, hPar, xleft, xright, width, height) == FALSE)
		return FALSE;

	// Loop until we are done - either the user quits or an error occurs
	while (!done)
	{
		// Check the thread message queue for a message and place it in 
		// message if there is one.
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = TRUE;
				break;
			}
			// Translate the virtual key messages into character messages
			TranslateMessage(&msg);
      
			// Dispatch the message to the window proceedure
			DispatchMessage(&msg);
		}
	}

	ReInitWindow();
	return msg.wParam;
}

/****************************************************************************/
//
// This function is the main routine containing the message loop. 
// Here we call the functions to create, initialize, and activate
// the program's main window and handle the various messages.
//

int PASCAL
WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	_prev_inst = prev_inst;
	_cmdline = cmdline;
	_cmdshow = cmdshow;
  
	return WinOpen(this_inst, NULL, 0, 0, 512, 512);
}

/****************************************************************************/

BOOL
TimerProc(HWND hwnd)
{ 
	if (!bQuit)
	{
		// Try to render the screen
		g_dwCurTime = timeGetTime();

		// Perform some timing stuff      
		g_dwDeltaTime = g_dwCurTime - g_dwLastTime;
		g_dwLastTime  = g_dwCurTime;
		g_dwFpsTime  += g_dwDeltaTime;
		g_dwFramesRendered++;

		// Update every second
		if (g_dwFpsTime > 1000)
		{
			char str[256]; 
			
			g_dwFps            = g_dwFramesRendered;
			g_dwFramesRendered = 0;
  
			sprintf ( (LPSTR)str, "OpenGLDemo - FPS: %.3f", 
						1000.0*(float)g_dwFps/(float)g_dwFpsTime );
			SetWindowText(hWin, (LPSTR)str);

			g_dwFpsTime         = 0;
		}
		UpdateTime();
		InvalidateRect(hwnd, NULL, FALSE);
	}

	return TRUE;
}

/****************************************************************************/
//
// Process the messages for the 'about' dialog.
//
BOOL FAR PASCAL
AboutDlgProc(HWND win, unsigned msg, WORD wparam, LONG lparam)
{
	// Check the message
	switch (msg)
	{
		// If we received an InitDialog message, just return true
		case WM_INITDIALOG:
			return TRUE;

		// If we received an OK button selection,
		// close the dialog and return true
		case WM_COMMAND:
			if (wparam == IDOK)
			{
				EndDialog(win, TRUE);
				return TRUE;
			}
			break;
	}
	// Otherwise, return false
	return FALSE;
}
 
/****************************************************************************/

void
OnCreate(HWND win)
{
	CheckMenuItem(
		GetMenu(win), ID_VIEW_ANTIALIAS,
		state.bSmooth ? MF_CHECKED : MF_UNCHECKED
	);
	CheckMenuItem(
		GetMenu(win), ID_VIEW_BLEND,
		state.bBlend ? MF_CHECKED : MF_UNCHECKED
	);
	CheckMenuItem(
		GetMenu(win), ID_VIEW_CULL,
		state.bCulling ? MF_CHECKED : MF_UNCHECKED
	);
	CheckMenuRadioItem(
		GetMenu(win), ID_VIEW_METHOD_NICEST, ID_VIEW_METHOD_FASTEST,
		state.nLineSmoothHint == GL_NICEST ? ID_VIEW_METHOD_NICEST :
		ID_VIEW_METHOD_FASTEST, MF_BYCOMMAND
	);
	CheckMenuRadioItem(
		GetMenu(win), 
		ID_VIEW_SHADE_FLAT, ID_VIEW_SHADE_GOURARD,
		state.nShadeModel == GL_SMOOTH ? ID_VIEW_SHADE_GOURARD :
		ID_VIEW_SHADE_FLAT, MF_BYCOMMAND
	);  
	CheckMenuItem(
		GetMenu(win), ID_VIEW_LINEAR,
		state.nLinear == 1 ? MF_CHECKED : MF_UNCHECKED
	);

	CheckMenuItem(GetMenu(win), ID_VIEW_MIDMAPS, MF_CHECKED);  

	CheckMenuRadioItem(
		GetMenu(win), 
		ID_VIEW_PERSP_NICEST, ID_VIEW_PERSP_FASTEST,
		state.nPerspCorrectionHint == GL_NICEST ? ID_VIEW_PERSP_NICEST :
		ID_VIEW_PERSP_FASTEST, MF_BYCOMMAND
	);  
	CheckMenuItem(
		GetMenu(win), ID_VIEW_NORMALS, 
		state.bNormals ? MF_CHECKED : MF_UNCHECKED
	);  
	CheckMenuItem(
		GetMenu(win), ID_VIEW_QUICK, 
		state.bQuick ? MF_CHECKED : MF_UNCHECKED
	);
	CheckMenuItem(
		GetMenu(win), ID_VIEW_SWING,
		state.bSwing ? MF_CHECKED : MF_UNCHECKED
	);  
	CheckMenuItem(
		GetMenu(win), ID_VIEW_ZBUFFER,
		state.bDepthTest ? MF_CHECKED : MF_UNCHECKED
	);
	CheckMenuItem(
		GetMenu(win), ID_VIEW_WIREFRAME,
		state.nPolygonMode == GL_LINE ? MF_CHECKED : MF_UNCHECKED
	);
}

/****************************************************************************/

void
OnCommand(HWND win, WPARAM wparam, LPARAM lparam)
{
	switch( wparam & 0xffff )
	{
		case ID_FILE_EXIT:
			PostQuitMessage(0);
			bQuit = TRUE;
			break;

		case ID_VIEW_ANTIALIAS:
			state.bSmooth = !state.bSmooth;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_ANTIALIAS,
				state.bSmooth ? MF_CHECKED : MF_UNCHECKED
			);
			EnableGraphicOption(GL_LINE_SMOOTH, state.bSmooth);
			EnableGraphicOption(GL_POLYGON_SMOOTH, state.bSmooth);
			break;

		case ID_VIEW_BLEND:
			state.da = -state.da;
			state.bBlend = !state.bBlend;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_BLEND,
				state.bBlend ? MF_CHECKED : MF_UNCHECKED
			);
			break;

		case ID_VIEW_CULL:
			state.bCulling = !state.bCulling;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_CULL,
				state.bCulling ? MF_CHECKED : MF_UNCHECKED
			);
			EnableGraphicOption(GL_CULL_FACE, state.bCulling);
			break;

		case ID_VIEW_METHOD_NICEST:
			if (state.nLineSmoothHint != GL_NICEST)
			{
				state.nLineSmoothHint = GL_NICEST;
				CheckMenuRadioItem(
					GetMenu(win), ID_VIEW_METHOD_NICEST, ID_VIEW_METHOD_FASTEST,
					ID_VIEW_METHOD_NICEST, MF_BYCOMMAND
				);
				glHint(GL_LINE_SMOOTH_HINT, state.nLineSmoothHint);
			}
			break;

		case ID_VIEW_METHOD_FASTEST:
			if (state.nLineSmoothHint != GL_FASTEST)
			{
				state.nLineSmoothHint = GL_FASTEST;
				CheckMenuRadioItem(
					GetMenu(win), ID_VIEW_METHOD_NICEST, ID_VIEW_METHOD_FASTEST,
					ID_VIEW_METHOD_FASTEST, MF_BYCOMMAND
				);
				glHint(GL_LINE_SMOOTH_HINT, state.nLineSmoothHint);
			}
			break;

		case ID_VIEW_SHADE_FLAT:
			if (state.nShadeModel != GL_FLAT)
			{
				state.nShadeModel = GL_FLAT;
				CheckMenuRadioItem(
					GetMenu(win), 
					ID_VIEW_SHADE_FLAT, ID_VIEW_SHADE_GOURARD,
					ID_VIEW_SHADE_FLAT, MF_BYCOMMAND
				);
				glShadeModel(state.nShadeModel);
			}
			break;

		case ID_VIEW_SHADE_GOURARD:
			if (state.nShadeModel != GL_SMOOTH)
			{
				state.nShadeModel = GL_SMOOTH;
				CheckMenuRadioItem(
					GetMenu(win), 
					ID_VIEW_SHADE_FLAT, ID_VIEW_SHADE_GOURARD,
					ID_VIEW_SHADE_GOURARD, MF_BYCOMMAND
				);
				glShadeModel(state.nShadeModel);
			}
			break;

		case ID_VIEW_LINEAR:
			state.nLinear = (state.nLinear == 1 ? 0 : 1);
			CheckMenuItem(
				GetMenu(win), ID_VIEW_LINEAR,
				state.nLinear == 1 ? MF_CHECKED : MF_UNCHECKED
			);
			break;

		case ID_VIEW_PERSP_NICEST:
			if (state.nPerspCorrectionHint != GL_NICEST)
			{
				state.nPerspCorrectionHint = GL_NICEST;
				CheckMenuRadioItem(
					GetMenu(win), 
					ID_VIEW_PERSP_NICEST, ID_VIEW_PERSP_FASTEST,
					ID_VIEW_PERSP_NICEST, MF_BYCOMMAND
				);
				glHint(GL_PERSPECTIVE_CORRECTION_HINT, state.nPerspCorrectionHint);
			}
			break;

		case ID_VIEW_PERSP_FASTEST:
			if (state.nPerspCorrectionHint != GL_FASTEST)
			{
				state.nPerspCorrectionHint = GL_FASTEST;
				CheckMenuRadioItem(
					GetMenu(win), 
					ID_VIEW_PERSP_NICEST, ID_VIEW_PERSP_FASTEST,
					ID_VIEW_PERSP_FASTEST, MF_BYCOMMAND
				);
				glHint(GL_PERSPECTIVE_CORRECTION_HINT, state.nPerspCorrectionHint);
			}
			break;

		case ID_VIEW_NORMALS:
			state.bNormals = !state.bNormals;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_NORMALS, 
				state.bNormals ? MF_CHECKED : MF_UNCHECKED
			);
			break;

		case ID_VIEW_QUICK:
			state.bQuick = !state.bQuick;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_QUICK, 
				state.bQuick ? MF_CHECKED : MF_UNCHECKED
			);
			break;

		case ID_VIEW_SWING:
			state.bSwing = !state.bSwing;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_SWING,
				state.bSwing ? MF_CHECKED : MF_UNCHECKED
			);
			break;

		case ID_VIEW_ZBUFFER:
			state.bDepthTest = !state.bDepthTest;
			CheckMenuItem(
				GetMenu(win), ID_VIEW_ZBUFFER,
				state.bDepthTest ? MF_CHECKED : MF_UNCHECKED
			);
			EnableGraphicOption(GL_DEPTH_TEST, state.bDepthTest);
			break;

		case ID_VIEW_WIREFRAME:
			state.nPolygonMode = (state.nPolygonMode == GL_LINE ? GL_FILL : GL_LINE);
			CheckMenuItem(
				GetMenu(win), ID_VIEW_WIREFRAME,
				state.nPolygonMode == GL_LINE ? MF_CHECKED : MF_UNCHECKED
			);
			glPolygonMode(GL_FRONT_AND_BACK, state.nPolygonMode);
			break;

		case ID_HELP_ABOUT:
				DialogBox(
					(HINSTANCE)GetWindowLong(win, GWL_HINSTANCE),
					MAKEINTRESOURCE(IDD_ABOUT), win,
					(DLGPROC)AboutDlgProc
				);
				break;
	}
	InvalidateRect(win, NULL, FALSE);
}

/****************************************************************************/
//
// Process the messages for the main application window
//
LONG FAR PASCAL
WindowProc(HWND win, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static BOOL bRoll = FALSE;
	static POINT anchor;

	// Handle the various message that we received
	switch (msg)
	{
		case WM_CREATE:
			OnCreate(win);
			break;

		case WM_KEYDOWN:
		{   
			// Check for any keyboard key depression
			switch (wparam)
			{
				case VK_SPACE:
					ResetOrientationGraphicEnv();
					CheckMenuItem(
						GetMenu(win), ID_VIEW_SWING,
						state.bSwing ? MF_CHECKED : MF_UNCHECKED
					);  
					break;

				case 'T':
					break;
				case VK_UP:
					break;
				case VK_DOWN:
					break;
				case VK_RIGHT:
					break;
				case VK_LEFT:
					break;
				case VK_HOME:
					break;
				case VK_END:
					break;
				case VK_INSERT:
					break;
				case VK_DELETE:
					break;
				case VK_PRIOR:  // PageUp
					break;
				case VK_NEXT:  // PageDown
					break;
				case VK_NUMPAD4:
					break;
			}
		}
		break;

	// If the user releases a key, see which one it was and do the appropriate action
	case WM_KEYUP:
		switch (wparam)
		{
			case 'T':
				break;
		}
		break;

	case WM_COMMAND:
		OnCommand(win, wparam, lparam);
		break;

	case WM_LBUTTONDOWN:
		RECT rc;
		bMovement = TRUE;
		GetClientRect(win, &rc);
		GetCursorPos(&anchor);
		ScreenToClient(hWin, &anchor);
		bRoll = (GetKeyState(VK_SHIFT) & 0x8000) >> 15;
		SetCapture(win);
		break;

	case WM_LBUTTONUP:
		bMovement = FALSE;
		bRoll = FALSE;
		ReleaseCapture();
		break;

	case WM_RBUTTONDOWN:
		break;               
    case WM_RBUTTONUP:
		break;

    case WM_MOUSEMOVE:
		if (bMovement & wparam & MK_LBUTTON)
		{
			POINT mouse;
			mouse.x = SIGN_EXTEND(LOWORD(lparam));
			mouse.y = SIGN_EXTEND(HIWORD(lparam));
			MotionOfGraphicEnv(&anchor, mouse, bRoll);
		}
	    break;
                
	case WM_TIMER:
		if (wparam == TIMERID)
			TimerProc(win);
		break;

	// We are quitting and destroying the window
	case WM_DESTROY:
		PostQuitMessage(0);
		bQuit = TRUE;
		break;

	case WM_SIZE:
	{
		int width = LOWORD(lparam);
		int height = HIWORD(lparam);
		if (width && height)
		{
			Reshape(hdc, width, height);
		}
	}
	break;

	case WM_ACTIVATE:
		break;

	// Handle the window paint message and update the window
	case WM_PAINT:
	{   
		RECT r;
		PAINTSTRUCT ps;

		// Retrieve the coordinates of the smallest rectangle that completely encloses the 
		// update region of the specified window (win)
		if (GetUpdateRect(win, &r, FALSE))
		{   
			BeginPaint(win, &ps);
				Display(hdc);
			EndPaint(win, &ps);
		}	  	
	}

	default:
		// Default to just handling any messages not explicitly handled above
		return DefWindowProc(win, msg, wparam, lparam);
	}
	return 0L;
}

