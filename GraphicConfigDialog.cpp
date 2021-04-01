//==========================================================================
//
//  File: GraphicConfigDialog.cpp
//
//==========================================================================

#include <windows.h>

#include "SaverSettings.h"
#include "resource.h"
#include "resource.hm"

extern SaverSettings* ss;

static const DWORD aHelpIDs[] =
{
	IDC_FLAT,		HIDC_FLAT,
	IDC_GOURAUD,	HIDC_GOURAUD,
	IDC_ANTIALIAS,	HIDC_ANTIALIAS,
	IDC_BLEND,		HIDC_BLEND,
	IDC_NORMALS,	HIDC_NORMALS,
	IDC_WIREFRAME,	HIDC_WIREFRAME,
	IDC_ZBUFFER,	HIDC_ZBUFFER,
	IDC_CULL,		HIDC_CULL,
	IDC_NOGLASS,	HIDC_NOGLASS,
	IDC_NOTEXTURES,	HIDC_NOTEXTURES,
	IDC_NEAREST,	HIDC_NEAREST,
	IDC_LINEAR,		HIDC_NEAREST,
	IDC_WMIPMAPS,	HIDC_WMIPMAPS,
	IDC_NMIPMAPS,	HIDC_WMIPMAPS,
	IDC_LMIPMAPS,	HIDC_WMIPMAPS,
	0, 0
};

/****************************************************************************/
//
// Process the messages for the 'GraphicConfig' dialog.
//
BOOL APIENTRY
GraphicConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code)
			{
				case PSN_SETACTIVE:
					CheckDlgButton(hwnd, IDC_BLEND, ss->state.bBlend);
					CheckDlgButton(hwnd, IDC_ANTIALIAS, ss->state.bSmooth);
					CheckDlgButton(hwnd, IDC_NORMALS, ss->state.bNormals);
					CheckDlgButton(
						hwnd, IDC_WIREFRAME, 
						(ss->state.nPolygonMode == GL_FILL) ? FALSE : TRUE
					);
					CheckDlgButton(hwnd, IDC_ZBUFFER, ss->state.bDepthTest);
					CheckDlgButton(hwnd, IDC_CULL, ss->state.bCulling);
					CheckDlgButton(hwnd, IDC_NOGLASS, ss->state.bNoGlass);
					EnableWindow(GetDlgItem(hwnd, IDC_BLEND), !ss->state.bNoGlass);
					CheckDlgButton(hwnd, IDC_NOTEXTURES, ss->state.bNoExtraTextures);
					CheckRadioButton(
						hwnd, IDC_FLAT, IDC_GOURAUD,
						ss->state.nShadeModel == GL_FLAT ? IDC_FLAT : IDC_GOURAUD
					);
					CheckRadioButton(
						hwnd, IDC_NEAREST, IDC_LINEAR,
						ss->state.nLinear == 1 ? IDC_LINEAR : IDC_NEAREST
					);
					CheckRadioButton(
						hwnd, IDC_WMIPMAPS, IDC_LMIPMAPS,
						IDC_WMIPMAPS + ss->state.nMipmap
					);
					break;

				case PSN_KILLACTIVE:
				{
					BOOL bBlend;
					ss->state.bCulling = IsDlgButtonChecked(hwnd, IDC_CULL);
					ss->state.bDepthTest = IsDlgButtonChecked(hwnd, IDC_ZBUFFER);
					ss->state.bNormals = IsDlgButtonChecked(hwnd, IDC_NORMALS);
					ss->state.bNoGlass = IsDlgButtonChecked(hwnd, IDC_NOGLASS);
					ss->state.bNoExtraTextures = IsDlgButtonChecked(hwnd, IDC_NOTEXTURES);
					ss->state.nLinear = 
						IsDlgButtonChecked(hwnd, IDC_LINEAR) ? 1 : 0;
					ss->state.nMipmap =
						IsDlgButtonChecked(hwnd, IDC_WMIPMAPS) ? 0 : 1;
					ss->state.nMipmap =
						IsDlgButtonChecked(hwnd, IDC_LMIPMAPS) ? 2 : ss->state.nMipmap;
					ss->state.nPolygonMode =
						IsDlgButtonChecked(hwnd, IDC_WIREFRAME) ? GL_LINE : GL_FILL;
					ss->state.nShadeModel =
						IsDlgButtonChecked(hwnd, IDC_FLAT) ? GL_FLAT : GL_SMOOTH;
					ss->state.bSmooth = IsDlgButtonChecked(hwnd, IDC_ANTIALIAS);
					bBlend = IsDlgButtonChecked(hwnd, IDC_BLEND);
					if (ss->state.bBlend != bBlend)
					{
						ss->state.da = -ss->state.da;
						ss->state.bBlend = bBlend;
					}
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
				break;

				case PSN_RESET:
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					return TRUE;
					break;

				case PSN_APPLY:
					ss->WriteGraphicConfigRegistry();
					SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
					break;
			}
			break;

		case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			if (id == IDC_NOGLASS)
			{
				HWND hblend = GetDlgItem(hwnd, IDC_BLEND);
				EnableWindow(hblend, !IsWindowEnabled(hblend));
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

