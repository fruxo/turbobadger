// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_SELECT_H
#define TB_SELECT_H

#include "tb_window.h"
#include "tb_scroll_container.h"
#include "tb_select_item.h"

namespace tinkerbell {

class TBMenuWindow;

// FIX: A maximum items showed option, and a automatic "Show more" item at the bottom.
// FIX: Item-quick-find by pressing one or two keys in the list!
//      -Implement with generic search-and-find in widgets. Should be available in all UI!
// FIX: gör även en default string source med checkboxar (multiselectable)??
// Indentera alla items, och ge menyn ett bg skin/underlay ifall någon har ikon!
// Treeview item, item column, och indent för items utan ikon.. använda samma indent system?

/** TBSelectList shows a scrollable list of items provided by a TBSelectItemSource. */

class TBSelectList : public TBWidget, public TBSelectItemViewer
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(TBSelectList, TBWidget);

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

	/** Get the ID of the selected item, or 0 if there is no item selected. */
	TBID GetSelectedItemID();

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
	TBOBJECT_SUBCLASS(TBSelectDropdown, TBButton);

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

	/** Get the ID of the selected item, or 0 if there is no item selected. */
	TBID GetSelectedItemID();

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

}; // namespace tinkerbell

#endif // TB_SELECT_H
