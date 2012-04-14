// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_SELECT_H
#define TB_SELECT_H

#include "tb_window.h"
#include "tb_scroll_container.h"
#include "tb_widgets_listener.h"
#include "tb_list.h"
#include "tb_editfield.h"

namespace tinkerbell {

// FIX: A maximum items showed option, and a automatic "Show more" item at the bottom.
// FIX: ScrollableContainer: Scrolla inte om man håller i en scrollbar eller draggar.
// FIX: Item-quick-find by pressing one or two keys in the list!
// FIX: gör även en default string source med checkboxar (multiselectable)??
// Indentera alla items, och ge menyn ett bg skin/underlay ifall någon har ikon!
// Treeview item, item column, och indent för items utan ikon.. använda samma indent system?
// FIX: ändring av source måste kunna updatera alla listor.

enum TB_SORT {
	TB_SORT_NONE,		///< No sorting. Items appear in list order.
	TB_SORT_ASCENDING,	///< Ascending sort.
	TB_SORT_DESCENDING	///< Descending sort.
};

/** TBSelectItemSource is a item provider interface for list widgets (TBSelectList and
	TBSelectDropdown).

	Instead of feeding all list widgets with all items all the time, the list widgets
	will ask TBSelectItemSource when it needs it. The list widgets may also apply
	filtering so only a subset of all the items are shown.

	CreateItemWidget can be overridden to create more advanced content than a single string. */

class TBSelectItemSource
{
public:
	TBSelectItemSource() : m_in_use_count(0), m_sort(TB_SORT_NONE) {}
	virtual ~TBSelectItemSource();

	/** Return true if a item matches the given filter text.
		By default, it returns true if GetItemString contains filter. */
	virtual bool Filter(int index, const char *filter);

	/** Get the string of a item. If a item has more than one string,
		return the one that should be used for inline-find (pressing keys
		in the list will scroll to the item starting with the same letters),
		and for sorting the list. */
	virtual const char *GetItemString(int index) = 0;

	/** Get the source to be used if this item should open a sub menu. */
	virtual TBSelectItemSource *GetItemSource(int index) { return nullptr; }

	/** Get the skin image to be painted before the text for this item. */
	virtual TBID GetItemImage(int index) { return TBID(); }

	/** Create the item representation widget(s). By default, it will create
		a TBTextField for string-only items, and other types for items that
		also has image or submenu. */
	virtual Widget *CreateItemWidget(int index);

	/** Get the number of items */
	virtual int GetNumItems() = 0;

	/** Get the number of items that would be shown with the given filter. */
	int GetNumVisibleItems(const char *filter);

	/** Set sort type. Default is TB_SORT_NONE. */
	void SetSort(TB_SORT sort) { m_sort = sort; }
	TB_SORT GetSort() const { return m_sort; }
private:
	friend class TBSelectList;
	friend class TBSelectDropdown;
	int m_in_use_count; ///< How many lists that currently have this source set.
	TB_SORT m_sort;
};

class TBGenericStringItem
{
public:
	TBGenericStringItem(const TBGenericStringItem& other) : str(other.str), id(other.id), sub_source(other.sub_source), user_ptr(other.user_ptr) {}
	TBGenericStringItem(const char *str) : str(str), sub_source(nullptr), user_ptr(nullptr) {}
	TBGenericStringItem(const char *str, TBID id) : str(str), id(id), sub_source(nullptr), user_ptr(nullptr) {}
	TBGenericStringItem(const char *str, TBSelectItemSource *sub_source) : str(str), sub_source(sub_source), user_ptr(nullptr) {}
	const TBGenericStringItem& operator = (const TBGenericStringItem &other) { str.Set(other.str); id = other.id; sub_source = other.sub_source; user_ptr = other.user_ptr; return *this; }

	void SetSkinImage(TBID image) { skin_image = image; }

	void *GetUserPtr() { return user_ptr; }
	void SetUserPtr(void *p) { user_ptr = p; }
public:
	TBStr str;
	TBID id;
	TBID skin_image;
	TBSelectItemSource *sub_source;
	void *user_ptr;
};

class TBGenericStringItemSource : public TBSelectItemSource
{
public:
	TBGenericStringItemSource() {}
	virtual ~TBGenericStringItemSource()				{ DeleteAllItems(); }
	virtual const char *GetItemString(int index)		{ return GetItem(index)->str; }
	virtual TBSelectItemSource *GetItemSource(int index){ return GetItem(index)->sub_source; }
	virtual TBID GetItemImage(int index)				{ return GetItem(index)->skin_image; }
	virtual int GetNumItems()							{ return m_items.GetNumItems(); }
	virtual Widget *CreateItemWidget(int index);

