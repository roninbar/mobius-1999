//==========================================================================
//
//  File: BehaviorDialog.cpp
//
//==========================================================================

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "SaverSettings.h"
#include "TransparentBitmap.h"
#include "resource.h"
#include "resource.hm"

extern SaverSettings* ss;
extern HINSTANCE hInstance;

#define BMPMONITOR_WIDTH	184
#define BMPMONITOR_HEIGHT	169

#define BMPBUTTON_WIDTH		25
#define BMPBUTTON_HEIGHT	23

static const DWORD aHelpIDs[] =
{
	IDC_ABOUT,		HIDC_ABOUT,
	IDC_SWING,		HIDC_SWING,
	IDC_USEMOUSE,	HIDC_USEMOUSE,
	IDC_BACK,		HIDC_BACK,
	IDC_QUICK,		HIDC_QUICK,
	IDC_EDIT1,		HIDC_SPIN1,
	IDC_SPIN1,		HIDC_SPIN1,
	IDC_MONITOR,	HIDC_COLOR1,
	IDC_COLOR1,		HIDC_COLOR1,
	IDC_COLOR2,		HIDC_COLOR1,
	IDC_COLOR3,		HIDC_COLOR1,
	IDC_COLOR4,		HIDC_COLOR1,
	0, 0
};

