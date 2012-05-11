// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_select.h"
#include "tb_widgets_listener.h"
#include <assert.h>
#include <stdlib.h>

// There is no re-entrant qsort in the standard but there is most often qsort_r or qsort_s.
// In addition to that mess, linux have a different qsort_r from BSD.
#ifdef WIN32
# define tb_qsort(base, num, width, cb, context) qsort_s(base, num, width, cb, context)
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

/** TBSimpleLayoutItemWidget is a item containing a layout with the following:
	-TBSkinImage showing the item image.
	-TBTextField showint the item string.
	-TBSkinImage showing the arrow for items with a submenu.
	It also handles submenu events. */

class TBSimpleLayoutItemWidget : public TBLayout
{
public:
	TBSimpleLayoutItemWidget(TBID image, TBSelectItemSource *source, const char *str);
	~TBSimpleLayoutItemWidget();
	virtual bool OnEvent(const WidgetEvent &ev);
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
	if (m_image_arrow.m_parent)
		RemoveChild(&m_image_arrow);
	RemoveChild(&m_textfield);
	if (m_image.m_parent)
		RemoveChild(&m_image);
}

bool TBSimpleLayoutItemWidget::OnEvent(const WidgetEvent &ev)
{
	if (m_source && ev.type == EVENT_TYPE_CLICK && ev.target == this && !m_window_pointer.Get())
	{
		// Open a new menu window for the submenu with this widget as target
		if (TBMenuWindow *menu = new TBMenuWindow(this, TBIDC("submenu")))
		{
			m_window_pointer.Set(menu);
			menu->Show(m_source, -1, nullptr, TB_ALIGN_RIGHT);
		}
		return true;
	}
	return false;
}

// == TBSelectItemSource ====================================================================================

TBSelectItemSource::~TBSelectItemSource()
{
	// If this assert trig, you are deleting a model that's still set on some
	// TBSelect widget. That might be dangerous.
	assert(m_in_use_count == 0);
}

bool TBSelectItemSource::Filter(int index, const char *filter)
{
	const char *str = GetItemString(index);
	if (str && stristr(str, filter))
		return true;
	return false;
}

