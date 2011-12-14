// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_WIDGETSLISTENER_H
#define TB_WIDGETSLISTENER_H

#include "tinkerbell.h"
#include "tb_linklist.h"
#include "tb_widgets.h"

namespace tinkerbell {

class Widget;

/** TBGlobalWidgetListener listens to some callbacks from all widgets in tinkerbell.
	You can set as many TBGlobalWidgetListener you want, by using AddListener.

	This is useful f.ex if you want to listen to when a textfield receive focus
	to display virtual keyboard, or when a widget of a certain type is added, to
	start animations or sound. */

class TBGlobalWidgetListener : public TBLinkOf<TBGlobalWidgetListener>
{
public:
	static void AddListener(TBGlobalWidgetListener *listener);
	static void RemoveListener(TBGlobalWidgetListener *listener);

	/** Called when widget is being deleted (in its destructor, so virtual functions are already gone). */
	virtual void OnWidgetDelete(Widget *widget) {}

	/** Called when widget has been added to a parent, after its parents OnChildAdded. */
	virtual void OnWidgetAdded(Widget *widget) {}

	/** Called when widget is about to be removed from a parent, after its parents OnChildRemove. */
	virtual void OnWidgetRemove(Widget *widget) {}

	/** Called when widget focus has changed on a widget. */
	virtual void OnWidgetFocusChanged(Widget *widget, bool focused) {}

	/** Called when a event is about to be invoked on a widget. This make it possible
		to intercept a events before they are handled, and block it (by returning true).
		Note, if returning true, other global listeners will still also be notified. */
	virtual bool OnWidgetInvokeEvent(const WidgetEvent &ev) { return false; }
private:
	friend class Widget;
	static void InvokeWidgetDelete(Widget *widget);
	static void InvokeWidgetAdded(Widget *widget);
	static void InvokeWidgetRemove(Widget *widget);
	static void InvokeWidgetFocusChanged(Widget *widget, bool focused);
	static bool InvokeWidgetInvokeEvent(const WidgetEvent &ev);
};

/** TBWidgetSafePointer keeps a pointer to a widget that will be set to
	nullptr if the widget is removed. Do not create excessive amounts of this
	object as it's expensive. Use it f.ex on the stack to detect self deletion.
	ex: (in OnEvent)
		TBWidgetSafePointer thiswidget(this);
		// Invoke callbacks that might remove this widget
		if (!thiswidget->Get())
			return;
		// Do some more things with thiswidget.
		*/
class TBWidgetSafePointer : private TBGlobalWidgetListener
{
public:
	TBWidgetSafePointer() : m_widget(nullptr)				{ }
	TBWidgetSafePointer(Widget *widget) : m_widget(nullptr)	{ Set(widget); }
	~TBWidgetSafePointer()									{ Set(nullptr); }

	virtual void OnWidgetDelete(Widget *widget)				{ if (widget == m_widget) Set(nullptr); }

	/** Set the widget pointer that should be nulled if deleted. */
	void Set(Widget *widget);

	/** Return the widget, or nullptr if it has been deleted. */
	Widget *Get() { return m_widget; }
private:
	Widget *m_widget;
};

};

#endif // TB_WIDGETSLISTENER_H
