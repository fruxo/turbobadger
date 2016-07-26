// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2016, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_shape_rasterizer.h"
#include "tb_font_renderer.h"
#include "tb_system.h"
#include "tb_blur.h"
#include "utf8/utf8.h"
#include <math.h>

namespace tb {

inline void blend_stencil(uint8 *dst, float a, float mul) {
	*dst = Clamp(*dst + (int) (a * mul * 255 + 0.5f), 0, 255);
}

inline void blend_col(uint8 *dst, uint8 stencil_a, const TBColor &c) {
	dst[0] += ((c.r - dst[0]) * stencil_a + 1) >> 8;
	dst[1] += ((c.g - dst[1]) * stencil_a + 1) >> 8;
	dst[2] += ((c.b - dst[2]) * stencil_a + 1) >> 8;
	dst[3] += ((c.a - dst[3]) * stencil_a + 1) >> 8;
}

TBShapeRasterizer::TBShapeRasterizer()
	: m_w(0), m_h(0), m_pixels(nullptr), m_stencil(nullptr) {
}

TBShapeRasterizer::~TBShapeRasterizer() {
	delete [] m_pixels;
	delete [] m_stencil;
}

bool TBShapeRasterizer::Initialize(int suggested_w, int suggested_h) {
	if (m_pixels || suggested_w <= 0 || suggested_h <= 0)
		return true;
	m_pixels = new uint32[suggested_w * suggested_h];
	m_stencil = new uint8[suggested_w * suggested_h];
	if (!m_pixels || !m_stencil) {
		delete [] m_pixels;
		delete [] m_stencil;
		m_pixels = nullptr;
		m_stencil = nullptr;
		return false;
	}
	m_w = suggested_w;
	m_h = suggested_h;
	ColorClear();
	StencilClear();
	return true;
}

void TBShapeRasterizer::StencilClear() {
	if (!m_stencil) return;
	memset(m_stencil, 0, m_w * m_h * sizeof(uint8));
}

void TBShapeRasterizer::ColorClear() {
	if (!m_pixels) return;
	memset(m_pixels, 0, m_w * m_h * sizeof(uint32));
}

void TBShapeRasterizer::StencilCircle(const TBRect &clip_rect, float cx, float cy, float radius, float mul) {
	if (!m_stencil) return;
	const TBRect r = clip_rect.Clip(TBRect(0, 0, m_w, m_h));
	cx -= 0.5f;
	cy -= 0.5f;
	for (int y = r.y; y < r.y + r.h; y++) {
		uint8 *dst_row = m_stencil + y * m_w;
		for (int x = r.x; x < r.x + r.w; x++) {
			float a = sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy)) / (radius + 0.5f);
			a = Clamp(1 - (a - (1 - 1 / radius)) * radius, 0.f, 1.f);
			blend_stencil(&dst_row[x], a, mul);
		}
	}
}

void TBShapeRasterizer::StencilRect(const TBRect &rect, float mul) {
	if (!m_stencil) return;
	const TBRect r = rect.Clip(TBRect(0, 0, m_w, m_h));
	for (int y = r.y; y < r.y + r.h; y++) {
		uint8 *dst_row = m_stencil + y * m_w;
		for (int x = r.x; x < r.x + r.w; x++)
			blend_stencil(&dst_row[x], 1.f, mul);
	}
}

void TBShapeRasterizer::ColorRect(const TBRect &rect, const TBColor &c) {
	if (!m_pixels || !m_stencil) return;
	TBColor premul_c = c;
	premul_c.Premultiply();
	for (int y = rect.y; y < rect.y + rect.h; y++) {
		uint32 *dst_row = m_pixels + y * m_w;
		uint8 *stencil_row = m_stencil + y * m_w;
		for (int x = rect.x; x < rect.x + rect.w; x++)
			blend_col((uint8*) &dst_row[x], stencil_row[x], premul_c);
	}
}

void TBShapeRasterizer::StencilInvert() {
	if (!m_stencil) return;
	for (int i = 0; i < m_w * m_h; i++) {
		m_stencil[i] = 255 - m_stencil[i];
	}
}

void TBShapeRasterizer::ColorUnpremultiply() {
	if (!m_pixels) return;
	uint8 *pixels8 = (uint8 *) m_pixels;
	for (int i = 0; i < m_w * m_h; i++) {
		if (const uint32 a32 = pixels8[3]) {
			pixels8[0] = pixels8[0] * 255 / a32;
			pixels8[1] = pixels8[1] * 255 / a32;
			pixels8[2] = pixels8[2] * 255 / a32;
		}
		pixels8 += 4;
	}
}