/****************************************************************************/
//
// Process the messages for the 'About' dialog.
//
BOOL FAR PASCAL
AboutDialogProc(HWND win, UINT msg, WPARAM wparam, LONG lparam)
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
//
// Process the messages for the 'Behavior' dialog.
//
BOOL APIENTRY
BehaviorDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	int nBtnCoordx[] = { 20, 140, 140, 20 };
	int nBtnCoordy[] = { 21, 21, 103, 103 };
	char chNum[10];
	HWND hspin, hedit;
	HWND hstb;
	HWND hmonitor;
	HBITMAP hbmpmonitor;
	HGDIOBJ holdbmp;
	RECT rectBmp, rectDlg;

	switch (msg)
	{
		case WM_INITDIALOG:
			hmonitor = GetDlgItem(hwnd, IDC_MONITOR);
			GetWindowRect(hwnd, &rectDlg);
			GetWindowRect(hmonitor, &rectBmp);
			rectBmp.top -= rectDlg.top;
			rectBmp.left -= rectDlg.left;
			MoveWindow(hmonitor, 
				rectBmp.left, rectBmp.top, 
				BMPMONITOR_WIDTH, BMPMONITOR_HEIGHT, TRUE
			);
			for (i=0; i < 4; i++)
			{
				hstb = GetDlgItem(hwnd, IDC_COLOR1+i);
				MoveWindow(hstb, 
					nBtnCoordx[i] + rectBmp.left, 
					nBtnCoordy[i] + rectBmp.top, 
					BMPBUTTON_WIDTH, BMPBUTTON_HEIGHT, TRUE
				);
			}
			return TRUE;

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis;
			lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlType == ODT_BUTTON)
			{
				HPEN hpen1, hpen2;
				HBRUSH hbrush;
				HGDIOBJ holdpen, holdbrush;
				POINT points[3];
				int nR, nG, nB;

				if (lpdis->CtlID == IDC_MONITOR)
				{
					hbmpmonitor = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MONITOR2));
					holdbmp = SelectObject(lpdis->hDC, hbmpmonitor);
					DrawTransparentBitmap(lpdis->hDC, hbmpmonitor, 0, 0, RGB(255, 0, 0));
					SelectObject(lpdis->hDC, holdbmp);
					DeleteObject(hbmpmonitor);
					for (i=0; i < 4; i++)
					{
						hstb = GetDlgItem(hwnd, IDC_COLOR1+i);
						InvalidateRect(hstb, NULL, FALSE);
					}
					return TRUE;
				}
				else if ( (lpdis->CtlID >= IDC_COLOR1) && (lpdis->CtlID <= IDC_COLOR4) )
				{
					i = lpdis->CtlID - IDC_COLOR1;
					nR = (int)(255 * ss->state.fColorR[i]);
					nG = (int)(255 * ss->state.fColorG[i]);
					nB = (int)(255 * ss->state.fColorB[i]);

					hpen1 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
					hpen2 = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
					hbrush = CreateSolidBrush(RGB(nR, nG, nB));
					holdpen   = SelectObject(lpdis->hDC, hpen1);
					holdbrush = SelectObject(lpdis->hDC, hbrush);
					Rectangle(lpdis->hDC, 
						lpdis->rcItem.left, lpdis->rcItem.top,
						lpdis->rcItem.right, lpdis->rcItem.bottom
					);
					SelectObject(lpdis->hDC, hpen2);
					points[0].x = lpdis->rcItem.left-1;
					points[0].y = points[1].y = lpdis->rcItem.bottom - 1;
					points[1].x = points[2].x = lpdis->rcItem.right-1;
					points[2].y = lpdis->rcItem.top - 1;
					Polyline(lpdis->hDC, points, 3);
					SelectObject(lpdis->hDC, holdpen);
					SelectObject(lpdis->hDC, holdbrush);
					DeleteObject(hpen1);
					DeleteObject(hpen2);
					DeleteObject(hbrush);

					return TRUE;
				}
			}
		}
		break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code)
			{
				case PSN_SETACTIVE:
					CheckDlgButton(hwnd, IDC_QUICK, ss->state.bQuick);
					CheckDlgButton(hwnd, IDC_SWING, ss->state.bSwing);
					CheckDlgButton(hwnd, IDC_USEMOUSE, ss->state.bUseMouse);
					CheckDlgButton(hwnd, IDC_BACK, !ss->state.bBlackBackground);
					hedit = GetDlgItem(hwnd, IDC_EDIT1);
					hspin = GetDlgItem(hwnd, IDC_SPIN1);
					sprintf(chNum, "%d", ss->state.nTwists);
					SendMessage(hedit, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)chNum);
					SendMessage(hspin, UDM_SETRANGE, 0L, MAKELONG(9, 0));
					SendMessage(hspin, UDM_SETPOS, 0L, MAKELONG(ss->state.nTwists, 0));
					break;

				case PSN_KILLACTIVE:
				{
					DWORD nSpinSel;
					hspin = GetDlgItem(hwnd, IDC_SPIN1);
					nSpinSel = SendMessage(hspin, UDM_GETPOS, 0, 0L);
					ss->state.nTwists = LOWORD(nSpinSel);
					ss->state.bQuick = IsDlgButtonChecked(hwnd, IDC_QUICK);
					ss->state.bSwing = IsDlgButtonChecked(hwnd, IDC_SWING);
					ss->state.bUseMouse = IsDlgButtonChecked(hwnd, IDC_USEMOUSE);
					ss->state.bBlackBackground = !IsDlgButtonChecked(hwnd, IDC_BACK);
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
				break;

				case PSN_RESET:
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					return TRUE;
					break;

				case PSN_APPLY:
					ss->WriteBehaviorRegistry();
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					break;
			}
			break;

		case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			BOOL bStatus;
			CHOOSECOLOR chooseColor;
			int nR, nG, nB;
			if ( (id >= IDC_COLOR1) && (id <= IDC_COLOR4) )
			{
				i = id - IDC_COLOR1;
				nR = (int)(255 * ss->state.fColorR[i]);
				nG = (int)(255 * ss->state.fColorG[i]);
				nB = (int)(255 * ss->state.fColorB[i]);
				chooseColor.lStructSize = sizeof(CHOOSECOLOR);
				chooseColor.hwndOwner = hwnd;
				chooseColor.rgbResult = RGB(nR, nG, nB);
				chooseColor.lpCustColors = ss->customColors;
				chooseColor.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;
				chooseColor.lpfnHook = NULL;
				chooseColor.lpTemplateName = NULL;
				bStatus = ChooseColor(&chooseColor);
				if (bStatus)
				{
					nR = GetRValue(chooseColor.rgbResult);
					nG = GetGValue(chooseColor.rgbResult);
					nB = GetBValue(chooseColor.rgbResult);
					ss->state.fColorR[i] = (float)nR / 255.0f;
					ss->state.fColorG[i] = (float)nG / 255.0f;
					ss->state.fColorB[i] = (float)nB / 255.0f;
					hstb = GetDlgItem(hwnd, id);
					InvalidateRect(hstb, NULL, FALSE);
				}
			}
			else if (id == IDC_ABOUT)
			{
				DialogBox(
					hInstance, MAKEINTRESOURCE(IDD_ABOUT), 
					hwnd, AboutDialogProc
				);
			}
		} 
		break;

		case WM_HELP:
		{
			LPHELPINFO lphi = (LPHELPINFO)lParam;
			if (lphi->iContextType == HELPINFO_WINDOW)
			{
				WinHelp((HWND)lphi->hItemHandle, "Moebius Clock.hlp", HELP_WM_HELP, (DWORD)(LPSTR)aHelpIDs);
			}
			return TRUE;
		}
		break;

		case WM_CONTEXTMENU:
			WinHelp((HWND)wParam, "Moebius Clock.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID)aHelpIDs);
			return TRUE;
			break;
	}
	return FALSE;
}

