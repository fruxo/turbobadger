#include "tdimage/tdimage.h"
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>

bool TdImage::LoadPNG(const char *filename)
{
	MakeEmpty();
	bool success = false;
	unsigned char *image_buf = NULL;
	if (FILE *file = fopen(filename, "rb"))
	{
		fseek(file, 0, SEEK_END);
		unsigned int enc_size = ftell(file);
		fseek(file, 0, SEEK_SET);
		unsigned char *enc_data = new unsigned char[enc_size];
		if (!enc_data)
		{
			fclose(file);
			return false;
		}
		enc_size = fread(enc_data, 1, enc_size, file);

		unsigned tmpw, tmph;
		if (LodePNG_decode32(&image_buf, &tmpw, &tmph, enc_data, enc_size) == 0)
		{
			width = tmpw;
			height = tmph;
			bits_per_pixel = 32;
			data = image_buf;
			success = true;
		}

		delete [] enc_data;
		fclose(file);
	}
	return success;
}

bool TdImage::SavePNG(const char *filename)
{
	FILE *f = fopen(filename, "wb");
	if (!f)
		return false;
	bool success = false;
	unsigned char *png_buf = NULL;
	size_t png_size;
	if (LodePNG_encode32(&png_buf, &png_size, (const unsigned char *)data, width, height) == 0)
	{
		success = true;
		if (fwrite(png_buf, 1, png_size, f) != png_size)
			success = false; // Failed to write all bytes. Not enough disk?
		free(png_buf);
	}
	fclose(f);
	return success;
}
