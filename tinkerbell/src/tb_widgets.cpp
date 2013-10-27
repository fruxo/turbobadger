// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets.h"
#include "tb_window.h"
#include "tb_widgets_listener.h"
#include "tb_renderer.h"
#include "tb_widgets_common.h"
#include "tb_widget_skin_condition_context.h"
#include "tb_system.h"
#include "tb_scroller.h"
#include "tb_font_renderer.h"
#include <assert.h>
#ifdef TB_ALWAYS_SHOW_EDIT_FOCUS
#include "tb_editfield.h"
#endif // TB_ALWAYS_SHOW_EDIT_FOCUS

namespace tinkerbell {

//static data
TBWidget *TBWidget::hovered_widget = nullptr;
TBWidget *TBWidget::captured_widget = nullptr;
TBWidget *TBWidget::focused_widget = nullptr;
int TBWidget::pointer_down_widget_x = 0;
int TBWidget::pointer_down_widget_y = 0;
int TBWidget::pointer_move_widget_x = 0;
int TBWidget::pointer_move_widget_y = 0;
bool TBWidget::cancel_click = false;
bool TBWidget::update_widget_states = true;
bool TBWidget::update_skin_states = true;
bool TBWidget::show_focus_state = false;

// == TBWidget::PaintProps ==============================================================

TBWidget::PaintProps::PaintProps()
{
	// Set the default properties, used for the root widgets
	// calling InvokePaint. The base values for all inheritance.
	text_color = g_tb_skin->GetDefaultTextColor();
}

// == TBWidget ==========================================================================

TBWidget::TBWidget()
	: m_parent(nullptr)
	, m_opacity(1.f)
	, m_state(WIDGET_STATE_NONE)
	, m_gravity(WIDGET_GRAVITY_DEFAULT)
	, m_layout_params(nullptr)
	, m_scroller(nullptr)
	, m_packed_init(0)
{
#ifdef TB_RUNTIME_DEBUG_INFO
	last_measure_time = 0;
	last_layout_time = 0;
#endif // TB_RUNTIME_DEBUG_INFO
}

TBWidget::~TBWidget()
{
	assert(!m_parent); ///< A widget must be removed from parent before deleted
	m_packed.is_dying = true;

	if (this == hovered_widget)
		hovered_widget = nullptr;
	if (this == captured_widget)
		captured_widget = nullptr;
	if (this == focused_widget)
		focused_widget = nullptr;

	TBWidgetListener::InvokeWidgetDelete(this);
	DeleteAllChildren();

	delete m_scroller;
	delete m_layout_params;

	assert(!m_listeners.HasLinks()); // There's still listeners added to this widget!
}

void TBWidget::SetRect(const TBRect &rect)
{
	if (m_rect.Equals(rect))
		return;

	TBRect old_rect = m_rect;
	m_rect = rect;

	if (old_rect.w != m_rect.w || old_rect.h != m_rect.h)
		OnResized(old_rect.w, old_rect.h);

	Invalidate();
}

void TBWidget::Invalidate()
{
	if (!GetVisibility() && !m_rect.IsEmpty())
		return;
	TBWidget *tmp = this;
	while (tmp)
	{
		tmp->OnInvalid();
		tmp = tmp->m_parent;
	}
}

void TBWidget::InvalidateStates()
{
	update_widget_states = true;
	update_skin_states = true;
}

void TBWidget::Die()
{
	if (m_packed.is_dying)
		return;
	m_packed.is_dying = true;
	OnDie();
	if (!TBWidgetListener::InvokeWidgetDying(this))
	{
		// No one was interested, so die immediately.
		if (m_parent)
			m_parent->RemoveChild(this);
		delete this;
	}
}

TBWidget *TBWidget::GetWidgetByIDInternal(const TBID &id, const TB_TYPE_ID type_id)
{
	if (m_id == id && (!type_id || IsOfTypeId(type_id)))
		return this;
	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (TBWidget *sub_child = child->GetWidgetByIDInternal(id, type_id))
			return sub_child;
	}
	return nullptr;
}

TBStr TBWidget::GetTextByID(const TBID &id)
{
	if (TBWidget *widget = GetWidgetByID(id))
		return widget->GetText();
	return "";
}

int TBWidget::GetValueByID(const TBID &id)
{
	if (TBWidget *widget = GetWidgetByID(id))
		return widget->GetValue();
	return 0;
}

void TBWidget::SetStateRaw(WIDGET_STATE state)
{
	if (m_state == state)
		return;
	m_state = state;
	Invalidate();
}

void TBWidget::SetState(WIDGET_STATE state, bool on)
{
	SetStateRaw(on ? m_state | state : m_state & ~state);
}

WIDGET_STATE TBWidget::GetAutoState() const
{
	WIDGET_STATE state = m_state;
	bool add_pressed_state = !cancel_click && this == captured_widget && this == hovered_widget;
	if (add_pressed_state)
		state |= WIDGET_STATE_PRESSED;
	if (this == hovered_widget && (!m_packed.no_automatic_hover_state || add_pressed_state))
		state |= WIDGET_STATE_HOVERED;
	if (this == focused_widget && show_focus_state)
		state |= WIDGET_STATE_FOCUSED;
#ifdef TB_ALWAYS_SHOW_EDIT_FOCUS
	else if (this == focused_widget && IsOfType<TBEditField>())
		state |= WIDGET_STATE_FOCUSED;
#endif
	return state;
}

//static
void TBWidget::SetAutoFocusState(bool on)
{
	if (show_focus_state == on)
		return;
	show_focus_state = on;
	if (focused_widget)
		focused_widget->Invalidate();
}

void TBWidget::SetOpacity(float opacity)
{
	opacity = CLAMP(opacity, 0, 1);
	if (m_opacity == opacity)
		return;
	m_opacity = opacity;
	Invalidate();
}

bool TBWidget::GetVisibility() const
{
	const TBWidget *tmp = this;
	while (tmp)
	{
		if (tmp->GetOpacity() == 0)
			return false;
		tmp = tmp->m_parent;
	}
	return true;
}

