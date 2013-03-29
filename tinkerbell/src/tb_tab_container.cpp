// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_tab_container.h"
#include <assert.h>

namespace tinkerbell {

// == TBTabLayout =======================================================================

int TBTabLayout::GetIndexFromChild(TBWidget *child) const
{
	int index = 0;
	for (TBWidget *tmp = GetFirstChild(); tmp; tmp = tmp->GetNext(), index++)
		if (tmp == child)
			return index;
	return -1;
}

void TBTabLayout::OnChildAdded(TBWidget *child)
{
	if (TBButton *button = TBSafeCast<TBButton>(child))
	{
		button->SetSqueezable(true);
		button->SetSkinBg("TBTabContainer.tab");
		button->GetID().Set("tab");
	}
}

// == TBTabContainer ====================================================================

TBTabContainer::TBTabContainer()
	: m_need_page_update(true)
	, m_current_page(0)
	, m_align(TB_ALIGN_TOP)
{
	AddChild(&m_root_layout);
	// Put the tab layout on top of the content in Z order so their skin can make
	// a seamless overlap over the border. Control which side they are layouted
	// to by calling SetLayoutOrder.
	m_root_layout.AddChild(&m_content_root);
	m_root_layout.AddChild(&m_tab_layout);
	m_root_layout.SetAxis(AXIS_Y);
	m_root_layout.SetSpacing(-1);
	m_root_layout.SetGravity(WIDGET_GRAVITY_ALL);
	m_root_layout.SetLayoutDistribution(LAYOUT_DISTRIBUTION_AVAILABLE);
	m_root_layout.SetLayoutOrder(LAYOUT_ORDER_TOP_TO_BOTTOM);
	m_tab_layout.SetLayoutDistributionPosition(LAYOUT_DISTRIBUTION_POSITION_CENTER);
	m_tab_layout.SetSkinBg("TBTabContainer.tablayout_x");
	m_tab_layout.SetLayoutPosition(LAYOUT_POSITION_RIGHT_BOTTOM);
	m_content_root.SetGravity(WIDGET_GRAVITY_ALL);
	m_content_root.SetSkinBg("TBTabContainer.container");
}

TBTabContainer::~TBTabContainer()
{
	m_root_layout.RemoveChild(&m_content_root);
	m_root_layout.RemoveChild(&m_tab_layout);
	RemoveChild(&m_root_layout);
}

void TBTabContainer::SetAxis(AXIS axis)
{
	m_root_layout.SetAxis(axis);
	m_tab_layout.SetAxis(axis == AXIS_X ? AXIS_Y : AXIS_X);
	m_tab_layout.SetSkinBg(axis == AXIS_X ? "TBTabContainer.tablayout_y" : "TBTabContainer.tablayout_x");
}

void TBTabContainer::SetCurrentPage(int index)
{
	if (index == m_current_page)
		return;
	m_current_page = index;

	// Update the pages visibility and tabs pressed value.
	index = 0;
	TBWidget *page = m_content_root.GetFirstChild();
	TBWidget *tab = m_tab_layout.GetFirstChild();
	for (   ; page && tab; page = page->GetNext(), tab = tab->GetNext(), index++)
	{
		bool active = index == m_current_page;
		page->SetOpacity(active ? 1.0f : 0.0f);
		tab->SetValue(active ? 1 : 0);
	}
}

void TBTabContainer::SetAlignment(TB_ALIGN align)
{
	bool horizontal = (align == TB_ALIGN_TOP || align == TB_ALIGN_BOTTOM);
	bool reverse = (align == TB_ALIGN_TOP || align == TB_ALIGN_LEFT);
	SetAxis(horizontal ? AXIS_Y : AXIS_X);
	m_root_layout.SetLayoutOrder(reverse ? LAYOUT_ORDER_TOP_TO_BOTTOM : LAYOUT_ORDER_BOTTOM_TO_TOP);
	m_tab_layout.SetLayoutPosition(reverse ? LAYOUT_POSITION_RIGHT_BOTTOM : LAYOUT_POSITION_LEFT_TOP);
	m_align = align;
}

bool TBTabContainer::OnEvent(const TBWidgetEvent &ev)
{
	if ((ev.type == EVENT_TYPE_CLICK || ev.type == EVENT_TYPE_POINTER_DOWN) &&
		ev.target->GetID() == TBIDC("tab"))
	{
		int clicked_index = m_tab_layout.GetIndexFromChild(ev.target);
		SetCurrentPage(clicked_index);
		return true;
	}
	return false;
}

void TBTabContainer::OnProcess()
{
	if (m_need_page_update)
	{
		m_need_page_update = false;
		// Force update SetCurrentPage
		int current_page = m_current_page;
		m_current_page = -1;
		SetCurrentPage(current_page);
	}
}

}; // namespace tinkerbell
