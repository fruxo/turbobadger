// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_common.h"
#include <assert.h>

namespace tinkerbell {

// == TBWidgetString =======================================

TBWidgetString::TBWidgetString()
	: m_text_align(TB_TEXT_ALIGN_CENTER)
{
}

int TBWidgetString::GetWidth()
{
	return g_renderer->GetStringWidth(m_text);
}

int TBWidgetString::GetHeight()
{
	return g_renderer->GetFontHeight();
}

void TBWidgetString::Paint(const TBRect &rect, const TBColor &color)
{
	int string_w = GetWidth();

	int x = rect.x;
	if (m_text_align == TB_TEXT_ALIGN_RIGHT)
		x += rect.w - string_w;
	else if (m_text_align == TB_TEXT_ALIGN_CENTER)
		x += MAX(0, (rect.w - string_w) / 2);
	int y = rect.y + (rect.h - GetHeight()) / 2;

	if (string_w <= rect.w)
		g_renderer->DrawString(x, y, color, m_text);
	else
	{
		// There's not enough room for the endie string
		// so it cut of and end with ellipsis (...)
		char end[2] = { (char)133, 0 };
		int endw = g_renderer->GetStringWidth(end);
		int startw = 0;
		int startlen = 0;
		while (m_text.CStr()[startlen])
		{
			int new_startw = g_renderer->GetStringWidth(m_text, startlen);
			if (new_startw + endw > rect.w)
				break;
			startw = new_startw;
			startlen++;
		}
		startlen = MAX(0, startlen - 1);
		g_renderer->DrawString(x, y, color, m_text, startlen);
		g_renderer->DrawString(x + startw, y, color, end);
	}
}

// == TBTextField =======================================

TBTextField::TBTextField()
	: m_squeezable(false)
{
	m_skin_bg.Set("TBTextField");
}

bool TBTextField::SetText(const char *text)
{
	if (m_text.m_text.Equals(text))
		return true;
	Invalidate();
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	return m_text.SetText(text);
}

void TBTextField::SetSqueezable(bool squeezable)
{
	if (squeezable == m_squeezable)
		return;
	m_squeezable = squeezable;
	Invalidate();
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
}

PreferredSize TBTextField::GetPreferredContentSize()
{
	PreferredSize ps;
	ps.pref_w = m_text.GetWidth();
	ps.pref_h = ps.min_h = m_text.GetHeight();
	if (!m_squeezable)
		ps.min_w = ps.pref_w;
	return ps;
}

void TBTextField::OnPaint(const PaintProps &paint_props)
{
	TBRect clip_rect = GetPaddingRect();
	TBRect old_clip_rect = g_renderer->SetClipRect(clip_rect, true);

	m_text.Paint(clip_rect, paint_props.text_color);

	g_renderer->SetClipRect(old_clip_rect, false);
}

// == TBButton =======================================

const int auto_click_first_delay = 500;
const int auto_click_repeat_delay = 100;

TBButton::TBButton()
	: m_auto_repeat_click(false)
{
	SetIsFocusable(true);
	SetClickByKey(true);
	m_skin_bg.Set("TBButton");
	AddChild(&m_layout);
	// Set the textfield gravity to all, even though it would display the same with default gravity.
	// This will make the buttons layout expand if there is space available, without forcing the parent
	// layout to grow to make the space available.
	m_textfield.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.AddChild(&m_textfield);
	m_layout.SetRect(GetPaddingRect());
	m_layout.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.SetPaintOverflowFadeout(false);
}

TBButton::~TBButton()
{
	m_layout.RemoveChild(&m_textfield);
	RemoveChild(&m_layout);
}

void TBButton::OnCaptureChanged(bool captured)
{
	if (captured && m_auto_repeat_click)
		PostMessageDelayed("auto_click", nullptr, auto_click_first_delay);
	else if (!captured)
	{
		if (TBMessage *msg = GetMessageByID("auto_click"))
			DeleteMessage(msg);
	}
}

void TBButton::OnSkinChanged()
{
	m_layout.SetRect(GetPaddingRect());
}

void TBButton::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBID("auto_click"))
	{
		assert(captured_widget == this);
		if (!is_panning && GetHitStatus(pointer_move_widget_x, pointer_move_widget_y))
		{
			WidgetEvent ev(EVENT_TYPE_CLICK, pointer_move_widget_x, pointer_move_widget_y, 0);
			captured_widget->InvokeEvent(ev);
		}
		if (auto_click_repeat_delay)
			PostMessageDelayed("auto_click", nullptr, auto_click_repeat_delay);
	}
}

