//==========================================================================
//
//  File: GeneralDialog.cpp
//
//==========================================================================

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "SaverSettings.h"
#include "TransparentBitmap.h"
#include "HotCorners.h"
#include "resource.h"
#include "resource.hm"

extern SaverSettings* ss;
extern HINSTANCE hInstance;

#define BMPMONITOR_WIDTH	184
#define BMPMONITOR_HEIGHT	169

#define BMPBUTTON_WIDTH		25
#define BMPBUTTON_HEIGHT	23

static bool firstTime = true;

static const DWORD aHelpIDs[] =
{
	ID_MUTE,		HID_MUTE,
	ID_THRESHOLD,	HID_THRESHOLD,
	ID_WAITBOX,		HID_WAITBOX,
	ID_WAITBUDDY,	HID_WAITTEXT,
	ID_WAITTEXT,	HID_WAITTEXT,
	IDC_UL,			HIDC_UL,
	IDC_UR,			HIDC_UL,
	IDC_DL,			HIDC_UL,
	IDC_DR,			HIDC_UL,
	IDC_MONITOR,	HIDC_UL,
	0, 0
};

/****************************************************************************/
//
// Process the messages for the 'General' dialog.
//
BOOL APIENTRY
GeneralDialogProc(HWND win, UINT msg, UINT wparam, LONG lparam)
{
	HWND hstaticOK, hstaticBad;
	HWND hmute;
	HWND hcomboTh;
	HWND hstaticPasswd, hbuddyPasswd, htextPasswd, hcomboPasswd;
	HWND hstb;
	HWND hmonitor;
	static HBITMAP hbcheck=0, hbdot=0, hbnone=0;
	HBITMAP hbmpmonitor;
	HGDIOBJ holdbmp;
	HANDLE hbuttonim;
	DWORD nText;
	char achNum[10];
	RECT rectBmp, rectDlg;
	int nBtnCoordx[] = { 17, 143, 143, 17 };
	int nBtnCoordy[] = { 18, 18, 106, 106 };
	int nComboSel, i;
	DWORD nSpinSel;

	// Check the message
	switch (msg)
	{
		// If we received an InitDialog message, just return true
		case WM_INITDIALOG:
			hmonitor = GetDlgItem(win, IDC_MONITOR);
			GetWindowRect(win, &rectDlg);
			GetWindowRect(hmonitor, &rectBmp);
			rectBmp.top -= rectDlg.top;
			rectBmp.left -= rectDlg.left;
			MoveWindow(hmonitor, 
				rectBmp.left, rectBmp.top, 
				BMPMONITOR_WIDTH, BMPMONITOR_HEIGHT, TRUE
			);
			return TRUE;

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis;
			lpdis = (LPDRAWITEMSTRUCT)lparam;
			if (lpdis->CtlType == ODT_BUTTON)
			{
				hbmpmonitor = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MONITOR));
				holdbmp = SelectObject(lpdis->hDC, hbmpmonitor);
				DrawTransparentBitmap(lpdis->hDC, hbmpmonitor, 0, 0, RGB(255, 0, 0));
				SelectObject(lpdis->hDC, holdbmp);
				DeleteObject(hbmpmonitor);
				for (i=0; i < 4; i++)
				{
					hstb = GetDlgItem(win, IDC_UL+i);
					InvalidateRect(hstb, NULL, FALSE);
				}
				return TRUE;
			}
		}
		break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lparam)->code) 
			{
				case PSN_SETACTIVE:
					hstaticOK  = GetDlgItem(win, ID_SAGEOK);
					hstaticBad = GetDlgItem(win, ID_SAGEBAD);
					hmute = GetDlgItem(win, ID_MUTE);
					hcomboTh = GetDlgItem(win, ID_THRESHOLD);
					hstaticPasswd = GetDlgItem(win, ID_WAITSUMMARY);
					hbuddyPasswd  = GetDlgItem(win, ID_WAITBUDDY);
					htextPasswd   = GetDlgItem(win, ID_WAITTEXT);
					hcomboPasswd  = GetDlgItem(win, ID_WAITBOX);
					ShowWindow(hmute, SW_HIDE);
					ShowWindow(hstaticPasswd, SW_HIDE);
					ShowWindow(hbuddyPasswd, SW_SHOW);
					ShowWindow(htextPasswd, SW_SHOW);
					ShowWindow(hcomboPasswd, SW_SHOW);
					if (CheckHots())
					{
						ShowWindow(hstaticOK, SW_SHOW);
						ShowWindow(hstaticBad, SW_HIDE);
					}
					else
					{
						ShowWindow(hstaticOK, SW_HIDE);
						ShowWindow(hstaticBad, SW_SHOW);
					}
					if (firstTime)
					{
						SendMessage(hcomboTh, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"High");
						SendMessage(hcomboTh, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Normal");
						SendMessage(hcomboTh, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Low");
						SendMessage(hcomboTh, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"Ignore mouse movement");
						SendMessage(hcomboPasswd, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"seconds");
						SendMessage(hcomboPasswd, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)"minutes");
						firstTime = false;
					}
					SendMessage(hcomboTh, CB_SETCURSEL, (WPARAM)ss->MouseThresholdIndex, 0L);
					SendMessage(hcomboPasswd, CB_SETCURSEL, (WPARAM)ss->PasswordDelayIndex, 0L);
					if (ss->PasswordDelayIndex) nText = ss->PasswordDelay / 60;
					else nText = ss->PasswordDelay;
					sprintf(achNum, "%d", nText);
					SendMessage(htextPasswd, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)achNum);
					SendMessage(hbuddyPasswd, UDM_SETRANGE, 0L, MAKELONG(99, 0));
					SendMessage(hbuddyPasswd, UDM_SETPOS, 0L, MAKELONG(nText, 0));
					CheckDlgButton(win, ID_MUTE, ss->MuteSound);
					hbcheck = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BCHECK));
					hbdot   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BDOT));
					hbnone  = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BNONE));
					hmonitor = GetDlgItem(win, IDC_MONITOR);
					GetWindowRect(win, &rectDlg);
					GetWindowRect(hmonitor, &rectBmp);
					rectBmp.top -= rectDlg.top;
					rectBmp.left -= rectDlg.left;
					for (i=0; i < 4; i++)
					{
						hstb = GetDlgItem(win, IDC_UL+i);
						switch (ss->Corners[i])
						{
							case '-': hbuttonim = hbnone;  break;
							case 'Y': hbuttonim = hbcheck; break;
							case 'N': hbuttonim = hbdot;   break;
						}
						SendMessage(hstb, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hbuttonim);
						MoveWindow(hstb, 
							nBtnCoordx[i] + rectBmp.left, 
							nBtnCoordy[i] + rectBmp.top, 
							BMPBUTTON_WIDTH, BMPBUTTON_HEIGHT, TRUE
						);
					}
					break;

				case PSN_RESET:
					SetWindowLong(win, DWL_MSGRESULT, FALSE);
					break;

				case PSN_KILLACTIVE:
					if (hbcheck) DeleteObject(hbcheck);
					if (hbdot)   DeleteObject(hbdot);
					if (hbnone)  DeleteObject(hbnone);
					hmute = GetDlgItem(win, ID_MUTE);
					hcomboTh = GetDlgItem(win, ID_THRESHOLD);
					hbuddyPasswd  = GetDlgItem(win, ID_WAITBUDDY);
					hcomboPasswd  = GetDlgItem(win, ID_WAITBOX);
					ss->MuteSound = IsDlgButtonChecked(win, ID_MUTE);
					nSpinSel = SendMessage(hbuddyPasswd, UDM_GETPOS, 0, 0L);
					nComboSel = SendMessage(hcomboPasswd, CB_GETCURSEL, 0, 0L);
					ss->PasswordDelayIndex = nComboSel;
					ss->PasswordDelay = LOWORD(nSpinSel);
					if (nComboSel) ss->PasswordDelay *= 60;
					nComboSel = SendMessage(hcomboTh, CB_GETCURSEL, 0, 0L);
					ss->MouseThresholdIndex = nComboSel;
					switch (nComboSel)
					{
						case 0:
							ss->MouseThreshold = 0;
							break;
						case 1:
							ss->MouseThreshold = 200;
							break;
						case 2:
							ss->MouseThreshold = 400;
							break;
						case 3:
							ss->MouseThreshold = 999999;
							break;
					}
					SetWindowLong(win, DWL_MSGRESULT, FALSE);
					return TRUE;
					break;

                case PSN_APPLY:
					ss->WriteGeneralRegistry();
					NotifyHots();
					SetWindowLong(win, DWL_MSGRESULT, TRUE);
					break;
			}

		case WM_COMMAND:
		{
			HMENU hpopup;
			POINT point;
			int nButton;		// 0=UL 1=UR 2=DR 3=DL
			int id = LOWORD(wparam);
			UINT nMenu;
			if ( (id >= IDC_UL) && (id <= IDC_DL) ) 
			{
				nButton = id - IDC_UL;
				hpopup = CreatePopupMenu();
				AppendMenu(hpopup, 
					MF_STRING | (ss->Corners[nButton] == '-' ? MF_CHECKED : 0),
					ID_DEFAULT, "Default");
				AppendMenu(hpopup,
					MF_STRING | (ss->Corners[nButton] == 'Y' ? MF_CHECKED : 0),
					ID_NOW, "Now");
				AppendMenu(hpopup,
					MF_STRING | (ss->Corners[nButton] == 'N' ? MF_CHECKED : 0),
					ID_NEVER, "Never");
				GetCursorPos(&point);
				nMenu = TrackPopupMenu(hpopup,
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON,
					point.x, point.y, 0, win, NULL);
				DestroyMenu(hpopup);
				hstb = GetDlgItem(win, id);
				switch (nMenu)
				{
					case 0: break;
					case ID_DEFAULT:
						ss->Corners[nButton] = '-';
						SendMessage(hstb, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hbnone);
						break;
					case ID_NOW:
						ss->Corners[nButton] = 'Y';
						SendMessage(hstb, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hbcheck);
						break;
					case ID_NEVER:
						ss->Corners[nButton] = 'N';
						SendMessage(hstb, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hbdot);
						break;
				}				
			}
		} 
		break;
		case WM_HELP:
		{
			LPHELPINFO lphi = (LPHELPINFO)lparam;
			if (lphi->iContextType == HELPINFO_WINDOW)
			{
				WinHelp((HWND)lphi->hItemHandle, "Moebius Clock.hlp", HELP_WM_HELP, (DWORD)(LPSTR)aHelpIDs);
			}
			return TRUE;
		}
		break;

		case WM_CONTEXTMENU:
			WinHelp((HWND)wparam, "Moebius Clock.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID)aHelpIDs);
			return TRUE;
			break;
   	}
	// Otherwise, return false
	return FALSE;
}
