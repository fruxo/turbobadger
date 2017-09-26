// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_font_renderer.h"
#include "tb_renderer.h"
#include "tb_system.h"

#ifdef TB_FONT_RENDERER_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include FT_STROKER_H

int num_fonts = 0;
bool ft_initialized = false;
static FT_Library g_freetype = nullptr;

using namespace tb;

/** Cache of truetype file data, so it isn't loaded multiple times for each font size */

class FreetypeFace;
static TBHashTableOf<FreetypeFace> ft_face_cache;

class FreetypeFace
{
public:
	FreetypeFace() : hashID(0), f_face(0), refCount(1), outline(false) { }
	~FreetypeFace()
	{
		if (hashID)
			ft_face_cache.Remove(hashID);
		FT_Done_Face(f_face);
	}
	void Release()
	{
		--refCount;
		if (!refCount)
			delete this;
	}

	uint32 hashID;
	TBTempBuffer ttf_buffer;
	FT_Face f_face;
	unsigned int refCount;
	bool outline;
};


/** FreetypeFontRenderer renders fonts using the freetype library. */
class FreetypeFontRenderer : public TBFontRenderer
{
public:
	FreetypeFontRenderer();
	~FreetypeFontRenderer();

	virtual TBFontFace *Create(TBFontManager *font_manager, const char *filename,
								const TBFontDescription &font_desc);

	virtual TBFontMetrics GetMetrics();
	virtual bool RenderGlyph(TBFontGlyphData *dst_bitmap, UCS4 cp, const TBColor &color);
	virtual void GetGlyphMetrics(TBGlyphMetrics *metrics, UCS4 cp);
private:
	bool Load(FreetypeFace *face, const TBFontDescription &font_desc);
	bool Load(const char *filename, const TBFontDescription &font_desc);

	FT_Size m_size;
	FreetypeFace *m_face;
	TBColor m_data[1024]; // 32x32
	static const int maxcw = 32;
};

FreetypeFontRenderer::FreetypeFontRenderer()
	: m_size(nullptr)
	, m_face(nullptr)
{
	num_fonts++;
}

FreetypeFontRenderer::~FreetypeFontRenderer()
{
	FT_Done_Size(m_size);
	if (m_face)
		m_face->Release();

	num_fonts--;
	if (num_fonts == 0 && ft_initialized)
	{
		FT_Done_FreeType(g_freetype);
		ft_initialized = false;
	}
}

TBFontMetrics FreetypeFontRenderer::GetMetrics()
{
	TBFontMetrics metrics;
	metrics.ascent = (int16) (m_size->metrics.ascender >> 6);
	metrics.descent = (int16) -(m_size->metrics.descender >> 6);
	metrics.height = (int16) (m_size->metrics.height >> 6);
	return metrics;
}

bool FreetypeFontRenderer::RenderGlyph(TBFontGlyphData *data, UCS4 cp, const TBColor & /*color*/)
{
	FT_Activate_Size(m_size);
	if (!m_face->outline) {
		FT_GlyphSlot slot = m_face->f_face->glyph;
		if (FT_Load_Char(m_face->f_face, cp, FT_LOAD_RENDER) ||
			slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
			return false;
		data->w = slot->bitmap.width;
		data->h = slot->bitmap.rows;
		data->stride = slot->bitmap.pitch;
		data->data8 = slot->bitmap.buffer;
		return data->data8 ? true : false;
	}
#if 0
#define FTC(CALL) do {													\
		FT_Error err = CALL;											\
		if (err) TBDebugPrint("Error %s:%d = %d\n", __FILE__, __LINE__, err); \
		/*else TBDebugPrint("OK %s:%d\n", __FILE__, __LINE__);	*/		\
	} while(0)

	else {
		FT_Face face = m_face->f_face;
		memset(&m_data[0], 0, sizeof(m_data));
		// initialize stroker, so you can create outline font
		FT_Stroker stroker;
		FTC(FT_Stroker_New(g_freetype, &stroker));
		//  2 * 64 result in 2px outline
		FT_Stroker_Set(stroker, 1 * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
		//...
		// generation of an outline for single glyph:
		FT_UInt glyphIndex = FT_Get_Char_Index(face, cp);
		FTC(FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP));
		float bearingX = face->glyph->metrics.horiBearingX / 64.f;
		float bearingY = face->glyph->metrics.horiBearingY / 64.f;
		float advance = face->glyph->advance.x / 64.f;
		assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);
		FT_Glyph glyph;
		FTC(FT_Get_Glyph(face->glyph, &glyph));
		FTC(FT_Glyph_StrokeBorder(&glyph, stroker, false, true));
		//FTC(FT_Glyph_Stroke(&glyph, stroker, true));
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
			FTC(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true));
		FT_BitmapGlyph slot = reinterpret_cast<FT_BitmapGlyph>(glyph);
		assert(glyph->format == FT_GLYPH_FORMAT_BITMAP);
		//assert(slot->bitmap.rows);
		// blit the outline
		TBDebugPrint("%d x %d, l:%d t:%d stride:%d\n",
					 slot->bitmap.width, slot->bitmap.rows,
					 slot->left, slot->top, slot->bitmap.pitch);
		int stride = Max((int)advance, slot->bitmap.pitch) + 1;
		TBColor outlineCol(255 - color.r, 255 - color.g, 255 - color.b);
		for (unsigned int rr = 0; rr < slot->bitmap.rows; rr++)
			for (unsigned int cc = 0; cc < slot->bitmap.width; cc++) {
				auto v = slot->bitmap.buffer[rr * slot->bitmap.pitch + cc];
				if (v) {
					int ix = rr * stride + cc + slot->left + 1;
					m_data[ix] = outlineCol;
					m_data[ix].a = v;
				}
			}
		FT_Done_Glyph(glyph);
		// generate the character
		FTC(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT));
		FTC(FT_Get_Glyph(face->glyph, &glyph));
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
			FTC(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true));
		slot = reinterpret_cast<FT_BitmapGlyph>(glyph);
		//assert(slot->bitmap.rows);
		// blit the text
		TBDebugPrint("%d x %d, l:%d t:%d stride:%d / %d\n",
					 slot->bitmap.width, slot->bitmap.rows,
					 slot->left, slot->top, slot->bitmap.pitch, stride);
