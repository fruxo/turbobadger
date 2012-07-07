// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include <assert.h>
#include <stdio.h>
#include "tb_renderer_gl.h"
#include "tb_bitmap_fragment.h"
#include "tb_font_renderer.h"

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

GLuint g_current_texture = 0;
class Batch *g_current_batch = 0;

void BindBitmap(TBBitmap *bitmap)
{
	GLuint texture = bitmap ? static_cast<TBBitmapGL*>(bitmap)->m_texture : 0;
	if (texture != g_current_texture)
	{
		g_current_texture = texture;
		glBindTexture(GL_TEXTURE_2D, g_current_texture);
	}
}

class Batch
{
public:
	Batch() : vertex_count(0), bitmap(nullptr), fragment(nullptr), is_flushing(false) {}
	void Flush();
	void AddVertex(int x, int y, float u, float v, uint32 color);
//private:
	Vertex vertex[VERTEX_BATCH_SIZE];
	int vertex_count;

	TBBitmap *bitmap;
	TBBitmapFragment *fragment;
	bool is_flushing;
};

void Batch::Flush()
{
	if (!vertex_count || is_flushing)
		return;

	// Prevent re-entrancy. Calling fragment->GetBitmap may end up calling TBBitmap::SetData
	// which will end up flushing any existing batch with that bitmap.
	is_flushing = true;

	if (fragment)
	{
		// Now it's time to ensure the bitmap data is up to date. A call to GetBitmap
		// with TB_VALIDATE_ALWAYS should guarantee that its data is validated.
		TBBitmap *frag_bitmap = fragment->GetBitmap(TB_VALIDATE_ALWAYS);
		assert(frag_bitmap == bitmap);
	}

	// Bind texture and array pointers
	BindBitmap(bitmap);
	if (g_current_batch != this)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void *) &vertex[0].r);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &vertex[0].u);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &vertex[0].x);
		g_current_batch = this;
	}

	// Flush
	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	vertex_count = 0;

	is_flushing = false;
}

void Batch::AddVertex(int x, int y, float u, float v, uint32 color)
{
	vertex[vertex_count].x = (float)x;
	vertex[vertex_count].y = (float)y;
	vertex[vertex_count].u = u;
	vertex[vertex_count].v = v;
	vertex[vertex_count].col = color;
	vertex_count++;

	if (vertex_count == VERTEX_BATCH_SIZE)
		Flush();
}

/** Extreemly simple batcher (only maintains one batch and flushes when texture changes) */
class BatchRenderer
{
public:
	float u, v, uu, vv; ///< Some temp variables

	void AddQuad(const TBRect &dst_rect, const TBRect &src_rect, uint32 color, TBBitmap *bitmap, TBBitmapFragment *fragment)
	{
		if (batch.bitmap != bitmap)
		{
			batch.Flush();
			batch.bitmap = bitmap;
		}
		batch.fragment = fragment;
		if (TBBitmapGL *b = static_cast<TBBitmapGL*>(bitmap))
		{
			u = (float)src_rect.x / b->m_w;
			v = (float)src_rect.y / b->m_h;
			uu = (float)(src_rect.x + src_rect.w) / b->m_w;
			vv = (float)(src_rect.y + src_rect.h) / b->m_h;
		}
		batch.AddVertex(dst_rect.x, dst_rect.y + dst_rect.h, u, vv, color);
		batch.AddVertex(dst_rect.x + dst_rect.w, dst_rect.y + dst_rect.h, uu, vv, color);
		batch.AddVertex(dst_rect.x, dst_rect.y, u, v, color);

		batch.AddVertex(dst_rect.x, dst_rect.y, u, v, color);
		batch.AddVertex(dst_rect.x + dst_rect.w, dst_rect.y + dst_rect.h, uu, vv, color);
		batch.AddVertex(dst_rect.x + dst_rect.w, dst_rect.y, uu, v, color);
	}
	void Flush()
	{
		batch.Flush();
	}
	void FlushBitmap(TBBitmap *bitmap)
	{
		// Flush the batch if it's using this bitmap (that is about to change or be deleted)
		if (batch.vertex_count && bitmap == batch.bitmap)
			batch.Flush();
	}
	void FlushBitmapFragment(TBBitmapFragment *bitmap_fragment)
	{
		// Flush the batch if it's using this fragment (that is about to change or be deleted)
		if (batch.vertex_count && bitmap_fragment == batch.fragment)
			batch.Flush();
	}
private:
	Batch batch; ///< The one and only batch. (this should be improved)
};

BatchRenderer batch;

// == TBBitmapGL ==================================================================================

TBBitmapGL::TBBitmapGL()
	: m_w(0), m_h(0), m_texture(0)
{
}