bool TBWidget::GetDisabled() const
{
	const TBWidget *tmp = this;
	while (tmp)
	{
		if (tmp->GetState(WIDGET_STATE_DISABLED))
			return true;
		tmp = tmp->m_parent;
	}
	return false;
}

void TBWidget::AddChild(TBWidget *child, WIDGET_Z z, WIDGET_INVOKE_INFO info)
{
	AddChildRelative(child, z == WIDGET_Z_TOP ? WIDGET_Z_REL_AFTER : WIDGET_Z_REL_BEFORE, nullptr, info);
}

void TBWidget::AddChildRelative(TBWidget *child, WIDGET_Z_REL z, TBWidget *reference, WIDGET_INVOKE_INFO info)
{
	assert(!child->m_parent);
	child->m_parent = this;

	if (reference)
	{
		if (z == WIDGET_Z_REL_BEFORE)
			m_children.AddBefore(child, reference);
		else
			m_children.AddAfter(child, reference);
	}
	else // If there is no reference widget, before means first and after means last.
	{
		if (z == WIDGET_Z_REL_BEFORE)
			m_children.AddFirst(child);
		else
			m_children.AddLast(child);
	}

	if (info == WIDGET_INVOKE_INFO_NORMAL)
	{
		OnChildAdded(child);
		child->OnAdded();
		TBWidgetListener::InvokeWidgetAdded(child);
	}
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	Invalidate();
}

void TBWidget::RemoveChild(TBWidget *child, WIDGET_INVOKE_INFO info)
{
	assert(child->m_parent);

	if (info == WIDGET_INVOKE_INFO_NORMAL)
	{
		// If we're not being deleted and delete the focused widget, try
		// to keep the focus in this widget by moving it to the next widget.
		if (!m_packed.is_dying && child == focused_widget)
			m_parent->MoveFocus(true);

		OnChildRemove(child);
		child->OnRemove();
		TBWidgetListener::InvokeWidgetRemove(child);
	}

	m_children.Remove(child);
	child->m_parent = nullptr;

	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	Invalidate();
}

void TBWidget::DeleteAllChildren()
{
	while (TBWidget *child = GetFirstChild())
	{
		RemoveChild(child);
		delete child;
	}
}

void TBWidget::SetZ(WIDGET_Z z)
{
	if (!m_parent)
		return;
	if (z == WIDGET_Z_TOP && this == m_parent->m_children.GetLast())
		return; // Already at the top
	if (z == WIDGET_Z_BOTTOM && this == m_parent->m_children.GetFirst())
		return; // Already at the top
	TBWidget *parent = m_parent;
	parent->RemoveChild(this, WIDGET_INVOKE_INFO_NO_CALLBACKS);
	parent->AddChild(this, z, WIDGET_INVOKE_INFO_NO_CALLBACKS);
}

void TBWidget::SetGravity(WIDGET_GRAVITY g)
{
	m_gravity = g;
}

void TBWidget::SetSkinBg(const TBID &skin_bg, WIDGET_INVOKE_INFO info)
{
	if (skin_bg == m_skin_bg)
		return;

	// Set the skin and m_skin_bg_expected. During InvokeProcess, we will detect
	// if any widget get a different element due to conditions and strong override.
	// If that happens, OnSkinChanged will be called and m_skin_bg_expected updated to
	// match that override.
	m_skin_bg = skin_bg;
	m_skin_bg_expected = skin_bg;

	Invalidate();
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);

	if (info == WIDGET_INVOKE_INFO_NORMAL)
		OnSkinChanged();
}

TBSkinElement *TBWidget::GetSkinBgElement()
{
	TBWidgetSkinConditionContext context(this);
	WIDGET_STATE state = GetAutoState();
	return g_tb_skin->GetSkinElementStrongOverride(m_skin_bg, static_cast<SKIN_STATE>(state), context);
}

TBWidget *TBWidget::FindScrollableWidget(bool scroll_x, bool scroll_y)
{
	TBWidget *candidate = this;
	while (candidate)
	{
		ScrollInfo scroll_info = candidate->GetScrollInfo();
		if ((scroll_x && scroll_info.CanScrollX()) ||
			(scroll_y && scroll_info.CanScrollY()))
			return candidate;
		candidate = candidate->GetParent();
	}
	return nullptr;
}

TBScroller *TBWidget::FindStartedScroller()
{
	TBWidget *candidate = this;
	while (candidate)
	{
		if (candidate->m_scroller && candidate->m_scroller->IsStarted())
			return candidate->m_scroller;
		candidate = candidate->GetParent();
	}
	return nullptr;
}

TBScroller *TBWidget::GetReadyScroller(bool scroll_x, bool scroll_y)
{
	if (TBScroller *scroller = FindStartedScroller())
		return scroller;
	// We didn't have any active scroller, so create one for the nearest scrollable parent.
	if (TBWidget *scrollable_widget = FindScrollableWidget(scroll_x, scroll_y))
		return scrollable_widget->GetScroller();
	return nullptr;
}

TBScroller *TBWidget::GetScroller()
{
	if (!m_scroller)
		m_scroller = new TBScroller(this);
	return m_scroller;
}

void TBWidget::ScrollToSmooth(int x, int y)
{
	ScrollInfo info = GetScrollInfo();
	int dx = x - info.x;
	int dy = y - info.y;
	if (TBScroller *scroller = GetReadyScroller(dx != 0, dy != 0))
		scroller->OnScrollBy(dx, dy, false);
}

void TBWidget::ScrollBySmooth(int dx, int dy)
{
	// Clip the values to the scroll limits, so we don't
	// scroll any parents.
	//int x = CLAMP(info.x + dx, info.min_x, info.max_x);
	//int y = CLAMP(info.y + dy, info.min_y, info.max_y);
	//dx = x - info.x;
	//dy = y - info.y;
	if (!dx && !dy)
		return;

	if (TBScroller *scroller = GetReadyScroller(dx != 0, dy != 0))
		scroller->OnScrollBy(dx, dy, true);
}

