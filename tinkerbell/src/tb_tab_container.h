// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_TAB_CONTAINER_H
#define TB_TAB_CONTAINER_H

#include "tb_widgets_common.h"

namespace tinkerbell {

/** TBTabLayout is a TBLayout used in TBTabContainer to apply
	some default properties on any TBButton added to it. */
class TBTabLayout : public TBLayout
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBTabLayout", TBLayout);

	/** Get the index (in the list of children) of the given child, or -1 if not found. */
	int GetIndexFromChild(Widget *child);

	virtual void OnChildAdded(Widget *child);
};

/** TBTabContainer - A container with tabs for multiple pages. */

class TBTabContainer : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBTabContainer", Widget);

	TBTabContainer();
	~TBTabContainer();

	/** Set along which axis the content should layouted.
		Use SetAlignment instead for more choice! */
	void SetAxis(AXIS axis);
	AXIS GetAxis() const { return m_root_layout.GetAxis(); }

	/** Set alignment of the tabs. */
	void SetAlignment(TB_ALIGN align);

	/** Set which page should be selected and visible. */
	void SetCurrentPage(int index);

	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnProcess();

	virtual Widget *GetContentRoot() { return &m_content_root; }
	TBLayout *GetTabLayout() { return &m_tab_layout; }
protected:
	TBLayout m_root_layout;
	TBTabLayout m_tab_layout;
	Widget m_content_root;
	bool m_need_page_update;
	int m_current_page;
};

};

#endif // TB_TAB_CONTAINER_H
