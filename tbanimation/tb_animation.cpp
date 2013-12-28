// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_animation.h"
#include "tb_window.h"
#include "tb_widgets.h"
#include "tb_widgets_common.h"
#include "tb_message_window.h"
#include "tb_list.h"

namespace tinkerbell {

TBLinkListOf<WidgetAnimationObject> widget_animations;

#define LERP(src, dst, progress) (src + (dst - src) * progress)

// == WidgetAnimationObject =============================================================

WidgetAnimationObject::WidgetAnimationObject(TBWidget *widget)
	: m_widget(widget)
{
	widget_animations.AddLast(this);
}

WidgetAnimationObject::~WidgetAnimationObject()
{
	widget_animations.Remove(this);
}

// == WidgetAnimationOpacity ============================================================

WidgetAnimationOpacity::WidgetAnimationOpacity(TBWidget *widget, float src_opacity, float dst_opacity, bool die)
	: WidgetAnimationObject(widget)
	, m_src_opacity(src_opacity)
	, m_dst_opacity(dst_opacity)
	, m_die(die)
{
}

void WidgetAnimationOpacity::OnAnimationStart()
{
	// Make sure we don't stay idle if nothing is scheduled (hack).
	if (!WidgetsAnimationManager::HasAnimationsRunning())
		m_widget->Invalidate();

	m_widget->SetOpacity(m_src_opacity);
}

void WidgetAnimationOpacity::OnAnimationUpdate(float progress)
{
	m_widget->SetOpacity(LERP(m_src_opacity, m_dst_opacity, progress));
}

void WidgetAnimationOpacity::OnAnimationStop(bool aborted)
{
	// If we're aborted, it may be because the widget is being deleted
	if (m_die && !aborted)
	{
		TBWidgetSafePointer the_widget(m_widget);
		if (m_widget->GetParent())
			m_widget->GetParent()->RemoveChild(m_widget);
		if (the_widget.Get())
			delete the_widget.Get();
	}
	else
		m_widget->SetOpacity(m_dst_opacity);
	delete this;
}

// == WidgetAnimationRect ===============================================================

WidgetAnimationRect::WidgetAnimationRect(TBWidget *widget, const TBRect &src_rect, const TBRect &dst_rect)
	: WidgetAnimationObject(widget)
	, m_src_rect(src_rect)
	, m_dst_rect(dst_rect)
	, m_mode(MODE_SRC_TO_DST)
{
}

WidgetAnimationRect::WidgetAnimationRect(TBWidget *widget, const TBRect &delta_rect, MODE mode)
	: WidgetAnimationObject(widget)
	, m_delta_rect(delta_rect)
	, m_mode(mode)
{
	assert(mode == MODE_DELTA_IN || mode == MODE_DELTA_OUT);
}

void WidgetAnimationRect::OnAnimationStart()
{
	// Make sure we don't stay idle if nothing is scheduled.
	if (!WidgetsAnimationManager::HasAnimationsRunning())
		m_widget->Invalidate();

	if (m_mode == MODE_SRC_TO_DST)
		m_widget->SetRect(m_src_rect);
}

void WidgetAnimationRect::OnAnimationUpdate(float progress)
{
	if (m_mode == MODE_DELTA_IN || m_mode == MODE_DELTA_OUT)
	{
		m_dst_rect = m_src_rect = m_widget->GetRect();
		if (m_dst_rect.Equals(TBRect()))
		{
			// Widget hasn't been laid out yet,
			// the animation was started too soon.
			AnimationManager::AbortAnimation(this);
			return;
		}
		if (m_mode == MODE_DELTA_IN)
		{
			m_dst_rect.x += m_delta_rect.x;
			m_dst_rect.y += m_delta_rect.y;
			m_dst_rect.w += m_delta_rect.w;
			m_dst_rect.h += m_delta_rect.h;
		}
		else
		{
			m_src_rect.x += m_delta_rect.x;
			m_src_rect.y += m_delta_rect.y;
			m_src_rect.w += m_delta_rect.w;
			m_src_rect.h += m_delta_rect.h;
		}
		m_mode = MODE_SRC_TO_DST;
	}
	TBRect rect;
	rect.x = (int) LERP(m_src_rect.x, m_dst_rect.x, progress);
	rect.y = (int) LERP(m_src_rect.y, m_dst_rect.y, progress);
	rect.w = (int) LERP(m_src_rect.w, m_dst_rect.w, progress);
	rect.h = (int) LERP(m_src_rect.h, m_dst_rect.h, progress);
	m_widget->SetRect(rect);
}

void WidgetAnimationRect::OnAnimationStop(bool aborted)
{
	if (m_mode == MODE_SRC_TO_DST) // m_dst_rect may still be unset if aborted.
		m_widget->SetRect(m_dst_rect);
	delete this;
}

// == WidgetsAnimationManager =====================================================

WidgetsAnimationManager widgets_animation_manager;

void WidgetsAnimationManager::Init()
{
	AnimationManager::Init();
	TBWidgetListener::AddGlobalListener(&widgets_animation_manager);
}

void WidgetsAnimationManager::Shutdown()
{
	TBWidgetListener::RemoveGlobalListener(&widgets_animation_manager);
	AnimationManager::Shutdown();
}

void WidgetsAnimationManager::Update()
{
	AnimationManager::Update();
}

bool WidgetsAnimationManager::HasAnimationsRunning()
{
	return AnimationManager::HasAnimationsRunning();
}

void WidgetsAnimationManager::AbortAnimations(TBWidget *widget)
{
	TBLinkListOf<WidgetAnimationObject>::Iterator iter = widget_animations.IterateForward();
	while (WidgetAnimationObject *wao = iter.GetAndStep())
	{
		if (wao->m_widget == widget)
		{
			// Abort the animation. This will both autoremove itself
			// and autodelete, so no need to do it here.
			AnimationManager::AbortAnimation(wao);
		}
	}
}

void WidgetsAnimationManager::OnWidgetDelete(TBWidget *widget)
{
	// Kill and delete all animations running for the widget being deleted.
	AbortAnimations(widget);
}

bool WidgetsAnimationManager::OnWidgetDying(TBWidget *widget)
{
	bool handled = false;
	if (TBWindow *window = TBSafeCast<TBWindow>(widget))
	{
		// Fade out dying windows
		if (AnimationObject *anim = new WidgetAnimationOpacity(window, 1.f, ALMOST_ZERO_OPACITY, true))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_BEZIER);
		handled = true;
	}
	if (TBMessageWindow *window = TBSafeCast<TBMessageWindow>(widget))
	{
		// Move out dying message windows
		if (AnimationObject *anim = new WidgetAnimationRect(window, TBRect(0, 50, 0, 0), WidgetAnimationRect::MODE_DELTA_IN))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_SPEED_UP);
		handled = true;
	}
	if (TBDimmer *dimmer = TBSafeCast<TBDimmer>(widget))
	{
		// Fade out dying dim layers
		if (AnimationObject *anim = new WidgetAnimationOpacity(dimmer, 1.f, ALMOST_ZERO_OPACITY, true))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_BEZIER);
		handled = true;
	}
	return handled;
}

void WidgetsAnimationManager::OnWidgetAdded(TBWidget *widget)
{
	if (TBWindow *window = TBSafeCast<TBWindow>(widget))
	{
		// Fade in new windows
		if (AnimationObject *anim = new WidgetAnimationOpacity(window, ALMOST_ZERO_OPACITY, 1.f, false))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_BEZIER);
	}
	if (TBMessageWindow *window = TBSafeCast<TBMessageWindow>(widget))
	{
		// Move in new message windows
		if (AnimationObject *anim = new WidgetAnimationRect(window, TBRect(0, -50, 0, 0), WidgetAnimationRect::MODE_DELTA_OUT))
			AnimationManager::StartAnimation(anim);
	}
	if (TBDimmer *dimmer = TBSafeCast<TBDimmer>(widget))
	{
		// Fade in dim layer
		if (AnimationObject *anim = new WidgetAnimationOpacity(dimmer, ALMOST_ZERO_OPACITY, 1.f, false))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_BEZIER);
	}
}

void WidgetsAnimationManager::OnWidgetRemove(TBWidget *widget)
{
}

}; // namespace tinkerbell