void TBWidget::ScrollBy(int dx, int dy)
{
	ScrollInfo info = GetScrollInfo();
	ScrollTo(info.x + dx, info.y + dy);
}

void TBWidget::ScrollByRecursive(int &dx, int &dy)
{
	TBWidget *tmp = this;
	while (tmp)
	{
		ScrollInfo old_info = tmp->GetScrollInfo();
		tmp->ScrollTo(old_info.x + dx, old_info.y + dy);
		ScrollInfo new_info = tmp->GetScrollInfo();
		dx -= new_info.x - old_info.x;
		dy -= new_info.y - old_info.y;
		if (!dx && !dy)
			break;
		tmp = tmp->m_parent;
	}
}

void TBWidget::ScrollIntoViewRecursive()
{
	TBRect scroll_to_rect = m_rect;
	TBWidget *tmp = this;
	while (tmp->m_parent)
	{
		tmp->m_parent->ScrollIntoView(scroll_to_rect);
		scroll_to_rect.x += tmp->m_parent->m_rect.x;
		scroll_to_rect.y += tmp->m_parent->m_rect.y;
		tmp = tmp->m_parent;
	}
}

void TBWidget::ScrollIntoView(const TBRect &rect)
{
	const ScrollInfo info = GetScrollInfo();
	int new_x = info.x;
	int new_y = info.y;

	const TBRect visible_rect = GetPaddingRect().Offset(info.x, info.y);

	if (rect.y <= visible_rect.y)
		new_y = rect.y;
	else if (rect.y + rect.h > visible_rect.y + visible_rect.h)
		new_y = rect.y + rect.h - visible_rect.h;

	if (rect.x <= visible_rect.x)
		new_x = rect.x;
	else if (rect.x + rect.w > visible_rect.x + visible_rect.w)
		new_x = rect.x + rect.w - visible_rect.w;

	ScrollTo(new_x, new_y);
}

bool TBWidget::SetFocus(WIDGET_FOCUS_REASON reason, WIDGET_INVOKE_INFO info)
{
	if (focused_widget == this)
		return true;
	if (GetDisabled() || !GetIsFocusable() || !GetVisibility() || GetIsDying())
		return false;

	// Update windows last focus
	TBWindow *window = GetParentWindow();
	if (window)
	{
		window->SetLastFocus(this);
		// If not active, just return. We should get focus when the window is activated.
		// Exception for windows that doesn't activate. They may contain focusable widgets.
		if (!window->IsActive() && (window->GetSettings() & WINDOW_SETTINGS_CAN_ACTIVATE))
			return true;
	}

	TBWidgetSafePointer old_focus(focused_widget);
	focused_widget = this;
	Invalidate();

	if (reason == WIDGET_FOCUS_REASON_NAVIGATION)
		ScrollIntoViewRecursive();

	if (info == WIDGET_INVOKE_INFO_NORMAL)
	{
		// A lot of weird bugs could happen if people mess with focus from OnFocusChanged.
		// Take some precaution and detect if it change again after OnFocusChanged(false).
		if (TBWidget *old = old_focus.Get())
		{
			// The currently focused widget still has the pressed state set by the emulated click
			// (By keyboard), so unset it before we unfocus it so it's not stuck in pressed state.
			if (old->m_packed.has_key_pressed_state)
			{
				old->SetState(WIDGET_STATE_PRESSED, false);
				old->m_packed.has_key_pressed_state = false;
			}
			old->OnFocusChanged(false);
		}
		if (old_focus.Get())
			TBWidgetListener::InvokeWidgetFocusChanged(old_focus.Get(), false);
		if (focused_widget && focused_widget == this)
			focused_widget->OnFocusChanged(true);
		if (focused_widget && focused_widget == this)
			TBWidgetListener::InvokeWidgetFocusChanged(focused_widget, true);
	}
	return true;
}

bool TBWidget::SetFocusRecursive(WIDGET_FOCUS_REASON reason)
{
	// Search for a child widget that accepts focus
	TBWidget *child = GetFirstChild();
	while (child && IsAncestorOf(child))
	{
		if (child->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN))
			return true;
		child = child->GetNextDeep();
	}
	return false;
}

bool TBWidget::MoveFocus(bool forward)
{
	TBWidget *origin = focused_widget;
	if (!origin)
		origin = this;

	TBWidget *root = origin->GetParentWindow();
	if (!root)
		root = origin->GetParentRoot();

	TBWidget *current = origin;
	while (current)
	{
		current = forward ? current->GetNextDeep() : current->GetPrevDeep();
		// Wrap around if we reach the end/beginning
		if (!current || !root->IsAncestorOf(current))
			current = forward ? root->GetFirstChild() : root->GetLastLeaf();
		// Break if we reached the origin again (we're not finding anything else)
		if (current == origin)
			break;
		// Try to focus what we found
		if (current && current->SetFocus(WIDGET_FOCUS_REASON_NAVIGATION))
			return true;
	}
	return false;
}

TBWidget *TBWidget::GetNextDeep() const
{
	if (m_children.GetFirst())
		return GetFirstChild();
	for (const TBWidget *widget = this; widget; widget = widget->m_parent)
		if (widget->next)
			return widget->GetNext();
	return nullptr;
}

TBWidget *TBWidget::GetPrevDeep() const
{
	if (!prev)
		return m_parent;
	TBWidget *widget = GetPrev();
	while (widget->m_children.GetLast())
		widget = widget->GetLastChild();
	return widget;
}

TBWidget *TBWidget::GetLastLeaf() const
{
	if (TBWidget *widget = GetLastChild())
	{
		while (widget->GetLastChild())
			widget = widget->GetLastChild();
		return widget;
	}
	return nullptr;
}

bool TBWidget::GetIsInteractable() const
{
	return !(m_opacity == 0 || GetIgnoreInput() || GetState(WIDGET_STATE_DISABLED) || GetIsDying());
}

