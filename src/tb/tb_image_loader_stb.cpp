// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_bitmap_fragment.h"
#include "tb_system.h"

namespace tb {

// Remove image formats we don't use to limit binary size.
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR
#define STBI_NO_JPG
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_TGA

// Include stb image - Tiny portable and reasonable fast image loader from http://nothings.org/
// Should not be used for content not distributed with your app (May not be secure and doesn't
// support all formats fully)
#include "thirdparty/stb_image.c"

class STBI_Loader : public TBImageLoader
{
public:
	int width, height;
	unsigned char *data;

	STBI_Loader() : width(0), height(0), data(nullptr) {}
	~STBI_Loader() { stbi_image_free(data); }

	virtual int Width() { return width; }
	virtual int Height() { return height; }
	virtual uint32 *Data() { return (uint32*)data; }
};

TBImageLoader *TBImageLoader::CreateFromFile(const char *filename)
{
	// Load directly from file
	/*int w, h, comp;
	if (unsigned char *data = stbi_load(filename, &w, &h, &comp, 4))
	{
		if (STBI_Loader *img = new STBI_Loader())
		{
			img->width = w;
			img->height = h;
			img->data = data;
			return img;
		}
		else
			stbi_image_free(data);
	}
	return nullptr;*/
	if (TBFile *file = TBFile::Open(filename, TBFile::MODE_READ))
	{
		long size = file->Size();
		if (unsigned char *data = new unsigned char[size])
		{
			size = file->Read(data, 1, size);

			int w, h, comp;
			if (unsigned char *img_data = stbi_load_from_memory(data, size, &w, &h, &comp, 4))
			{
				if (STBI_Loader *img = new STBI_Loader())
				{
					img->width = w;
					img->height = h;
					img->data = img_data;
					delete [] data;
					delete file;
					return img;
				}
				else
					stbi_image_free(img_data);
			}

			delete data;
		}
		delete file;
	}
	return nullptr;
}

}; // namespace tb
