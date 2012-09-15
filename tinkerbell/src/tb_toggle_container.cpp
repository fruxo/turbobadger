// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_toggle_container.h"
#include "tb_widgets_reader.h"
#include "parser/TBNodeTree.h"

namespace tinkerbell {

// == TBSectionHeader =====================================

TB_WIDGET_FACTORY(TBSectionHeader, TBValue::TYPE_INT, WIDGET_Z_TOP) { }

TBSectionHeader::TBSectionHeader()
{
	SetSkinBg("TBSectionHeader");
	SetGravity(WIDGET_GRAVITY_LEFT | WIDGET_GRAVITY_RIGHT);
	SetToggleMode(true);
}

bool TBSectionHeader::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.target == this && ev.type == EVENT_TYPE_CHANGED && GetParent()->GetParent())
	{
		if (TBSection *section = TBSafeCast(TBSection, GetParent()->GetParent()))
		{
			section->GetContainer()->SetValue(GetValue());

			// Try to scroll the container into view when expanded
			section->SetPendingScrollIntoView(GetValue() ? true : false);
		}
	}
	return TBButton::OnEvent(ev);
}

// == TBSectionHeader =====================================

TB_WIDGET_FACTORY(TBSection, TBValue::TYPE_INT, WIDGET_Z_TOP) { }

TBSection::TBSection()
	: m_pending_scroll(false)
{
	SetGravity(WIDGET_GRAVITY_LEFT | WIDGET_GRAVITY_RIGHT);

	SetSkinBg("TBSection", WIDGET_INVOKE_INFO_NO_CALLBACKS);
	m_layout.SetSkinBg("TBSection.layout", WIDGET_INVOKE_INFO_NO_CALLBACKS);

	m_toggle_container.SetSkinBg("TBSection.container");
	m_toggle_container.SetToggle(TBToggleContainer::TOGGLE_EXPANDED);
	m_toggle_container.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.SetAxis(AXIS_Y);
	m_layout.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.SetLayoutSize(LAYOUT_SIZE_AVAILABLE);

	AddChild(&m_layout);
	m_layout.AddChild(&m_header);
	m_layout.AddChild(&m_toggle_container);
}

TBSection::~TBSection()
{
	m_layout.RemoveChild(&m_toggle_container);
	m_layout.RemoveChild(&m_header);
	RemoveChild(&m_layout);
}

void TBSection::SetValue(int value)
{
	m_header.SetValue(value);
	m_toggle_container.SetValue(value);
}

void TBSection::OnProcessAfterChildren()
{
	if (m_pending_scroll)
	{
		m_pending_scroll = false;
		ScrollIntoViewRecursive();
	}
}

// == TBToggleContainer ===================================

TB_WIDGET_FACTORY(TBToggleContainer, TBValue::TYPE_INT, WIDGET_Z_TOP)
{
	if (const char *toggle = info->node->GetValueString("toggle", nullptr))
	{
		if (stristr(toggle, "enabled"))			widget->SetToggle(TBToggleContainer::TOGGLE_ENABLED);
		else if (stristr(toggle, "opacity"))	widget->SetToggle(TBToggleContainer::TOGGLE_OPACITY);
		else if (stristr(toggle, "expanded"))	widget->SetToggle(TBToggleContainer::TOGGLE_EXPANDED);
	}
	widget->SetInvert(info->node->GetValueInt("invert", widget->GetInvert()) ? true : false);
}

TBToggleContainer::TBToggleContainer()
	: m_toggle(TOGGLE_NOTHING)
	, m_invert(false)
	, m_value(0)
{
	SetSkinBg("TBToggleContainer", WIDGET_INVOKE_INFO_NO_CALLBACKS);
}

void TBToggleContainer::SetToggle(TOGGLE toggle)
{
	if (toggle == m_toggle)
		return;

	if (m_toggle == TOGGLE_EXPANDED)
		InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);

	m_toggle = toggle;
	UpdateInternal();
}

void TBToggleContainer::SetInvert(bool invert)
{
	if (invert == m_invert)
		return;
	m_invert = invert;
	UpdateInternal();
}

void TBToggleContainer::SetValue(int value)
{
	if (value == m_value)
		return;
	m_value = value;
	UpdateInternal();
}

void TBToggleContainer::UpdateInternal()
{
	bool on = GetIsOn();
	switch (m_toggle)
	{
	case TOGGLE_ENABLED:
		SetState(WIDGET_STATE_DISABLED, !on);
		break;
	case TOGGLE_OPACITY:
		SetOpacity(on ? 1.f : 0);
		break;
	case TOGGLE_EXPANDED:
		// Relayout. GetPreferredSize() will apply the changed size.
		InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
		// Also disable when collapsed so tab focus skips the children.
		SetState(WIDGET_STATE_DISABLED, !on);
		break;
	};
}

PreferredSize TBToggleContainer::GetPreferredSize()
{
	PreferredSize ps = TBWidget::GetPreferredSize();
	if (m_toggle == TOGGLE_EXPANDED)
	{
		if (!GetIsOn())
		{
			if (GetParent()->GetAxis() == AXIS_X)
				ps.min_w = ps.pref_w = ps.max_w = 0;
			else
				ps.min_h = ps.pref_h = ps.max_h = 0;
		}
	}
	return ps;
}

}; // namespace tinkerbell