Widget *TBSelectItemSource::CreateItemWidget(int index)
{
// FIX: Flytta till TBGenericStringItemSource endå?
	const char *string = GetItemString(index);
	TBSelectItemSource *source = GetItemSource(index);
	TBID image = GetItemImage(index);
	if (source || image)
	{
		if (TBSimpleLayoutItemWidget *itemwidget = new TBSimpleLayoutItemWidget(image, source, string))
			return itemwidget;
	}
	else if (string && *string == '-')
	{
		if (TBSeparator *separator = new TBSeparator)
		{
			separator->SetGravity(WIDGET_GRAVITY_ALL);
			separator->SetSkinBg("TBSelectItem.separatorX");
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

int TBSelectItemSource::GetNumVisibleItems(const char *filter)
{
	if (!filter)
		return GetNumItems();
	int num_visible_items = 0;
	int num_items = GetNumItems();
	for (int i = 0; i < num_items; i++)
	{
		if (Filter(i, filter))
			num_visible_items++;
	}
	return num_visible_items;
}

// == TBGenericStringItemSource ==================================

Widget *TBGenericStringItemSource::CreateItemWidget(int index)
{
	if (Widget *widget = TBSelectItemSource::CreateItemWidget(index))
	{
		TBGenericStringItem *item = m_items[index];
		widget->GetID().Set(item->id);
		return widget;
	}
	return nullptr;
}

bool TBGenericStringItemSource::AddItem(TBGenericStringItem *item, int index)
{
	return m_items.Add(item);
}

void TBGenericStringItemSource::DeleteItem(int index)
{
	m_items.Delete(index);
}

void TBGenericStringItemSource::DeleteAllItems()
{
	m_items.DeleteAll();
}

// == TBSelectList ==============================================

TBSelectList::TBSelectList()
	: m_source(nullptr)
	, m_value(-1)
	, m_list_is_invalid(false)
	, m_scroll_to_current(false)
{
	SetIsFocusable(true);
	m_skin_bg.Set("TBSelectList");
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
}

TBSelectList::~TBSelectList()
{
	m_container.GetContentRoot()->RemoveChild(&m_layout);
	RemoveChild(&m_container);
	SetSource(nullptr);
}

void TBSelectList::SetSource(TBSelectItemSource *source)
{
	if (m_source == source)
		return;

	if (m_source)
		m_source->m_in_use_count--;
	m_source = source;
	if (m_source)
		m_source->m_in_use_count++;

	InvalidateList();
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
	while (Widget *child = m_layout.GetContentRoot()->GetFirstChild())
	{
		child->m_parent->RemoveChild(child);
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

	// Create new items
	for (int i = 0; i < num_sorted_items; i++)
	{
		int item_index = sorted_index[i];
		if (Widget *widget = m_source->CreateItemWidget(item_index))
		{
			// Use item data as widget to index lookup
			widget->m_data = item_index;
			m_layout.GetContentRoot()->AddChild(widget);
		}
	}
	delete [] sorted_index;

	SelectItem(m_value, true);
	m_scroll_to_current = true;
}

void TBSelectList::SetValue(int value)
{
	if (value == m_value)
		return;

	SelectItem(m_value, false);
	m_value = value;
	SelectItem(m_value, true);
	ScrollToSelectedItem();

	WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	if (Widget *widget = GetItemWidget(m_value))
		ev.ref_id = widget->m_id;
	InvokeEvent(ev);
}

void TBSelectList::SelectItem(int index, bool selected)
{
	if (Widget *widget = GetItemWidget(index))
		widget->SetState(WIDGET_STATE_SELECTED, selected);
}

Widget *TBSelectList::GetItemWidget(int index)
{
	if (index == -1)
		return nullptr;
	for (Widget *tmp = m_layout.GetContentRoot()->GetFirstChild(); tmp; tmp = tmp->GetNext())
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
	if (Widget *widget = GetItemWidget(m_value))
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

bool TBSelectList::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && ev.target->m_parent == m_layout.GetContentRoot())
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
			WidgetEvent ev(EVENT_TYPE_CLICK, 0, 0);
			if (Widget *widget = GetItemWidget(m_value))
				ev.ref_id = widget->m_id;
			target_list->InvokeEvent(ev);
		}
		return true;
	}
	else if (ev.type == EVENT_TYPE_KEY_DOWN && m_source && m_layout.GetContentRoot()->GetFirstChild())
	{
		Widget *item_root = m_layout.GetContentRoot();
		Widget *current = GetItemWidget(m_value);
// FIX: Hoppa över disabled, spacers etc. Kanske kan användamig utav focus koden?
//      also do that from the keyboardhandling in dropdown (when it's not open!)
		if (ev.special_key == TB_KEY_HOME)
			SetValue(item_root->GetFirstChild()->m_data);
		else if (ev.special_key == TB_KEY_END)
			SetValue(item_root->GetLastChild()->m_data);
		else if (ev.special_key == TB_KEY_UP)
		{
			if (current && current->GetPrev())
				SetValue(current->GetPrev()->m_data);
			else
				SetValue(item_root->GetFirstChild()->m_data);
		}
		else if (ev.special_key == TB_KEY_DOWN)
		{
			if (current && current->GetNext())
				SetValue(current->GetNext()->m_data);
			else
				SetValue(item_root->GetLastChild()->m_data);
		}
		else
			return false;
		return true;
	}
	return false;
}

// == TBSelectDropdown ==========================================

TBSelectDropdown::TBSelectDropdown()
	: m_source(nullptr)
	, m_value(-1)
{
	m_skin_bg.Set("TBSelectDropdown");
	m_arrow.SetSkinBg("TBSelectDropdown.arrow");
	GetContentRoot()->AddChild(&m_arrow);
}

TBSelectDropdown::~TBSelectDropdown()
{
	GetContentRoot()->RemoveChild(&m_arrow);
	SetSource(nullptr);
}

void TBSelectDropdown::SetSource(TBSelectItemSource *source)
{
	// Update use count
	if (m_source)
		m_source->m_in_use_count--;
	m_source = source;
	if (m_source)
		m_source->m_in_use_count++;

	m_value = -1;
	if (m_source && m_source->GetNumItems())
		SetValue(0);
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

	WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
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

bool TBSelectDropdown::OnEvent(const WidgetEvent &ev)
{
	if (ev.target == this && ev.type == EVENT_TYPE_CLICK)
	{
		OpenWindow();
		return true;
	}
	else if (ev.target->GetID() == TBIDC("TBSelectDropdown.window") && ev.type == EVENT_TYPE_CLICK)
	{
		if (TBMenuWindow *menu_window = TBSafeCast(TBMenuWindow, ev.target->GetParentWindow()))
			SetValue(menu_window->GetList()->GetValue());
		return true;
	}
	return false;
}

// == TBMenuWindow ==========================================

TBMenuWindow::TBMenuWindow(Widget *target, TBID id)
	: TBWidgetSafePointer(target)
	, m_select_list(nullptr)
{
	m_id.Set(id);
}

bool TBMenuWindow::Show(TBSelectItemSource *source, int initial_value, const TBPoint *pos_in_root, TB_ALIGN align)
{
	SetSettings(WINDOW_SETTINGS_NONE);
	m_skin_bg.Set("TBMenuWindow");

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

	Widget *root = Get()->GetParentRoot();
	root->AddChild(this);
	return true;
}

TBRect TBMenuWindow::GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align)
{
	Widget *target = Get();
	Widget *root = Get()->GetParentRoot();
	PreferredSize ps = GetPreferredSize();

	TBRect target_rect;
	TBPoint pos;
	int w = MIN(ps.pref_w, root->m_rect.w);
	int h = MIN(ps.pref_h, root->m_rect.h);
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
			h = MIN(h, root->m_rect.h / 2 - target->m_rect.h);
		target_rect = target->m_rect;
	}

	int x, y;
	if (align == TB_ALIGN_BOTTOM)
	{
		x = pos.x;
		y = pos.y + target_rect.h + h > root->m_rect.h ? pos.y - h : pos.y + target_rect.h;
	}
	else if (align == TB_ALIGN_TOP)
	{
		x = pos.x;
		y = pos.y - h < 0 ? pos.y + target_rect.h : pos.y - h;
	}
	else if (align == TB_ALIGN_RIGHT)
	{
		x = pos.x + target_rect.w + w > root->m_rect.w ? pos.x - w : pos.x + target_rect.w;
		y = MIN(pos.y, root->m_rect.h - h);
	}
	else //if (align == TB_ALIGN_LEFT)
	{
		x = pos.x - w < 0 ? pos.x + target_rect.w : pos.x - w;
		y = MIN(pos.y, root->m_rect.h - h);
	}

	return TBRect(x, y, w, h);
}