	bool AddItem(TBGenericStringItem *item, int index);
	bool AddItem(TBGenericStringItem *item)				{ return AddItem(item, GetNumItems() - 1); }
	TBGenericStringItem *GetItem(int index)				{ return m_items[index]; }

	void DeleteItem(int index);
	void DeleteAllItems();
private:
	TBListOf<TBGenericStringItem> m_items;
};

class TBSelectList : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSelectList", Widget);

	TBSelectList();
	~TBSelectList();

	/** Set the source which should provide the items for this select.
		This source needs to live longer than this widget. */
	void SetSource(TBSelectItemSource *source);
	TBSelectItemSource *GetSource() { return m_source; }

	/** Set filter string so only matching items will be showed.
		Set nullptr or empty string to remove filter and show all items. */
	void SetFilter(const char *filter);
	const char *GetFilter() { return m_filter; }

	/** Make the list update its items to reflect the items from the
		in the current source. The update will take place next time
		the list is validated. */
	void InvalidateList();

	/** Make sure the list is reflecting the current items in the source. */
	void ValidateList();

	/** The value is the selected item. In lists with multiple selectable
		items it's the item that is the current focus. */
	virtual void SetValue(int value);
	virtual int GetValue() { return m_value; }

	void SelectItem(int index, bool selected);
	Widget *GetItemWidget(int index);

	/** Scroll to the current selected item. The scroll may be delayed until
		the items has been layouted if the layout is currently invalid. */
	void ScrollToSelectedItem();

	/** Return the scrollcontainer used in this list. */
	TBScrollContainer *GetScrollContainer() { return &m_container; }

	virtual void OnSkinChanged();
	virtual void OnProcess();
	virtual void OnProcessAfterChildren();
	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	TBScrollContainer m_container;
	TBLayout m_layout;
	TBSelectItemSource *m_source;
	int m_value;
	TBStr m_filter;
	bool m_list_is_invalid;
	bool m_scroll_to_current;
};

class TBSelectDropdown : public TBButton
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSelectDropdown", TBButton);

	TBSelectDropdown();
	~TBSelectDropdown();

	/** Set the source which should provide the items for this select.
		This source needs to live longer than this widget. */
	void SetSource(TBSelectItemSource *source);
	TBSelectItemSource *GetSource() { return m_source; }

	/** Set the selected item. */
	virtual void SetValue(int value);
	virtual int GetValue() { return m_value; }

	/** Open the window if the model has items. */
	void OpenWindow();

	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	TBSelectItemSource *m_source;
	TBSkinImage m_arrow;
	int m_value;
	TBWidgetSafePointer m_window_pointer; ///< Points to the dropdown window if opened
};

/** TBMenuWindow is a popup window that shows a list of items (TBSelectList).
	When selected it will invoke the click on the item and then close itself.
	It may open sub items as new windows at the same time as this window is open. */
class TBMenuWindow : public TBWindow, private TBWidgetSafePointer
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBMenuWindow", TBWindow);

	TBMenuWindow(Widget *target, TBID id);

	bool Show(TBSelectItemSource *source, int initial_value = -1, const TBPoint *pos_in_root = nullptr, TB_ALIGN align = TB_ALIGN_BOTTOM);
	TBRect GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align);

	TBSelectList *GetList() { return m_select_list; }

	virtual Widget *GetEventDestination() { return Get(); }

	virtual bool OnEvent(const WidgetEvent &ev);
private:
	virtual void OnWidgetFocusChanged(Widget *widget, bool focused);
	virtual bool OnWidgetInvokeEvent(const WidgetEvent &ev);
	virtual void OnWidgetDelete(Widget *widget);
	TBSelectList *m_select_list;
};

/** TBSelectList is a select widget with no popups. Instead it has two
	arrow buttons that cycle between the choices.
	By default it is a number widget.

	FIX: Should also be possible to set a list of strings that will be
		shown instead of numbers.
*/
class TBInlineSelect : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBInlineSelect", Widget);

	TBInlineSelect();
	~TBInlineSelect();

	/** Set along which axis the content should layouted. */
	void SetAxis(AXIS axis) { m_layout.SetAxis(axis); }
	AXIS GetAxis() const { return m_layout.GetAxis(); }

	void SetLimits(int min, int max);
	int GetMinValue() const { return m_min; }
	int GetMaxValue() const { return m_max; }

	virtual void SetValue(int value) { SetValueInternal(value, true); }
	virtual int GetValue() { return m_value; }

	virtual void OnSkinChanged();
	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	TBButton m_buttons[2];
	TBLayout m_layout;
	TBEditField m_editfield;
	int m_value;
	int m_min, m_max;
	void SetValueInternal(int value, bool update_text);
};

};

#endif // TB_SELECT_H
