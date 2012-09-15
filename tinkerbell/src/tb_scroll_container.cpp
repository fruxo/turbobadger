// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_scroll_container.h"
#include <assert.h>

namespace tinkerbell {

// == TBScrollBarVisibility ===================================

TBScrollBarVisibility TBScrollBarVisibility::Solve(SCROLL_MODE mode, int content_w, int content_h,
																	int available_w, int available_h,
																	int scrollbar_x_h, int scrollbar_y_w)
{
	TBScrollBarVisibility visibility;
	visibility.visible_w = available_w;
	visibility.visible_h = available_h;

	if (mode == SCROLL_MODE_X_Y)
	{
		visibility.y_on = true;
		visibility.x_on = true;
		visibility.visible_w -= scrollbar_y_w;
		visibility.visible_h -= scrollbar_x_h;
	}
	else if (mode == SCROLL_MODE_Y)
	{
		visibility.y_on = true;
		visibility.visible_w -= scrollbar_y_w;
	}
	else if (mode == SCROLL_MODE_Y_AUTO)
	{
		if (content_h > available_h)
		{
			visibility.y_on = true;
			visibility.visible_w -= scrollbar_y_w;
		}
	}
	else if (mode == SCROLL_MODE_X_AUTO_Y_AUTO)
	{
		int thickness = 16;
		if (content_w > visibility.visible_w)
		{
			visibility.x_on = true;
			visibility.visible_h = available_h - scrollbar_x_h;
		}
		if (content_h > visibility.visible_h)
		{
			visibility.y_on = true;
			visibility.visible_w = available_w - scrollbar_y_w;
		}
		if (content_w > visibility.visible_w)
		{
			visibility.x_on = true;
			visibility.visible_h = available_h - scrollbar_x_h;
		}
	}
	return visibility;
}

// == TBScrollContainerRoot ===================================

void TBScrollContainerRoot::OnPaintChildren(const PaintProps &paint_props)
{
	TBRect old_clip_rect = g_renderer->SetClipRect(GetPaddingRect(), true);
	TBWidget::OnPaintChildren(paint_props);
	g_renderer->SetClipRect(old_clip_rect, false);
}

void TBScrollContainerRoot::GetChildTranslation(int &x, int &y) const
{
	TBScrollContainer *sc = static_cast<TBScrollContainer *>(GetParent());
	x = (int) -sc->m_scrollbar_x.GetValue();
	y = (int) -sc->m_scrollbar_y.GetValue();
}

// == TBScrollContainer =======================================

TBScrollContainer::TBScrollContainer()
	: m_adapt_to_content_size(false)
	, m_adapt_content_size(false)
	, m_layout_is_invalid(false)
	, m_mode(SCROLL_MODE_X_Y)
{
	AddChild(&m_scrollbar_x);
	AddChild(&m_scrollbar_y);
	AddChild(&m_root);
	m_root.SetGravity(WIDGET_GRAVITY_ALL);
	m_scrollbar_x.SetGravity(WIDGET_GRAVITY_BOTTOM | WIDGET_GRAVITY_LEFT_RIGHT);
	m_scrollbar_y.SetGravity(WIDGET_GRAVITY_RIGHT | WIDGET_GRAVITY_TOP_BOTTOM);
	m_scrollbar_y.SetAxis(AXIS_Y);
	int scrollbar_y_w = m_scrollbar_y.GetPreferredSize().pref_w;
	int scrollbar_x_h = m_scrollbar_x.GetPreferredSize().pref_h;
	m_scrollbar_x.SetRect(TBRect(0, - scrollbar_x_h, - scrollbar_y_w, scrollbar_x_h));
	m_scrollbar_y.SetRect(TBRect(- scrollbar_y_w, 0, scrollbar_y_w, 0));
	m_root.SetRect(TBRect(0, 0, - scrollbar_y_w, - scrollbar_x_h));
}

TBScrollContainer::~TBScrollContainer()
{
	RemoveChild(&m_root);
	RemoveChild(&m_scrollbar_y);
	RemoveChild(&m_scrollbar_x);
}

void TBScrollContainer::SetScrollMode(SCROLL_MODE mode)
{
	if (mode == m_mode)
		return;
	m_mode = mode;
	InvalidateLayout(INVALIDATE_LAYOUT_TARGET_ONLY);
}

TBRect TBScrollContainer::GetVisibleRect()
{
	int visible_w = GetRect().w;
	int visible_h = GetRect().h;
	if (m_scrollbar_x.GetOpacity())
		visible_h -= m_scrollbar_x.GetPreferredSize().pref_h;
	if (m_scrollbar_y.GetOpacity())
		visible_w -= m_scrollbar_y.GetPreferredSize().pref_w;
	return TBRect(m_scrollbar_x.GetValue(), m_scrollbar_y.GetValue(), visible_w, visible_h);
}

void TBScrollContainer::ScrollTo(int x, int y)
{
	int old_x = m_scrollbar_x.GetValue();
	int old_y = m_scrollbar_y.GetValue();
	m_scrollbar_x.SetValue(x);
	m_scrollbar_y.SetValue(y);
	if (old_x != m_scrollbar_x.GetValue() ||
		old_y != m_scrollbar_y.GetValue())
		Invalidate();
}

void TBScrollContainer::ScrollIntoView(const TBRect &rect)
{
	TBRect visible_rect = GetVisibleRect();
	int new_x = m_scrollbar_x.GetValue();
	int new_y = m_scrollbar_y.GetValue();
	if (rect.y < visible_rect.y)
		new_y = rect.y;
	if (rect.x < visible_rect.x)
		new_x = rect.x;
	if (rect.y + rect.h > visible_rect.y + visible_rect.h)
		new_y = rect.y + rect.h - visible_rect.h;
	if (rect.x + rect.w > visible_rect.x + visible_rect.w)
		new_x = rect.x + rect.w - visible_rect.w;

	ScrollTo(new_x, new_y);
}

void TBScrollContainer::ScrollBy(int &dx, int &dy)
{
	int old_x = m_scrollbar_x.GetValue();
	int old_y = m_scrollbar_y.GetValue();
	ScrollTo(m_scrollbar_x.GetValue() + dx,
			 m_scrollbar_y.GetValue() + dy);
	dx -= m_scrollbar_x.GetValue() - old_x;
	dy -= m_scrollbar_y.GetValue() - old_y;
}

void TBScrollContainer::InvalidateLayout(INVALIDATE_LAYOUT il)
{
	m_layout_is_invalid = true;
	// No recursion up to parents here.
}

PreferredSize TBScrollContainer::GetPreferredContentSize()
{
	PreferredSize ps;
	ps.pref_w = ps.pref_h = 100;
	ps.min_w = ps.min_h = 50;
	if (m_adapt_to_content_size)
	{
		if (TBWidget *content_child = m_root.GetFirstChild())
		{
			ps = content_child->GetPreferredSize();
			int scrollbar_y_w = m_scrollbar_y.GetPreferredSize().pref_w;
			int scrollbar_x_h = m_scrollbar_x.GetPreferredSize().pref_h;

			ps.pref_w += scrollbar_y_w;
			ps.max_w += scrollbar_y_w;

			if (m_mode == SCROLL_MODE_X_Y ||
				m_mode == SCROLL_MODE_X_AUTO_Y_AUTO)
			{
				ps.pref_h += scrollbar_x_h;
				ps.max_h += scrollbar_x_h;
			}
		}
	}
	return ps;
}

bool TBScrollContainer::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CHANGED && (ev.target == &m_scrollbar_x || ev.target == &m_scrollbar_y))
	{
		Invalidate();
		return true;
	}
	else if (ev.type == EVENT_TYPE_WHEEL)
	{
		double old_val = m_scrollbar_y.GetValueDouble();
		m_scrollbar_y.SetValueDouble(old_val + ev.delta * 40);
		return m_scrollbar_y.GetValueDouble() != old_val;
	}
	return false;
}

