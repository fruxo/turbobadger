// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_listener.h"

namespace tinkerbell {

TBLinkListOf<TBGlobalWidgetListener> listeners;

// == TBGlobalWidgetListener ================================================================================

void TBGlobalWidgetListener::AddListener(TBGlobalWidgetListener *listener)
{
	listeners.AddLast(listener);
}

void TBGlobalWidgetListener::RemoveListener(TBGlobalWidgetListener *listener)
{
	listeners.Remove(listener);
}

void TBGlobalWidgetListener::InvokeWidgetDelete(Widget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetDelete(widget);
}

bool TBGlobalWidgetListener::InvokeWidgetDying(Widget *widget)
{
	bool handled = false;
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetDying(widget);
	return handled;
}

void TBGlobalWidgetListener::InvokeWidgetAdded(Widget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetAdded(widget);
}

void TBGlobalWidgetListener::InvokeWidgetRemove(Widget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetRemove(widget);
}

void TBGlobalWidgetListener::InvokeWidgetFocusChanged(Widget *widget, bool focused)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetFocusChanged(widget, focused);
}

bool TBGlobalWidgetListener::InvokeWidgetInvokeEvent(const WidgetEvent &ev)
{
	bool handled = false;
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetInvokeEvent(ev);
	return handled;
}

// == TBWidgetSafePointer ===================================================================================

void TBWidgetSafePointer::Set(Widget *widget)
{
	if (!m_widget && widget)
		TBGlobalWidgetListener::AddListener(this);
	else if (m_widget && !widget)
		TBGlobalWidgetListener::RemoveListener(this);
	m_widget = widget;
}

}; // namespace tinkerbell
