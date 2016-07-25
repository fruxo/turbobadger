// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2016, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_BLUR_H
#define TB_BLUR_H

#include "tb_core.h"

namespace tb {

/** Blur the source buffer into target (both buffers modified) using the given
	radius.

	CREDITS:
	This is a port of Ivan Kuckirs blur, based on Fast image convolutions by
	Wojciech Jarosz. See: http://blog.ivank.net/fastest-gaussian-blur.html
*/
void TBGaussBlur(uint8 *source, uint8 *target, int width, int height, float radius);

} // namespace tb

#endif // TB_BLUR_H
