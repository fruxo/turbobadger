#include "tdimage/tdimage.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR
#define STBI_NO_JPG
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_TGA
#include "stb_image.c"

// stbi is faster than lodepng: 1118ms -> 770ms
// stbi is about same size: 161kb -> 163kb of Demo.exe with only png. (145kb without image loading support)
// stbi supports more formats (191kb with all)
// stbi use no STL

// TEST: Mem leaks?
// TEST: 64bit support?

//bool TdImage::SavePNG(const char *filename)
//{
//	return false;
//}

tinkerbell::TBImageLoader *tinkerbell::TBImageLoader::CreateFromFile(const char *filename)
{
	int w, h, comp;
	if (unsigned char *data = stbi_load(filename, &w, &h, &comp, 4))
	{
		if (TdImage *img = new TdImage())
		{
			img->width = w;
			img->height = h;
			img->bits_per_pixel = 32;
			img->data = data;
			return img;
		}
		else
			stbi_image_free(data);
	}
	return nullptr;
}
