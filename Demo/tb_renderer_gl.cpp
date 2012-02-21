// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include <assert.h>
#include <stdio.h>
#include "tb_renderer_gl.h"
#include "tb_bitmap_fragment.h"

#include "tdfont/tdfont.h"
//// TEMP HACK
extern TdFont *g_uifont;
extern TdFont *g_uisymbolfont;

namespace tinkerbell {

// == Batching ====================================================================================

#define VERTEX_BATCH_SIZE 3 * 256

#define VER_COL(r, g, b, a) (((a)<<24) + ((b)<<16) + ((g)<<8) + r)
#define VER_COL_OPACITY(a) (0x00ffffff + (((uint32)a) << 24))

struct Vertex {
	float x, y;
	float u, v;
	union {
		unsigned char r, g, b, a;
		uint32 col;
	};
};

class Batch
{
public:
	Batch() : vertex_count(0) {}
	void Flush();
	void AddVertex(float x, float y, float u, float v, uint32 color);
//private:
	Vertex vertex[VERTEX_BATCH_SIZE];
	int vertex_count;
	//GLuint m_current_texture;
};

Batch batch; ///< The one and only batch. (this should be improved)

void Batch::Flush()
{
	if (!vertex_count)
		return;
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	vertex_count = 0;
}

void Batch::AddVertex(float x, float y, float u, float v, uint32 color)
{
	vertex[vertex_count].x = x;
	vertex[vertex_count].y = y;
	vertex[vertex_count].u = u;
	vertex[vertex_count].v = v;
	vertex[vertex_count].col = color;
	vertex_count++;

	if (vertex_count == VERTEX_BATCH_SIZE)
		Flush();
}

// == TBBitmapGL ==================================================================================

TBBitmapGL::TBBitmapGL()
	: m_w(0), m_h(0), m_texture(0)
{
}

TBBitmapGL::~TBBitmapGL()
{
	glDeleteTextures(1, &m_texture);
}

bool TBBitmapGL::Init(int width, int height, uint32 *data)
{
	assert(width = TBGetNearestPowerOfTwo(width));
	assert(height = TBGetNearestPowerOfTwo(height));

	m_w = width;
	m_h = height;

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

// == TBRendererGL ================================================================================

TBRendererGL::TBRendererGL()
	: m_opacity(255), m_translation_x(0), m_translation_y(0), m_current_texture(0)
{
}

void TBRendererGL::BeginDrawString(TdFont *font)
{
}

void TBRendererGL::EndDrawString(TdFont *font)
{
}

void TBRendererGL::DrawGlyph(float x, float y, TDFNT_GLYPH *glyph, unsigned int texture)
{
	float u = glyph->u / (float)glyph->iw;
	float v = glyph->v / (float)glyph->ih;
	float uu = (glyph->u + glyph->w) / (float)glyph->iw;
	float vv = (glyph->v + glyph->h) / (float)glyph->ih;
	DrawTexture(TBRect(x, y, glyph->w, glyph->h), u, v, uu, vv, texture);
}

void TBRendererGL::BeginPaint(int render_target_w, int render_target_h)
{
	m_screen_rect.Set(0, 0, render_target_w, render_target_h);
	m_clip_rect = m_screen_rect;
	m_current_texture = 0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_screen_rect.w, m_screen_rect.h, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, m_screen_rect.w, m_screen_rect.h);
	glScissor(0, 0, m_screen_rect.w, m_screen_rect.h);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void *) &batch.vertex[0].r);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &batch.vertex[0].u);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &batch.vertex[0].x);

	TdFontRenderer::SetRendererBackend(this);
}

void TBRendererGL::EndPaint()
{
	batch.Flush();
}

void TBRendererGL::Translate(int dx, int dy)
{
	m_translation_x += dx;
	m_translation_y += dy;
}

void TBRendererGL::SetOpacity(float opacity)
{
	int8 opacity8 = (uint8) (opacity * 255);
	if (opacity8 == m_opacity)
		return;
	m_opacity = opacity8;
}

float TBRendererGL::GetOpacity()
{
	return m_opacity / 255.f;
}

TBRect TBRendererGL::SetClipRect(const TBRect &rect, bool add_to_current)
{
	TBRect old_clip_rect = m_clip_rect;
	m_clip_rect = rect;
	m_clip_rect.x += m_translation_x;
	m_clip_rect.y += m_translation_y;

	if (add_to_current)
		m_clip_rect = m_clip_rect.Clip(old_clip_rect);

	batch.Flush();
	glScissor(m_clip_rect.x, m_screen_rect.h - (m_clip_rect.y + m_clip_rect.h), m_clip_rect.w, m_clip_rect.h);

	old_clip_rect.x -= m_translation_x;
	old_clip_rect.y -= m_translation_y;
	return old_clip_rect;
}

TBRect TBRendererGL::GetClipRect()
{
	TBRect curr_clip_rect = m_clip_rect;
	curr_clip_rect.x -= m_translation_x;
	curr_clip_rect.y -= m_translation_y;
	return curr_clip_rect;
}

