// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets.h"
#include "tb_window.h"
#include "tb_widgets_listener.h"
#include "tb_renderer.h"
#include "tb_widgets_common.h"
#include "tb_system.h"
#include "tb_font_renderer.h"
#include <assert.h>

namespace tinkerbell {

//static data
Widget *Widget::hovered_widget = nullptr;
Widget *Widget::captured_widget = nullptr;
Widget *Widget::focused_widget = nullptr;
int Widget::pointer_down_widget_x = 0;
int Widget::pointer_down_widget_y = 0;
int Widget::pointer_move_widget_x = 0;
int Widget::pointer_move_widget_y = 0;
bool Widget::is_panning = false;
bool Widget::update_widget_states = true;
bool Widget::show_focus_state = false;

Widget::PaintProps::PaintProps()
{
	// Set the default properties, used for the root widgets
	// calling InvokePaint. The base values for all inheritance.
	text_color = g_tb_skin->GetDefaultTextColor();
}

Widget::Widget()
	: m_parent(nullptr)
	, m_state(WIDGET_STATE_NONE)
	, m_opacity(1.f)
	, m_data(0)
	, m_gravity(WIDGET_GRAVITY_DEFAULT)
	, m_packed_init(0)
{
}

Widget::~Widget()
{
	assert(!m_parent); ///< A widget must be removed from parent before deleted

	if (this == hovered_widget)
		hovered_widget = nullptr;
	if (this == captured_widget)
		captured_widget = nullptr;
	if (this == focused_widget)
		focused_widget = nullptr;

	TBGlobalWidgetListener::InvokeWidgetDelete(this);

	while (Widget *child = GetFirstChild())
	{
		RemoveChild(child);
		delete child;
	}
}

void Widget::SetRect(const TBRect &rect)
{
	if (m_rect.Equals(rect))
		return;

	TBRect old_rect = m_rect;
	m_rect = rect;

	if (old_rect.w != m_rect.w || old_rect.h != m_rect.h)
		OnResized(old_rect.w, old_rect.h);

	Invalidate();
}

void Widget::Invalidate()
{
	if (!GetVisibility() && !m_rect.IsEmpty())
		return;
	Widget *tmp = this;
	while (tmp)
	{
		tmp->OnInvalid();
		tmp = tmp->m_parent;
	}
}

void Widget::InvalidateStates()
{
	update_widget_states = true;
}

void Widget::Die()
{
	if (m_packed.is_dying)
		return;
	m_packed.is_dying = true;
	OnDie();
	if (!TBGlobalWidgetListener::InvokeWidgetDying(this))
	{
		// No one was interested, so die immediately.
		if (m_parent)
			m_parent->RemoveChild(this);
		delete this;
	}
}

Widget *Widget::GetWidgetByID(const TBID &id, const char *classname)
{
	if (m_id == id && (!classname || IsOfType(classname)))
		return this;
	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (Widget *sub_child = child->GetWidgetByID(id, classname))
			return sub_child;
	}
	return nullptr;
}

void Widget::SetState(WIDGET_STATE state, bool on)
{
	if (GetState(state) == on)
		return;
	if (on)
		m_state |= state;
	else
		m_state &= ~state;
	Invalidate();
}

uint32 Widget::GetAutoState() const
{
	uint32 state = m_state;
	if (!is_panning && this == captured_widget && this == hovered_widget)
		state |= WIDGET_STATE_PRESSED;
	if (this == hovered_widget)
		state |= WIDGET_STATE_HOVERED;
	if (this == focused_widget && show_focus_state)
		state |= WIDGET_STATE_FOCUSED;
#ifdef TB_ALWAYS_SHOW_EDIT_FOCUS
	else if (this == focused_widget && IsOfType("TBEditField"))
		state |= WIDGET_STATE_FOCUSED;
#endif
	return state;
}

//static
void Widget::SetAutoFocusState(bool on)
{
	if (show_focus_state == on)
		return;
	show_focus_state = on;
	if (focused_widget)
		focused_widget->Invalidate();
}

