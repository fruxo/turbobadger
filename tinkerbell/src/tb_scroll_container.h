// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_SCROLL_CONTAINER_H
#define TB_SCROLL_CONTAINER_H

#include "tb_widgets_common.h"

namespace tinkerbell {

enum SCROLL_MODE {
	SCROLL_MODE_X_Y,			///< X and Y always
	SCROLL_MODE_Y,				///< Y always (X never)
	SCROLL_MODE_Y_AUTO,			///< Y auto (X never)
	SCROLL_MODE_X_AUTO_Y_AUTO	///< X auto, Y auto
};

/** TBScrollContainerRoot - Internal for TBScrollContainer */

class TBScrollContainerRoot : public Widget
{
private: // May only be used by TBScrollContainer.
	friend class TBScrollContainer;
	TBScrollContainerRoot() {}
public:
	virtual void OnPaintChildren(const PaintProps &paint_props);
	virtual void GetChildTranslation(int &x, int &y) const;
};

/** TBScrollBarVisibility - Helper for TBScrollContainer or any other scrollable
	container that needs to solve scrollbar visibility according to SCROLL_MODE. */
class TBScrollBarVisibility
{
public:
	TBScrollBarVisibility () : x_on(false), y_on(false), visible_w(0), visible_h(0) {}

	static TBScrollBarVisibility Solve(SCROLL_MODE mode, int content_w, int content_h,
														int available_w, int available_h,
														int scrollbar_x_h, int scrollbar_y_w);
public:
	bool x_on, y_on;
	int visible_w, visible_h;
};

/** TBScrollContainer - A container with scrollbars that can scroll its children. */

class TBScrollContainer : public Widget
{
friend class TBScrollContainerRoot;
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBScrollContainer", Widget);

	TBScrollContainer();
	~TBScrollContainer();

	/** Set to true if the preferred size of this container should adapt to the preferred
		size of the content. This is disabled by default. */
	void SetAdaptToContentSize(bool adapt) { m_adapt_to_content_size = adapt; }
	bool GetAdaptToContentSize() { return m_adapt_to_content_size; }

	void SetScrollMode(SCROLL_MODE mode);
	SCROLL_MODE GetScrollMode() { return m_mode; }

	TBRect GetVisibleRect();
	void ScrollTo(int x, int y);
	virtual void ScrollIntoView(const TBRect &rect);
	virtual void ScrollBy(int &dx, int &dy);

	virtual void InvalidateLayout(INVALIDATE_LAYOUT il);

	virtual PreferredSize GetPreferredContentSize();

	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnProcess();
	virtual void OnResized(int old_w, int old_h);

	virtual Widget *GetContentRoot() { return &m_root; }
protected:
	TBScrollBar m_scrollbar_x;
	TBScrollBar m_scrollbar_y;
	TBScrollContainerRoot m_root;
	bool m_adapt_to_content_size;
	bool m_layout_is_invalid;
	SCROLL_MODE m_mode;
	void ValidateLayout();
};

};

#endif // TB_SCROLL_CONTAINER_H