TBBitmapGL::~TBBitmapGL()
{
	// Must flush and unbind before we delete the texture
	batch.FlushBitmap(this);
	if (m_texture == g_current_texture)
		BindBitmap(nullptr);

	glDeleteTextures(1, &m_texture);
}

bool TBBitmapGL::Init(int width, int height, uint32 *data)
{
	assert(width == TBGetNearestPowerOfTwo(width));
	assert(height == TBGetNearestPowerOfTwo(height));

	m_w = width;
	m_h = height;

	glGenTextures(1, &m_texture);
	BindBitmap(this);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	SetData(data);

	return true;
}

void TBBitmapGL::SetData(uint32 *data)
{
	batch.FlushBitmap(this);
	BindBitmap(this);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

// == TBRendererGL ================================================================================

TBRendererGL::TBRendererGL()
	: m_opacity(255), m_translation_x(0), m_translation_y(0)
{
}

void TBRendererGL::BeginPaint(int render_target_w, int render_target_h)
{
	m_screen_rect.Set(0, 0, render_target_w, render_target_h);
	m_clip_rect = m_screen_rect;
	g_current_texture = 0;
	g_current_batch = nullptr;

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
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
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

void TBRendererGL::DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmapFragment *bitmap_fragment)
{
	if (TBBitmap *bitmap = bitmap_fragment->GetBitmap(TB_VALIDATE_FIRST_TIME))
		batch.AddQuad(	dst_rect.Offset(m_translation_x, m_translation_y),
						src_rect.Offset(bitmap_fragment->m_rect.x, bitmap_fragment->m_rect.y),
						VER_COL_OPACITY(m_opacity), bitmap, bitmap_fragment);
}

void TBRendererGL::DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmap *bitmap)
{
	batch.AddQuad(dst_rect.Offset(m_translation_x, m_translation_y), src_rect, VER_COL_OPACITY(m_opacity), bitmap, nullptr);
}

void TBRendererGL::DrawBitmapColored(const TBRect &dst_rect, const TBRect &src_rect, const TBColor &color, TBBitmapFragment *bitmap_fragment)
{
	if (TBBitmap *bitmap = bitmap_fragment->GetBitmap(TB_VALIDATE_FIRST_TIME))
	{
		uint32 a = (color.a * m_opacity) / 255;
		batch.AddQuad(	dst_rect.Offset(m_translation_x, m_translation_y),
						src_rect.Offset(bitmap_fragment->m_rect.x, bitmap_fragment->m_rect.y),
						VER_COL(color.r, color.g, color.b, a), bitmap, bitmap_fragment);
	}
}

void TBRendererGL::DrawBitmapColored(const TBRect &dst_rect, const TBRect &src_rect, const TBColor &color, TBBitmap *bitmap)
{
	uint32 a = (color.a * m_opacity) / 255;
	batch.AddQuad(dst_rect.Offset(m_translation_x, m_translation_y), src_rect, VER_COL(color.r, color.g, color.b, a), bitmap, nullptr);
}

void TBRendererGL::DrawBitmapTile(const TBRect &dst_rect, TBBitmap *bitmap)
{
	batch.AddQuad(dst_rect.Offset(m_translation_x, m_translation_y), TBRect(0, 0, dst_rect.w, dst_rect.h), VER_COL_OPACITY(m_opacity), bitmap, nullptr);
}

void TBRendererGL::DrawRect(const TBRect &dst_rect, const TBColor &color)
{
	if (dst_rect.IsEmpty())
		return;
	// Top
	DrawRectFill(TBRect(dst_rect.x, dst_rect.y, dst_rect.w, 1), color);
	// Bottom
	DrawRectFill(TBRect(dst_rect.x, dst_rect.y + dst_rect.h - 1, dst_rect.w, 1), color);
	// Left
	DrawRectFill(TBRect(dst_rect.x, dst_rect.y + 1, 1, dst_rect.h - 2), color);
	// Right
	DrawRectFill(TBRect(dst_rect.x + dst_rect.w - 1, dst_rect.y + 1, 1, dst_rect.h - 2), color);
}

void TBRendererGL::DrawRectFill(const TBRect &dst_rect, const TBColor &color)
{
	if (dst_rect.IsEmpty())
		return;
	uint32 a = (color.a * m_opacity) / 255;
	batch.AddQuad(dst_rect.Offset(m_translation_x, m_translation_y), TBRect(), VER_COL(color.r, color.g, color.b, a), nullptr, nullptr);
}

void TBRendererGL::FlushBitmapFragment(TBBitmapFragment *bitmap_fragment)
{
	batch.FlushBitmapFragment(bitmap_fragment);
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
