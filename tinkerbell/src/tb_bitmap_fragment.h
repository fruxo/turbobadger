// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_BITMAP_FRAGMENT_H
#define TB_BITMAP_FRAGMENT_H

#include "tinkerbell.h"
#include "tb_hashtable.h"
#include "tb_list.h"

namespace tinkerbell {

class TBBitmapFragment;
class TBBitmap;

/** Return the nearest power of two from val.
	F.ex 110 -> 128, 256->256, 257->512 etc. */
int TBGetNearestPowerOfTwo(int val);

/** TBImageloader is a class used to load skin images. It can be implemented
	in any way the system wants, but the system has to provide a image loader
	capable of handling all image formats used in the skin. */
class TBImageLoader
{
public:
	/** Static method used to create an image loader. The system must implement this
		function and create an implementation of the TBImageLoader interface. */
	static TBImageLoader *CreateFromFile(const char *filename);

	virtual ~TBImageLoader() {}

	/** Return the width of the loaded bitmap. */
	virtual int Width() = 0;

	/** Return the height of the loaded bitmap. */
	virtual int Height() = 0;

	/** Return the data of the loaded bitmap.
		This data should always be in 32bit RGBA format. */
	virtual uint32 *Data() = 0;
};

/** TBBitmapFragmentMap is used to pack multiple bitmaps into a single TBBitmap.
	When initialized (in a size suitable for a TBBitmap) is also creates a software buffer
	that will make up the TBBitmap when all fragments have been added. */
class TBBitmapFragmentMap
{
public:
	TBBitmapFragmentMap();
	~TBBitmapFragmentMap();

	/** Initialize the map with the given size. The size should be a power of two since
		it will be used to create a TBBitmap (texture memory). */
	bool Init(int bitmap_w, int bitmap_h);

	/** Create a new fragment with the given size and data in this map.
		Returns nullptr if there is not enough room in this map or on any other fail. */
	TBBitmapFragment *CreateNewFragment(int frag_w, int frag_h, uint32 *frag_data, bool add_border);

	/** Return the bitmap for this map. If the data is not not valid (up to date),
		it will be updated before returning. */
	TBBitmap *GetBitmap();
private:
	friend class TBBitmapFragmentManager;
	bool ValidateBitmap();
	void CopyData(TBBitmapFragment *frag, uint32 *frag_data, int border);
	struct ROW {
		int y, height, used_width;
	};
	TBListAutoDeleteOf<ROW> rows;
	int m_bitmap_w, m_bitmap_h;
	uint32 *m_bitmap_data;
	TBBitmap *m_bitmap;
	bool m_need_update;
};

/** TBBitmapFragment represents a sub part of a TBBitmap.
	It's owned by TBBitmapFragmentManager which pack multiple
	TBBitmapFragment within TBBitmaps to reduce texture switching. */
class TBBitmapFragment
{
public:
	int Width() { return m_rect.w; }
	int Height() { return m_rect.h; }
	TBBitmap *GetBitmap() { return m_map->GetBitmap(); }
public:
	TBBitmapFragmentMap *m_map;
	TBRect m_rect;
};

/** TBBitmapFragmentManager manages loading bitmaps of arbitrary size,
	pack as many of them into as few TBBitmap as possible.

	It also makes sure that only one instance of each file is loaded,
	so f.ex loading "foo.png" many times still load and allocate one
	TBBitmapFragment. */
class TBBitmapFragmentManager
{
public:
	~TBBitmapFragmentManager();

	/** Get the fragment with the given image filename. If it's not already loaded,
		it will be loaded into a new fragment with the filename as id.
		returns nullptr on fail. */
	TBBitmapFragment *GetFragmentFromFile(const char *filename, bool dedicated_map);

	/** Get the fragment with the given id, or nullptr if it doesn't exist. */
	TBBitmapFragment *GetFragment(const TBID &id) const;

	/** Create a new fragment from the given data.
		@param id The id that should be used to identify the fragment.
		@param dedicated_map if true, it will get a dedicated map.
		@param data_w the width of the data.
		@param data_h the height of the data.
		@param data pointer to the data in BGRA32 format.
		@param add_border if true, a 1px border will be added if needed so stretching won't get filtering artifacts. */
	TBBitmapFragment *CreateNewFragment(const TBID &id, bool dedicated_map, int data_w, int data_h, uint32 *data, bool add_border);

	/** Clear all loaded bitmaps and all created bitmap fragments and maps.
		After this call, do not keep any pointers to any TBBitmapFragment created
		by this fragment manager. */
	void Clear();

	/** Validate bitmaps on fragment maps that has changed. */
	bool ValidateBitmaps();

	/** Get number of fragment maps that is currently used. */
	int GetNumMaps() const { return m_fragment_maps.GetNumItems(); }
#ifdef _DEBUG
	/** Render the maps on screen, to analyze fragment positioning. */
	void Debug();
#endif
private:
	TBListOf<TBBitmapFragmentMap> m_fragment_maps;
	TBHashTableOf<TBBitmapFragment> m_fragments;
};

}; // namespace tinkerbell

#endif // TB_BITMAP_FRAGMENT_H
