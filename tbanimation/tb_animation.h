// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_ANIMATION_H
#define TB_ANIMATION_H

#include "tb_widgets_listener.h"
#include "Animation.h"

namespace tinkerbell {

/** Base class for widget animations. This animation object will
	be deleted automatically if the widget is deleted. */
class WidgetAnimationObject : public AnimationObject, public TBLinkOf<WidgetAnimationObject>
{
public:
	WidgetAnimationObject(Widget *widget);
	virtual ~WidgetAnimationObject();
public:
	Widget *m_widget;
};

class WidgetAnimationOpacity : public WidgetAnimationObject
{
public:
	WidgetAnimationOpacity(Widget *widget, float src_opacity, float dst_opacity, bool die);
	virtual void OnAnimationStart();
	virtual void OnAnimationUpdate(float progress);
	virtual void OnAnimationStop(bool aborted);
private:
	float m_src_opacity;
	float m_dst_opacity;
	bool m_die;
};

class WidgetAnimationRect : public WidgetAnimationObject
{
public:
	WidgetAnimationRect(Widget *widget, const TBRect &src_rect, const TBRect &dst_rect);
	virtual void OnAnimationStart();
	virtual void OnAnimationUpdate(float progress);
	virtual void OnAnimationStop(bool aborted);
private:
	TBRect m_src_rect;
	TBRect m_dst_rect;
};

class WidgetsAnimationManager : public TBGlobalWidgetListener
{
public:
	static void Init();
	static void Shutdown();
	static void Update();
	static bool HasAnimationsRunning();

	// == TBGlobalWidgetListener ==================
	virtual void OnWidgetDelete(Widget *widget);
	virtual bool OnWidgetDying(Widget *widget);
	virtual void OnWidgetAdded(Widget *widget);
	virtual void OnWidgetRemove(Widget *widget);
};

}; // namespace tinkerbell

#endif // TB_ANIMATION_H
