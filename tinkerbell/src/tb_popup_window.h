// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_POPUP_WINDOW_H
#define TB_POPUP_WINDOW_H

#include "tb_window.h"
#include "tb_widgets_listener.h"

namespace tinkerbell {

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

	bool Show(const TBPoint *pos_in_root = nullptr, TB_ALIGN align = TB_ALIGN_BOTTOM);

	TBRect GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align);

	virtual TBWidget *GetEventDestination() { return m_target.Get(); }
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