WIDGET_HIT_STATUS TBButton::GetHitStatus(int x, int y)
{
	// Never hit any of the children to the button. We always want to the button itself.
	return Widget::GetHitStatus(x, y) ? WIDGET_HIT_STATUS_HIT_NO_CHILDREN : WIDGET_HIT_STATUS_NO_HIT;
}

// == TBClickLabel ==========================================================================================

TBClickLabel::TBClickLabel()
{
	AddChild(&m_layout);
	m_layout.AddChild(&m_textfield);
	m_layout.SetRect(GetPaddingRect());
	m_layout.SetGravity(WIDGET_GRAVITY_ALL);
}

TBClickLabel::~TBClickLabel()
{
	m_layout.RemoveChild(&m_textfield);
	RemoveChild(&m_layout);
}

bool TBClickLabel::OnEvent(const WidgetEvent &ev)
{
	// Get a widget from the layout that isn't the textfield, or just bail out
	// if we only have the textfield.
	if (m_layout.m_children.GetFirst() == m_layout.m_children.GetLast())
		return false;
	Widget *click_target = (m_layout.GetFirstChild() == &m_textfield ? m_layout.GetLastChild() : m_layout.GetFirstChild());
	// Invoke the event on it, as if it was invoked on the target itself.
	if (click_target && ev.target == &m_textfield)
	{
		click_target->SetState(WIDGET_STATE_PRESSED, (m_textfield.GetAutoState() & WIDGET_STATE_PRESSED) ? true : false);
		WidgetEvent target_ev(ev.type, ev.target_x - click_target->m_rect.x, ev.target_y - click_target->m_rect.y);
		return click_target->InvokeEvent(target_ev);
	}
	return false;
}

// == TBSkinImage =======================================

PreferredSize TBSkinImage::GetPreferredSize()
{
	PreferredSize ps = Widget::GetPreferredSize();
	// FIX: Make it stretched proportionally if shrunk.
	ps.max_w = ps.pref_w;
	ps.max_h = ps.pref_h;
	return ps;
}

// == TBSeparator ===========================================

// == TBProgressSpinner =====================================

// FIX: Add spin_speed to skin!
// FIX: Make it post messages only if visible
const int spin_speed = 1000/30; ///< How fast should the spinner animation animate.

TBProgressSpinner::TBProgressSpinner()
	: m_value(0)
	, m_frame(0)
{
	m_skin_bg.Set("TBProgressSpinner");
	m_skin_fg.Set("TBProgressSpinner.fg");
}

void TBProgressSpinner::SetValue(int value)
{
	assert(value >= 0); // If this happens, you probably have unballanced Begin/End calls.
	m_value = value;
	if (value > 0)
	{
		// Start animation
		if (!GetMessageByID(TBID(1)))
		{
			m_frame = 0;
			PostMessageDelayed(TBID(1), nullptr, spin_speed);
		}
	}
	else
	{
		// Stop animation
		if (TBMessage *msg = GetMessageByID(TBID(1)))
			DeleteMessage(msg);
	}
}

void TBProgressSpinner::OnPaint(const PaintProps &paint_props)
{
	if (IsRunning())
	{
		TBSkinElement *e = g_tb_skin->GetSkinElement(m_skin_fg);
		if (e && e->bitmap)
		{
			int size = e->bitmap->Height();
			int num_frames = e->bitmap->Width() / e->bitmap->Height();
			int current_frame = m_frame % num_frames;
			g_renderer->DrawBitmap(TBRect(0, 0, m_rect.w, m_rect.h), TBRect(current_frame * size, 0, size, size), e->bitmap);
		}
	}
}

