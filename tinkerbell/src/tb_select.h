// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
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

class TBMenuWindow;
class TBSelectItemSource;

// FIX: A maximum items showed option, and a automatic "Show more" item at the bottom.
// FIX: Item-quick-find by pressing one or two keys in the list!
//      -Implement with generic search-and-find in widgets. Should be available in all UI!
// FIX: gör även en default string source med checkboxar (multiselectable)??
// Indentera alla items, och ge menyn ett bg skin/underlay ifall någon har ikon!
// Treeview item, item column, och indent för items utan ikon.. använda samma indent system?

enum TB_SORT {
	TB_SORT_NONE,		///< No sorting. Items appear in list order.
	TB_SORT_ASCENDING,	///< Ascending sort.
	TB_SORT_DESCENDING	///< Descending sort.
};

/** TBSelectItemViewer is the viewer for items provided by TBSelectItemSource.
	There can be multiple viewers for each source. The viewer will recieve
	callbacks when the source is changed, so it can update itself.
*/
class TBSelectItemViewer : public TBLinkOf<TBSelectItemViewer>
{
public:
	TBSelectItemViewer() : m_source(nullptr) {}
	virtual ~TBSelectItemViewer() {}

	/** Set the source which should provide the items for this viewer.
		This source needs to live longer than this viewer.
		Set nullptr to unset currently set source. */
	void SetSource(TBSelectItemSource *source);
	TBSelectItemSource *GetSource() const { return m_source; }

	/** Called when the source has changed or been unset by calling SetSource. */
	virtual void OnSourceChanged() = 0;

	/** Called when the item at the given index has changed in a way that should
		update the viewer. */
	virtual void OnItemChanged(int index) = 0;

	/** Called when the item at the given index has been added. */
	virtual void OnItemAdded(int index) = 0;

	/** Called when the item at the given index has been removed. */
	virtual void OnItemRemoved(int index) = 0;

	/** Called when all items have been removed. */
	virtual void OnAllItemsRemoved() = 0;
protected:
	TBSelectItemSource *m_source;
};

/** TBSelectItemSource is a item provider interface for list widgets (TBSelectList and
	TBSelectDropdown).

	Instead of feeding all list widgets with all items all the time, the list widgets
	will ask TBSelectItemSource when it needs it. The list widgets may also apply
	filtering so only a subset of all the items are shown.

	CreateItemWidget can be overridden to create any set of widget content for each item.

	This class has no storage of items. If you want an array storage of items,
	use the subclass TBSelectItemSourceList. If you implement your own storage,
	remember to call InvokeItem[Added/*] to notify viewers that they need to update.
*/

class TBSelectItemSource
{
public:
	TBSelectItemSource() : m_sort(TB_SORT_NONE) {}
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
	virtual TBSelectItemSource *GetItemSubSource(int index) { return nullptr; }

	/** Get the skin image to be painted before the text for this item. */
	virtual TBID GetItemImage(int index) { return TBID(); }

	/** Create the item representation widget(s). By default, it will create
		a TBTextField for string-only items, and other types for items that
		also has image or submenu. */
	virtual TBWidget *CreateItemWidget(int index, TBSelectItemViewer *viewer);

	/** Get the number of items */
	virtual int GetNumItems() = 0;

	/** Set sort type. Default is TB_SORT_NONE. */
	void SetSort(TB_SORT sort) { m_sort = sort; }
	TB_SORT GetSort() const { return m_sort; }

	/** Invoke OnItemChanged on all open viewers for this source. */
	void InvokeItemChanged(int index, TBSelectItemViewer *exclude_viewer = nullptr);
	void InvokeItemAdded(int index);
	void InvokeItemRemoved(int index);
	void InvokeAllItemsRemoved();
private:
	friend class TBSelectItemViewer;
	TBLinkListOf<TBSelectItemViewer> m_viewers;
	TB_SORT m_sort;
};

/** TBSelectItemSourceList is a item provider for list widgets (TBSelectList and
	TBSelectDropdown). It stores items of the type specified by the template in an array. */

