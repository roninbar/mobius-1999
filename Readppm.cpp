#include <windows.h>
#include <stdio.h>

#define max3(a, b, c) (max(max((a), (b)), (c)))

unsigned char *ReadPPM(const char *filename, int *width, int *height)
{
	FILE *file= fopen(filename, "r");

	char magic[10];

	fscanf(file, "%s", magic);
	fscanf(file, "%d %d", width, height);

	int maxcolor;

	fscanf(file, "%d", &maxcolor);

	int npixels= *width * *height;
	unsigned char (*data)[4]= new unsigned char[npixels][4];
	unsigned char (*p)[4]= data - 1;

	fseek(file, -3 * npixels, SEEK_END);

	while (npixels--)
	{
		fread(++p, 3, 1, file);
		(*p)[3]= max3((*p)[0], (*p)[1], (*p)[2]);
	}

	fclose(file);

	return (unsigned char *) data;
} 

