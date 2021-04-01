//==========================================================================
//
//  File: OpenGLenv.cpp
//
//==========================================================================

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include "OpenGLenv.h"
#include "texture.h"
#include "resource.h"
#include "SaverSettings.h"

#define max3(a, b, c) (max(max((a), (b)), (c)))

extern SaverSettings* ss;

/****************************************************************************/

// Colors
const GLfloat no_color[]= { 0.0, 0.0, 0.0, 0.0 };
const GLfloat black[]= { 0.0, 0.0, 0.0, 1.0 };
const GLfloat white[]= { 1.0, 1.0, 1.0, 1.0 };
const GLfloat gray5[]= { 0.5, 0.5, 0.5, 1.0 };
const GLfloat gray25[]= { 0.25, 0.25, 0.25, 1.0 };
const GLfloat gray75[]= { 0.75, 0.75, 0.75, 1.0 };
const GLfloat gray125[]= { 0.125, 0.125, 0.125, 1.0 };
const GLfloat gray375[]= { 0.375, 0.375, 0.375, 1.0 };
const GLfloat gray625[]= { 0.625, 0.625, 0.625, 1.0 };
const GLfloat gray875[]= { 0.875, 0.875, 0.875, 1.0 };
const GLfloat red[]= { 1.0, 0.0, 0.0, 1.0 };
const GLfloat red5[]= { 0.5, 0.0, 0.0, 1.0 };
const GLfloat green[]= { 0.0, 1.0, 0.0, 1.0 };
const GLfloat green5[]= { 0.0, 0.5, 0.0, 1.0 };
const GLfloat green25[]= { 0.0, 0.25, 0.0, 1.0 };
const GLfloat green75[]= { 0.0, 0.75, 0.0, 1.0 };
const GLfloat blue[]= { 0.0, 0.0, 1.0, 1.0 };
const GLfloat blue5[]= { 0.0, 0.0, 0.5, 1.0 };
const GLfloat yellow[]= { 1.0, 1.0, 0.0, 1.0 };
const GLfloat yellow5[]= { 0.5, 0.5, 0.0, 1.0 };
const GLfloat yellow25[]= { 0.25, 0.25, 0.0, 1.0 };
const GLfloat yellow75[]= { 0.75, 0.75, 0.0, 1.0 };
const GLfloat magenta[]= { 1.0, 0.0, 1.0, 1.0 };
const GLfloat magenta5[]= { 0.5, 0.0, 0.5, 1.0 };
const GLfloat cyan[]= { 0.0, 1.0, 1.0, 1.0 };
const GLfloat cyan5[]= { 0.0, 0.5, 0.5, 1.0 };
const GLfloat orange[]= { 1.0, 0.8f, 0.0, 1.0 };
const GLfloat gold[]= { 1.0, 0.8f, 0.5, 1.0 };
const GLfloat copper[]= { 0.5, 0.3f, 0.125, 1.0 };

// Tranlucent colors
const GLfloat white_5[]= { 1.0, 1.0, 1.0, 0.5 };
const GLfloat white_25[]= { 1.0, 1.0, 1.0, 0.25 };
const GLfloat white_75[]= { 1.0, 1.0, 1.0, 0.75 };

// Texture filter methods
const GLenum filter[2][3]= {
	GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
};

// Constants
const double step= M_PI / 120.0;
const double R= 10.0, h= 1.0;
const double epsilon= 0.001;
const double t0= M_PI, dt0= 0.0;

// Global variables
GLuint hours[4], envmap, face; // Texture objects
GLuint frisbee, disc, rim; // Display lists
struct {
	double msec, sec, min, hour, day;
} time;
double t= t0, dt= dt0;
double alpha= 1.0;

int frames= 0;

/****************************************************************************/

void
EnableGraphicOption(GLenum which, BOOL bEnable)
{
	if (bEnable) glEnable(which);
	else glDisable(which);
}

/****************************************************************************/

void
ResetOrientationGraphicEnv()
{
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -25.0);
	t = t0;
	dt= dt0;
}

/****************************************************************************/