void Widget::SetOpacity(float opacity)
{
	if (m_opacity == opacity)
		return;
	m_opacity = opacity;
	Invalidate();
}

bool Widget::GetVisibility() const
{
	const Widget *tmp = this;
	while (tmp)
	{
		if (tmp->GetOpacity() == 0)
			return false;
		tmp = tmp->m_parent;
	}
	return true;
}

bool Widget::GetDisabled() const
{
	const Widget *tmp = this;
	while (tmp)
	{
		if (tmp->GetState(WIDGET_STATE_DISABLED))
			return true;
		tmp = tmp->m_parent;
	}
	return false;
}

void Widget::AddChild(Widget *child, WIDGET_Z z, WIDGET_INVOKE_INFO info)
{
	assert(!child->m_parent);
	child->m_parent = this;
	if (z == WIDGET_Z_TOP)
		m_children.AddLast(child);
	else
		m_children.AddFirst(child);

	if (info == WIDGET_INVOKE_INFO_NORMAL)
	{
		OnChildAdded(child);
		child->OnAdded();
		TBGlobalWidgetListener::InvokeWidgetAdded(child);
	}
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	Invalidate();
}

void Widget::RemoveChild(Widget *child, WIDGET_INVOKE_INFO info)
{
	assert(child->m_parent);

	if (info == WIDGET_INVOKE_INFO_NORMAL)
	{
		OnChildRemove(child);
		child->OnRemove();
		TBGlobalWidgetListener::InvokeWidgetRemove(child);
	}

	m_children.Remove(child);
	child->m_parent = nullptr;

	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	Invalidate();
}

void Widget::SetZ(WIDGET_Z z)
{
	if (!m_parent)
		return;
	if (z == WIDGET_Z_TOP && this == m_parent->m_children.GetLast())
		return; // Already at the top
	if (z == WIDGET_Z_BOTTOM && this == m_parent->m_children.GetFirst())
		return; // Already at the top
	Widget *parent = m_parent;
	parent->RemoveChild(this, WIDGET_INVOKE_INFO_NO_CALLBACKS);
	parent->AddChild(this, z, WIDGET_INVOKE_INFO_NO_CALLBACKS);
}

void Widget::SetGravity(WIDGET_GRAVITY g)
{
	m_gravity = g;
}

void Widget::SetSkinBg(const TBID &skin_bg)
{
	if (skin_bg == m_skin_bg)
		return;
	m_skin_bg = skin_bg;
	Invalidate();
	InvalidateLayout(INVALIDATE_LAYOUT_RECURSIVE);
	OnSkinChanged();
}

TBSkinElement *Widget::GetSkinBgElement() const
{
	return g_tb_skin->GetSkinElement(m_skin_bg);
}

void Widget::ScrollByRecursive(int &dx, int &dy)
{
	Widget *tmp = this;
	while (tmp)
	{
		tmp->ScrollBy(dx, dy);
		if (!dx && !dy)
			break;
		tmp = tmp->m_parent;
	}
}

void Widget::ScrollIntoViewRecursive()
{
	TBRect scroll_to_rect = m_rect;
	Widget *tmp = this;
	while (tmp->m_parent)
	{
		tmp->m_parent->ScrollIntoView(scroll_to_rect);
		scroll_to_rect.x += tmp->m_parent->m_rect.x;
		scroll_to_rect.y += tmp->m_parent->m_rect.y;
		tmp = tmp->m_parent;
	}
}

bool Widget::SetFocus(WIDGET_FOCUS_REASON reason, WIDGET_INVOKE_INFO info)
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
		if (!window->IsActive())
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
		if (old_focus.Get())
			old_focus.Get()->OnFocusChanged(false);
		if (old_focus.Get())
			TBGlobalWidgetListener::InvokeWidgetFocusChanged(old_focus.Get(), false);
		if (focused_widget && focused_widget == this)
			focused_widget->OnFocusChanged(true);
		if (focused_widget && focused_widget == this)
			TBGlobalWidgetListener::InvokeWidgetFocusChanged(focused_widget, false);
	}
	return true;
}

