// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_ANIMATION_H
#define TB_ANIMATION_H

#include "tb_widgets_listener.h"
#include "Animation.h"

namespace tinkerbell {

/** Don't use 0.0 for opacity animations since that may break focus code.
	At the moment a window should appear and start fading in from opacity 0,
	it would also attempt setting the focus to it, but if opacity is 0 it will
	think focus should not be set in that window and fail. */
#define ALMOST_ZERO_OPACITY 0.001f

/** Base class for widget animations. This animation object will
	be deleted automatically if the widget is deleted. */
class WidgetAnimationObject : public AnimationObject, public TBLinkOf<WidgetAnimationObject>
{
public:
	WidgetAnimationObject(TBWidget *widget);
	virtual ~WidgetAnimationObject();
public:
	TBWidget *m_widget;
};

class WidgetAnimationOpacity : public WidgetAnimationObject
{
public:
	WidgetAnimationOpacity(TBWidget *widget, float src_opacity, float dst_opacity, bool die);
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
	WidgetAnimationRect(TBWidget *widget, const TBRect &src_rect, const TBRect &dst_rect);
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
	/** Init the widgets animation manager and AnimationManager. */
	static void Init();

	/** Shutdown the widgets animation manager and AnimationManager. */
	static void Shutdown();

	/** Update all running animations. This will call AnimationManager::Update. */
	static void Update();

	/** Return true if there is running animations. */
	static bool HasAnimationsRunning();

	/** Abort all animations that are running for the given widget. */
	static void AbortAnimations(TBWidget *widget);

private:
	// == TBGlobalWidgetListener ==================
	virtual void OnWidgetDelete(TBWidget *widget);
	virtual bool OnWidgetDying(TBWidget *widget);
	virtual void OnWidgetAdded(TBWidget *widget);
	virtual void OnWidgetRemove(TBWidget *widget);
};

}; // namespace tinkerbell

#endif // TB_ANIMATION_H
