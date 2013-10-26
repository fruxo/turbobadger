// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_listener.h"

namespace tinkerbell {

TBLinkListOf<TBWidgetListener> listeners;

// == TBWidgetListener ================================================================================

void TBWidgetListener::AddGlobalListener(TBWidgetListener *listener)
{
	listeners.AddLast(listener);
}

void TBWidgetListener::RemoveGlobalListener(TBWidgetListener *listener)
{
	listeners.Remove(listener);
}

void TBWidgetListener::InvokeWidgetDelete(TBWidget *widget)
{
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetDelete(widget);
}

bool TBWidgetListener::InvokeWidgetDying(TBWidget *widget)
{
	bool handled = false;
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetDying(widget);
	return handled;
}

void TBWidgetListener::InvokeWidgetAdded(TBWidget *widget)
{
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetAdded(widget);
}

void TBWidgetListener::InvokeWidgetRemove(TBWidget *widget)
{
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetRemove(widget);
}

void TBWidgetListener::InvokeWidgetFocusChanged(TBWidget *widget, bool focused)
{
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetFocusChanged(widget, focused);
}

bool TBWidgetListener::InvokeWidgetInvokeEvent(TBWidget *widget, const TBWidgetEvent &ev)
{
	bool handled = false;
	TBLinkListOf<TBWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetInvokeEvent(widget, ev);
	return handled;
}

// == TBWidgetSafePointer ===================================================================================

void TBWidgetSafePointer::Set(TBWidget *widget)
{
	if (!m_widget && widget)
		TBWidgetListener::AddGlobalListener(this);
	else if (m_widget && !widget)
		TBWidgetListener::RemoveGlobalListener(this);
	m_widget = widget;
}

}; // namespace tinkerbell
