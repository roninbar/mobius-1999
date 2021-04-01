//==========================================================================
//
//  File: OpenGLenv.h
//
//==========================================================================

#ifndef OPENGLENV_H
#define OPENGLENV_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

/****************************************************************************/

class GraphicEnvState
{
public:
	GraphicEnvState() : 
		nLinear(1), nMipmap(2), da(-0.02f), bBlend(TRUE), nTwists(3),
		bNormals(FALSE), bQuick(FALSE), bSwing(TRUE), bBlackBackground(TRUE),
		bCulling(TRUE), bSmooth(TRUE), bDepthTest(TRUE), bUseMouse(FALSE),
		bNoGlass(FALSE), bNoExtraTextures(FALSE),
		nLineSmoothHint(GL_FASTEST), nShadeModel(GL_SMOOTH),
		nPerspCorrectionHint(GL_FASTEST), nPolygonMode(GL_FILL)
		{ 
			fColorR[0] = fColorR[1] = 1.0f;		// red - top corners
			fColorG[0] = fColorG[1] = 0.0f;
			fColorB[0] = fColorB[1] = 0.0f;
			fColorR[2] = fColorR[3] = 1.0f;		// orange - bottom corners
			fColorG[2] = fColorG[3] = 0.8f;
			fColorB[2] = fColorB[3] = 0.0f;
		};

	short	nLinear;
	short	nMipmap;
	short	nTwists;			// Number of 180-degree twists
	float	da;
	BOOL	bBlend;
	BOOL	bNormals;
	BOOL	bQuick;
	BOOL	bSwing;
	BOOL	bCulling;
	BOOL	bSmooth;
	BOOL	bDepthTest;
	BOOL	bBlackBackground;
	BOOL	bUseMouse;
	BOOL	bNoGlass;
	BOOL	bNoExtraTextures;
	GLint	nLineSmoothHint;			// GL_NICEST or GL_FASTEST
	GLint	nShadeModel;				// GL_SMOOTH or GL_FLAT
	GLint	nPerspCorrectionHint;		// GL_NICEST or GL_FASTEST
	GLint	nPolygonMode;				// GL_FILL or GL_LINE
	float	fColorR[4];
	float	fColorG[4];
	float	fColorB[4];
};

/****************************************************************************/

BOOL InitGraphicEnv(HWND wnd, HDC* phdc, HGLRC* phglrc);
void DestroyGraphicEnv(HWND hwnd, HDC hdc, HGLRC hglrc);
void EnableGraphicOption(GLenum which, BOOL bEnable);
void ResetOrientationGraphicEnv();
void MotionOfGraphicEnv(POINT* pAnchor, POINT mouse, BOOL roll);

void InitState();
void InitTexture(HWND hwnd, HDC hdc);
void InitDisplayLists();

void Display(HDC hdc);
void Reshape(HDC hdc, int width, int height);

void UpdateTime();

#endif