WIDGET_HIT_STATUS TBWidget::GetHitStatus(int x, int y)
{
	if (!GetIsInteractable())
		return WIDGET_HIT_STATUS_NO_HIT;
	return x >= 0 && y >= 0 && x < m_rect.w && y < m_rect.h ? WIDGET_HIT_STATUS_HIT : WIDGET_HIT_STATUS_NO_HIT;
}

TBWidget *TBWidget::GetWidgetAt(int x, int y, bool include_children) const
{
	int child_translation_x, child_translation_y;
	GetChildTranslation(child_translation_x, child_translation_y);
	x -= child_translation_x;
	y -= child_translation_y;

	TBWidget *tmp = GetFirstChild();
	TBWidget *last_match = nullptr;
	while (tmp)
	{
		WIDGET_HIT_STATUS hit_status = tmp->GetHitStatus(x - tmp->m_rect.x, y - tmp->m_rect.y);
		if (hit_status)
		{
			if (include_children && hit_status != WIDGET_HIT_STATUS_HIT_NO_CHILDREN)
			{
				last_match = tmp->GetWidgetAt(x - tmp->m_rect.x, y - tmp->m_rect.y, include_children);
				if (!last_match)
					last_match = tmp;
			}
			else
				last_match = tmp;
		}
		tmp = tmp->GetNext();
	}
	return last_match;
}

bool TBWidget::IsAncestorOf(TBWidget *other_widget) const
{
	while (other_widget)
	{
		if (other_widget == this)
			return true;
		other_widget = other_widget->m_parent;
	}
	return false;
}

bool TBWidget::IsEventDestinationFor(TBWidget *other_widget) const
{
	while (other_widget)
	{
		if (other_widget == this)
			return true;
		other_widget = other_widget->GetEventDestination();
	}
	return false;
}

TBWidget *TBWidget::GetParentRoot()
{
	TBWidget *tmp = this;
	while (tmp->m_parent)
		tmp = tmp->m_parent;
	return tmp;
}

TBWindow *TBWidget::GetParentWindow()
{
	TBWidget *tmp = this;
	while (tmp && !tmp->IsOfType<TBWindow>())
		tmp = tmp->m_parent;
	return static_cast<TBWindow *>(tmp);
}

void TBWidget::AddListener(TBWidgetListener *listener)
{
	m_listeners.AddLast(listener);
}

void TBWidget::RemoveListener(TBWidgetListener *listener)
{
	m_listeners.Remove(listener);
}

void TBWidget::OnPaintChildren(const PaintProps &paint_props)
{
	if (!m_children.GetFirst())
		return;

	// Translate renderer with child translation
	int child_translation_x, child_translation_y;
	GetChildTranslation(child_translation_x, child_translation_y);
	g_renderer->Translate(child_translation_x, child_translation_y);

	TBRect clip_rect = g_renderer->GetClipRect();

	// Invoke paint on all children that are in the current visible rect.
	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (clip_rect.Intersects(child->m_rect))
			child->InvokePaint(paint_props);
	}

	// Invoke paint of overlay elements on all children that are in the current visible rect.
	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (clip_rect.Intersects(child->m_rect))
		{
			TBWidgetSkinConditionContext context(child);
			g_tb_skin->PaintSkinOverlay(child->m_rect, child->GetSkinBgElement(), static_cast<SKIN_STATE>(child->GetAutoState()), context);
		}
	}

	// Draw generic focus skin if the focused widget is one of the children, and the skin
	// doesn't have a skin state for focus which would already be painted.
	if (focused_widget && focused_widget->m_parent == this)
	{
		TBWidgetSkinConditionContext context(focused_widget);
		TBSkinElement *skin_element = focused_widget->GetSkinBgElement();
		if (!skin_element || !skin_element->HasState(SKIN_STATE_FOCUSED, context))
		{
			WIDGET_STATE state = focused_widget->GetAutoState();
			if (state & SKIN_STATE_FOCUSED)
				g_tb_skin->PaintSkin(focused_widget->m_rect, TBIDC("generic_focus"), static_cast<SKIN_STATE>(state), context);
		}
	}

	g_renderer->Translate(-child_translation_x, -child_translation_y);
}

void TBWidget::OnResized(int old_w, int old_h)
{
	int dw = m_rect.w - old_w;
	int dh = m_rect.h - old_h;

	TBWidget *child = GetFirstChild();
	while (child)
	{
		TBRect rect = child->m_rect;
		if ((child->m_gravity & WIDGET_GRAVITY_LEFT) && (child->m_gravity & WIDGET_GRAVITY_RIGHT))
			rect.w += dw;
		else if(child->m_gravity & WIDGET_GRAVITY_RIGHT)
			rect.x += dw;
		if ((child->m_gravity & WIDGET_GRAVITY_TOP) && (child->m_gravity & WIDGET_GRAVITY_BOTTOM))
			rect.h += dh;
		else if (child->m_gravity & WIDGET_GRAVITY_BOTTOM)
			rect.y += dh;
		child->SetRect(rect);
		child = child->GetNext();
	}
}

void TBWidget::OnInflateChild(TBWidget *child)
{
	// If the child pull towards only one edge (per axis), stick to that edge
	// and use the preferred size. Otherwise fill up all available space.
	TBRect padding_rect = GetPaddingRect();
	TBRect child_rect = padding_rect;
	WIDGET_GRAVITY gravity = child->GetGravity();
	bool fill_x = (gravity & WIDGET_GRAVITY_LEFT) && (gravity & WIDGET_GRAVITY_RIGHT);
	bool fill_y = (gravity & WIDGET_GRAVITY_TOP) && (gravity & WIDGET_GRAVITY_BOTTOM);
	if (!fill_x || !fill_y)
	{
		PreferredSize ps = child->GetPreferredSize();
		if (!fill_x)
		{
			child_rect.w = ps.pref_w;
			if (gravity & WIDGET_GRAVITY_RIGHT)
				child_rect.x = padding_rect.x + padding_rect.w - child_rect.w;
		}
		if (!fill_y)
		{
			child_rect.h = ps.pref_h;
			if (gravity & WIDGET_GRAVITY_BOTTOM)
				child_rect.y = padding_rect.y + padding_rect.h - child_rect.h;
		}
	}
	child->SetRect(child_rect);
}

