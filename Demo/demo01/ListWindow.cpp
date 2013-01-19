#include "ListWindow.h"

// == AdcanvedItemWidget ======================================================

AdcanvedItemWidget::AdcanvedItemWidget(AdvancedItem *item, AdvancedItemSource *source,
										TBSelectItemViewer *source_viewer, int index)
	: m_source(source)
	, m_source_viewer(source_viewer)
	, m_index(index)
{
	SetSkinBg("TBSelectItem");
	SetLayoutDistribution(LAYOUT_DISTRIBUTION_AVAILABLE);
	SetPaintOverflowFadeout(false);

	if (TBSkinImage *image = new TBSkinImage)
	{
		image->SetSkinBg("Icon48");
		image->SetIgnoreInput(true);
		AddChild(image);
	}

	if (TBTextField *textfield = new TBTextField)
	{
		textfield->SetText(item->str);
		textfield->SetTextAlign(TB_TEXT_ALIGN_LEFT);
		textfield->SetIgnoreInput(true);
		AddChild(textfield);
	}

	if (TBCheckBox *checkbox = new TBCheckBox)
	{
		checkbox->SetValue(item->GetChecked() ? true : false);
		checkbox->GetID().Set("check");
		AddChild(checkbox);
	}
}

bool AdcanvedItemWidget::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && ev.target->IsOfType<TBCheckBox>())
	{
		AdvancedItem *item = m_source->GetItem(m_index);
		item->SetChecked(ev.target->GetValue() ? true : false);

		m_source->InvokeItemChanged(m_index, m_source_viewer);
		return true;
	}
	return TBLayout::OnEvent(ev);
}

// == AdvancedItemSource ======================================================

TBWidget *AdvancedItemSource::CreateItemWidget(int index, TBSelectItemViewer *viewer)
{
	if (TBLayout *layout = new AdcanvedItemWidget(GetItem(index), this, viewer, index))
		return layout;
	return nullptr;
}

// == ListWindow ==============================================================

ListWindow::ListWindow(TBSelectItemSource *source)
{
	LoadResourceFile("Demo/demo01/ui_resources/test_select.tb.txt");
	if (TBSelectList *select = GetWidgetByIDAndType<TBSelectList>("list"))
	{
		select->SetSource(source);
		select->GetScrollContainer()->SetScrollMode(SCROLL_MODE_Y_AUTO);
	}
}

bool ListWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CHANGED && ev.target->GetID() == TBIDC("filter"))
	{
		if (TBSelectList *select = GetWidgetByIDAndType<TBSelectList>("list"))
			select->SetFilter(ev.target->GetText());
		return true;
	}
	return DemoWindow::OnEvent(ev);
}

// == AdvancedListWindow ==============================================================

AdvancedListWindow::AdvancedListWindow(AdvancedItemSource *source)
	: m_source(source)
{
	LoadResourceFile("Demo/demo01/ui_resources/test_select_advanced.tb.txt");
	if (TBSelectList *select = GetWidgetByIDAndType<TBSelectList>("list"))
	{
		select->SetSource(source);
		select->GetScrollContainer()->SetScrollMode(SCROLL_MODE_X_AUTO_Y_AUTO);
	}
}

bool AdvancedListWindow::OnEvent(const TBWidgetEvent &ev)
{
	TBSelectList *select = GetWidgetByIDAndType<TBSelectList>("list");
	if (select && ev.type == EVENT_TYPE_CHANGED && ev.target->GetID() == TBIDC("filter"))
	{
		select->SetFilter(ev.target->GetText());
		return true;
	}
	else if (select && ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("add"))
	{
		TBStr name = GetTextByID(TBIDC("name"));
		if (!name.IsEmpty())
			m_source->AddItem(new AdvancedItem(name, TBIDC("boy_item")));
		return true;
	}
	else if (select && ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("delete"))
	{
		if (select->GetValue() >= 0 && select->GetValue() < m_source->GetNumItems())
			m_source->DeleteItem(select->GetValue());
		return true;
	}
	else if (select && ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("delete all"))
	{
		m_source->DeleteAllItems();
		return true;
	}
	return DemoWindow::OnEvent(ev);
}