void Widget::MoveFocus(bool forward)
{
	Widget *origin = focused_widget;
	if (!origin)
		origin = this;

	Widget *root = origin->GetParentWindow();
	if (!root)
		root = origin->GetParentRoot();

	Widget *current = origin;
	while (current)
	{
		current = forward ? current->GetNextDeep() : current->GetPrevDeep();
		// Wrap around if we reach the end/beginning
		if (!current || !root->IsParentOf(current))
			current = forward ? root->GetFirstChild() : root->GetLastChild();
		// Break if we reached the origin again (we're not finding anything else)
		if (current == origin)
			break;
		// Try to focus what we found
		if (current && current->SetFocus(WIDGET_FOCUS_REASON_NAVIGATION))
			return;
	}
}

Widget *Widget::GetNextDeep() const
{
	if (m_children.GetFirst())
		return GetFirstChild();
	for (const Widget *widget = this; widget; widget = widget->m_parent)
		if (widget->next)
			return widget->GetNext();
	return nullptr;
}

Widget *Widget::GetPrevDeep() const
{
	if (!prev)
		return m_parent;
	Widget *widget = GetPrev();
	while (widget->m_children.GetLast())
		widget = widget->GetLastChild();
	return widget;
}

WIDGET_HIT_STATUS Widget::GetHitStatus(int x, int y)
{
	if (m_opacity == 0 || GetIgnoreInput() || GetState(WIDGET_STATE_DISABLED) || GetIsDying())
		return WIDGET_HIT_STATUS_NO_HIT;
	return x >= 0 && y >= 0 && x < m_rect.w && y < m_rect.h ? WIDGET_HIT_STATUS_HIT : WIDGET_HIT_STATUS_NO_HIT;
}

Widget *Widget::GetWidgetAt(int x, int y, bool include_children) const
{
	int child_translation_x, child_translation_y;
	GetChildTranslation(child_translation_x, child_translation_y);
	x -= child_translation_x;
	y -= child_translation_y;

	Widget *tmp = GetFirstChild();
	Widget *last_match = nullptr;
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

bool Widget::IsParentOf(Widget *other_widget) const
{
	while (other_widget)
	{
		if (other_widget == this)
			return true;
		other_widget = other_widget->m_parent;
	}
	return false;
}

bool Widget::IsEventDestinationFor(Widget *other_widget) const
{
	while (other_widget)
	{
		if (other_widget == this)
			return true;
		other_widget = other_widget->GetEventDestination();
	}
	return false;
}

Widget *Widget::GetParentRoot()
{
	Widget *tmp = this;
	while (tmp->m_parent)
		tmp = tmp->m_parent;
	return tmp;
}

TBWindow *Widget::GetParentWindow()
{
	Widget *tmp = this;
	while (tmp && !tmp->IsOfType("TBWindow"))
		tmp = tmp->m_parent;
	return static_cast<TBWindow *>(tmp);
}

void Widget::OnPaintChildren(const PaintProps &paint_props)
{
	if (!m_children.GetFirst())
		return;

	// Translate renderer with child translation
	int child_translation_x, child_translation_y;
	GetChildTranslation(child_translation_x, child_translation_y);
	g_renderer->Translate(child_translation_x, child_translation_y);

	TBRect clip_rect = g_renderer->GetClipRect();

	// Invoke paint on all children that are in the current visible rect.
	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (clip_rect.Intersects(child->m_rect))
			child->InvokePaint(paint_props);
	}

	// Invoke paint of overlay elements on all children that are in the current visible rect.
	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
	{
		if (clip_rect.Intersects(child->m_rect))
			g_tb_skin->PaintSkinOverlay(child->m_rect, child->GetSkinBgElement(), child->GetAutoState());
	}

	// Draw generic focus skin if the focused widget is one of the children, and the skin
	// doesn't have a skin state for focus which would already be painted.
	if (focused_widget && focused_widget->m_parent == this)
	{
		TBSkinElement *skin_element = focused_widget->GetSkinBgElement();
		if (!skin_element || !skin_element->HasState(SKIN_STATE_FOCUSED))
		{
			uint32 state = focused_widget->GetAutoState();
			if (state & SKIN_STATE_FOCUSED)
				g_tb_skin->PaintSkin(focused_widget->m_rect, TBIDC("generic_focus"), state);
		}
	}

	g_renderer->Translate(-child_translation_x, -child_translation_y);
}