TBRect TBWidget::GetPaddingRect()
{
	TBRect padding_rect(0, 0, m_rect.w, m_rect.h);
	if (TBSkinElement *e = GetSkinBgElement())
	{
		padding_rect.x += e->padding_left;
		padding_rect.y += e->padding_top;
		padding_rect.w -= e->padding_left + e->padding_right;
		padding_rect.h -= e->padding_top + e->padding_bottom;
	}
	return padding_rect;
}

PreferredSize TBWidget::OnCalculatePreferredContentSize()
{
	// The default preferred size is calculated to satisfy the children
	// in the best way. Since this is the default, it's probably not a
	// layouting widget and children are resized purely by gravity.

	// Allow this widget a larger maximum if our gravity wants both ways,
	// otherwise don't grow more than the largest child.
	bool apply_max_w = !((m_gravity & WIDGET_GRAVITY_LEFT) && (m_gravity & WIDGET_GRAVITY_RIGHT));
	bool apply_max_h = !((m_gravity & WIDGET_GRAVITY_TOP) && (m_gravity & WIDGET_GRAVITY_BOTTOM));

	PreferredSize ps;
	if (GetFirstChild())
	{
		if (apply_max_w)
			ps.max_w = 0;
		if (apply_max_h)
			ps.max_h = 0;
	}

	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
	{
		PreferredSize child_ps = child->GetPreferredSize();
		ps.pref_w = MAX(ps.pref_w, child_ps.pref_w);
		ps.pref_h = MAX(ps.pref_h, child_ps.pref_h);
		ps.min_w = MAX(ps.min_w, child_ps.min_w);
		ps.min_h = MAX(ps.min_h, child_ps.min_h);
		if (apply_max_w)
			ps.max_w = MAX(ps.max_w, child_ps.max_w);
		if (apply_max_h)
			ps.max_h = MAX(ps.max_h, child_ps.max_h);
		ps.constraints_dependant |= child_ps.constraints_dependant;
	}

	return ps;
}

PreferredSize TBWidget::OnCalculatePreferredSize()
{
	PreferredSize ps = OnCalculatePreferredContentSize();
	assert(ps.pref_w >= ps.min_w);
	assert(ps.pref_h >= ps.min_h);

	// Grow the contet size by the skin padding to get the preferred size.
	if (TBSkinElement *e = GetSkinBgElement())
	{
		// If the preferred size is not set by the widget, calculate
		// it based on the size of the skin bitmap (minus expansion)
		if (ps.pref_w == 0 && e->bitmap)
			ps.pref_w = e->bitmap->Width() - e->expand * 2;
		else
		{
			ps.min_w += e->padding_left + e->padding_right;
			ps.pref_w += e->padding_left + e->padding_right;
		}

		if (ps.pref_h == 0 && e->bitmap)
			ps.pref_h = e->bitmap->Height() - e->expand * 2;
		else
		{
			ps.min_h += e->padding_top + e->padding_bottom;
			ps.pref_h += e->padding_top + e->padding_bottom;
		}

		// Sizes below the skin cut size would start to shrink the skin below pretty,
		// so assume that's the default minimum size if it's not specified (minus expansion)
		int skin_min_w = e->cut * 2 - e->expand * 2;
		int skin_min_h = e->cut * 2 - e->expand * 2;

		if (e->min_width != SKIN_VALUE_NOT_SPECIFIED)
			ps.min_w = e->min_width;
		else
			ps.min_w = MAX(ps.min_w, skin_min_w);

		if (e->min_height != SKIN_VALUE_NOT_SPECIFIED)
			ps.min_h = e->min_height;
		else
			ps.min_h = MAX(ps.min_h, skin_min_h);

		// Use max size from skin if specified, or add padding
		if (e->max_width != SKIN_VALUE_NOT_SPECIFIED)
			ps.max_w = e->max_width;
		else
			ps.max_w += e->padding_left + e->padding_right;

		if (e->max_height != SKIN_VALUE_NOT_SPECIFIED)
			ps.max_h = e->max_height;
		else
			ps.max_h += e->padding_top + e->padding_bottom;

		// Sanitize result
		ps.pref_w = MAX(ps.pref_w, ps.min_w);
		ps.pref_h = MAX(ps.pref_h, ps.min_h);
	}
	return ps;
}

PreferredSize TBWidget::GetPreferredSize()
{
	if (m_packed.is_cached_ps_valid)
		return m_cached_ps;
	TB_IF_DEBUG_SETTING(LAYOUT_PS_DEBUGGING, last_measure_time = TBSystem::GetTimeMS());
	m_packed.is_cached_ps_valid = 1;
	m_cached_ps = OnCalculatePreferredSize();

	// Override the calculated ps with any specified layout parameter.
	if (m_layout_params)
	{
		#define LP_OVERRIDE(param)	if (m_layout_params->param != LayoutParams::UNSPECIFIED) \
										m_cached_ps.param = m_layout_params->param;
		LP_OVERRIDE(min_w);
		LP_OVERRIDE(min_h);
		LP_OVERRIDE(max_w);
		LP_OVERRIDE(max_h);
		LP_OVERRIDE(pref_w);
		LP_OVERRIDE(pref_h);

		// Sanitize results
		m_cached_ps.max_w = MAX(m_cached_ps.max_w, m_cached_ps.min_w);
		m_cached_ps.max_h = MAX(m_cached_ps.max_h, m_cached_ps.min_h);
		m_cached_ps.pref_w = MAX(m_cached_ps.pref_w, m_cached_ps.min_w);
		m_cached_ps.pref_h = MAX(m_cached_ps.pref_h, m_cached_ps.min_h);
	}
	return m_cached_ps;
}

void TBWidget::SetLayoutParams(const LayoutParams &lp)
{
	if (!m_layout_params)
		m_layout_params = new LayoutParams;
	if (!m_layout_params)
		return;
	*m_layout_params = lp;
	m_packed.is_cached_ps_valid = 0;
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
}

