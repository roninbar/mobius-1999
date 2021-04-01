//==========================================================================
//
//  File: Password.cpp
//
//==========================================================================

#include "Password.h"
#include "Debug.h"

//******************************************************************************************

BOOL
VerifyPassword(HWND hwnd)
{
	// Under NT, we return TRUE immediately. This lets the saver quit, 
	// and the system manages passwords.
	// Under '95, we call VerifyScreenSavePwd.
	// This checks the appropriate registry key and, if necessary, 
	// pops up a verify dialog
	OSVERSIONINFO osv;

	osv.dwOSVersionInfoSize = sizeof(osv);
	GetVersionEx(&osv);
	if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return TRUE;

	HINSTANCE hpwdcpl = ::LoadLibrary("PASSWORD.CPL");
	if (hpwdcpl == NULL)
	{
		Debug("Unable to load PASSWORD.CPL. Aborting");
		return TRUE;
	}
	typedef BOOL (WINAPI *VERIFYSCREENSAVEPWD)(HWND hwnd);
	VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
	VerifyScreenSavePwd =
		(VERIFYSCREENSAVEPWD)GetProcAddress(hpwdcpl, "VerifyScreenSavePwd");
	if (VerifyScreenSavePwd == NULL)
	{
		Debug("Unable to get VerifyPwProc address. Aborting");
		FreeLibrary(hpwdcpl);
		return TRUE;
	}
	Debug("About to call VerifyPwProc");
	BOOL bres = VerifyScreenSavePwd(hwnd);
	FreeLibrary(hpwdcpl);
	return bres;
}

//******************************************************************************************

void
ChangePassword(HWND hwnd)
{
	// This only ever gets called under '95, when started with the /a option.
	HINSTANCE hmpr = ::LoadLibrary("MPR.DLL");
	if (hmpr == NULL)
	{
		Debug("MPR.DLL not found: cannot change password.");
		return;
	}
	typedef VOID (WINAPI *PWDCHANGEPASSWORD) (LPCSTR lpcRegkeyname, HWND hwnd, UINT uiReserved1, UINT uiReserved2);
	PWDCHANGEPASSWORD PwdChangePassword =
		(PWDCHANGEPASSWORD)::GetProcAddress(hmpr, "PwdChangePasswordA");
	if (PwdChangePassword == NULL)
	{
		FreeLibrary(hmpr);
		Debug("PwdChangeProc not found: cannot change password");
		return;
	}
	PwdChangePassword("SCRSAVE", hwnd, 0, 0);
	FreeLibrary(hmpr);
}
