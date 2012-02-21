// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_RENDERER_H
#define TB_RENDERER_H

#include "tinkerbell.h"

namespace tinkerbell {

#define ALL_TO_TERMINATION 2147483647

class TBBitmapFragment;

/** TBBitmap is a minimal interface for bitmap to be painted by TBRenderer. */

class TBBitmap
{
public:
	virtual ~TBBitmap() {}
	virtual int Width() = 0;
	virtual int Height() = 0;
};

/** TBRenderer is a minimal interface for painting strings and bitmaps. */

class TBRenderer
{
public:
	/** Should be called before invoking paint on any widget.
		render_target_w and render_target_h should be the size of the render target
		that the renderer renders to. I.e window size, screen size or frame buffer object. */
	virtual void BeginPaint(int render_target_w, int render_target_h) = 0;
	virtual void EndPaint() = 0;

	/** Translate all drawing with the given offset */
	virtual void Translate(int dx, int dy) = 0;

	/** Set the current opacity that should apply to all drawing (0.f-1.f). */
	virtual void SetOpacity(float opacity) = 0;
	virtual float GetOpacity() = 0;

	/** Set a clip rect to the renderer. add_to_current should be true when
		pushing a new cliprect that should clip inside the last clip rect,
		and false when restoring.
		It will return the clip rect that was in use before this call. */
	virtual TBRect SetClipRect(const TBRect &rect, bool add_to_current) = 0;

	/** Get the current clip rect. Note: This may be different from the rect
		sent to SetClipRect, due to intersecting with the previous cliprect! */
	virtual TBRect GetClipRect() = 0;

	virtual void DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmapFragment *bitmap_fragment) = 0;

	/** Draw the src_rect part of the bitmap stretched to dst_rect. */
	virtual void DrawBitmap(const TBRect &dst_rect, const TBRect &src_rect, TBBitmap *bitmap) = 0;

	/** Draw the bitmap tiled into dst_rect. */
	virtual void DrawBitmapTile(const TBRect &dst_rect, TBBitmap *bitmap) = 0;

	/** Draw a 1px thick rectangle outline. */
	virtual void DrawRect(const TBRect &dst_rect, const TBColor &color) = 0;

	/** Draw a filled rectangle. */
	virtual void DrawRectFill(const TBRect &dst_rect, const TBColor &color) = 0;

	/** Draw string at position x, y (marks the upper left corner of the text). */
	virtual void DrawString(int x, int y, const char *str, int len = ALL_TO_TERMINATION) = 0;

	/** Measure the width of the given string. Should measure len characters or to the null
		termination (whatever comes first). */
	virtual int GetStringWidth(const char *str, int len = ALL_TO_TERMINATION) = 0;

	/** Get height of the font in pixels. */
	virtual int GetFontHeight() = 0;

	/** Create a new TBBitmap from the given data (in BGRA32 format).
		Width and height mustbe a power of two.
		Return nullptr if fail. */
	virtual TBBitmap *CreateBitmap(int width, int height, uint32 *data) = 0;
};

}; // namespace tinkerbell

#endif // TB_RENDERER_H