void
MotionOfGraphicEnv(POINT* pAnchor, POINT mouse, BOOL roll)
{
	double dx = mouse.x - pAnchor->x, dy = mouse.y - pAnchor->y;
	double distance = sqrt(dx * dx + dy * dy);
	double u[3]= {
		roll ? 0.0 : dy,
		roll ? 0.0 : dx,
		roll ? -dx : 0.0
	};
	double M[4][4], v[3]= { 0.0, 0.0, 0.0 };

	glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *) M);

	for (int i= 0; i < 3; i++)
		for (int j= 0; j < 3; j++)
			v[i]+= M[i][j] * u[j];

	glRotated(0.25 * distance, v[0], v[1], v[2]);

	pAnchor->x = mouse.x;
	pAnchor->y = mouse.y;
}

/****************************************************************************/

static BOOL 
bSetupPixelFormat(HDC hdc) 
{ 
	PIXELFORMATDESCRIPTOR pfd, *ppfd; 
	int pixelformat; 
 
	ppfd = &pfd; 
	memset(&pfd, 0, sizeof(pfd));

	ppfd->nSize		= sizeof(PIXELFORMATDESCRIPTOR);
	ppfd->nVersion	= 1;
	ppfd->dwFlags	=	PFD_DRAW_TO_WINDOW | 
						PFD_SUPPORT_OPENGL | 
						PFD_DOUBLEBUFFER;

	ppfd->iPixelType= PFD_TYPE_RGBA;
	ppfd->cColorBits= 32;

	pixelformat = ChoosePixelFormat(hdc, &pfd);
    if (pixelformat == 0)
	{
		MessageBox(
			NULL, "ChoosePixelFormat() failed:  "
			"Cannot find a suitable pixel format.", "Error", MB_OK
		);
		return 0;
	}
 
	if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
	{
		MessageBox(
			NULL, "SetPixelFormat() failed:  "
			"Cannot set format specified.", "Error", MB_OK
		);
		return 0;
	}
	DescribePixelFormat(hdc, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    return TRUE; 
} 

/****************************************************************************/

BOOL
InitGraphicEnv(HWND wnd, HDC* phdc, HGLRC* phglrc)
{
	// Get the windows device context
	*phdc = GetDC(wnd);
	if (!bSetupPixelFormat(*phdc)) 
		return FALSE;
 
	*phglrc = wglCreateContext(*phdc); 
	wglMakeCurrent(*phdc, *phglrc); 

	return TRUE;
}
		
/****************************************************************************/

void
DestroyGraphicEnv(HWND hwnd, HDC hdc, HGLRC hglrc)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hwnd, hdc);
}

/****************************************************************************/

void
InitState()
{
	// Polygon Mode
	glPolygonMode(GL_FRONT_AND_BACK, ss->state.nPolygonMode);

	// Material properties
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 128);
	glEnable(GL_COLOR_MATERIAL); // for ambient and diffuse reflection

	// Lighting
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	// Face Culling
	EnableGraphicOption(GL_CULL_FACE, ss->state.bCulling);

	// Blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Antialiasing
	EnableGraphicOption(GL_LINE_SMOOTH, ss->state.bSmooth);
	EnableGraphicOption(GL_POLYGON_SMOOTH, ss->state.bSmooth);

	// Z-Buffering
	EnableGraphicOption(GL_DEPTH_TEST, ss->state.bDepthTest);

	// Perspective Correction Hint
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, ss->state.nPerspCorrectionHint);

	// Shade Model
	glShadeModel(ss->state.nShadeModel);

	// Line Smooth Hint
	glHint(GL_LINE_SMOOTH_HINT, ss->state.nLineSmoothHint);

	// Clear Color
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear Depth
	glClearDepth(1.0);

	// Model/View
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -25.0);
}

/****************************************************************************/