void TBShapeRasterizer::StencilRectRadius(const TBRect &rect, int r1, int r2, int r3, int r4, float mul) {
	if (!m_stencil) return;
	// Sanitize radius.
	const int max_cut = Min(rect.w, rect.h) / 2;
	r1 = Clamp(r1, 0, max_cut);
	r2 = Clamp(r2, 0, max_cut);
	r3 = Clamp(r3, 0, max_cut);
	r4 = Clamp(r4, 0, max_cut);

	const TBRect rect_ul = TBRect(rect.x, rect.y, r1, r1);
	const TBRect rect_ur = TBRect(rect.x + rect.w - r2, rect.y, r2, r2);
	const TBRect rect_ll = TBRect(rect.x, rect.y + rect.h - r3, r3, r3);
	const TBRect rect_lr = TBRect(rect.x + rect.w - r4, rect.y + rect.h - r4, r4, r4);

	TBRegion rgn;
	rgn.Set(rect);
	rgn.ExcludeRect(rect_ul);
	rgn.ExcludeRect(rect_ur);
	rgn.ExcludeRect(rect_ll);
	rgn.ExcludeRect(rect_lr);

	StencilCircle(rect_ul, (float) (rect.x + r1), (float) (rect.y + r1), (float) r1, mul);
	StencilCircle(rect_ur, (float) (rect.x + rect.w - r2), (float) (rect.y + r2), (float) r2, mul);
	StencilCircle(rect_ll, (float) (rect.x + r3), (float) (rect.y + rect.h - r3), (float) r3, mul);
	StencilCircle(rect_lr, (float) (rect.x + rect.w - r4), (float) (rect.y + rect.h - r4), (float) r4, mul);

	for (int i = 0; i < rgn.GetNumRects(); i++)
		StencilRect(rgn.GetRect(i), mul);
}

void TBShapeRasterizer::StencilBlur(float radius) {
	if (!m_stencil) return;
	if (radius <= 0) return;

	if (uint8 *new_stencil = new uint8[m_w * m_h]) {
		TBGaussBlur(m_stencil, new_stencil, m_w, m_h, radius);
		delete [] m_stencil;
		m_stencil = new_stencil;
	}
}

void TBShapeRasterizer::StencilGlyph(const TBFontDescription &fd, const char *glyph_str, float mul) {
	if (!g_font_manager->HasFontFace(fd) && !g_font_manager->CreateFontFace(fd)) {
		TBDebugPrint("Skin error: The glyph font could not be loaded in size %d!\n", fd.GetSize());
		return;
	}
	TBFontFace *font = g_font_manager->GetFontFace(fd);
	if (!font)
		return;
	TBFontRenderer *fr = font->GetFontRenderer();
	if (!fr)
		return;

	const UCS4 cp = utf8::decode(glyph_str, glyph_str + strlen(glyph_str));

	TBFontGlyphData glyph_data;
	if (fr->RenderGlyph(&glyph_data, cp) && glyph_data.data8) {
		if (!Initialize(glyph_data.w, glyph_data.h))
			return;

		TBRect dst_rect = TBRect(0, 0, glyph_data.w, glyph_data.h).CenterIn(TBRect(0, 0, m_w, m_h));
		StencilStencil(dst_rect.x, dst_rect.y, glyph_data.data8, glyph_data.w, glyph_data.h, glyph_data.stride, mul);
	}
}

void TBShapeRasterizer::StencilStencil(int x, int y, const uint8 *st, int st_w, int st_h, int st_stride, float mul) {
	if (!m_stencil) return;
	const TBRect clip = TBRect(0, 0, m_w, m_h);
	int w = st_w, h = st_h;
	int srcx = 0, srcy = 0;

	// Clip rect and adjusting src offset accordingly
	if (x < clip.x) {
		const int diff = clip.x - x;
		srcx += diff;
		x += diff;
		w -= diff;
	}
	if (y < clip.y) {
		const int diff = clip.y - y;
		srcy += diff;
		y += diff;
		h -= diff;
	}
	if (x + w > clip.x + clip.w) {
		w -= (x + w) - (clip.x + clip.w);
	}
	if (y + h > clip.y + clip.h) {
		h -= (y + h) - (clip.y + clip.h);
	}
	if (w <= 0 || h <= 0)
		return;

	// Draw
	for (int j = 0; j < h; j++) {
		uint8 *dst = m_stencil + x + (j + y) * m_w;
		const uint8 *src = st + srcx + (j + srcy) * st_stride;
		for (int x = 0; x < w; x++)
			blend_stencil(&dst[x], src[x] / 255.f, mul);
	}
}

} // namespace tb
