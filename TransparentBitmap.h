//==========================================================================
//
//  File: TransparentBitmap.h
//
//==========================================================================

#ifndef TRANSPARENTBITMAP_H
#define TRANSPARENTBITMAP_H

#include <windows.h>

void DrawTransparentBitmap(
			HDC hdc, HBITMAP hBitmap, short xStart,
			short yStart, COLORREF cTransparentColor
);

#endif