#if 1
		for (unsigned int rr = 0; rr < slot->bitmap.rows; rr++)
			for (unsigned int cc = 0; cc < slot->bitmap.width; cc++) {
				auto v = slot->bitmap.buffer[rr * slot->bitmap.pitch + cc];
				if (v) {
					int ix = rr * stride + cc + slot->left + 1;
					m_data[ix] = color;
					m_data[ix].a = v;
				}
			}
#endif
		data->w = stride;
		data->h = slot->bitmap.rows;
		data->stride = stride;
		data->rgb = true;
		data->data32 = (unsigned int *)&m_data[0];
		// done
		FT_Done_Glyph(glyph);
		FT_Stroker_Done(stroker);
		return true;
	}
#else
	return false;
#endif
}

void FreetypeFontRenderer::GetGlyphMetrics(TBGlyphMetrics *metrics, UCS4 cp)
{
	FT_Activate_Size(m_size);
	FT_GlyphSlot slot = m_face->f_face->glyph;
	if (FT_Load_Char(m_face->f_face, cp, FT_LOAD_RENDER))
		return;
	metrics->advance = (int16) (slot->advance.x >> 6);
	metrics->x = slot->bitmap_left;
	metrics->y = - slot->bitmap_top;
}

bool FreetypeFontRenderer::Load(FreetypeFace *face, const TBFontDescription &font_desc)
{
	int size = font_desc.GetSize();
	// Should not be possible to have a face if freetype is not initialized
	assert(ft_initialized);
	m_face = face;
	m_face->outline = font_desc.GetOutline();
	if (FT_New_Size(m_face->f_face, &m_size) ||
		FT_Activate_Size(m_size) ||
		FT_Set_Pixel_Sizes(m_face->f_face, 0, size))
		return false;
	return true;
}

bool FreetypeFontRenderer::Load(const char *filename, const TBFontDescription &font_desc)
{
	if (!ft_initialized)
		ft_initialized = !FT_Init_FreeType(&g_freetype);
	if (!ft_initialized)
		return false;

	m_face = new FreetypeFace();
	if (!m_face)
		return false;

	if (!m_face->ttf_buffer.AppendFile(filename))
		return false;

	unsigned char *ttf_ptr = (unsigned char *) m_face->ttf_buffer.GetData();
	if (FT_New_Memory_Face(g_freetype, ttf_ptr, m_face->ttf_buffer.GetAppendPos(), 0, &m_face->f_face))
		return false;
	return Load(m_face, font_desc);
}

TBFontFace *FreetypeFontRenderer::Create(TBFontManager *font_manager, const char *filename, const TBFontDescription &font_desc)
{
	if (FreetypeFontRenderer *fr = new FreetypeFontRenderer())
	{
		TBID face_cache_id(filename);
		FreetypeFace *f = ft_face_cache.Get(face_cache_id);
		if (f)
		{
			++f->refCount;
			if (fr->Load(f, font_desc))
				if (TBFontFace *font = new TBFontFace(font_manager->GetGlyphCache(), fr, font_desc))
					return font;
		}
		else if (fr->Load(filename, font_desc))
		{
			if (ft_face_cache.Add(face_cache_id, fr->m_face))
				fr->m_face->hashID = face_cache_id;
			if (TBFontFace *font = new TBFontFace(font_manager->GetGlyphCache(), fr, font_desc))
				return font;
		}

		delete fr;
	}
	return nullptr;
}

void register_freetype_font_renderer()
{
	if (FreetypeFontRenderer *fr = new FreetypeFontRenderer)
		g_font_manager->AddRenderer(fr);
}

#endif // TB_FONT_RENDERER_FREETYPE
