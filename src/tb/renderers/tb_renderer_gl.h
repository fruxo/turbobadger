// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_RENDERER_GL_H
#define TB_RENDERER_GL_H

#include "renderers/tb_renderer_batcher.h"

#if defined(TB_RENDERER_GLES_1) || defined(TB_RENDERER_GL)
#ifdef TB_RENDERER_GLES_1
#include <EGL/egl.h>
#include <GLES/gl.h>
#elif defined(_WIN32)
#include <windows.h> // make gl.h compile
#include <GL/gl.h>
#elif defined(MACOSX)
#include <OpenGL/gl.h>
#elif defined(ANDROID)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

namespace tb {

class TBRendererGL;

class TBBitmapGL : public TBBitmap
{
public:
	TBBitmapGL(TBRendererGL *renderer);
	~TBBitmapGL();
	bool Init(int width, int height, uint32 *data);
	virtual int Width() { return m_w; }
	virtual int Height() { return m_h; }
	virtual void SetData(uint32 *data);
public:
	TBRendererGL *m_renderer;
	int m_w, m_h;
	GLuint m_texture;
};

class TBRendererGL : public TBRendererBatcher
{
public:
	TBRendererGL();

	// == TBRenderer ====================================================================

	virtual void BeginPaint(int render_target_w, int render_target_h);
	virtual void EndPaint();

	virtual TBBitmap *CreateBitmap(int width, int height, uint32 *data);

	// == TBRendererBatcher ===============================================================

	virtual void RenderBatch(Batch *batch);
	virtual void SetClipRect(const TBRect &rect);
public:
};

}; // namespace tb

#endif // defined(TB_RENDERER_GLES_1) || defined(TB_RENDERER_GL)
#endif // TB_RENDERER_GL_H
