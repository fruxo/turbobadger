// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_GEOMETRY_H
#define TB_GEOMETRY_H

#include "tinkerbell.h"

namespace tinkerbell {

class TBRegion
{
public:
	TBRegion();
	~TBRegion();

	/** Remove the rect at the given index. */
	void RemoveRect(int index);

	/** Remove the rect at the given index.
		This method will change the order of rectangles after index. */
	void RemoveRectFast(int index);

	/** Remove all rectangles so the region becomes empty.
		If free_memory is false, the internal buffers will be reused
		if more rectangles are added again under its life time. */
	void RemoveAll(bool free_memory = true);

	/** Set the region to the given rect. */
	bool Set(const TBRect &rect);

	/** Add the rect without doing any overlap check.
		If coalesce is true, it will coalesce the rectangle
		with existing rectangles if possible (until there's
		nothing more to coalesce it with). */
	bool AddRect(const TBRect &rect, bool coalesce);

	/** Include the rect in the region.
		This will add only the parts that's not already in the region so the result doesn't
		contain overlap parts. This assumes there's no overlap in the region already! */
	bool IncludeRect(const TBRect &include_rect);

	/** Exclude the rect from the region. */
	bool ExcludeRect(const TBRect &exclude_rect);

	/** Add the rectangles that's left of rect after excluding exclude_rect. */
	bool AddExcludingRects(const TBRect &rect, const TBRect &exclude_rect, bool coalesce);

	bool IsEmpty() const							{ return m_num_rects == 0; }
	int GetNumRects() const							{ return m_num_rects; }
	const TBRect &GetRect(int index) const;
private:
	TBRect *m_rects;
	int m_num_rects;
	int m_capacity;
	bool GrowIfNeeded();
};

}; // namespace tinkerbell

#endif // TB_GEOMETRY_H