void TBWidget::InvalidateLayout(INVALIDATE_LAYOUT il)
{
	m_packed.is_cached_ps_valid = 0;
	if (il == INVALIDATE_LAYOUT_RECURSIVE && m_parent)
		m_parent->InvalidateLayout(il);
}

void TBWidget::InvokeProcess()
{
	InvokeSkinUpdatesInternal(false);
	InvokeProcessInternal();
}

void TBWidget::InvokeSkinUpdatesInternal(bool force_update)
{
	if (!update_skin_states && !force_update)
		return;
	update_skin_states = false;

	// Check if the skin we get is different from what we expect. That might happen
	// if the skin has some strong override dependant a condition that has changed.
	// If that happens, call OnSkinChanged so the widget can react to that (possibly
	// invalidating its layout).
	if (TBSkinElement *skin_elm = GetSkinBgElement())
	{
		if (skin_elm->id != m_skin_bg_expected)
		{
			OnSkinChanged();
			m_skin_bg_expected = skin_elm->id;
			// FIX: We should probably invalidate layout here automatically!
			// InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
		}
	}

	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
		child->InvokeSkinUpdatesInternal(true);
}

void TBWidget::InvokeProcessInternal()
{
	OnProcess();

	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
		child->InvokeProcessInternal();

	OnProcessAfterChildren();
}

void TBWidget::InvokeProcessStates(bool force_update)
{
	if (!update_widget_states && !force_update)
		return;
	update_widget_states = false;

	OnProcessStates();

	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
		child->InvokeProcessStates(true);
}

void TBWidget::InvokePaint(const PaintProps &parent_paint_props)
{
	// Don't paint invisible widgets
	if (m_opacity == 0 || m_rect.IsEmpty())
		return;

	WIDGET_STATE state = GetAutoState();
	TBSkinElement *skin_element = GetSkinBgElement();

	// Multiply current opacity with widget opacity, skin opacity and state opacity.
	float old_opacity = g_renderer->GetOpacity();
	float opacity = old_opacity * m_opacity;
	if (skin_element)
		opacity *= skin_element->opacity;
	if (state & WIDGET_STATE_DISABLED)
		opacity *= g_tb_skin->GetDefaultDisabledOpacity();
	if (opacity == 0)
		return;

	// FIX: This does not give the correct result! Must use a new render target!
	g_renderer->SetOpacity(opacity);

	int trns_x = m_rect.x, trns_y = m_rect.y;
	g_renderer->Translate(trns_x, trns_y);

	// Paint background skin
	TBRect local_rect(0, 0, m_rect.w, m_rect.h);
	TBWidgetSkinConditionContext context(this);
	TBSkinElement *used_element = g_tb_skin->PaintSkin(local_rect, skin_element, static_cast<SKIN_STATE>(state), context);
	assert(!!used_element == !!skin_element);

	TB_IF_DEBUG_SETTING(LAYOUT_BOUNDS, g_renderer->DrawRect(local_rect, TBColor(255, 255, 255, 50)));

	// Inherit properties from parent if not specified in the used skin for this widget.
	PaintProps paint_props = parent_paint_props;
	if (used_element && used_element->text_color != 0)
		paint_props.text_color = used_element->text_color;

	// Paint content
	OnPaint(paint_props);

	if (used_element)
		g_renderer->Translate(used_element->content_ofs_x, used_element->content_ofs_y);

	// Paint children
	OnPaintChildren(paint_props);

#ifdef TB_RUNTIME_DEBUG_INFO
	if (TB_DEBUG_SETTING(LAYOUT_PS_DEBUGGING))
	{
		// Layout debug painting. Paint recently layouted widgets with red and
		// recently measured widgets with yellow.
		// Invalidate to keep repainting until we've timed out (so it's removed).
		const double debug_time = 300;
		const double now = TBSystem::GetTimeMS();
		if (now < last_layout_time + debug_time)
		{
			g_renderer->DrawRect(local_rect, TBColor(255, 30, 30, 200));
			Invalidate();
		}
		if (now < last_measure_time + debug_time)
		{
			g_renderer->DrawRect(local_rect.Shrink(1, 1), TBColor(255, 255, 30, 200));
			Invalidate();
		}
	}
#endif // TB_RUNTIME_DEBUG_INFO

	if (used_element)
		g_renderer->Translate(-used_element->content_ofs_x, -used_element->content_ofs_y);

	g_renderer->Translate(-trns_x, -trns_y);
	g_renderer->SetOpacity(old_opacity);
}

bool TBWidget::InvokeEvent(TBWidgetEvent &ev)
{
	ev.target = this;

	// First call the global listener about this event.
	// Who knows, maybe some listener will block the event or cause us
	// to be deleted.
	TBWidgetSafePointer this_widget(this);
	if (TBWidgetListener::InvokeWidgetInvokeEvent(this, ev))
		return true;

	if (!this_widget.Get())
		return true; // We got removed so we actually handled this event.

	if (ev.type == EVENT_TYPE_CHANGED)
		m_connection.SyncFromWidget(this);

	if (!this_widget.Get())
		return true; // We got removed so we actually handled this event.

	// Always update states after some event types.
	switch (ev.type)
	{
	case EVENT_TYPE_CLICK:
	case EVENT_TYPE_CHANGED:
	case EVENT_TYPE_KEY_DOWN:
	case EVENT_TYPE_KEY_UP:
		InvalidateStates();
	default:
		break;
	};

	// Call OnEvent on this widgets and travel up through its parents if not handled.
	bool handled = false;
	TBWidget *tmp = this;
	while (tmp && !(handled = tmp->OnEvent(ev)))
		tmp = tmp->GetEventDestination();
	return handled;
}

