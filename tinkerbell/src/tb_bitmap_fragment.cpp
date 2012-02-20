// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_bitmap_fragment.h"
#include "tb_renderer.h"
#include "tb_system.h"

namespace tinkerbell {

const int default_frag_map_w = 512;
const int default_frag_map_h = 512;

int TBGetNearestPowerOfTwo(int val)
{
	int i;
	for(i = 31; i >= 0; i--)
		if ((val - 1) & (1<<i))
			break;
	return (1<<(i + 1));
}

// == TBBitmapFragmentMap ===================================================================================

TBBitmapFragmentMap::TBBitmapFragmentMap()
	: m_bitmap_w(0)
	, m_bitmap_h(0)
	, m_bitmap_data(nullptr)
	, m_bitmap(nullptr)
{
}

bool TBBitmapFragmentMap::Init(int bitmap_w, int bitmap_h)
{
	m_bitmap_data = new uint32[bitmap_w * bitmap_h];
	m_bitmap_w = bitmap_w;
	m_bitmap_h = bitmap_h;
	if (m_bitmap_data)
#ifdef _DEBUG
		memset(m_bitmap_data, 0xFF, bitmap_w * bitmap_h * sizeof(uint32));
#else
		memset(m_bitmap_data, 0x00, bitmap_w * bitmap_h * sizeof(uint32));
#endif
	return m_bitmap_data ? true : false;
}

TBBitmapFragmentMap::~TBBitmapFragmentMap()
{
	delete m_bitmap;
	delete [] m_bitmap_data;
}

TBBitmapFragment *TBBitmapFragmentMap::CreateNewFragment(int frag_w, int frag_h, uint32 *frag_data)
{
	// Finding available space works like this:
	// The map size is sliced up horizontally in rows (initially just one row covering
	// the entire map). When adding a new fragment, put it in the row with smallest height.
	// If the smallest row is empty, it may slice the row to make a even smaller row.

	// When a image is stretched up to a larger size, the filtering will read
	// pixels closest (but outside) of the src_rect. When we pack images together
	// those pixels would be read from neighbour images, so we must add border space
	// around each image to avoid artifacts. We must also fill in that border with
	// the "clamp" of the image itself so we don't get any filtering artifacts at all.
	// Allways add border except when we're using the entire map for one fragment.
	int border = 0;
	int needed_w = frag_w;
	int needed_h = frag_h;
	if (needed_w != m_bitmap_w || needed_h != m_bitmap_h)
	{
		border = 1;
		needed_w += 2;
		needed_h += 2;
	}

	if (!rows.GetNumItems())
	{
		// Create a row covering the entire bitmap.
		ROW *row;
		if (!rows.GrowIfNeeded() || !(row = new ROW))
			return nullptr;
		row->y = 0;
		row->height = m_bitmap_h;
		row->used_width = 0;
		rows.Add(row);
	}
	// Get the smallest row where we fit
	ROW *best_row = nullptr;
	for (int i = 0; i < rows.GetNumItems(); i++)
	{
		ROW *row = rows[i];
		if (!best_row || row->height < best_row->height)
		{
			// This is the best row so far, if we fit
			if (needed_h <= row->height && needed_w <= m_bitmap_w - row->used_width)
				best_row = row;
		}
	}
	// Return if we're full
	if (!best_row)
		return nullptr;
	// If the row is unused, create a smaller row to only consume needed height for fragment
	if (best_row->used_width == 0 && needed_h < best_row->height)
	{
		ROW *row;
		if (!rows.GrowIfNeeded() || !(row = new ROW))
			return nullptr;
		row->y = best_row->y + needed_h;
		row->height = best_row->height - needed_h;
		row->used_width = 0;
		rows.Add(row);
		best_row->height = needed_h;
	}
	// Allocate the fragment and copy the fragment data into the map data.
	if (TBBitmapFragment *frag = new TBBitmapFragment)
	{
		frag->m_map = this;
		frag->m_rect.Set(best_row->used_width + border, best_row->y + border, frag_w, frag_h);
		best_row->used_width += needed_w;
		CopyData(frag, frag_data, border);
		return frag;
	}
	return nullptr;
}

void TBBitmapFragmentMap::CopyData(TBBitmapFragment *frag, uint32 *frag_data, int border)
{
	// Copy the bitmap data
	uint32 *dst = m_bitmap_data + frag->m_rect.x + frag->m_rect.y * m_bitmap_w;
	uint32 *src = frag_data;
	for (int i = 0; i < frag->m_rect.h; i++)
	{
		memcpy(dst, src, frag->m_rect.w * sizeof(uint32));
		dst += m_bitmap_w;
		src += frag->m_rect.w;
	}
	// Copy the bitmap data to the border around the fragment
	if (border)
	{
		TBRect rect = frag->m_rect.Expand(border, border);
		// Copy vertical edges
		dst = m_bitmap_data + rect.x + (rect.y + 1) * m_bitmap_w;
		src = frag_data;
		for (int i = 0; i < frag->m_rect.h; i++)
		{
			dst[0] = src[0];
			dst[rect.w - 1] = src[frag->m_rect.w - 1];
			dst += m_bitmap_w;
			src += frag->m_rect.w;
		}
		// Copy horizontal edges
		memcpy(m_bitmap_data + rect.x + 1 + rect.y * m_bitmap_w, frag_data, frag->m_rect.w * sizeof(uint32));
		memcpy(m_bitmap_data + rect.x + 1 + (rect.y + rect.h - 1) * m_bitmap_w, frag_data + (frag->m_rect.h - 1) * frag->m_rect.w, frag->m_rect.w * sizeof(uint32));
	}
}

bool TBBitmapFragmentMap::CreateBitmap()
{
	if (!m_bitmap)
		m_bitmap = g_renderer->CreateBitmap(m_bitmap_w, m_bitmap_h, m_bitmap_data);
	return m_bitmap ? true : false;
}

// == TBBitmapFragmentManager =============================================================================

TBBitmapFragmentManager::~TBBitmapFragmentManager()
{
	Clear();
}

TBBitmapFragment *TBBitmapFragmentManager::CreateNewBitmapFragment(const char *filename, bool dedicated_map)
{
	TBBitmapFragment *frag = nullptr;
	// If we already have a fragment for this filename, return that
	if (!dedicated_map)
	{
		frag = m_fragments.Get(TBGetHash(filename));
		if (frag)
			return frag;
	}
	// Load the file
	TBImageLoader *img = TBImageLoader::CreateFromFile(filename);
	if (!img)
	{
		TBDebugOut("CreateNewBitmapFragment: Failed to load bitmap!");
		return nullptr;
	}
	// Create a fragment in any of the fragment maps. Doing it in the reverse order
	// would be faster since it's most likely to succeed, but we want to maximize
	// the amount of fragments per map, so do it in the creation order.
	if (!dedicated_map)
	{
		for (int i = 0; i < m_fragment_maps.GetNumItems(); i++)
		{
			if (frag = m_fragment_maps[i]->CreateNewFragment(img->Width(), img->Height(), img->Data()))
				break;
		}
	}
	// If we couldn't create the fragment in any map, create a new map where we know it will fit.
	if (!frag)
	{
		int po2w = TBGetNearestPowerOfTwo(MAX(img->Width(), default_frag_map_w));
		int po2h = TBGetNearestPowerOfTwo(MAX(img->Height(), default_frag_map_h));
		if (dedicated_map)
		{
			po2w = TBGetNearestPowerOfTwo(img->Width());
			po2h = TBGetNearestPowerOfTwo(img->Height());
		}
		TBBitmapFragmentMap *fm = new TBBitmapFragmentMap();
		if (fm && fm->Init(po2w, po2h))
		{
			m_fragment_maps.Add(fm);
			frag = fm->CreateNewFragment(img->Width(), img->Height(), img->Data());
		}
		else
			delete fm;
	}
	// Finally, add the new fragment to the hash and delete the img since it's not copied into the map.
	if (frag)
		m_fragments.Add(TBGetHash(filename), frag);
	delete img;
	return frag;
}

void TBBitmapFragmentManager::Clear()
{
	m_fragment_maps.DeleteAll();
	m_fragments.DeleteAll();
}

bool TBBitmapFragmentManager::CreateBitmaps()
{
	bool success = true;
	for (int i = 0; i < m_fragment_maps.GetNumItems(); i++)
		if (!m_fragment_maps[i]->CreateBitmap())
			success = false;
	return success;
}

#ifdef _DEBUG
void TBBitmapFragmentManager::Debug()
{
	int x = 0;
	for (int i = 0; i < m_fragment_maps.GetNumItems(); i++)
	{
		TBBitmapFragmentMap *fm = m_fragment_maps[i];
		g_renderer->DrawBitmap(TBRect(x, 0, fm->m_bitmap_w, fm->m_bitmap_h), TBRect(0, 0, fm->m_bitmap_w, fm->m_bitmap_h), fm->m_bitmap);
		x += fm->m_bitmap_w + 5;
	}
}
#endif // _DEBUG

}; // namespace tinkerbell
