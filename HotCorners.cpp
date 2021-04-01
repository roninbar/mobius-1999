//==========================================================================
//
//  File: HotCorners.cpp
//
//==========================================================================

#include <windows.h>

//******************************************************************************************
// CheckHots: this routine checks for Hot Corner services.
// It first tries with SAGE.DLL, which comes with Windows Plus!
// Failint this it tries with ScrHots, a third-party hot-corner
// service program written by the author that is freely
// distributable and works with NT and '95.
// It's not actually used in the saver, but code is included here
// just to be useful.
BOOL
CheckHots()
{
	typedef BOOL (WINAPI *SYSTEMAGENTDETECT)();
	HINSTANCE sagedll = LoadLibrary("Sage.dll");
	if (sagedll != NULL)
	{
		SYSTEMAGENTDETECT detectproc =
			(SYSTEMAGENTDETECT)GetProcAddress(sagedll, "System_Agent_Detect");
		BOOL res = FALSE;
		if (detectproc != NULL) res = detectproc();
		FreeLibrary(sagedll);
		if (res) return TRUE;
	}
	HINSTANCE hotsdll = LoadLibrary("ScrHots.dll");
	if (hotsdll != NULL)
	{
		SYSTEMAGENTDETECT detectproc =
			(SYSTEMAGENTDETECT)GetProcAddress(hotsdll, "System_Agent_Detect");
		BOOL res = FALSE;
		if (detectproc != NULL) res = detectproc();
		FreeLibrary(hotsdll);
		if (res) return TRUE;
	}
	return FALSE;
}

//******************************************************************************************
// NotifyHots: if you make any changes to the hot corner
// information in the registry, you must call NotifyHots
// to inform the hot corner services of your change.
// It's not actually used in the saver, but code is included here
// just to be useful.
void __fastcall
NotifyHots()
{
	typedef VOID (WINAPI *SCREENSAVERCHANGED)();
	HINSTANCE sagedll = LoadLibrary("Sage.DLL");
	if (sagedll != NULL)
	{
		SCREENSAVERCHANGED changedproc =
			(SCREENSAVERCHANGED)GetProcAddress(sagedll, "Screen_Saver_Changed");
		if (changedproc != NULL) changedproc();
		FreeLibrary(sagedll);
	}
	HINSTANCE hotsdll = LoadLibrary("ScrHots.dll");
	if (hotsdll != NULL)
	{
		SCREENSAVERCHANGED changedproc =
			(SCREENSAVERCHANGED)GetProcAddress(hotsdll, "Screen_Saver_Changed");
		if (changedproc != NULL) changedproc();
		FreeLibrary(hotsdll);
	}
}

