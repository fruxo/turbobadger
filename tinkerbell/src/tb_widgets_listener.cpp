// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_listener.h"

namespace tinkerbell {

/** WidgetListenerIterator - Internal iterator for listeners in WidgetListenerIterator.
	Since calling a listener callback may result in deletion of the next or even all following
	listeners, this iterator class must be used. It will make sure that the next pointer is
	updated if pointing to a listener that is deleted. */
class WidgetListenerIterator : public TBLinkOf<WidgetListenerIterator>
{
private:
	TBGlobalWidgetListener *m_current;
public:
	WidgetListenerIterator();
	~WidgetListenerIterator();
	TBGlobalWidgetListener *GetAndStep();
	void RemoveListener(TBGlobalWidgetListener *listener);
};

TBLinkListOf<TBGlobalWidgetListener> listeners;
TBLinkListOf<WidgetListenerIterator> iterators;

// == WidgetListenerIterator ================================================================================

WidgetListenerIterator::WidgetListenerIterator()
	: m_current(listeners.GetFirst())
{
	iterators.AddLast(this);
}

WidgetListenerIterator::~WidgetListenerIterator()
{
	iterators.Remove(this);
}

TBGlobalWidgetListener *WidgetListenerIterator::GetAndStep()
{
	if (!m_current)
		return nullptr;
	TBGlobalWidgetListener *current = m_current;
	m_current = m_current->GetNext();
	return current;
}

void WidgetListenerIterator::RemoveListener(TBGlobalWidgetListener *listener)
{
	// Step to the next if the listener is being removed
	if (m_current == listener)
		m_current = m_current->GetNext();
}

// == TBGlobalWidgetListener ================================================================================

void TBGlobalWidgetListener::AddListener(TBGlobalWidgetListener *listener)
{
	listeners.AddLast(listener);
}

void TBGlobalWidgetListener::RemoveListener(TBGlobalWidgetListener *listener)
{
	// If this listener is removed, make sure no iterator points at it
	for (WidgetListenerIterator *iterator = iterators.GetFirst(); iterator; iterator = iterator->GetNext())
		iterator->RemoveListener(listener);

	listeners.Remove(listener);
}

void TBGlobalWidgetListener::InvokeWidgetDelete(Widget *widget)
{
	WidgetListenerIterator iterator;
	while (TBGlobalWidgetListener *listener = iterator.GetAndStep())
		listener->OnWidgetDelete(widget);
}

void TBGlobalWidgetListener::InvokeWidgetAdded(Widget *widget)
{
	WidgetListenerIterator iterator;
	while (TBGlobalWidgetListener *listener = iterator.GetAndStep())
		listener->OnWidgetAdded(widget);
}

void TBGlobalWidgetListener::InvokeWidgetRemove(Widget *widget)
{
	WidgetListenerIterator iterator;
	while (TBGlobalWidgetListener *listener = iterator.GetAndStep())
		listener->OnWidgetRemove(widget);
}

void TBGlobalWidgetListener::InvokeWidgetFocusChanged(Widget *widget, bool focused)
{
	WidgetListenerIterator iterator;
	while (TBGlobalWidgetListener *listener = iterator.GetAndStep())
		listener->OnWidgetFocusChanged(widget, focused);
}

bool TBGlobalWidgetListener::InvokeWidgetInvokeEvent(const WidgetEvent &ev)
{
	bool handled = false;
	WidgetListenerIterator iterator;
	while (TBGlobalWidgetListener *listener = iterator.GetAndStep())
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
