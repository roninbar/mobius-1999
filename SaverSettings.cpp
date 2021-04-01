//==========================================================================
//
//  File: SaverSettings.cpp
//
//==========================================================================

#include <windows.h>
#include <regstr.h>
#include <stdio.h>
#include "SaverSettings.h"

#define REGSTR_PATH_PLUSSCR (REGSTR_PATH_SETUP "\\Screen Savers")
#define REGSTR_PATH_CONFIG  ("Software\\Moebius Clock\\Moebius Clock Saver")

//******************************************************************************************

SaverSettings::SaverSettings() : 
					state(), hwnd(NULL), ReallyClose(FALSE), idTimer(0),
					hdc(NULL), hglrc(NULL)
{ 
	int i;

	for (i=0; i < 16; i++)
		customColors[i] = RGB(255, 255, 255);
}

//******************************************************************************************

void
SaverSettings::ReadGeneralRegistry()
{
	PasswordDelay = 15;			// default values in case they're not in registry
	PasswordDelayIndex = 0;
	MouseThreshold = 0;
	MouseThresholdIndex = 1;
	IsDialogActive = FALSE;
	Corners[0] = '-'; Corners[1] = '-'; Corners[2] = '-'; Corners[3] = '-';
	Corners[4] = '\0';

	LONG res;
	HKEY skey;
	DWORD valtype, valsize, val;
	char achVal[5];

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_PLUSSCR, 0, KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS) return;
	valsize = sizeof(val);
	res = RegQueryValueEx(skey, "Password Delay", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) PasswordDelay = val;
	valsize = sizeof(val);
	res = RegQueryValueEx(skey, "Password Delay Index", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) PasswordDelayIndex = val;
	valsize = sizeof(val);
	res = RegQueryValueEx(skey, "Mouse Threshold", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) MouseThreshold = val;
	valsize = sizeof(val);
	res = RegQueryValueEx(skey, "Mouse Threshold Index", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) MouseThresholdIndex = val;
	valsize = sizeof(val);
	res = RegQueryValueEx(skey, "Mute Sound", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) MuteSound = val;
	valsize = sizeof(char)*5;
	res = RegQueryValueEx(skey, "Mouse Corners", 0, &valtype, (LPBYTE)achVal, &valsize);
	if (res == ERROR_SUCCESS)
	{
		Corners[0] = achVal[0]; Corners[1] = achVal[1]; Corners[2] = achVal[2];
		Corners[3] = achVal[3]; Corners[4] = achVal[4];
	}
	RegCloseKey(skey);
}

//******************************************************************************************

