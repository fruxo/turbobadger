// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2016, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_SHAPE_RASTERIZER_H
#define TB_SHAPE_RASTERIZER_H

#include "tb_core.h"
#include "tb_geometry.h"
#include "tb_color.h"
#include "tb_font_desc.h"

namespace tb {

/** TBShapeRasterizer does software rendering to internal stencil and pixel buffer.
	It's purpose it to generate bitmap data for use in TBBitmapFragment.

	Rendering shapes is done to stencil buffer, which is then used to colorize pixel buffer.
	The stencil can be cleared to repeate the process and blend multiple layers.
*/
class TBShapeRasterizer
{
public:
	TBShapeRasterizer();
	~TBShapeRasterizer();

	/** Initialize buffers for the suggested size if not already initialized.
		Returns true if initialized. */
	bool Initialize(int suggested_w, int suggested_h);

	void StencilClear();
	void StencilCircle(const TBRect &clip_rect, float cx, float cy, float radius, float mul);
	void StencilRect(const TBRect &rect, float mul);
	void StencilRectRadius(const TBRect &rect, int r1, int r2, int r3, int r4, float mul);
	void StencilInvert();
	void StencilBlur(float radius);
	void StencilGlyph(const TBFontDescription &fd, const char *glyph_str, float mul);
	void StencilStencil(int x, int y, const uint8 *st, int st_w, int st_h, int st_stride, float mul);

	void ColorClear();
	void ColorRect(const TBRect &rect, const TBColor &c);
	//void ColorLinear(const TBRect &rect, float rad_angle, const TBColor *colors, int num_colors);
	void ColorUnpremultiply();

	/** Get the pointer to pixel data.
		NOTE: This is premultiplied data, unless calling ColorUnpremultiply first. */
	inline uint32 *GetPixels() const { return m_pixels; }
	inline int GetWidth() const { return m_w; }
	inline int GetHeight() const { return m_h; }
private:
	int m_w, m_h;
	uint32 *m_pixels;
	uint8 *m_stencil;
};

} // namespace tb

#endif // TB_SHAPE_RASTERIZER_H