void
init_hours_texture(HWND hwnd, HDC hdc)
{

	int width, height;
	HBITMAP hbitmap;
	BITMAPINFO bitmapinfo;
	unsigned char* texture;
	unsigned char* bitmap;
	unsigned char *p, *q;

	glGenTextures(4, hours);

	hbitmap = (HBITMAP)LoadImage(
		(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDB_HOURS0), IMAGE_BITMAP,
		0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION
	);
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 0;
	GetDIBits(hdc, hbitmap, 0, 0, NULL, &bitmapinfo, DIB_RGB_COLORS);
	width  = bitmapinfo.bmiHeader.biWidth;
	height = bitmapinfo.bmiHeader.biHeight;
	bitmap  = new unsigned char[bitmapinfo.bmiHeader.biSizeImage];
	texture = new unsigned char[width * height * 4];
	for (int i=0; i < 4; i++)
	{
		if (i > 0)
			hbitmap = (HBITMAP)LoadImage(
				(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
				MAKEINTRESOURCE(IDB_HOURS0+i), IMAGE_BITMAP,
				0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION
			);
		GetDIBits(hdc, hbitmap, 0, height, (LPVOID)bitmap, &bitmapinfo, DIB_RGB_COLORS);
		p = texture;
		q = bitmap;
		for (int j=0; j < width*height; j++)
		{
			*(p++) = *(q++);
			*(p++) = *(q++);
			*(p++) = *(q++);
			*(p) = max3( *(p-1), *(p-2), *(p-3) );
			p++;
		}
		glBindTexture(GL_TEXTURE_2D, hours[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		DeleteObject(hbitmap);
	}
	delete[] texture;
	delete[] bitmap;
}

/****************************************************************************/

void
loadTextureFromFile(GLuint &object, const char *filename)
{
	int width, height, components;

	unsigned char *texture= readTexture(filename, &width, &height, &components);
	if (texture == NULL)
	{
		//logfile << "Failed to read " << filename << "\n";
		exit(-1);
	}
	glGenTextures(1, &object);
	glBindTexture(GL_TEXTURE_2D, object);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	int err= gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	if (err != 0)
	{
		const unsigned char *errstr= gluErrorString(err);
		//logfile << "Mipmap error: " << errstr << "\n";
	}
	free(texture);

	//check_for_gl_error("creating texture #%d from %s", object, filename);
}

/****************************************************************************/

void
loadTextureFromResource(HWND hwnd, HDC hdc, GLuint& object, UINT bitmapResID)
{
	int width, height;
	HBITMAP hbitmap;
	BITMAPINFO bitmapinfo;
	unsigned char* texture;

	hbitmap = (HBITMAP)LoadImage(
		(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
		MAKEINTRESOURCE(bitmapResID), IMAGE_BITMAP,
		0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION
	);
	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 0;
	GetDIBits(hdc, hbitmap, 0, 0, NULL, &bitmapinfo, DIB_RGB_COLORS);
	width  = bitmapinfo.bmiHeader.biWidth;
	height = bitmapinfo.bmiHeader.biHeight;
	texture = new unsigned char[bitmapinfo.bmiHeader.biSizeImage];

	GetDIBits(hdc, hbitmap, 0, height, (LPVOID)texture, &bitmapinfo, DIB_RGB_COLORS);

	glGenTextures(1, &object);
	glBindTexture(GL_TEXTURE_2D, object);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, texture);

	delete[] texture;
	DeleteObject(hbitmap);
}

/****************************************************************************/

void
InitTexture(HWND hwnd, HDC hdc)
{
	init_hours_texture(hwnd, hdc);
	if (!ss->state.bNoGlass)
	{
		loadTextureFromResource(hwnd, hdc, envmap, IDB_CAFE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	if (!ss->state.bNoExtraTextures)
	{
		loadTextureFromResource(hwnd, hdc, face, IDB_OGLLOGO);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

/****************************************************************************/

void
draw_backdrop()
{
	GLfloat firstColor[4];
	GLfloat secondColor[4];
	GLfloat thirdColor[4];
	GLfloat fourthColor[4];

	firstColor[0]  = ss->state.fColorR[0];
	firstColor[1]  = ss->state.fColorG[0];
	firstColor[2]  = ss->state.fColorB[0];
	secondColor[0] = ss->state.fColorR[1];
	secondColor[1] = ss->state.fColorG[1];
	secondColor[2] = ss->state.fColorB[1];
	thirdColor[0] = ss->state.fColorR[2];
	thirdColor[1] = ss->state.fColorG[2];
	thirdColor[2] = ss->state.fColorB[2];
	fourthColor[0] = ss->state.fColorR[3];
	fourthColor[1] = ss->state.fColorG[3];
	fourthColor[2] = ss->state.fColorB[3];
	firstColor[3] = secondColor[3] = thirdColor[3] = fourthColor[3] = 1.0f;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -40.0);

	glBegin(GL_QUADS);
	{
		glColor4fv(fourthColor);
		glVertex2f(-30.0, -20.0);
		glColor4fv(thirdColor);
		glVertex2f(+30.0, -20.0);
		glColor4fv(secondColor);
		glVertex2f(+30.0, +20.0);
		glColor4fv(firstColor);
		glVertex2f(-30.0, +20.0);
	}
	glEnd();

	glPopMatrix();

	glPopAttrib();
}

/****************************************************************************/

void
draw_frisbee()
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_NORMALIZE);

	glPushMatrix();
	glScalef(1.0, 1.0, 0.2f);

	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(0.0, 0.0, R);

		const double r= R * sin(step);
		const double z= R * cos(step);

		for (double f= -M_PI; f < +M_PI + epsilon; f+= step)
		{
			const double x= r * cos(f), y= r * sin(f);

			glNormal3d(x, y, z);
			glVertex3d(x, y, z);
		}
	}
	glEnd();

	for (double t= step; t < 0.5 * M_PI - epsilon; t+= step)
	{
		const double r0= R * sin(t), r1= R * sin(t + step);
		const double z0= R * cos(t), z1= R * cos(t + step);

		glBegin(GL_TRIANGLE_STRIP);
		{
			for (double f= -M_PI; f < +M_PI + epsilon; f+= step)
			{
				const double x0= r0 * cos(f), x1= r1 * cos(f);
				const double y0= r0 * sin(f), y1= r1 * sin(f);

				glNormal3d(x0, y0, z0);
				glVertex3d(x0, y0, z0);
				glNormal3d(x1, y1, z1);
				glVertex3d(x1, y1, z1);
			}
		}
		glEnd();
	}

	glPopMatrix();
	glPopAttrib();
}

/****************************************************************************/

double texx(double x, double y)
{
	return x;
}

double texy(double x, double y)
{
	return y;
}

/****************************************************************************/

void
draw_disc()
{
	const double R= 9.9;
	const double dr= R / 6.0;

	glNormal3f(0.0, 0.0, 1.0);

	glBegin(GL_TRIANGLE_FAN);
	{
		glTexCoord2d(texx(0.0, 0.0), texy(0.0, 0.0));
		glVertex2d(0.0, 0.0);
		for (double t= 0.0; t < 2 * M_PI + epsilon; t+= step)
		{
			double x= dr * cos(t), y= dr * sin(t);
			glTexCoord2d(texx(x / R, y / R), texy(x / R, y / R));
			glVertex2d(x, y);
		}
	}
	glEnd();

	for (double r= dr; r < R - epsilon; r+= dr)
	{
		glBegin(GL_TRIANGLE_STRIP);
		{
			for (double t= 0.0; t < 2 * M_PI + epsilon; t+= step)
			{
				double x0= r * cos(t), y0= r * sin(t);
				glTexCoord2d(texx(x0 / R, y0 / R), texy(x0 / R, y0 / R));
				glVertex2d(x0, y0);

				double x1= (r + dr) * cos(t), y1= (r + dr) * sin(t);
				glTexCoord2d(texx(x1 / R, y1 / R), texy(x1 / R, y1 / R));
				glVertex2d(x1 ,y1);
			}
		}
		glEnd();
	}
}

/****************************************************************************/

void
draw_rim()
{
	glBegin(GL_TRIANGLE_STRIP);
	{
		for (double t= 0.0; t < 2 * M_PI + epsilon; t+= step)
		{
			glNormal3d(cos(t), sin(t), 0.0);
			glVertex3d(R * cos(t), R * sin(t), +h);
			glVertex3d(R * cos(t), R * sin(t), -h);
		}
	}
	glEnd();
}

/****************************************************************************/

void
draw_hours_strip(int base)
{
	const double R= 9.0, h= 1.0;
	const double torsion= 2 * ss->state.nTwists * M_PI * time.day;
	const GLfloat *color[]= { blue, green, yellow, red };

	for (int i= 0; i < 2; i++)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_TEXTURE_2D);


		glBindTexture(GL_TEXTURE_2D, hours[base + i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter[ss->state.nLinear][ss->state.nMipmap]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter[ss->state.nLinear][0]);

		glBegin(GL_TRIANGLE_STRIP);
		{
			for (float s= 0.0; s < 1.0 + epsilon; s+= float(step / M_PI))
			{
				double t= (double(i) + s) * M_PI;
				double tt= ss->state.nTwists * 0.5 * t - torsion;
				double ct= cos(t), st= sin(t);
				double ctt= cos(tt), stt= sin(tt);
				double r1= R - h * ctt;
				double r2= R + h * ctt;
				double z1= -h * stt;
				double z2= +h * stt;

				GLfloat vcolor[4];

				for (int k = 0; k < 4; k++)
				{
					vcolor[k] = (1 - s) * color[base + i][k] + s * color[(base + i + 1) % 4][k];
				}

				glColor4fv(vcolor);
				glNormal3d(-st * stt, -ct * stt, ct * ct * ctt + ctt * st * st);

				glTexCoord2d(t / M_PI, 0.0);
				glVertex3d(r2 * st, r2 * ct, z2);
				glTexCoord2d(t / M_PI, 1.0);
				glVertex3d(r1 * st, r1 * ct, z1);
			}
		}
		glEnd();

		glPopAttrib();

		if (ss->state.bNormals) 
		{
			glPushAttrib(GL_LIGHTING_BIT);
			glDisable(GL_LIGHTING);
			glColor4fv(white);

			glBegin(GL_LINES);
			{
				for (double t= i * M_PI; t < (i + 1) * M_PI + epsilon; t+= M_PI / 24)
				{
					double tt= ss->state.nTwists * 0.5 * t - torsion;
					double ct= cos(t), st= sin(t);
					double ctt= cos(tt), stt= sin(tt);
					double r1= R - h * ctt;
					double r2= R + h * ctt;
					double z1= -h * stt;
					double z2= +h * stt;

					double n[3]= { -st * stt, -ct * stt, ct * ct * ctt + ctt * st * st };

					glVertex3d(r2 * st, r2 * ct, z2);
					glVertex3d(r2 * st + n[0], r2 * ct + n[1], z2 + n[2]);
					glVertex3d(r1 * st, r1 * ct, z1);
					glVertex3d(r1 * st + n[0], r1 * ct + n[1], z1 + n[2]);
				}
			}
			glEnd();

			glPopAttrib();
		}
	}
}

/****************************************************************************/

void
draw_hand(GLdouble rotation, GLdouble length, GLdouble width)
{
	glPushMatrix();
	glRotated(rotation, 0.0, 0.0, 1.0);

	glNormal3d(0.0, 0.0, 1.0);
	glRectd(-width, -0.2 * length, +width, +length);

	glPopMatrix();
}

/****************************************************************************/

void
draw_cap()
{
	const double R= 0.5, h= 0.1;
	const double norm= sqrt(h * h + R * R);
	const double nR= R / norm, nh= h / norm;

	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(0.0, 0.0, h);
		for (double t= 0.0; t < 2 * M_PI + epsilon; t+= 4 * step)
		{
			glNormal3d(nh * cos(t), nh * sin(t), nR);
			glVertex2d(R * cos(t), R * sin(t));
		}
	}
	glEnd();
}

/****************************************************************************/

void
draw_face()
{
	if (!ss->state.bNoExtraTextures)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, face);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter[ss->state.nLinear][ss->state.nMipmap]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter[ss->state.nLinear][0]);
		glCallList(disc);

		glPopAttrib();
	}
	else
	{
		glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
		glDisable(GL_LIGHTING);
		glColor4fv(black);
		glCallList(disc);
		glPopAttrib();
	}
}