void TBWidget::InvokePointerDown(int x, int y, int click_count, MODIFIER_KEYS modifierkeys, bool touch)
{
	if (!captured_widget)
	{
		SetCapturedWidget(GetWidgetAt(x, y, true));
		SetHoveredWidget(captured_widget, touch);
		//captured_button = button;

		// Hide focus when we use the pointer, if it's not on the focused widget.
		if (focused_widget != captured_widget)
			SetAutoFocusState(false);

		// Get the closest parent window and bring it to the top
		TBWindow *window = captured_widget ? captured_widget->GetParentWindow() : nullptr;
		if (window)
			window->Activate();
	}
	if (captured_widget)
	{
		// Check if there's any started scroller that should be stopped.
		TBWidget *tmp = captured_widget;
		while (tmp)
		{
			if (tmp->m_scroller && tmp->m_scroller->IsStarted())
			{
				// When we touch down to stop a scroller, we don't
				// want the touch to end up causing a click.
				cancel_click = true;
				tmp->m_scroller->Stop();
				break;
			}
			tmp = tmp->GetParent();
		}

		// Focus the captured widget or the closest
		// focusable parent if it isn't focusable.
		TBWidget *focus_target = captured_widget;
		while (focus_target)
		{
			if (focus_target->SetFocus(WIDGET_FOCUS_REASON_POINTER))
				break;
			focus_target = focus_target->m_parent;
		}
	}
	if (captured_widget)
	{
		captured_widget->ConvertFromRoot(x, y);
		pointer_move_widget_x = pointer_down_widget_x = x;
		pointer_move_widget_y = pointer_down_widget_y = y;
		TBWidgetEvent ev(EVENT_TYPE_POINTER_DOWN, x, y, touch, modifierkeys);
		ev.count = click_count;
		captured_widget->InvokeEvent(ev);
	}
}

void TBWidget::InvokePointerUp(int x, int y, MODIFIER_KEYS modifierkeys, bool touch)
{
	if (captured_widget)
	{
		captured_widget->ConvertFromRoot(x, y);
		TBWidgetEvent ev_up(EVENT_TYPE_POINTER_UP, x, y, touch, modifierkeys);
		TBWidgetEvent ev_click(EVENT_TYPE_CLICK, x, y, touch, modifierkeys);
		captured_widget->InvokeEvent(ev_up);
		if (!cancel_click && captured_widget && captured_widget->GetHitStatus(x, y))
			captured_widget->InvokeEvent(ev_click);
		if (captured_widget) // && button == captured_button
			captured_widget->ReleaseCapture();
	}
}

void TBWidget::InvokePointerMove(int x, int y, MODIFIER_KEYS modifierkeys, bool touch)
{
	SetHoveredWidget(GetWidgetAt(x, y, true), touch);
	TBWidget *target = captured_widget ? captured_widget : hovered_widget;
	if (target)
	{
		target->ConvertFromRoot(x, y);
		pointer_move_widget_x = x;
		pointer_move_widget_y = y;

		TBWidgetEvent ev(EVENT_TYPE_POINTER_MOVE, x, y, touch, modifierkeys);
		if (target->InvokeEvent(ev))
			return;

		// The move event was not handled, so handle panning of scrollable widgets.
		HandlePanningOnMove(x, y);
	}
}

void TBWidget::HandlePanningOnMove(int x, int y)
{
	if (!captured_widget)
		return;

	// Check pointer movement
	const int dx = pointer_down_widget_x - x;
	const int dy = pointer_down_widget_y - y;
	const int threshold = TBSystem::GetPanThreshold();
	const bool maybe_start_panning_x = ABS(dx) >= threshold;
	const bool maybe_start_panning_y = ABS(dy) >= threshold;

	// Do panning, or attempt starting panning (we don't know if any widget is scrollable yet)
	if (captured_widget->m_packed.is_panning || maybe_start_panning_x || maybe_start_panning_y)
	{
		int start_compensation_x = 0, start_compensation_y = 0;
		if (!captured_widget->m_packed.is_panning)
		{
			// When we start panning, deduct the extra distance caused by the
			// start threshold from the delta so we don't start with a sudden jump.
			int extra = threshold - 1;
			if (maybe_start_panning_x)
				start_compensation_x = dx < 0 ? extra : -extra;
			if (maybe_start_panning_y)
				start_compensation_y = dy < 0 ? extra : -extra;
		}

		// Get any active scroller and feed it with pan actions.
		TBScroller *scroller = captured_widget->GetReadyScroller(dx != 0, dy != 0);
		if (!scroller)
			return;

		int old_translation_x = 0, old_translation_y = 0;
		captured_widget->GetScrollRoot()->GetChildTranslation(old_translation_x, old_translation_y);

		if (scroller->OnPan(dx + start_compensation_x, dy + start_compensation_y))
		{
			// Scroll delta changed, so we are now panning!
			captured_widget->m_packed.is_panning = true;
			cancel_click = true;

			// If the captured widget (or its scroll root) has panned, we have to compensate the
			// pointer down coordinates so we won't accumulate the difference the following pan.
			int new_translation_x = 0, new_translation_y = 0;
			captured_widget->GetScrollRoot()->GetChildTranslation(new_translation_x, new_translation_y);
			pointer_down_widget_x += new_translation_x - old_translation_x + start_compensation_x;
			pointer_down_widget_y += new_translation_y - old_translation_y + start_compensation_y;
		}
	}
}

void TBWidget::InvokeWheel(int x, int y, int delta_x, int delta_y, MODIFIER_KEYS modifierkeys)
{
	SetHoveredWidget(GetWidgetAt(x, y, true), true);
	TBWidget *target = captured_widget ? captured_widget : hovered_widget;
	if (target)
	{
		target->ConvertFromRoot(x, y);
		pointer_move_widget_x = x;
		pointer_move_widget_y = y;
		TBWidgetEvent ev(EVENT_TYPE_WHEEL, x, y, true, modifierkeys);
		ev.delta_x = delta_x;
		ev.delta_y = delta_y;
		target->InvokeEvent(ev);
	}
}

