// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_bitmap_fragment.h"
#include "tb_system.h"
#include "tb_tempbuffer.h"

#ifdef TB_IMAGE_LOADER_STB

// Configure stb image and remove some features we don't use to reduce binary size.
//#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
//#define STBI_SIMD
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_HDR
#define STBI_NO_GIF

// Disable unused function warnings for stb_image.h. Since STB_IMAGE_STATIC is
// defined, it will contain a couple of unused static functions.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// Include stb image - Tiny portable and reasonable fast image loader from http://nothings.org/
// Should not be used for content not distributed with your app (May not be secure and doesn't
// support all formats fully)
#include "thirdparty/stb_image.h"

#define NANOSVG_IMPLEMENTATION
#include "thirdparty/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "thirdparty/nanosvgrast.h"

#pragma GCC diagnostic pop

namespace tb {

class STBI_Loader : public TBImageLoader
{
public:
	int width, height;
	unsigned char *data;

	STBI_Loader() : width(0), height(0), data(nullptr) {}
	~STBI_Loader() { stbi_image_free(data); }

	virtual int Width() { return width; }
	virtual int Height() { return height; }
	virtual uint32_t *Data() { return (uint32_t*)data; }
};

class NSVG_Loader : public TBImageLoader
{
public:
	int width, height;
	unsigned char *data;

	NSVG_Loader() : width(0), height(0), data(nullptr) {}
	~NSVG_Loader() { free(data); }

	virtual int Width() { return width; }
	virtual int Height() { return height; }
	virtual uint32_t *Data() { return (uint32_t*)data; }
};

TBImageLoader *TBImageLoader::CreateFromFile(const TBStr & filename, float dpi)
{
	TBTempBuffer buf;
	if (buf.AppendFile(filename))
	{
		int w, h, comp;
		if (strstr((const char *)filename, ".svg")) {
			if (NSVGimage * image = nsvgParse((char *)buf.GetData(), "px", dpi)) {
				struct NSVGrasterizer * rast = nsvgCreateRasterizer();
				unsigned char * img_data = (unsigned char *)malloc(image->width * image->height * 4);
				nsvgRasterize(rast, image, 0,0,1, img_data, image->width, image->height, image->width * 4);
				nsvgDeleteRasterizer(rast);
				nsvgDelete(image);
				if (NSVG_Loader * img = new NSVG_Loader()) {
					img->width = image->width;
					img->height = image->height;
					img->data = img_data;
					return img;
				}
				else
					free(img_data);
			}
		}
		else if (unsigned char *img_data = stbi_load_from_memory(
			(unsigned char*) buf.GetData(), buf.GetAppendPos(), &w, &h, &comp, 4))
		{
			if (STBI_Loader *img = new STBI_Loader())
			{
				img->width = w;
				img->height = h;
				img->data = img_data;
				return img;
			}
			else
				stbi_image_free(img_data);
		}
	}
	return nullptr;
}

} // namespace tb

#endif // TB_IMAGE_LOADER_STB
