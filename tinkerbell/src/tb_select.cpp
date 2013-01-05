// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_select.h"
#include "tb_widgets_listener.h"
#include "tb_language.h"
#include <assert.h>
#include <stdlib.h>

// There is no re-entrant qsort in the standard but there is most often qsort_r or qsort_s.
// In addition to that mess, linux have a different qsort_r from BSD.
#ifdef WIN32
# define tb_qsort(base, num, width, cb, context) qsort_s(base, num, width, cb, context)
# define tb_sort_cb(context, a, b) int select_list_sort_cb(void *context, const void *a, const void *b)
#elif defined(ANDROID)
// FIX: There's no sort on android! Implement!
# define tb_qsort(base, num, width, cb, context)
# define tb_sort_cb(context, a, b) int select_list_sort_cb(void *context, const void *a, const void *b)
#elif defined(LINUX)
# define tb_qsort(base, num, width, cb, context) qsort_r(base, num, width, cb, context)
# define tb_sort_cb(context, a, b) int select_list_sort_cb(const void *a, const void *b, void *context)
#else
# define tb_qsort(base, num, width, cb, context) qsort_r(base, num, width, context, cb)
# define tb_sort_cb(context, a, b) int select_list_sort_cb(void *context, const void *a, const void *b)
#endif

namespace tinkerbell {

// == Sort callback for sorting items ===================================================

struct SELECT_LIST_SORT_CONTEXT {
	TBSelectItemSource *source;
};

tb_sort_cb(_context, _a, _b)
{
	SELECT_LIST_SORT_CONTEXT *context = static_cast<SELECT_LIST_SORT_CONTEXT *>(_context);
	int a = *((int*) _a);
	int b = *((int*) _b);
	const char *str_a = context->source->GetItemString(a);
	const char *str_b = context->source->GetItemString(b);
	if (context->source->GetSort() == TB_SORT_DESCENDING)
		return -strcmp(str_a, str_b);
	return strcmp(str_a, str_b);
}

// == TBSelectList ==============================================

TBSelectList::TBSelectList()
	: m_value(-1)
	, m_list_is_invalid(false)
	, m_scroll_to_current(false)
	, m_header_lng_string_id(TBIDC("TBList.header"))
{
	SetSource(&m_default_source);
	SetIsFocusable(true);
	SetSkinBg("TBSelectList", WIDGET_INVOKE_INFO_NO_CALLBACKS);
	m_container.SetGravity(WIDGET_GRAVITY_ALL);
	m_container.SetRect(GetPaddingRect());
	AddChild(&m_container);
	m_layout.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.SetAxis(AXIS_Y);
	m_layout.SetSpacing(0);
	m_layout.SetLayoutPosition(LAYOUT_POSITION_LEFT_TOP);
	m_layout.SetLayoutDistributionPosition(LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP);
	m_layout.SetLayoutSize(LAYOUT_SIZE_AVAILABLE);
	m_container.GetContentRoot()->AddChild(&m_layout);
	m_container.SetScrollMode(SCROLL_MODE_Y_AUTO);
	m_container.SetAdaptContentSize(true);
}

TBSelectList::~TBSelectList()
{
	m_container.GetContentRoot()->RemoveChild(&m_layout);
	RemoveChild(&m_container);
	SetSource(nullptr);
}

void TBSelectList::OnSourceChanged()
{
	InvalidateList();
}

void TBSelectList::OnItemChanged(int index)
{
	if (m_list_is_invalid) // We're updating all widgets soon.
		return;

	TBWidget *old_widget = GetItemWidget(index);
	if (!old_widget) // We don't have this widget so we have nothing to update.
		return;

	// Replace the old widget representing the item, with a new one. Preserve its state.
	WIDGET_STATE old_state = old_widget->GetStateRaw();

	if (TBWidget *widget = CreateAndAddItemAfter(index, old_widget))
		widget->SetStateRaw(old_state);

	old_widget->GetParent()->RemoveChild(old_widget);
	delete old_widget;
}

void TBSelectList::OnItemAdded(int index)
{
	if (m_list_is_invalid) // We're updating all widgets soon.
		return;

	// Sorting, filtering etc. makes it messy to handle dynamic addition of items.
	// Resort to invalidate the entire list (may even be faster anyway)
	InvalidateList();
}

void TBSelectList::OnItemRemoved(int index)
{
	if (m_list_is_invalid) // We're updating all widgets soon.
		return;

	// Sorting, filtering etc. makes it messy to handle dynamic addition of items.
	// Resort to invalidate the entire list (may even be faster anyway)
	InvalidateList();
}

void TBSelectList::OnAllItemsRemoved()
{
	InvalidateList();
	m_value = -1;
}

void TBSelectList::SetFilter(const char *filter)
{
	TBStr new_filter;
	if (filter && *filter)
		new_filter.Set(filter);
	if (m_filter.Equals(new_filter))
		return;
	m_filter.Set(new_filter);
	InvalidateList();
}

void TBSelectList::SetHeaderString(const TBID& id)
{
	if (m_header_lng_string_id == id)
		return;
	m_header_lng_string_id = id;
	InvalidateList();
}

void TBSelectList::InvalidateList()
{
	if (m_list_is_invalid)
		return;
	m_list_is_invalid = true;
	Invalidate();
}

void TBSelectList::ValidateList()
{
	if (!m_list_is_invalid)
		return;
	m_list_is_invalid = false;
	// FIX: Could delete and create only the changed items (faster filter change)

	// Remove old items
	while (TBWidget *child = m_layout.GetContentRoot()->GetFirstChild())
	{
		child->GetParent()->RemoveChild(child);
		delete child;
	}
	if (!m_source || !m_source->GetNumItems())
		return;

	// Create a sorted list of the items we should include using the current filter.
	int num_sorted_items = 0;
	int *sorted_index = new int[m_source->GetNumItems()];
	if (!sorted_index)
		return; // Out of memory

	// Populate the sorted index list
	for (int i = 0; i < m_source->GetNumItems(); i++)
		if (m_filter.IsEmpty() || m_source->Filter(i, m_filter))
			sorted_index[num_sorted_items++] = i;

	// Sort
	if (m_source->GetSort() != TB_SORT_NONE)
	{
		SELECT_LIST_SORT_CONTEXT context = { m_source };
		tb_qsort(sorted_index, num_sorted_items, sizeof(int), select_list_sort_cb, &context);
	}

	// Show header if we only show a subset of all items.
	if (!m_filter.IsEmpty())
	{
		if (TBWidget *widget = new TBTextField())
		{
			TBStr str;
			str.SetFormatted(g_tb_lng->GetString(m_header_lng_string_id), num_sorted_items, m_source->GetNumItems());
			widget->SetText(str);
			widget->SetSkinBg(TBIDC("TBList.header"));
			widget->SetState(WIDGET_STATE_DISABLED, true);
			widget->SetGravity(WIDGET_GRAVITY_ALL);
			widget->m_data = (uint32)-1;
			m_layout.GetContentRoot()->AddChild(widget);
		}
	}

	// Create new items
	for (int i = 0; i < num_sorted_items; i++)
		CreateAndAddItemAfter(sorted_index[i], nullptr);
	delete [] sorted_index;

	SelectItem(m_value, true);

	// FIX: Should not scroll just because we update the list. Only automatically first time!
	m_scroll_to_current = true;
}

TBWidget *TBSelectList::CreateAndAddItemAfter(int index, TBWidget *reference)
{
	if (TBWidget *widget = m_source->CreateItemWidget(index, this))
	{
		// Use item data as widget to index lookup
		widget->m_data = index;
		m_layout.GetContentRoot()->AddChildRelative(widget, WIDGET_Z_REL_AFTER, reference);
		return widget;
	}
	return nullptr;
}

void TBSelectList::SetValue(int value)
{
	if (value == m_value)
		return;

	SelectItem(m_value, false);
	m_value = value;
	SelectItem(m_value, true);
	ScrollToSelectedItem();

	TBWidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	if (TBWidget *widget = GetItemWidget(m_value))
		ev.ref_id = widget->GetID();
	InvokeEvent(ev);
}

void TBSelectList::SelectItem(int index, bool selected)
{
	if (TBWidget *widget = GetItemWidget(index))
		widget->SetState(WIDGET_STATE_SELECTED, selected);
}

TBWidget *TBSelectList::GetItemWidget(int index)
{
	if (index == -1)
		return nullptr;
	for (TBWidget *tmp = m_layout.GetContentRoot()->GetFirstChild(); tmp; tmp = tmp->GetNext())
		if (tmp->m_data == index)
			return tmp;
	return nullptr;
}

void TBSelectList::ScrollToSelectedItem()
{
	if (m_list_is_invalid)
	{
		m_scroll_to_current = true;
		return;
	}
	m_scroll_to_current = false;
	if (TBWidget *widget = GetItemWidget(m_value))
		m_container.ScrollIntoView(widget->GetRect());
	else
		m_container.ScrollTo(0, 0);
}

void TBSelectList::OnSkinChanged()
{
	m_container.SetRect(GetPaddingRect());
}

void TBSelectList::OnProcess()
{
	ValidateList();
}

void TBSelectList::OnProcessAfterChildren()
{
	if (m_scroll_to_current)
		ScrollToSelectedItem();
}

bool TBSelectList::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && ev.target->GetParent() == m_layout.GetContentRoot())
	{
		// SetValue (EVENT_TYPE_CHANGED) might cause something to delete this (f.ex closing
		// the dropdown menu. We want to sent another event, so ensure we're still around.
		TBWidgetSafePointer this_widget(this);

		int index = ev.target->m_data;
		SetValue(index);

		// If we're still around, invoke the click event too.
		if (this_widget.Get())
		{
			TBSelectList *target_list = this;
			// If the parent window is a TBMenuWindow, we will iterate up the event destination
			// chain to find the top TBMenuWindow and invoke the event there.
			// That way events in submenus will reach the caller properly, and seem like it was
			// invoked on the top menu.
			TBWindow *window = GetParentWindow();
			while (TBMenuWindow *menu_win = TBSafeCast(TBMenuWindow, window))
			{
				target_list = menu_win->GetList();
				window = menu_win->GetEventDestination()->GetParentWindow();
			}

			// Invoke the click event on the target list
			TBWidgetEvent ev(EVENT_TYPE_CLICK, 0, 0);
			if (TBWidget *widget = GetItemWidget(m_value))
				ev.ref_id = widget->GetID();
			target_list->InvokeEvent(ev);
		}
		return true;
	}
	else if (ev.type == EVENT_TYPE_KEY_DOWN)
	{
		return ChangeValue(ev.special_key);
	}
	return false;
}