bool TBWidget::InvokeKey(int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down)
{
	bool handled = false;
	if (focused_widget)
	{
		// Emulate a click on the focused widget when pressing space or enter
		if (!modifierkeys && focused_widget->GetClickByKey() &&
			!focused_widget->GetDisabled() &&
			!focused_widget->GetIsDying() &&
			(special_key == TB_KEY_ENTER || key == ' '))
		{
			// Set the pressed state while the key is down, if it
			// didn't already have the pressed state.
			static bool check_pressed_state = true;
			static bool had_pressed_state = false;
			if (down && check_pressed_state)
			{
				had_pressed_state = focused_widget->GetState(WIDGET_STATE_PRESSED);
				check_pressed_state = false;
			}
			if (!down)
				check_pressed_state = true;

			if (!had_pressed_state)
			{
				focused_widget->SetState(WIDGET_STATE_PRESSED, down);
				focused_widget->m_packed.has_key_pressed_state = down;
			}

			// Invoke the click event
			if (!down)
			{
				TBWidgetEvent ev(EVENT_TYPE_CLICK, m_rect.w / 2, m_rect.h / 2, true);
				focused_widget->InvokeEvent(ev);
			}
			handled = true;
		}
		else
		{
			// Invoke the key event on the focused widget
			TBWidgetEvent ev(down ? EVENT_TYPE_KEY_DOWN : EVENT_TYPE_KEY_UP);
			ev.key = key;
			ev.special_key = special_key;
			ev.modifierkeys = modifierkeys;
			handled = focused_widget->InvokeEvent(ev);
		}
	}

	// Move focus between widgets
	if (down && !handled && special_key == TB_KEY_TAB)
	{
		handled = MoveFocus(!(modifierkeys & TB_SHIFT));

		// Show the focus when we move it by keyboard
		if (handled)
			SetAutoFocusState(true);
	}
	return handled;
}

void TBWidget::ReleaseCapture()
{
	if (this == captured_widget)
		SetCapturedWidget(nullptr);
}

void TBWidget::ConvertToRoot(int &x, int &y) const
{
	const TBWidget *tmp = this;
	while (tmp->m_parent)
	{
		x += tmp->m_rect.x;
		y += tmp->m_rect.y;
		tmp = tmp->m_parent;

		if (tmp)
		{
			int child_translation_x, child_translation_y;
			tmp->GetChildTranslation(child_translation_x, child_translation_y);
			x += child_translation_x;
			y += child_translation_y;
		}
	}
}

void TBWidget::ConvertFromRoot(int &x, int &y) const
{
	const TBWidget *tmp = this;
	while (tmp->m_parent)
	{
		x -= tmp->m_rect.x;
		y -= tmp->m_rect.y;
		tmp = tmp->m_parent;

		if (tmp)
		{
			int child_translation_x, child_translation_y;
			tmp->GetChildTranslation(child_translation_x, child_translation_y);
			x -= child_translation_x;
			y -= child_translation_y;
		}
	}
}

// static
void TBWidget::SetHoveredWidget(TBWidget *widget, bool touch)
{
	if (TBWidget::hovered_widget == widget)
		return;
	if (widget && widget->GetState(WIDGET_STATE_DISABLED))
		return;

	// We may apply hover state automatically so the widget might need to be updated.
	if (TBWidget::hovered_widget)
		TBWidget::hovered_widget->Invalidate();

	TBWidget::hovered_widget = widget;

	if (TBWidget::hovered_widget)
	{
		TBWidget::hovered_widget->Invalidate();

		// Cursor based movement should set hover state automatically, but touch
		// events should not (since touch doesn't really move unless pressed).
		TBWidget::hovered_widget->m_packed.no_automatic_hover_state = touch;
	}
}

// static
void TBWidget::SetCapturedWidget(TBWidget *widget)
{
	if (TBWidget::captured_widget == widget)
		return;
	if (widget && widget->GetState(WIDGET_STATE_DISABLED))
		return;

	// Stop panning when capture change (most likely changing to nullptr because of InvokePointerUp)
	// Notify any active scroller so it may begin scrolling.
	if (TBWidget::captured_widget)
	{
		if (TBScroller *scroller = TBWidget::captured_widget->FindStartedScroller())
		{
			if (TBWidget::captured_widget->m_packed.is_panning)
				scroller->OnPanReleased();
			else
				scroller->Stop();
		}
		TBWidget::captured_widget->m_packed.is_panning = false;
	}
	cancel_click = false;

	TBWidget *old_capture = TBWidget::captured_widget;

	// We apply pressed state automatically so the widget might need to be updated.
	if (TBWidget::captured_widget)
		TBWidget::captured_widget->Invalidate();

	TBWidget::captured_widget = widget;

	if (old_capture)
		old_capture->OnCaptureChanged(false);

	if (TBWidget::captured_widget)
	{
		TBWidget::captured_widget->Invalidate();
		TBWidget::captured_widget->OnCaptureChanged(true);
	}
}

bool TBWidget::SetFontDescription(const TBFontDescription &font_desc)
{
	if (m_font_desc == font_desc)
		return true;

	// Set the font description only if we have a matching font, or succeed creating one.
	if (g_font_manager->HasFontFace(font_desc))
		m_font_desc = font_desc;
	else if (TBFontFace * font = g_font_manager->CreateFontFace(font_desc))
		m_font_desc = font_desc;
	else
		return false;

	InvokeFontChanged();
	return true;
}

void TBWidget::InvokeFontChanged()
{
	OnFontChanged();

	// Recurse to children that inherit the font
	for (TBWidget *child = GetFirstChild(); child; child = child->GetNext())
		if (child->m_font_desc.GetFontFaceID() == 0)
			child->InvokeFontChanged();
}

TBFontDescription TBWidget::GetCalculatedFontDescription() const
{
	const TBWidget *tmp = this;
	while (tmp)
	{
		if (tmp->m_font_desc.GetFontFaceID() != 0)
			return tmp->m_font_desc;
		tmp = tmp->m_parent;
	}
	return g_font_manager->GetDefaultFontDescription();
}

TBFontFace *TBWidget::GetFont() const
{
	return g_font_manager->GetFontFace(GetCalculatedFontDescription());
}

}; // namespace tinkerbell