template<class T>
class TBSelectItemSourceList : public TBSelectItemSource
{
public:
	TBSelectItemSourceList() {}
	virtual ~TBSelectItemSourceList()					{ DeleteAllItems(); }
	virtual const char *GetItemString(int index)		{ return GetItem(index)->str; }
	virtual TBSelectItemSource *GetItemSubSource(int index){ return GetItem(index)->sub_source; }
	virtual TBID GetItemImage(int index)				{ return GetItem(index)->skin_image; }
	virtual int GetNumItems()							{ return m_items.GetNumItems(); }
	virtual TBWidget *CreateItemWidget(int index, TBSelectItemViewer *viewer)
	{
		if (TBWidget *widget = TBSelectItemSource::CreateItemWidget(index, viewer))
		{
			T *item = m_items[index];
			widget->GetID().Set(item->id);
			return widget;
		}
		return nullptr;
	}

	/** Add a new item at the given index. */
	bool AddItem(T *item, int index)
	{
		if (m_items.Add(item, index))
		{
			InvokeItemAdded(index);
			return true;
		}
		return false;
	}

	/** Add a new item last. */
	bool AddItem(T *item)				{ return AddItem(item, m_items.GetNumItems()); }

	/** Get the item at the given index. */
	T *GetItem(int index)				{ return m_items[index]; }

	/** Delete the item at the given index. */
	void DeleteItem(int index)
	{
		if (!m_items.GetNumItems())
			return;
		m_items.Delete(index);
		InvokeItemRemoved(index);
	}

	/** Delete all items. */
	void DeleteAllItems()
	{
		if (!m_items.GetNumItems())
			return;
		m_items.DeleteAll();
		InvokeAllItemsRemoved();
	}
private:
	TBListOf<T> m_items;
};

/** TBGenericStringItem item for TBGenericStringItemSource.
	It has a string and may have a skin image and sub item source. */
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

/** TBGenericStringItemSource is a item source list providing items of type TBGenericStringItem. */

class TBGenericStringItemSource : public TBSelectItemSourceList<TBGenericStringItem> { };

/** TBSelectList shows a scrollable list of items provided by a TBSelectItemSource. */

class TBSelectList : public TBWidget, public TBSelectItemViewer
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSelectList", TBWidget);

	TBSelectList();
	~TBSelectList();

	/** Get the default item source for this widget. This source can be used to add
		items of type TBGenericStringItem to this widget.
		It is the item source that is fed from resource files.

		If you need to add other types of items, or if you want to share item sources
		between several TBSelectDropDown/TBSelectList widgets, use SetSource using a
		external item source. */
	TBGenericStringItemSource *GetDefaultSource() { return &m_default_source; }

	/** Set filter string so only matching items will be showed.
		Set nullptr or empty string to remove filter and show all items. */
	void SetFilter(const char *filter);
	const char *GetFilter() const { return m_filter; }

	/** Set the language string id for the header. The header is shown
		at the top of the list when only a subset of all items are shown. */
	void SetHeaderString(const TBID& id);

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

	/** Change the value to a non disabled item that is visible with the current
		filter. Returns true if it successfully found another item.
		Valid keys:
			TB_KEY_UP - Previous item.
			TB_KEY_DOWN - Next item.
			TB_KEY_HOME - First item.
			TB_KEY_END - Last item. */
	bool ChangeValue(SPECIAL_KEY key);

	/** Set the selected state of the item at the given index. If you want
		to unselect the previously selected item, use SetValue. */
	void SelectItem(int index, bool selected);
	TBWidget *GetItemWidget(int index);

	/** Scroll to the current selected item. The scroll may be delayed until
		the items has been layouted if the layout is currently invalid. */
	void ScrollToSelectedItem();

	/** Return the scrollcontainer used in this list. */
	TBScrollContainer *GetScrollContainer() { return &m_container; }

	virtual void OnSkinChanged();
	virtual void OnProcess();
	virtual void OnProcessAfterChildren();
	virtual bool OnEvent(const TBWidgetEvent &ev);

	// == TBSelectItemViewer ==================================================
	virtual void OnSourceChanged();
	virtual void OnItemChanged(int index);
	virtual void OnItemAdded(int index);
	virtual void OnItemRemoved(int index);
	virtual void OnAllItemsRemoved();