bool TBSelectList::ChangeValue(SPECIAL_KEY key)
{
	if (!m_source || !m_layout.GetContentRoot()->GetFirstChild())
		return false;

	bool forward;
	if (key == TB_KEY_HOME || key == TB_KEY_DOWN)
		forward = true;
	else if (key == TB_KEY_END || key == TB_KEY_UP)
		forward = false;
	else
		return false;

	TBWidget *item_root = m_layout.GetContentRoot();
	TBWidget *current = GetItemWidget(m_value);
	TBWidget *origin = nullptr;
	if (key == TB_KEY_HOME || (!current && key == TB_KEY_DOWN))
		current = item_root->GetFirstChild();
	else if (key == TB_KEY_END || (!current && key == TB_KEY_UP))
		current = item_root->GetLastChild();
	else
		origin = current;

	while (current)
	{
		if (current != origin && !current->GetDisabled())
			break;
		current = forward ? current->GetNext() : current->GetPrev();
	}
	// Select and focus what we found
	if (current)
	{
		SetValue(current->m_data);
		return true;
	}
	return false;
}

// == TBSelectDropdown ==========================================

TBSelectDropdown::TBSelectDropdown()
	: m_value(-1)
{
	SetSource(&m_default_source);
	SetSkinBg("TBSelectDropdown", WIDGET_INVOKE_INFO_NO_CALLBACKS);
	m_arrow.SetSkinBg("TBSelectDropdown.arrow", WIDGET_INVOKE_INFO_NO_CALLBACKS);
	GetContentRoot()->AddChild(&m_arrow);
}