void
SaverSettings::ReadGraphicConfigRegistry()
{
	LONG res;
	HKEY skey;
	DWORD valtype, valsize, val;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0, KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS) return;
	valsize = sizeof(val);
	
	res = RegQueryValueEx(skey, "Alpha Blending", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bBlend = val;
	res = RegQueryValueEx(skey, "Antialiasing", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bSmooth = val;
	res = RegQueryValueEx(skey, "Backface Culling", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bCulling = val;
	res = RegQueryValueEx(skey, "Use Z-Buffer", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bDepthTest = val;
	res = RegQueryValueEx(skey, "Show Normals", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bNormals = val;
	res = RegQueryValueEx(skey, "No Glass", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bNoGlass = val;
	res = RegQueryValueEx(skey, "No Extra Textures", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bNoExtraTextures = val;
	res = RegQueryValueEx(skey, "Linear", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.nLinear = (short)val;
	res = RegQueryValueEx(skey, "Mipmaps", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.nMipmap = (short)val;
	res = RegQueryValueEx(skey, "Show Wireframe", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS)
		state.nPolygonMode = (val ? GL_LINE : GL_FILL);
	res = RegQueryValueEx(skey, "Gouraud Shading", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS)
		state.nShadeModel = (val ? GL_SMOOTH : GL_FLAT);
	if (!state.bBlend)
		state.da = -state.da;

	RegCloseKey(skey);
}  

//******************************************************************************************

void
SaverSettings::ReadBehaviorRegistry()
{
	int i;
	char str[20];
	LONG res;
	HKEY skey;
	DWORD valtype, valsize, val;
	float fval;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0, KEY_ALL_ACCESS, &skey);
	if (res != ERROR_SUCCESS) return;
	valsize = sizeof(fval);
	
	for (i=0; i < 4; i++)
	{
		sprintf(str, "Color %d Red", i+1);
		res = RegQueryValueEx(skey, str, 0, &valtype, (LPBYTE)&fval, &valsize);
		if (res == ERROR_SUCCESS) state.fColorR[i] = fval;
		sprintf(str, "Color %d Green", i+1);
		res = RegQueryValueEx(skey, str, 0, &valtype, (LPBYTE)&fval, &valsize);
		if (res == ERROR_SUCCESS) state.fColorG[i] = fval;
		sprintf(str, "Color %d Blue", i+1);
		res = RegQueryValueEx(skey, str, 0, &valtype, (LPBYTE)&fval, &valsize);
		if (res == ERROR_SUCCESS) state.fColorB[i] = fval;
	}
	res = RegQueryValueEx(skey, "Speed Up Clock", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bQuick = val;
	res = RegQueryValueEx(skey, "Swing Clock", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bSwing = val;
	res = RegQueryValueEx(skey, "Use Backdrop", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS)
		val == TRUE ? state.bBlackBackground = FALSE : state.bBlackBackground = TRUE;
	res = RegQueryValueEx(skey, "Use Mouse", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.bUseMouse = val;
	res = RegQueryValueEx(skey, "Moebius Strip Twists", 0, &valtype, (LPBYTE)&val, &valsize);
	if (res == ERROR_SUCCESS) state.nTwists = (short)val;

	RegCloseKey(skey);
}

//******************************************************************************************

void
SaverSettings::WriteGeneralRegistry()
{
	LONG res;
	HKEY skey;
	DWORD val, disp;

	res = RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_PLUSSCR, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &skey, &disp);
	if (res != ERROR_SUCCESS) return;

	val = PasswordDelay;
	RegSetValueEx(skey, "Password Delay", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = PasswordDelayIndex;
	RegSetValueEx(skey, "Password Delay Index", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = MouseThreshold;
	RegSetValueEx(skey, "Mouse Threshold", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = MouseThresholdIndex;
	RegSetValueEx(skey, "Mouse Threshold Index", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = MuteSound;
	RegSetValueEx(skey, "Mute Sound", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	RegSetValueEx(skey, "Mouse Corners", 0, REG_SZ, (CONST BYTE*)Corners, sizeof(char)*5);

	RegCloseKey(skey);
}

//******************************************************************************************

void
SaverSettings::WriteGraphicConfigRegistry()
{
	LONG res;
	HKEY skey;
	DWORD val, disp;

	res = RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &skey, &disp);
	if (res != ERROR_SUCCESS) return;

	val = state.bBlend;
	RegSetValueEx(skey, "Alpha Blending", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bSmooth;
	RegSetValueEx(skey, "Antialiasing", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bCulling;
	RegSetValueEx(skey, "Backface Culling", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bDepthTest;
	RegSetValueEx(skey, "Use Z-Buffer", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bNormals;
	RegSetValueEx(skey, "Show Normals", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bNoGlass;
	RegSetValueEx(skey, "No Glass", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bNoExtraTextures;
	RegSetValueEx(skey, "No Extra Textures", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.nLinear;
	RegSetValueEx(skey, "Linear", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.nMipmap;
	RegSetValueEx(skey, "Mipmaps", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = (state.nPolygonMode == GL_LINE ? TRUE : FALSE);
	RegSetValueEx(skey, "Show Wireframe", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = (state.nShadeModel == GL_SMOOTH ? TRUE : FALSE);
	RegSetValueEx(skey, "Gouraud Shading", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

	RegCloseKey(skey);
}

//******************************************************************************************

void
SaverSettings::WriteBehaviorRegistry()
{
	int i;
	char str[20];
	LONG res;
	HKEY skey;
	DWORD disp, val;
	float fval;

	res = RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CONFIG, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &skey, &disp);
	if (res != ERROR_SUCCESS) return;

	for (i=0; i < 4; i++)
	{
		fval = state.fColorR[i];
		sprintf(str, "Color %d Red", i+1);
		RegSetValueEx(skey, str, 0, REG_DWORD, (CONST BYTE*)&fval, sizeof(fval));
		fval = state.fColorG[i];
		sprintf(str, "Color %d Green", i+1);
		RegSetValueEx(skey, str, 0, REG_DWORD, (CONST BYTE*)&fval, sizeof(fval));
		fval = state.fColorB[i];
		sprintf(str, "Color %d Blue", i+1);
		RegSetValueEx(skey, str, 0, REG_DWORD, (CONST BYTE*)&fval, sizeof(fval));
	}
	val = state.bQuick;
	RegSetValueEx(skey, "Speed Up Clock", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bSwing;
	RegSetValueEx(skey, "Swing Clock", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.bUseMouse;
	RegSetValueEx(skey, "Use Mouse", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	state.bBlackBackground == TRUE ? val = FALSE : val = TRUE;
	RegSetValueEx(skey, "Use Backdrop", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));
	val = state.nTwists;
	RegSetValueEx(skey, "Moebius Strip Twists", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

	RegCloseKey(skey);
}

//******************************************************************************************

void
SaverSettings::CloseSaverWindow()
{
	ReallyClose = TRUE;
	PostMessage(hwnd, WM_CLOSE, 0, 0);
}

//******************************************************************************************

void
SaverSettings::StartDialog()
{
	IsDialogActive = TRUE;
	SendMessage(hwnd, WM_SETCURSOR, 0, 0);
}

//******************************************************************************************

void
SaverSettings::EndDialog()
{
	IsDialogActive = FALSE;
	SendMessage(hwnd, WM_SETCURSOR, 0, 0);
	GetCursorPos(&InitCursorPos);
}
  
