#include "tdimage/tdimage.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#define strcasecmp stricmp
#endif // WIN32

bool TdImage::NewData(int new_width, int new_height)
{
	if (new_width == width && new_height == height)
		return true;
	MakeEmpty();
	if (data = new unsigned char[new_width * new_height * 4])
	{
		width = new_width;
		height = new_height;
		bits_per_pixel = 32;
	}
	return data ? true : false;
}

TdImage *TdImage::Create(int width, int height)
{
	TdImage *img = new TdImage;
	if (img && img->NewData(width, height))
		return img;
	delete img;
	return 0;
}

TdImage *TdImage::CreateFromFile(const char *filename)
{
	const char *suff;
	int o = 0;
	while (filename[o] != 0) o++;
	suff = &filename[o - 3];
	if (strcasecmp(suff, "png") == 0)
	{
		TdImage *img = new TdImage();
		if (img && img->LoadPNG(filename))
			return img;
		delete img;
		return 0;
	}
	return 0;
}

tinkerbell::TBImageLoader *tinkerbell::TBImageLoader::CreateFromFile(const char *filename)
{
	return TdImage::CreateFromFile(filename);
}