protected:
	TBScrollContainer m_container;
	TBLayout m_layout;
	TBGenericStringItemSource m_default_source;
	int m_value;
	TBStr m_filter;
	bool m_list_is_invalid;
	bool m_scroll_to_current;
	TBID m_header_lng_string_id;
private:
	TBWidget *CreateAndAddItemAfter(int index, TBWidget *reference);
};

/** TBSelectDropdown shows a button that opens a popup with a TBSelectList with items
	provided by a TBSelectItemSource. */

class TBSelectDropdown : public TBButton, public TBSelectItemViewer
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSelectDropdown", TBButton);

	TBSelectDropdown();
	~TBSelectDropdown();

	/** Get the default item source for this widget. This source can be used to add
		items of type TBGenericStringItem to this widget.
		It is the item source that is fed from resource files.

		If you need to add other types of items, or if you want to share item sources
		between several TBSelectDropDown/TBSelectList widgets, use SetSource using a
		external item source. */
	TBGenericStringItemSource *GetDefaultSource() { return &m_default_source; }

	/** Set the selected item. */
	virtual void SetValue(int value);
	virtual int GetValue() { return m_value; }

	/** Open the window if the model has items. */
	void OpenWindow();

	/** Return the menu window if it's open, or nullptr. */
	TBMenuWindow *GetMenuIfOpen() const;

	virtual bool OnEvent(const TBWidgetEvent &ev);

	// == TBSelectItemViewer ==================================================
	virtual void OnSourceChanged();
	virtual void OnItemChanged(int index);
	virtual void OnItemAdded(int index) {}
	virtual void OnItemRemoved(int index) {}
	virtual void OnAllItemsRemoved() {}
protected:
	TBGenericStringItemSource m_default_source;
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

	TBMenuWindow(TBWidget *target, TBID id);

	bool Show(TBSelectItemSource *source, int initial_value = -1, const TBPoint *pos_in_root = nullptr, TB_ALIGN align = TB_ALIGN_BOTTOM);
	TBRect GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align);

	TBSelectList *GetList() { return m_select_list; }

	virtual TBWidget *GetEventDestination() { return Get(); }

	virtual bool OnEvent(const TBWidgetEvent &ev);
private:
	virtual void OnWidgetFocusChanged(TBWidget *widget, bool focused);
	virtual bool OnWidgetInvokeEvent(const TBWidgetEvent &ev);
	virtual void OnWidgetDelete(TBWidget *widget);
	TBSelectList *m_select_list;
};

/** TBSelectList is a select widget with no popups. Instead it has two
	arrow buttons that cycle between the choices.
	By default it is a number widget.

	FIX: Should also be possible to set a list of strings that will be
		shown instead of numbers.
*/
class TBInlineSelect : public TBWidget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBInlineSelect", TBWidget);

	TBInlineSelect();
	~TBInlineSelect();

	/** Set along which axis the content should layouted. */
	virtual void SetAxis(AXIS axis) { m_layout.SetAxis(axis); }
	virtual AXIS GetAxis() const { return m_layout.GetAxis(); }

	void SetLimits(int min, int max);
	int GetMinValue() const { return m_min; }
	int GetMaxValue() const { return m_max; }

	virtual void SetValue(int value) { SetValueInternal(value, true); }
	virtual int GetValue() { return m_value; }

	virtual void OnSkinChanged();
	virtual bool OnEvent(const TBWidgetEvent &ev);
protected:
	TBButton m_buttons[2];
	TBLayout m_layout;
	TBEditField m_editfield;
	int m_value;
	int m_min, m_max;
	void SetValueInternal(int value, bool update_text);
};

}; // namespace tinkerbell

#endif // TB_SELECT_H
