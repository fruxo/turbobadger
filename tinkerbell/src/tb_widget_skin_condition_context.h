// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_WIDGET_SKIN_CONDITION_CONTEXT_H
#define TB_WIDGET_SKIN_CONDITION_CONTEXT_H

#include "tb_widgets.h"
#include "tb_skin.h"

namespace tinkerbell {

/** Check if a condition is true for a widget when painting a skin. */

class TBWidgetSkinConditionContext : public TBSkinConditionContext
{
public:
	TBWidgetSkinConditionContext(TBWidget *widget) : m_widget(widget) {}
	virtual bool GetCondition(TBSkinCondition::TARGET target, const TBSkinCondition::CONDITION_INFO &info);
private:
	bool GetCondition(TBWidget *widget, const TBSkinCondition::CONDITION_INFO &info);
	TBWidget *m_widget;
};

}; // namespace tinkerbell

#endif // TB_WIDGET_SKIN_CONDITION_CONTEXT_H
