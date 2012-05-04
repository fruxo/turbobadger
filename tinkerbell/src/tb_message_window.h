// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_MESSAGE_WINDOW_H
#define TB_MESSAGE_WINDOW_H

#include "tb_window.h"
#include "tb_widgets_listener.h"

namespace tinkerbell {

enum TB_MSG {
	TB_MSG_OK,
	TB_MSG_OK_CANCEL,
	TB_MSG_YES_NO
};

/** TBMessageWindowSettings contains additional settings for TBMessageWindow. */
class TBMessageWindowSettings
{
public:
	TBMessageWindowSettings() : msg(TB_MSG_OK), dimmer(false) {}
	TBMessageWindowSettings(TB_MSG msg, TBID icon_skin) : msg(msg), icon_skin(icon_skin), dimmer(false) {}
public:
	TB_MSG msg;			///< The type of response for the message.
	TBID icon_skin;		///< The icon skin (0 for no icon)
	bool dimmer;		///< Set to true to dim background widgets by a TBDimmer.
};

/** TBMessageWindow is a window for showing simple messages.
	Events invoked in this window will travel up through the target widget.

	When the user click any of its buttons, it will invoke a click event
	(with the window ID), with the clicked buttons id as ref_id.
	Then it will delete itself.

	If the target widget is deleted while this window is alive, the
	window will delete itself. */
class TBMessageWindow : public TBWindow, private TBWidgetSafePointer
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBMessageWindow", TBWindow);

	TBMessageWindow(Widget *target, TBID id);
	virtual ~TBMessageWindow();

	bool Show(const char *title, const char *message, TBMessageWindowSettings *settings = nullptr);

	virtual Widget *GetEventDestination() { return Get(); }

	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnDie();
private:
	void AddButton(TBID id, bool focused);
	virtual void OnWidgetDelete(Widget *widget);
	TBWidgetSafePointer m_dimmer;
};

}; // namespace tinkerbell

#endif // TB_MESSAGE_WINDOW_H