/****************************************************************************/

void
draw_glass()
{
	glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
	glDisable(GL_POLYGON_SMOOTH);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glBindTexture(GL_TEXTURE_2D, envmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter[ss->state.nLinear][ss->state.nMipmap]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter[ss->state.nLinear][0]);
	glCallList(frisbee);

	glPopAttrib();
}

/****************************************************************************/

void
draw_clock()
{
	// Inside of rim
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	glColor4fv(gold);
	glCullFace(GL_FRONT);
	glCallList(rim);
	glPopAttrib();

	// Face
	glColor4fv(gold);
	glPushMatrix();
	glTranslated(0.0, 0.0, -h);
	draw_face();
	glPopMatrix();

	// Hours strip
	glPushAttrib(GL_POLYGON_BIT);
	glCullFace(GL_BACK);
	draw_hours_strip(0);
	glCullFace(GL_FRONT);
	draw_hours_strip(2);
	glPopAttrib();

	glPushAttrib(GL_LIGHTING_BIT);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gray5);

	glPushMatrix();

	// Hours hand
	glColor4fv(gray25);
	draw_hand(-30.0 * time.hour, 5.0, 0.1f);
	glTranslatef(0.0, 0.0, 0.2f);

	// Minutes hand
	glColor4fv(gray25);
	draw_hand(-6.0 * time.min, 7.0, 0.1f);
	glTranslatef(0.0, 0.0, 0.2f);

	// Seconds hand
	glColor4fv(red5);
	draw_hand(-6.0 * time.sec, 8.0, 0.02f);
	glTranslatef(0.0, 0.0, 0.2f);

	// Hands axis cap
	glColor4fv(gray125);
	draw_cap();

	glPopMatrix();

	glPopAttrib();

	// Case
	glPushAttrib(GL_LIGHTING_BIT);
	glMateriali(GL_FRONT, GL_SHININESS, 32);
	glColor4fv(copper);
	glPushMatrix();
	glRotated(180.0, 0.0, 1.0, 0.0);
	glTranslated(0.0, 0.0, +h);
	glCallList(frisbee);
	glPopMatrix();
	glCallList(rim);
	glPopAttrib();

	if (!ss->state.bNoGlass)
	{
		// Fade Glass
		alpha= alpha + ss->state.da;
		alpha= min(alpha, 1.0);
		alpha= max(alpha, 0.3);

		void (__stdcall *proc[2])(GLenum)= { glDisable, glEnable };
		
		proc[alpha < 1.0](GL_BLEND);

		// Glass cover
		glPushMatrix();
		glTranslated(0.0, 0.0, +h);
		glColor4d(1.0, 1.0, 1.0, alpha);
		draw_glass();
		glPopMatrix();
	}
}

