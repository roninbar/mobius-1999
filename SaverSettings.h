//==========================================================================
//
//  File: SaverSettings.h
//
//==========================================================================

#ifndef SAVERSETTINGS_H
#define SAVERSETTINGS_H

#include "OpenGLenv.h"

//******************************************************************************************

class SaverSettings
{
public:
	HWND	hwnd;
	HDC		hdc;
	HGLRC	hglrc;
	UINT	idTimer;         // a timer id, because this particular saver uses a timer
	DWORD	customColors[16];

	DWORD	PasswordDelay;   // in seconds
	DWORD	PasswordDelayIndex;
	DWORD	MouseThreshold;  // in pixels
	DWORD	MouseThresholdIndex;
	BOOL	MuteSound;
	char	Corners[5];
	POINT	InitCursorPos;
	DWORD	InitTime;        // in ms
	BOOL	IsDialogActive;
	BOOL	ReallyClose;     // for NT, so we know if a WM_CLOSE came from us or it.

	GraphicEnvState state;
	
	SaverSettings();

	void ReadGeneralRegistry();
	void WriteGeneralRegistry();
	void ReadGraphicConfigRegistry();
	void WriteGraphicConfigRegistry();
	void ReadBehaviorRegistry();
	void WriteBehaviorRegistry();
	void CloseSaverWindow();
	void StartDialog();
	void EndDialog();
};

//******************************************************************************************

#endif
