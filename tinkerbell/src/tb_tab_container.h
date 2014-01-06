// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                     See tb_core.h for more information.                    ==
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
	TBOBJECT_SUBCLASS(TBTabLayout, TBLayout);

	/** Get the index (in the list of children) of the given child, or -1 if not found. */
	int GetIndexFromChild(TBWidget *child) const;

	virtual void OnChildAdded(TBWidget *child);
};

/** TBTabContainer - A container with tabs for multiple pages. */

class TBTabContainer : public TBWidget
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(TBTabContainer, TBWidget);

	TBTabContainer();
	~TBTabContainer();

	/** Set along which axis the content should layouted.
		Use SetAlignment instead for more choice! Also, calling
		SetAxis directly does not update the current alignment. */
	virtual void SetAxis(AXIS axis);
	virtual AXIS GetAxis() const { return m_root_layout.GetAxis(); }

	/** Set alignment of the tabs. */
	void SetAlignment(TB_ALIGN align);
	TB_ALIGN GetAlignment() const { return m_align; }

	/** Set which page should be selected and visible. */
	void SetCurrentPage(int index);

	virtual bool OnEvent(const TBWidgetEvent &ev);
	virtual void OnProcess();

	virtual TBWidget *GetContentRoot() { return &m_content_root; }
	TBLayout *GetTabLayout() { return &m_tab_layout; }
protected:
	TBLayout m_root_layout;
	TBTabLayout m_tab_layout;
	TBWidget m_content_root;
	bool m_need_page_update;
	int m_current_page;
	TB_ALIGN m_align;
};

};

#endif // TB_TAB_CONTAINER_H
