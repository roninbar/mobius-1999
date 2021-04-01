//==========================================================================
//
//  File: Dialogs.h
//
//==========================================================================

#ifndef DIALOGS_H
#define DIALOGS_H

#include <windows.h>

/****************************************************************************/

BOOL APIENTRY
GeneralDialogProc(HWND win, UINT msg, UINT wparam, LONG lparam);

BOOL APIENTRY
GraphicConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY
BehaviorDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/****************************************************************************/

#endif