/****************************************************************************/

void
InitDisplayLists()
{
	frisbee = glGenLists(1);
	glNewList(frisbee, GL_COMPILE);
	{
		draw_frisbee();
	}
	glEndList();

	disc = glGenLists(1);
	glNewList(disc, GL_COMPILE);
	{
		draw_disc();
	}
	glEndList();

	rim = glGenLists(1);
	glNewList(rim, GL_COMPILE);
	{
		draw_rim();
	}
	glEndList();
}

/****************************************************************************/

void
Display(HDC hdc)
{
	static BOOL bfirstTime = TRUE;

	const double xrot = 30 * (1.0 + cos(t));
	const double zrot = 30 * sin(t);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glRotated(xrot, 1.0, 0.0, 0.0);
	glRotated(zrot, 0.0, 0.0, 1.0);

	if (!ss->state.bBlackBackground)
		draw_backdrop();
	draw_clock();

	glPopMatrix();

	if (ss->state.bSwing)
	{
		t+= dt;
		dt+= -0.001 * t;
	}

	SwapBuffers(hdc);

	frames++;
}

/****************************************************************************/

void
Reshape(HDC hdc, int width, int height)
{
	const double w= width, h= height;
	const double aspect= w / h;
	double fw, fh;

	glViewport(0, 0, width, height);

	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (width < height)
	{
		fw= 0.5;
		fh= fw / aspect;
	}
	else
	{
		fh= 0.5;
		fw= fh * aspect;
	}

	glFrustum(-fw, +fw, -fh, +fh, 1.0, 100.0);

	glPopAttrib();
}

/****************************************************************************/

void
UpdateTime()
{
	SYSTEMTIME systime;

	GetLocalTime(&systime);
	time.msec = (double) systime.wMilliseconds;
	time.sec  = (double) systime.wSecond + time.msec / 1000.0;
	time.min  = (double) systime.wMinute + time.sec / 60.0;
	time.hour = (double) systime.wHour + time.min / 60.0;
	time.day  = time.hour / 24.0;
	if (ss->state.bQuick)
	{
		time.sec= 0.0;
		time.min*= 900.0;
		time.hour*= 900.0;
		time.day*= 900.0;
	}
}