void TBRendererGL::DrawTexture(const TBRect &dst_rect, float u, float v, float uu, float vv, GLuint texture)
{
	DrawTexture(dst_rect, u, v, uu, vv, texture, VER_COL_OPACITY(m_opacity));
}

void TBRendererGL::DrawTexture(const TBRect &dst_rect, float u, float v, float uu, float vv, GLuint texture, uint32 color)
{
	if (texture != m_current_texture)
	{
		batch.Flush();
		m_current_texture = texture;
		glBindTexture(GL_TEXTURE_2D, m_current_texture);
	}
	int dst_x = dst_rect.x + m_translation_x;
	int dst_y = dst_rect.y + m_translation_y;

	batch.AddVertex(dst_x, dst_y + dst_rect.h, u, vv, color);
	batch.AddVertex(dst_x + dst_rect.w, dst_y + dst_rect.h, uu, vv, color);
	batch.AddVertex(dst_x, dst_y, u, v, color);

	batch.AddVertex(dst_x, dst_y, u, v, color);
	batch.AddVertex(dst_x + dst_rect.w, dst_y + dst_rect.h, uu, vv, color);
	batch.AddVertex(dst_x + dst_rect.w, dst_y, uu, v, color);
}

void TBRendererGL::DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmapFragment *bitmap_fragment)
{
	if (bitmap_fragment->m_map->GetBitmap())
		DrawBitmap(dst_rect, src_rect.Offset(bitmap_fragment->m_rect.x, bitmap_fragment->m_rect.y), bitmap_fragment->GetBitmap());
}

void TBRendererGL::DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmap *bitmap)
{
	TBBitmapGL *b = (TBBitmapGL *) bitmap;
	float u = (float)src_rect.x / b->m_w;
	float v = (float)src_rect.y / b->m_h;
	float uu = (float)(src_rect.x + src_rect.w) / b->m_w;
	float vv = (float)(src_rect.y + src_rect.h) / b->m_h;
	DrawTexture(dst_rect, u, v, uu, vv, b->m_texture);
}

void TBRendererGL::DrawBitmapTile(const TBRect &dst_rect, TBBitmap *bitmap)
{
	TBBitmapGL *b = (TBBitmapGL *) bitmap;
	float uu = (float)(dst_rect.w) / b->m_w;
	float vv = (float)(dst_rect.h) / b->m_h;
	DrawTexture(dst_rect, 0, 0, uu, vv, b->m_texture);
}

void TBRendererGL::DrawRect(const TBRect &dst_rect, const TBColor &color)
{
	if (dst_rect.IsEmpty())
		return;

	// FIX: Optimize this!
	batch.Flush();
	glDisable(GL_TEXTURE_2D);

	TBRect rect = dst_rect.Offset(m_translation_x, m_translation_y);
	uint32 col32 = VER_COL(color.r, color.g, color.b, color.a);
	batch.AddVertex(rect.x + 0.5f, rect.y + 0.5f, 0, 0, col32);
	batch.AddVertex(rect.x + rect.w - 1 + 0.5f, rect.y + 0.5f, 0, 0, col32);
	batch.AddVertex(rect.x + rect.w - 1 + 0.5f, rect.y + rect.h - 1 + 0.5f, 0, 0, col32);
	batch.AddVertex(rect.x + 0.5f, rect.y + rect.h - 1 + 0.5f, 0, 0, col32);
	batch.AddVertex(rect.x + 0.5f, rect.y + 0.5f, 0, 0, col32);

	glDrawArrays(GL_LINE_STRIP, 0, batch.vertex_count);
	batch.vertex_count = 0;
	glEnable(GL_TEXTURE_2D);
}

void TBRendererGL::DrawRectFill(const TBRect &dst_rect, const TBColor &color)
{
	if (dst_rect.IsEmpty())
		return;

	DrawTexture(dst_rect, 0, 0, 0, 0, 0, VER_COL(color.r, color.g, color.b, color.a));
}

void TBRendererGL::DrawString(int x, int y, const char *str, int len)
{
	if (!g_uifont)
		return;
	TdFontRenderer::DrawString(g_uifont, x /*+ m_translation_x*/, y /*+ m_translation_y*/, TDFNT_LEFT_TOP, str, len);
}

int TBRendererGL::GetStringWidth(const char *str, int len)
{
	if (!g_uifont)
		return 0;
	return TdFontRenderer::GetStringWidth(g_uifont, str, len);
}

int TBRendererGL::GetFontHeight()
{
	if (!g_uifont)
		return 0;
	return TdFontRenderer::GetFontHeight(g_uifont);
}

TBBitmap *TBRendererGL::CreateBitmap(int width, int height, uint32 *data)
{
	TBBitmapGL *bitmap = new TBBitmapGL;
	if (!bitmap || !bitmap->Init(width, height, data))
	{
		delete bitmap;
		return nullptr;
	}
	return bitmap;
}

}; // namespace tinkerbell