TBSelectDropdown::~TBSelectDropdown()
{
	GetContentRoot()->RemoveChild(&m_arrow);
	SetSource(nullptr);
}

void TBSelectDropdown::OnSourceChanged()
{
	m_value = -1;
	if (m_source && m_source->GetNumItems())
		SetValue(0);
}

void TBSelectDropdown::OnItemChanged(int index)
{
}

void TBSelectDropdown::SetValue(int value)
{
	if (value == m_value || !m_source)
		return;
	m_value = value;

	if (m_value < 0)
		SetText("");
	else if (m_value < m_source->GetNumItems())
		SetText(m_source->GetItemString(m_value));

	TBWidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	InvokeEvent(ev);
}

void TBSelectDropdown::OpenWindow()
{
	if (!m_source || !m_source->GetNumItems() || m_window_pointer.Get())
		return;

	if (TBMenuWindow *window = new TBMenuWindow(this, TBIDC("TBSelectDropdown.window")))
	{
		m_window_pointer.Set(window);
		window->SetSkinBg("TBSelectDropdown.window");
		window->Show(m_source, GetValue());
	}
}

TBMenuWindow *TBSelectDropdown::GetMenuIfOpen() const
{
	return TBSafeCast(TBMenuWindow, m_window_pointer.Get());
}

