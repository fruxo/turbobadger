// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_font_renderer.h"
#include "tb_renderer.h"
#include "tb_system.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

int num_fonts = 0;
bool ft_initialized = false;
static FT_Library g_freetype = nullptr;

using namespace tinkerbell;

/** FreetypeFontRenderer renders fonts using the freetype library. */
class FreetypeFontRenderer : public TBFontRenderer
{
public:
	FreetypeFontRenderer();
	~FreetypeFontRenderer();

	bool Load(const char *filename, int size);

	virtual TBFontFace *Create(const char *filename, int size);

	virtual TBFontMetrics GetMetrics();
	virtual bool RenderGlyph(TBFontGlyphData *dst_bitmap, UCS4 cp);
	virtual void GetGlyphMetrics(TBGlyphMetrics *metrics, UCS4 cp);
private:
	unsigned char *ttf_buffer;
	FT_Size m_size;
	FT_Face m_face;
};

FreetypeFontRenderer::FreetypeFontRenderer()
	: ttf_buffer(nullptr)
	, m_size(nullptr)
	, m_face(nullptr)
{
	num_fonts++;
}

FreetypeFontRenderer::~FreetypeFontRenderer()
{
	FT_Done_Size(m_size);
	FT_Done_Face(m_face);

	num_fonts--;
	if (num_fonts == 0 && ft_initialized)
	{
		FT_Done_FreeType(g_freetype);
		ft_initialized = false;
	}

	delete [] ttf_buffer;
}

TBFontMetrics FreetypeFontRenderer::GetMetrics()
{
	TBFontMetrics metrics;
	metrics.ascent = m_size->metrics.ascender >> 6;
	metrics.descent = -(m_size->metrics.descender >> 6);
	metrics.height = m_size->metrics.height >> 6;
	return metrics;
}

bool FreetypeFontRenderer::RenderGlyph(TBFontGlyphData *data, UCS4 cp)
{
	FT_GlyphSlot slot = m_face->glyph;
	if (FT_Load_Char(m_face, cp, FT_LOAD_RENDER) ||
		slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
		return false;
	data->w = slot->bitmap.width;
	data->h = slot->bitmap.rows;
	data->stride = slot->bitmap.pitch;
	data->data8 = slot->bitmap.buffer;
	return data->data8 ? true : false;
}

void FreetypeFontRenderer::GetGlyphMetrics(TBGlyphMetrics *metrics, UCS4 cp)
{
	FT_GlyphSlot slot = m_face->glyph;
	if (FT_Load_Char(m_face, cp, FT_LOAD_RENDER))
		return;
	metrics->advance = slot->advance.x >> 6;
	metrics->x = slot->bitmap_left;
	metrics->y = - slot->bitmap_top;
}

bool FreetypeFontRenderer::Load(const char *filename, int size)
{
	if (!ft_initialized)
		ft_initialized = !FT_Init_FreeType(&g_freetype);
	if (!ft_initialized)
		return false;

	TBFile *f = TBFile::Open(filename, TBFile::MODE_READ);
	if (!f)
		return false;

	size_t ttf_buf_size = f->Size();
	ttf_buffer = new unsigned char[ttf_buf_size];
	if (ttf_buffer)
		ttf_buf_size = f->Read(ttf_buffer, 1, ttf_buf_size);
	delete f;

	if (!ttf_buffer)
		return false;

	if (FT_New_Memory_Face(g_freetype, ttf_buffer, ttf_buf_size, 0, &m_face))
		return false;
	if (FT_New_Size(m_face, &m_size) ||
		FT_Activate_Size(m_size) ||
		FT_Set_Pixel_Sizes(m_face, 0, size))
		return false;
	return true;
}

TBFontFace *FreetypeFontRenderer::Create(const char *filename, int size)
{
	if (FreetypeFontRenderer *fr = new FreetypeFontRenderer())
	{
		if (fr->Load(filename, size))
			if (TBFontFace *font = new TBFontFace(fr, size))
				return font;
		delete fr;
	}
	return nullptr;
}

void register_freetype_font_renderer()
{
	if (FreetypeFontRenderer *fr = new FreetypeFontRenderer)
		g_font_manager->AddRenderer(fr);
}