void TBProgressSpinner::OnMessageReceived(TBMessage *msg)
{
	m_frame++;
	Invalidate();
	// Keep animation running
	PostMessageOnTime(TBID(1), nullptr, msg->GetFireTime() + spin_speed);
}

// == TBRadioCheckBox =======================================

TBRadioCheckBox::TBRadioCheckBox()
	: m_value(0)
{
	SetIsFocusable(true);
	SetClickByKey(true);
}

void TBRadioCheckBox::ToggleGroup(Widget *root, Widget *toggled)
{
	if (root != toggled && root->m_group_id == toggled->m_group_id)
		root->SetValue(0);
	for (Widget *child = root->GetFirstChild(); child; child = child->GetNext())
		ToggleGroup(child, toggled);
}

void TBRadioCheckBox::SetValue(int value)
{
	if (m_value == value)
		return;
	m_value = value;

	SetState(WIDGET_STATE_SELECTED, value ? true : false);

	Invalidate();
	WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	InvokeEvent(ev);

	if (!value || !m_group_id)
		return;
	// Toggle all other widgets in the same group. First get a root widget
	// for the search.
	Widget *group = this;
	while (group && !group->GetIsGroupRoot())
		group = group->m_parent;
	if (group)
	{
		ToggleGroup(group, this);
	}
}

PreferredSize TBRadioCheckBox::GetPreferredSize()
{
	PreferredSize ps = Widget::GetPreferredSize();
	ps.min_w = ps.max_w = ps.pref_w;
	ps.min_h = ps.max_h = ps.pref_h;
	return ps;
}

bool TBRadioCheckBox::OnEvent(const WidgetEvent &ev)
{
	if (ev.target == this && ev.type == EVENT_TYPE_CLICK)
	{
		// Toggle the value, if it's not a grouped widget with value on.
		if (!(m_group_id && GetValue()))
		{
			SetValue(!GetValue());
		}
	}
	return false;
}

// == TBScrollBar =======================================

TBScrollBar::TBScrollBar()
	: m_axis(AXIS_Y) ///< Make SetAxis below always succeed and set the skin
	, m_value(0)
	, m_min(0)
	, m_max(1)
	, m_visible(1)
	, m_to_pixel_factor(0)
{
	SetAxis(AXIS_X);
	AddChild(&m_handle);
}

TBScrollBar::~TBScrollBar()
{
	RemoveChild(&m_handle);
}

void TBScrollBar::SetAxis(AXIS axis)
{
	if (axis == m_axis)
		return;
	m_axis = axis;
	if (axis == AXIS_X)
	{
		m_skin_bg.Set("TBScrollBarBgX");
		m_handle.m_skin_bg.Set("TBScrollBarFgX");
	}
	else
	{
		m_skin_bg.Set("TBScrollBarBgY");
		m_handle.m_skin_bg.Set("TBScrollBarFgY");
	}
	Invalidate();
}

void TBScrollBar::SetLimits(double min, double max, double visible)
{
	min = MIN(min, max);
	visible = MAX(visible, 0);
	if (min == m_min && max == m_max && m_visible == visible)
		return;
	m_min = min;
	m_max = max;
	m_visible = visible;
	SetValueDouble(m_value);
	UpdateHandle();
}

void TBScrollBar::SetValueDouble(double value)
{
	value = CLAMP(value, m_min, m_max);
	if (value == m_value)
		return;
	m_value = value;

	UpdateHandle();
	WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	InvokeEvent(ev);
}

bool TBScrollBar::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_POINTER_MOVE && captured_widget == &m_handle)
	{
		if (m_to_pixel_factor > 0)
		{
			int dx = ev.target_x - pointer_down_widget_x;
			int dy = ev.target_y - pointer_down_widget_y;
			double delta_val = (m_axis == AXIS_X ? dx : dy) / m_to_pixel_factor;
			SetValueDouble(m_value + delta_val);
		}
		return true;
	}
	else if (ev.type == EVENT_TYPE_POINTER_MOVE && ev.target == this)
		return true;
	else if (ev.type == EVENT_TYPE_POINTER_DOWN && ev.target == this)
	{
		bool after_handle = (m_axis == AXIS_X ? ev.target_x > m_handle.m_rect.x : ev.target_y > m_handle.m_rect.y);
		SetValueDouble(m_value + (after_handle ? m_visible : -m_visible));
		return true;
	}
	else if (ev.type == EVENT_TYPE_WHEEL)
	{
		double old_val = m_value;
		SetValueDouble(m_value + ev.delta * 40);
		return m_value != old_val;
	}
	return false;
}