bool TBSelectDropdown::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.target == this && ev.type == EVENT_TYPE_CLICK)
	{
		// Open the menu, or set the value and close it if already open (this will
		// happen when clicking by keyboard since that will call click on this button)
		if (TBMenuWindow *menu_window = GetMenuIfOpen())
		{
			TBWidgetSafePointer tmp(this);
			int value = menu_window->GetList()->GetValue();
			menu_window->Die();
			if (tmp.Get())
				SetValue(value);
		}
		else
			OpenWindow();
		return true;
	}
	else if (ev.target->GetID() == TBIDC("TBSelectDropdown.window") && ev.type == EVENT_TYPE_CLICK)
	{
		// Set the value of the clicked item
		if (TBMenuWindow *menu_window = GetMenuIfOpen())
			SetValue(menu_window->GetList()->GetValue());
		return true;
	}
	else if (ev.target == this && m_source && ev.IsKeyEvent())
	{
		if (TBMenuWindow *menu_window = GetMenuIfOpen())
		{
			// Redirect the key strokes to the list
			TBWidgetEvent redirected_ev(ev);
			return menu_window->GetList()->InvokeEvent(redirected_ev);
		}
	}
	return false;
}

// == TBMenuWindow ==========================================

TBMenuWindow::TBMenuWindow(TBWidget *target, TBID id)
	: TBWidgetSafePointer(target)
	, m_select_list(nullptr)
{
	SetID(id);
	SetSkinBg("TBMenuWindow", WIDGET_INVOKE_INFO_NO_CALLBACKS);
}