void TBScrollContainer::OnProcess()
{
	ValidateLayout();
}

void TBScrollContainer::ValidateLayout()
{
	if (!m_layout_is_invalid)
		return;
	m_layout_is_invalid = false;

	if (TBWidget *content_child = m_root.GetFirstChild())
	{
		PreferredSize ps = content_child->GetPreferredSize();

		int scrollbar_x_h = m_scrollbar_x.GetPreferredSize().pref_h;
		int scrollbar_y_w = m_scrollbar_y.GetPreferredSize().pref_w;
		TBScrollBarVisibility visibility = TBScrollBarVisibility::Solve(m_mode, ps.pref_w, ps.pref_h,
																		GetRect().w, GetRect().h,
																		scrollbar_x_h, scrollbar_y_w);
		m_scrollbar_x.SetOpacity(visibility.x_on ? 1.f : 0.f);
		m_scrollbar_y.SetOpacity(visibility.y_on ? 1.f : 0.f);
		m_root.SetRect(TBRect(0, 0, visibility.visible_w, visibility.visible_h));

		int content_w, content_h;
		if (m_adapt_content_size)
		{
			content_w = MAX(ps.pref_w, m_root.GetRect().w);
			content_h = MAX(ps.pref_h, m_root.GetRect().h);
			if (!visibility.x_on && m_root.GetRect().w < ps.pref_w)
				content_w = MIN(ps.pref_w, m_root.GetRect().w);
		}
		else
		{
			content_w = ps.pref_w;
			content_h = ps.pref_h;
		}
		content_child->SetRect(TBRect(0, 0, content_w, content_h));
		double limit_max_w = MAX(0, content_w - m_root.GetRect().w);
		double limit_max_h = MAX(0, content_h - m_root.GetRect().h);
		m_scrollbar_x.SetLimits(0, limit_max_w, m_root.GetRect().w);
		m_scrollbar_y.SetLimits(0, limit_max_h, m_root.GetRect().h);
	}
}

void TBScrollContainer::OnResized(int old_w, int old_h)
{
	TBWidget::OnResized(old_w, old_h);
	InvalidateLayout(INVALIDATE_LAYOUT_TARGET_ONLY);
	ValidateLayout();
}

}; // namespace tinkerbell
