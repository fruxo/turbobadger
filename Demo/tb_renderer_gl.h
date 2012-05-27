// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_RENDERER_GL_H
#define TB_RENDERER_GL_H

#ifdef _WIN32
#include <windows.h> // make gl.h compile
#include <GL/gl.h>
#elif defined(MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif // _WIN32

#include "tb_renderer.h"
#include "tdfont/tdfont.h"

namespace tinkerbell {

class TBBitmapGL : public TBBitmap
{
public:
	TBBitmapGL();
	~TBBitmapGL();
	bool Init(int width, int height, uint32 *data);
	virtual int Width() { return m_w; }
	virtual int Height() { return m_h; }
public:
	int m_w, m_h;
	GLuint m_texture;
};

class TBRendererGL : public TBRenderer, public TdFontRendererBackend
{
public:
	TBRendererGL();

	// == TdFontRendererBackend ===================================

	virtual void BeginDrawString(TdFont *font);
	virtual void EndDrawString(TdFont *font);
	virtual void DrawGlyph(float x, float y, TDFNT_GLYPH *glyph, unsigned int texture);

	// == TBRenderer ==============================================

	virtual void BeginPaint(int render_target_w, int render_target_h);
	virtual void EndPaint();

	virtual void Translate(int x, int y);
	virtual void SetOpacity(float opacity);
	virtual float GetOpacity();
	virtual TBRect SetClipRect(const TBRect &rect, bool add_to_current);
	virtual TBRect GetClipRect();
	virtual void DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmapFragment *bitmap_fragment);
	virtual void DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmap *bitmap);
	virtual void DrawBitmapTile(const TBRect &dst_rect, TBBitmap *bitmap);
	virtual void DrawRect(const TBRect &dst_rect, const TBColor &color);
	virtual void DrawRectFill(const TBRect &dst_rect, const TBColor &color);
	virtual void DrawString(int x, int y, const TBColor &color, const char *str, int len);
	virtual int GetStringWidth(const char *str, int len);
	virtual int GetFontHeight();
	virtual int GetFontBaseline();

	virtual TBBitmap *CreateBitmap(int width, int height, uint32 *data);
public:
	void DrawTexture(const TBRect &dst_rect, float u, float v, float uu, float vv, GLuint texture);
	void DrawTexture(const TBRect &dst_rect, float u, float v, float uu, float vv, GLuint texture, uint32 color);
	uint8 m_opacity;
	TBRect m_screen_rect;
	TBRect m_clip_rect;
	int m_translation_x;
	int m_translation_y;
	GLuint m_current_texture;
};

}; // namespace tinkerbell

#endif // TB_RENDERER_GL_H