bool TBMenuWindow::Show(TBSelectItemSource *source, int initial_value, const TBPoint *pos_in_root, TB_ALIGN align)
{
	SetSettings(WINDOW_SETTINGS_NONE);

	if (m_select_list = new TBSelectList)
	{
		m_select_list->SetIsFocusable(false); ///< Avoid it autoclosing its window on click
		m_select_list->SetSkinBg("");
		m_select_list->GetScrollContainer()->SetAdaptToContentSize(true);
		m_select_list->SetValue(initial_value);
		m_select_list->SetSource(source);
		m_select_list->SetRect(GetPaddingRect());
		m_select_list->SetGravity(WIDGET_GRAVITY_ALL);
		m_select_list->ValidateList();
		AddChild(m_select_list);
	}

	// Calculate and set a good size for the dropdown window
	SetRect(GetAlignedRect(pos_in_root, align));

	TBWidget *root = Get()->GetParentRoot();
	root->AddChild(this);
	return true;
}

TBRect TBMenuWindow::GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align)
{
	TBWidget *target = Get();
	TBWidget *root = Get()->GetParentRoot();
	PreferredSize ps = GetPreferredSize();

	TBRect target_rect;
	TBPoint pos;
	int w = MIN(ps.pref_w, root->GetRect().w);
	int h = MIN(ps.pref_h, root->GetRect().h);
	if (pos_in_root)
	{
		pos = *pos_in_root;
	}
	else
	{
		target->ConvertToRoot(pos.x, pos.y);
		// FIX: add a minimum_width! This would shrink it for every submenu!
		//w = CLAMP(ps.pref_w, target->m_rect.w, root->m_rect.w);

		// If the menu is aligned top or bottom, limit its height to the worst case available height.
		// Being in the center of the root, that is half the root height minus the target rect.
		if (align == TB_ALIGN_TOP || align == TB_ALIGN_BOTTOM)
			h = MIN(h, root->GetRect().h / 2 - target->GetRect().h);
		target_rect = target->GetRect();
	}

	int x, y;
	if (align == TB_ALIGN_BOTTOM)
	{
		x = pos.x;
		y = pos.y + target_rect.h + h > root->GetRect().h ? pos.y - h : pos.y + target_rect.h;
	}
	else if (align == TB_ALIGN_TOP)
	{
		x = pos.x;
		y = pos.y - h < 0 ? pos.y + target_rect.h : pos.y - h;
	}
	else if (align == TB_ALIGN_RIGHT)
	{
		x = pos.x + target_rect.w + w > root->GetRect().w ? pos.x - w : pos.x + target_rect.w;
		y = MIN(pos.y, root->GetRect().h - h);
	}
	else //if (align == TB_ALIGN_LEFT)
	{
		x = pos.x - w < 0 ? pos.x + target_rect.w : pos.x - w;
		y = MIN(pos.y, root->GetRect().h - h);
	}

	return TBRect(x, y, w, h);
}

bool TBMenuWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && m_select_list == ev.target)
	{
		TBWidgetSafePointer this_widget(this);

		// Invoke the click on the target
		TBWidgetEvent target_ev(EVENT_TYPE_CLICK);
		target_ev.ref_id = ev.ref_id;
		InvokeEvent(target_ev);

		// If target got deleted, close
		if (this_widget.Get())
			Close();
		return true;
	}
	return TBWindow::OnEvent(ev);
}

void TBMenuWindow::OnWidgetFocusChanged(TBWidget *widget, bool focused)
{
	Close();
}

bool TBMenuWindow::OnWidgetInvokeEvent(const TBWidgetEvent &ev)
{
	if ((ev.type == EVENT_TYPE_POINTER_DOWN || ev.type == EVENT_TYPE_CONTEXT_MENU) &&
		!IsEventDestinationFor(ev.target))
		Close();
	return false;
}

void TBMenuWindow::OnWidgetDelete(TBWidget *widget)
{
	TBWidgetSafePointer::OnWidgetDelete(widget);
	// If the target widget is deleted, close!
	if (!Get())
		Close();
}

}; // namespace tinkerbell