bool TBMenuWindow::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && m_select_list == ev.target)
	{
		TBWidgetSafePointer this_widget(this);

		// Invoke the click on the target
		WidgetEvent target_ev(EVENT_TYPE_CLICK);
		target_ev.ref_id = ev.ref_id;
		InvokeEvent(target_ev);

		// If target got deleted, close
		if (this_widget.Get())
			Close();
		return true;
	}
	return TBWindow::OnEvent(ev);
}

void TBMenuWindow::OnWidgetFocusChanged(Widget *widget, bool focused)
{
	Close();
}

bool TBMenuWindow::OnWidgetInvokeEvent(const WidgetEvent &ev)
{
	if ((ev.type == EVENT_TYPE_POINTER_DOWN || ev.type == EVENT_TYPE_CONTEXT_MENU) &&
		!IsEventDestinationFor(ev.target))
		Close();
	return false;
}

void TBMenuWindow::OnWidgetDelete(Widget *widget)
{
	TBWidgetSafePointer::OnWidgetDelete(widget);
	// If the target widget is deleted, close!
	if (!Get())
		Close();
}

// == TBInlineSelect ========================================================================================

TBInlineSelect::TBInlineSelect()
	: m_value(0)
	, m_min(0)
	, m_max(100)
{
	SetSkinBg("TBInlineSelect");
	AddChild(&m_layout);
	m_layout.AddChild(&m_buttons[0]);
	m_layout.AddChild(&m_editfield);
	m_layout.AddChild(&m_buttons[1]);
	m_layout.SetRect(GetPaddingRect());
	m_layout.SetGravity(WIDGET_GRAVITY_ALL);
	m_layout.SetSpacing(0);
	m_buttons[0].SetSkinBg("TBButton.flat");
	m_buttons[1].SetSkinBg("TBButton.flat");
	m_buttons[0].GetContentRoot()->AddChild(new TBSkinImage("arrow.left"));
	m_buttons[1].GetContentRoot()->AddChild(new TBSkinImage("arrow.right"));
	m_buttons[0].SetIsFocusable(false);
	m_buttons[1].SetIsFocusable(false);
	m_buttons[0].GetID().Set("dec");
	m_buttons[1].GetID().Set("inc");
	m_buttons[0].SetAutoRepeat(true);
	m_buttons[1].SetAutoRepeat(true);
	m_editfield.SetTextAlign(TB_TEXT_ALIGN_CENTER);
	m_editfield.SetEditType(EDIT_TYPE_NUMBER);
	m_editfield.SetText("0");
}

TBInlineSelect::~TBInlineSelect()
{
	m_layout.RemoveChild(&m_buttons[1]);
	m_layout.RemoveChild(&m_editfield);
	m_layout.RemoveChild(&m_buttons[0]);
	RemoveChild(&m_layout);
}

void TBInlineSelect::SetLimits(int min, int max)
{
	assert(min <= max);
	m_min = min;
	m_max = max;
	SetValue(m_value);
}

// FIX: axis måste byta knapparnas bild och laytouten!
// FIX: avfokusering ska sätta den rätta texten
void TBInlineSelect::SetValueInternal(int value, bool update_text)
{
	value = CLAMP(value, m_min, m_max);
	if (value == m_value)
		return;
	m_value = value;

	if (update_text)
	{
		TBStr strval;
		strval.SetFormatted("%d", m_value);
		m_editfield.SetText(strval);
	}

	//FIX: some of theese in tinkerbell doesn't check if the widget is removed afterwards!
	//WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	//InvokeEvent(ev);
}

void TBInlineSelect::OnSkinChanged()
{
	m_layout.SetRect(GetPaddingRect());
}

bool TBInlineSelect::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_KEY_DOWN)
	{
		if (ev.special_key == TB_KEY_UP || ev.special_key == TB_KEY_DOWN)
		{
			int dv = ev.special_key == TB_KEY_UP ? 1 : -1;
			SetValue(GetValue() + dv);
			return true;
		}
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("dec"))
	{
		SetValue(GetValue() - 1);
		return true;
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("inc"))
	{
		SetValue(GetValue() + 1);
		return true;
	}
	else if (ev.type == EVENT_TYPE_CHANGED && ev.target == &m_editfield)
	{
		TBStr text;
		m_editfield.GetText(text);
		SetValueInternal(atoi(text), false);
	}
	return false;
}

}; // namespace tinkerbell
