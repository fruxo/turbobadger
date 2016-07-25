// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2016, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_blur.h"
#include <math.h>

namespace tb {

void BoxBlurH(uint8 *scl, uint8 *tcl, int w, int h, int r) {
	float iarr = 1 / (float) (r + r + 1);
	for (int i = 0; i < h; i++) {
		int ti = i * w, li = ti, ri = ti + r;
		float fv = scl[ti], lv = scl[ti + w - 1], val = (r + 1) * fv;
		for (int j = 0; j < r; j++)
			val += scl[ti + j];
		for (int j = 0; j <= r; j++) {
			val += scl[ri++] - fv;
			tcl[ti++] = (uint8) roundf(val * iarr);
		}
		for (int j = r + 1; j < w - r; j++) {
			val += scl[ri++] - scl[li++];
			tcl[ti++] = (uint8) roundf(val * iarr);
		}
		for (int j = w - r; j < w; j++) {
			val += lv - scl[li++];
			tcl[ti++] = (uint8) roundf(val * iarr);
		}
	}
}

void BoxBlurT(uint8 *scl, uint8 *tcl, int w, int h, int r) {
	float iarr = 1 / (float) (r + r + 1);
	for (int i = 0; i < w; i++) {
		int ti = i, li = ti, ri = ti + r * w;
		float fv = scl[ti], lv = scl[ti + w * (h - 1)], val = (r + 1) * fv;
		for (int j = 0; j < r; j++)
			val += scl[ti + j * w];
		for (int j = 0; j <= r ; j++) {
			val += scl[ri] - fv;
			tcl[ti] = (uint8) roundf(val * iarr);
			ri += w;
			ti += w;
		}
		for (int j = r + 1; j < h - r; j++) {
			val += scl[ri] - scl[li];
			tcl[ti] = (uint8) roundf(val * iarr);
			li += w;
			ri += w;
			ti += w;
		}
		for (int j = h - r; j < h; j++) {
			val += lv - scl[li];
			tcl[ti] = (uint8) roundf(val * iarr);
			li += w;
			ti += w;
		}
	}
}

void BoxBlur(uint8 *scl, uint8 *tcl, int w, int h, int r) {
	memcpy(tcl, scl, w * h);
	if (r <= 0) return;
	BoxBlurH(tcl, scl, w, h, r);
	BoxBlurT(scl, tcl, w, h, r);
}

void BoxesForGauss(float sigma, int n, int *dst) {
	const float wIdeal = sqrtf((12 * sigma * sigma / (float) n) + 1);
	int wl = (int) floorf(wIdeal);
	if (wl % 2 == 0)
		wl--;
	const int wu = wl + 2;
	const float mIdeal = (float) (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
	const int m = (int) roundf(mIdeal);

	for (int i = 0; i < n; i++)
		dst[i] = i < m ? wl : wu;
}

void TBGaussBlur(uint8 *source, uint8 *target, int width, int height, float radius) {
	int bxs[3];
	BoxesForGauss(radius, 3, bxs);
	BoxBlur(source, target, width, height, (bxs[0] - 1) / 2);
	BoxBlur(target, source, width, height, (bxs[1] - 1) / 2);
	BoxBlur(source, target, width, height, (bxs[2] - 1) / 2);
}

} // namespace tb
