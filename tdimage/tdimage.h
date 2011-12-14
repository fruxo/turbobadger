#ifndef TD_IMAGE_H
#define TD_IMAGE_H

class TdImage
{
public:
	int width, height, bits_per_pixel;
	unsigned char *data;
public:
	TdImage() : width(0), height(0), bits_per_pixel(0), data(0) {}
	~TdImage() { MakeEmpty(); }

	void MakeEmpty() { delete [] data; data = 0; width = height = bits_per_pixel = 0; }

	bool NewData(int new_width, int new_height);
	bool LoadPNG(const char *filename);
	bool SavePNG(const char *filename);

	static TdImage *Create(int width, int height);
	static TdImage *CreateFromFile(const char *filename);
};

#endif // TD_IMAGE_H
