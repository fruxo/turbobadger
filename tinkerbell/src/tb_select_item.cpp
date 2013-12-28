// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_select.h"
#include "tb_menu_window.h"
#include "tb_widgets_listener.h"
#include "tb_language.h"
#include <assert.h>
#include <stdlib.h>

namespace tinkerbell {

/** TBSimpleLayoutItemWidget is a item containing a layout with the following:
	-TBSkinImage showing the item image.
	-TBTextField showing the item string.
	-TBSkinImage showing the arrow for items with a submenu.
	It also handles submenu events. */

class TBSimpleLayoutItemWidget : public TBLayout
{
public:
	TBSimpleLayoutItemWidget(TBID image, TBSelectItemSource *source, const char *str);
	~TBSimpleLayoutItemWidget();
	virtual bool OnEvent(const TBWidgetEvent &ev);
private:
	TBSelectItemSource *m_source;
	TBTextField m_textfield;
	TBSkinImage m_image;
	TBSkinImage m_image_arrow;
	TBWidgetSafePointer m_window_pointer; ///< Points to the submenu window if opened
};

// == TBSimpleLayoutItemWidget ==============================================================================

TBSimpleLayoutItemWidget::TBSimpleLayoutItemWidget(TBID image, TBSelectItemSource *source, const char *str)
	: m_source(source)
{
	SetSkinBg("TBSelectItem");
	SetLayoutDistribution(LAYOUT_DISTRIBUTION_AVAILABLE);
	SetPaintOverflowFadeout(false);

	if (image)
	{
		m_image.SetSkinBg(image);
		m_image.SetIgnoreInput(true);
		AddChild(&m_image);
	}

	m_textfield.SetText(str);
	m_textfield.SetTextAlign(TB_TEXT_ALIGN_LEFT);
	m_textfield.SetIgnoreInput(true);
	AddChild(&m_textfield);

	if (source)
	{
		m_image_arrow.SetSkinBg("arrow.right");
		m_image_arrow.SetIgnoreInput(true);
		AddChild(&m_image_arrow);
	}
}

TBSimpleLayoutItemWidget::~TBSimpleLayoutItemWidget()
{
	if (m_image_arrow.GetParent())
		RemoveChild(&m_image_arrow);
	RemoveChild(&m_textfield);
	if (m_image.GetParent())
		RemoveChild(&m_image);
}

bool TBSimpleLayoutItemWidget::OnEvent(const TBWidgetEvent &ev)
{
	if (m_source && ev.type == EVENT_TYPE_CLICK && ev.target == this)
	{
		if (!m_window_pointer.Get())
		{
			// Open a new menu window for the submenu with this widget as target
			if (TBMenuWindow *menu = new TBMenuWindow(this, TBIDC("submenu")))
			{
				m_window_pointer.Set(menu);
				menu->Show(m_source, TBPopupAlignment(TB_ALIGN_RIGHT), -1);
			}
		}
		return true;
	}
	return false;
}

// == TBSelectItemViewer ==============================================================================

void TBSelectItemViewer::SetSource(TBSelectItemSource *source)
{
	if (m_source == source)
		return;

	if (m_source)
		m_source->m_viewers.Remove(this);
	m_source = source;
	if (m_source)
		m_source->m_viewers.AddLast(this);

	OnSourceChanged();
}

// == TBSelectItemSource ====================================================================================

TBSelectItemSource::~TBSelectItemSource()
{
	// If this assert trig, you are deleting a model that's still set on some
	// TBSelect widget. That might be dangerous.
	assert(!m_viewers.HasLinks());
}

bool TBSelectItemSource::Filter(int index, const char *filter)
{
	const char *str = GetItemString(index);
	if (str && stristr(str, filter))
		return true;
	return false;
}

TBWidget *TBSelectItemSource::CreateItemWidget(int index, TBSelectItemViewer *viewer)
{
	const char *string = GetItemString(index);
	TBSelectItemSource *sub_source = GetItemSubSource(index);
	TBID image = GetItemImage(index);
	if (sub_source || image)
	{
		if (TBSimpleLayoutItemWidget *itemwidget = new TBSimpleLayoutItemWidget(image, sub_source, string))
			return itemwidget;
	}
	else if (string && *string == '-')
	{
		if (TBSeparator *separator = new TBSeparator)
		{
			separator->SetGravity(WIDGET_GRAVITY_ALL);
			separator->SetSkinBg("TBSelectItem.separator");
			return separator;
		}
	}
	else if (TBTextField *textfield = new TBTextField)
	{
		textfield->SetSkinBg("TBSelectItem");
		textfield->SetText(string);
		textfield->SetTextAlign(TB_TEXT_ALIGN_LEFT);
		return textfield;
	}
	return nullptr;
}

void TBSelectItemSource::InvokeItemChanged(int index, TBSelectItemViewer *exclude_viewer)
{
	TBLinkListOf<TBSelectItemViewer>::Iterator iter = m_viewers.IterateForward();
	while (TBSelectItemViewer *viewer = iter.GetAndStep())
		if (viewer != exclude_viewer)
			viewer->OnItemChanged(index);
}

void TBSelectItemSource::InvokeItemAdded(int index)
{
	TBLinkListOf<TBSelectItemViewer>::Iterator iter = m_viewers.IterateForward();
	while (TBSelectItemViewer *viewer = iter.GetAndStep())
		viewer->OnItemAdded(index);
}

void TBSelectItemSource::InvokeItemRemoved(int index)
{
	TBLinkListOf<TBSelectItemViewer>::Iterator iter = m_viewers.IterateForward();
	while (TBSelectItemViewer *viewer = iter.GetAndStep())
		viewer->OnItemRemoved(index);
}

void TBSelectItemSource::InvokeAllItemsRemoved()
{
	TBLinkListOf<TBSelectItemViewer>::Iterator iter = m_viewers.IterateForward();
	while (TBSelectItemViewer *viewer = iter.GetAndStep())
		viewer->OnAllItemsRemoved();
}

}; // namespace tinkerbell