void Widget::OnResized(int old_w, int old_h)
{
	int dw = m_rect.w - old_w;
	int dh = m_rect.h - old_h;

	Widget *child = GetFirstChild();
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

TBRect Widget::GetPaddingRect()
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

PreferredSize Widget::GetPreferredContentSize()
{
	// The widget may have multiple children, so combine the
	// PreferredSize from all of them.
////FIX: detta gör att det egentligen inte behövs göras i de andra ställena det görs!
	PreferredSize ps;
	if (GetFirstChild())
	{
		ps.max_w = ps.max_h = 0;
	}

	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
	{
		PreferredSize child_ps = child->GetPreferredSize();
		ps.pref_w = MAX(ps.pref_w, child_ps.pref_w);
		ps.pref_h = MAX(ps.pref_h, child_ps.pref_h);
		ps.min_w = MAX(ps.min_w, child_ps.min_w);
		ps.min_h = MAX(ps.min_h, child_ps.min_h);
		ps.max_w = MAX(ps.max_w, child_ps.max_w);
		ps.max_h = MAX(ps.max_h, child_ps.max_h);
	}

	return ps;
}

PreferredSize Widget::GetPreferredSize()
{
	PreferredSize ps = GetPreferredContentSize();
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

void Widget::InvokeProcess()
{
	OnProcess();

	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
		child->InvokeProcess();

	OnProcessAfterChildren();
}

void Widget::InvokeProcessStates(bool force_update)
{
	if (!update_widget_states && !force_update)
		return;
	update_widget_states = false;

	OnProcessStates();

	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
		child->InvokeProcessStates(true);
}

void Widget::InvokePaint(const PaintProps &parent_paint_props)
{
	// Don't paint invisible widgets
	if (m_opacity == 0)
		return;

	uint32 state = GetAutoState();
	TBSkinElement *skin_element = GetSkinBgElement();

	// Multiply current opacity with widget opacity, skin opacity and state opacity.
	float old_opacity = g_renderer->GetOpacity();
	float opacity = old_opacity * m_opacity;
	if (skin_element)
		opacity *= skin_element->opacity;
	if (state & WIDGET_STATE_DISABLED)
		opacity *= 0.3f;
	if (opacity == 0)
		return;

	// FIX: This does not give the correct result! Must use a new render target!
	g_renderer->SetOpacity(opacity);

	int trns_x = m_rect.x, trns_y = m_rect.y;
	g_renderer->Translate(trns_x, trns_y);

	// Paint background skin
	TBRect local_rect(0, 0, m_rect.w, m_rect.h);
	TBSkinElement *used_element = g_tb_skin->PaintSkin(local_rect, skin_element, state);
	assert(!!used_element == !!skin_element);

	TB_IF_GFX_DEBUG(g_renderer->DrawRectDebug(local_rect, 255, 255, 255, 50));

	// Inherit properties from parent if not specified in the used skin for this widget.
	PaintProps paint_props = parent_paint_props;
	if (used_element && used_element->text_color != TBColor())
		paint_props.text_color = used_element->text_color;

	// Paint content
	OnPaint(paint_props);

	if (used_element)
		g_renderer->Translate(used_element->content_ofs_x, used_element->content_ofs_y);

	// Paint children
	OnPaintChildren(paint_props);

	if (used_element)
		g_renderer->Translate(-used_element->content_ofs_x, -used_element->content_ofs_y);

	g_renderer->Translate(-trns_x, -trns_y);
	g_renderer->SetOpacity(old_opacity);
}

bool Widget::InvokeEvent(TBWidgetEvent &ev)
{
	ev.target = this;

	// First call the global listener about this event.
	// Who knows, maybe some listener will block the event or cause us
	// to be deleted.
	TBWidgetSafePointer this_widget(this);
	if (TBGlobalWidgetListener::InvokeWidgetInvokeEvent(ev))
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
	Widget *tmp = this;
	while (tmp && !(handled = tmp->OnEvent(ev)))
		tmp = tmp->GetEventDestination();
	return handled;
}

void Widget::InvokePointerDown(int x, int y, int click_count, MODIFIER_KEYS modifierkeys)
{
	if (!captured_widget)
	{
		SetCapturedWidget(GetWidgetAt(x, y, true));
		SetHoveredWidget(captured_widget);
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
		// Focus the captured widget or the closest
		// focusable parent if it isn't focusable.
		Widget *focus_target = captured_widget;
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
		TBWidgetEvent ev(EVENT_TYPE_POINTER_DOWN, x, y, modifierkeys);
		ev.count = click_count;
		captured_widget->InvokeEvent(ev);
	}
}

void Widget::InvokePointerUp(int x, int y, MODIFIER_KEYS modifierkeys)
{
////ha ej button? ha ett contectmenu event ist'llet (kan vara longclick och keyboardmenyknappen)
///men gör man program vill man ha mittenknappen också!
//även back, forward events från musen
//ha radius (för finger?) måste kunna leta rätt på närliggande/mest överlappande vy
	if (captured_widget)
	{
		captured_widget->ConvertFromRoot(x, y);
		TBWidgetEvent ev_up(EVENT_TYPE_POINTER_UP, x, y, modifierkeys);
		TBWidgetEvent ev_click(EVENT_TYPE_CLICK, x, y, modifierkeys);
		captured_widget->InvokeEvent(ev_up);
		if (!is_panning && captured_widget && captured_widget->GetHitStatus(x, y))
			captured_widget->InvokeEvent(ev_click);
		if (captured_widget) // && button == captured_button
			captured_widget->ReleaseCapture();
	}
}

void Widget::InvokePointerMove(int x, int y, MODIFIER_KEYS modifierkeys)
{
	SetHoveredWidget(GetWidgetAt(x, y, true));
	Widget *target = captured_widget ? captured_widget : hovered_widget;
	if (target)
	{
		target->ConvertFromRoot(x, y);
		pointer_move_widget_x = x;
		pointer_move_widget_y = y;

		TBWidgetEvent ev(EVENT_TYPE_POINTER_MOVE, x, y, modifierkeys);
		if (target->InvokeEvent(ev))
			return;

		// The move event was not handled, so handle panning of scrollable widgets.
		HandlePanningOnMove(x, y);
	}
}

void Widget::HandlePanningOnMove(int x, int y)
{
	if (!captured_widget)
		return;

	// Check pointer movement
	int dx = pointer_down_widget_x - x;
	int dy = pointer_down_widget_y - y;
	int threshold = TBSystem::GetPanThreshold();
	bool maybe_start_panning = (ABS(dx) >= threshold || ABS(dy) >= threshold);

	// Do panning, or attempt starting panning (we don't know if any widget is scrollable yet)
	if (is_panning || maybe_start_panning)
	{
		int old_translation_x = 0, old_translation_y = 0;
		captured_widget->GetChildTranslation(old_translation_x, old_translation_y);

		int old_dx = dx, old_dy = dy;
		captured_widget->ScrollByRecursive(dx, dy);
		if (old_dx != dx || old_dy != dy)
		{
			// Scroll delta changed, so we are now panning!
			is_panning = true;
			// If the captured widget has panned too, we have to compensate the pointer down
			// coordinates so next pan handling isn't off.
			int new_translation_x = 0, new_translation_y = 0;
			captured_widget->GetChildTranslation(new_translation_x, new_translation_y);
			pointer_down_widget_x += new_translation_x - old_translation_x;
			pointer_down_widget_y += new_translation_y - old_translation_y;
		}
	}
}

void Widget::InvokeWheel(int x, int y, int delta, MODIFIER_KEYS modifierkeys)
{
	SetHoveredWidget(GetWidgetAt(x, y, true));
	Widget *target = captured_widget ? captured_widget : hovered_widget;
	if (target)
	{
		target->ConvertFromRoot(x, y);
		pointer_move_widget_x = x;
		pointer_move_widget_y = y;
		TBWidgetEvent ev(EVENT_TYPE_WHEEL, x, y, modifierkeys);
		ev.delta = delta;
		target->InvokeEvent(ev);
	}
}

void Widget::InvokeKey(int key, int special_key, MODIFIER_KEYS modifierkeys, bool down)
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
				focused_widget->SetState(WIDGET_STATE_PRESSED, down);

			// Invoke the click event
			if (!down)
			{
				TBWidgetEvent ev(EVENT_TYPE_CLICK, m_rect.w / 2, m_rect.h / 2);
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
		MoveFocus(!(modifierkeys & TB_SHIFT));

		// Show the focus when we move it by keyboard
		SetAutoFocusState(true);
	}
}

void Widget::ReleaseCapture()
{
	if (this == captured_widget)
		SetCapturedWidget(nullptr);
}

void Widget::ConvertToRoot(int &x, int &y) const
{
	const Widget *tmp = this;
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

void Widget::ConvertFromRoot(int &x, int &y) const
{
	const Widget *tmp = this;
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

void Widget::SetHoveredWidget(Widget *widget)
{
	if (Widget::hovered_widget == widget)
		return;
	if (widget && widget->GetState(WIDGET_STATE_DISABLED))
		return;

	// We apply hover state automatically so the widget might need to be updated.
	if (Widget::hovered_widget)
		Widget::hovered_widget->Invalidate();

	Widget::hovered_widget = widget;

	if (Widget::hovered_widget)
		Widget::hovered_widget->Invalidate();
}

void Widget::SetCapturedWidget(Widget *widget)
{
	if (Widget::captured_widget == widget)
		return;
	if (widget && widget->GetState(WIDGET_STATE_DISABLED))
		return;

	// Stop panning when capture change (most likely changing to nullptr because of InvokePointerUp)
	is_panning = false;

	Widget *old_capture = Widget::captured_widget;

	// We apply pressed state automatically so the widget might need to be updated.
	if (Widget::captured_widget)
		Widget::captured_widget->Invalidate();

	Widget::captured_widget = widget;

	if (old_capture)
		old_capture->OnCaptureChanged(false);

	if (Widget::captured_widget)
	{
		Widget::captured_widget->Invalidate();
		Widget::captured_widget->OnCaptureChanged(true);
	}
}

bool Widget::SetFontDescription(const TBFontDescription &font_desc)
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

void Widget::InvokeFontChanged()
{
	OnFontChanged();

	// Recurse to children that inherit the font
	for (Widget *child = GetFirstChild(); child; child = child->GetNext())
		if (child->m_font_desc.GetID() == 0)
			child->InvokeFontChanged();
}

TBFontDescription Widget::GetCalculatedFontDescription() const
{
	const Widget *tmp = this;
	while (tmp)
	{
		if (tmp->m_font_desc.GetID() != 0)
			return tmp->m_font_desc;
		tmp = tmp->m_parent;
	}
	return g_font_manager->GetDefaultFontDescription();
}

TBFontFace *Widget::GetFont() const
{
	return g_font_manager->GetFontFace(GetCalculatedFontDescription());
}

}; // namespace tinkerbell
