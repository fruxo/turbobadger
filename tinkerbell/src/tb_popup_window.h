// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_POPUP_WINDOW_H
#define TB_POPUP_WINDOW_H

#include "tb_window.h"
#include "tb_widgets_listener.h"

namespace tinkerbell {

/** TBPopupAlignment describes the preferred alignment of a popup
	relative to a target widget or a given point.

	It calculates the rect to be used to match these preferences
	for any given popup and target. */
class TBPopupAlignment
{
public:
	static const int UNSPECIFIED = TB_INVALID_DIMENSION;

	/** Align relative to the target widget. */
	TBPopupAlignment(TB_ALIGN align = TB_ALIGN_BOTTOM)
		: pos_in_root(UNSPECIFIED, UNSPECIFIED)
		, align(align)
		, expand_to_target_width(true) {}

	/** Align relative to the given position (coordinates relative to the root widget). */
	TBPopupAlignment(const TBPoint &pos_in_root, TB_ALIGN align = TB_ALIGN_BOTTOM)
		: pos_in_root(pos_in_root)
		, align(align)
		, expand_to_target_width(true) {}

	/** Calculate a good rect for the given popup window using its preferred size and
		the preferred alignment information stored in this class. */
	TBRect GetAlignedRect(TBWidget *popup, TBWidget *target) const;

	TBPoint pos_in_root;
	TB_ALIGN align;
	/** If true, the width of the popup will be at least the same as the target widget
		if the alignment is TB_ALIGN_TOP or TB_ALIGN_BOTTOM. */
	bool expand_to_target_width;
};

/** TBPopupWindow is a popup window that redirects any child widgets events
	through the given target. It will automatically close on click events that
	are not sent through this popup. */

class TBPopupWindow : public TBWindow, private TBGlobalWidgetListener
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(TBPopupWindow, TBWindow);

	TBPopupWindow(TBWidget *target);
	~TBPopupWindow();

	bool Show(const TBPopupAlignment &alignment);

	virtual TBWidget *GetEventDestination() { return m_target.Get(); }

	virtual bool OnEvent(const TBWidgetEvent &ev);
private:
	TBWidgetSafePointer m_target;
	// TBGlobalWidgetListener
	virtual void OnWidgetFocusChanged(TBWidget *widget, bool focused);
	virtual bool OnWidgetInvokeEvent(TBWidget *widget, const TBWidgetEvent &ev);
	virtual void OnWidgetDelete(TBWidget *widget);
	virtual bool OnWidgetDying(TBWidget *widget);
};

}; // namespace tinkerbell

#endif // TB_POPUP_WINDOW_H
