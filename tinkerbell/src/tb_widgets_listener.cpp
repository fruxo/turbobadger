// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
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

void TBGlobalWidgetListener::InvokeWidgetDelete(TBWidget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetDelete(widget);
}

bool TBGlobalWidgetListener::InvokeWidgetDying(TBWidget *widget)
{
	bool handled = false;
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetDying(widget);
	return handled;
}

void TBGlobalWidgetListener::InvokeWidgetAdded(TBWidget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetAdded(widget);
}

void TBGlobalWidgetListener::InvokeWidgetRemove(TBWidget *widget)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetRemove(widget);
}

void TBGlobalWidgetListener::InvokeWidgetFocusChanged(TBWidget *widget, bool focused)
{
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		listener->OnWidgetFocusChanged(widget, focused);
}

bool TBGlobalWidgetListener::InvokeWidgetInvokeEvent(TBWidget *widget, const TBWidgetEvent &ev)
{
	bool handled = false;
	TBLinkListOf<TBGlobalWidgetListener>::Iterator iter = listeners.IterateForward();
	while (TBGlobalWidgetListener *listener = iter.GetAndStep())
		handled |= listener->OnWidgetInvokeEvent(widget, ev);
	return handled;
}

// == TBWidgetSafePointer ===================================================================================

void TBWidgetSafePointer::Set(TBWidget *widget)
{
	if (!m_widget && widget)
		TBGlobalWidgetListener::AddListener(this);
	else if (m_widget && !widget)
		TBGlobalWidgetListener::RemoveListener(this);
	m_widget = widget;
}

}; // namespace tinkerbell