void TBScrollBar::UpdateHandle()
{
	// Calculate the mover size and position
	bool horizontal = m_axis == AXIS_X;
	int available_pixels = horizontal ? m_rect.w : m_rect.h;
	int available_thickness_pixels = horizontal ? m_rect.h : m_rect.w;

	int visible_pixels = available_pixels;

	if (m_max - m_min > 0 && m_visible > 0)
	{
		double visible_proportion = m_visible / (m_visible + m_max - m_min);
		visible_pixels = (int)(visible_proportion * available_pixels);

		// Limit the size of the indicator to the slider thickness
		visible_pixels = MAX(visible_pixels, available_thickness_pixels);

		m_to_pixel_factor = (double)(available_pixels - visible_pixels) / (m_max - m_min)/*+ 0.5*/;
	}
	else
	{
		m_to_pixel_factor = 0;

		// If we can't scroll anything, make the handle invisible
		visible_pixels = 0;
	}

	int pixel_pos = (int)(m_value * m_to_pixel_factor);

	TBRect rect;
	if (horizontal)
		rect.Set(pixel_pos, 0, visible_pixels, m_rect.h);
	else
		rect.Set(0, pixel_pos, m_rect.w, visible_pixels);

	m_handle.SetRect(rect);
}

// == TBContainer ===================================

TBContainer::TBContainer()
{
	m_skin_bg.Set("TBContainer");
}

// == TBMover =======================================

TBMover::TBMover()
{
	m_skin_bg.Set("TBMover");
}

bool TBMover::OnEvent(const WidgetEvent &ev)
{
	if (!m_parent)
		return false;
	if (ev.type == EVENT_TYPE_POINTER_MOVE && captured_widget == this)
	{
		int dx = ev.target_x - pointer_down_widget_x;
		int dy = ev.target_y - pointer_down_widget_y;
		TBRect rect = m_parent->m_rect.Offset(dx, dy);
		if (m_parent->m_parent)
		{
			// Apply limit.
			rect.x = CLAMP(rect.x, -pointer_down_widget_x, m_parent->m_parent->m_rect.w - pointer_down_widget_x);
			rect.y = CLAMP(rect.y, -pointer_down_widget_y, m_parent->m_parent->m_rect.h - pointer_down_widget_y);
		}
		m_parent->SetRect(rect);
		return true;
	}
	return false;
}

// == TBResizer =======================================

TBResizer::TBResizer()
{
	m_skin_bg.Set("TBResizer");
}

bool TBResizer::OnEvent(const WidgetEvent &ev)
{
	if (!m_parent)
		return false;
	if (ev.type == EVENT_TYPE_POINTER_MOVE && captured_widget == this)
	{
		int dx = ev.target_x - pointer_down_widget_x;
		int dy = ev.target_y - pointer_down_widget_y;
		TBRect rect = m_parent->m_rect;
		rect.w += dx;
		rect.h += dy;
		// Apply limit. We should not use minimum size since we can squeeze
		// the layout much more, and provide scroll/pan when smaller.
		rect.w = MAX(rect.w, 50);
		rect.h = MAX(rect.h, 50);
		m_parent->SetRect(rect);
	}
	else
		return false;
	return true;
}

// == TBDimmer =======================================

TBDimmer::TBDimmer()
{
	m_skin_bg.Set("TBDimmer");
	SetGravity(WIDGET_GRAVITY_ALL);
}

void TBDimmer::OnAdded()
{
	SetRect(TBRect(0, 0, m_parent->m_rect.w, m_parent->m_rect.h));
}

}; // namespace tinkerbell
