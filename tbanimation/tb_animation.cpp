// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_animation.h"
#include "tb_window.h"
#include "tb_widgets.h"
#include "tb_widgets_common.h"
#include "tb_list.h"

namespace tinkerbell {

TBListOf<WidgetAnimationObject> widget_animations;

#define LERP(src, dst, progress) (src + (dst - src) * progress)

// == WidgetAnimationObject =============================================================

WidgetAnimationObject::WidgetAnimationObject(Widget *widget)
	: m_widget(widget)
{
	widget_animations.Add(this);
}

WidgetAnimationObject::~WidgetAnimationObject()
{
	// FIX: This is slow! don't use TBList! Use linked list!
	widget_animations.Remove(widget_animations.Find(this));
}

// == WidgetAnimationOpacity ============================================================

WidgetAnimationOpacity::WidgetAnimationOpacity(Widget *widget, float src_opacity, float dst_opacity)
	: WidgetAnimationObject(widget)
	, m_src_opacity(src_opacity)
	, m_dst_opacity(dst_opacity)
{
}

void WidgetAnimationOpacity::OnAnimationStart()
{
	m_widget->SetOpacity(m_src_opacity);
}

void WidgetAnimationOpacity::OnAnimationUpdate(float progress)
{
	m_widget->SetOpacity(LERP(m_src_opacity, m_dst_opacity, progress));
}

void WidgetAnimationOpacity::OnAnimationStop(bool aborted)
{
	m_widget->SetOpacity(m_dst_opacity);
	delete this;
}

// == WidgetAnimationRect ===============================================================

WidgetAnimationRect::WidgetAnimationRect(Widget *widget, const TBRect &src_rect, const TBRect &dst_rect)
	: WidgetAnimationObject(widget)
	, m_src_rect(src_rect)
	, m_dst_rect(dst_rect)
{
}

void WidgetAnimationRect::OnAnimationStart()
{
	m_widget->SetRect(m_src_rect);
}

void WidgetAnimationRect::OnAnimationUpdate(float progress)
{
	TBRect rect;
	rect.x = (int) LERP(m_src_rect.x, m_dst_rect.x, progress);
	rect.y = (int) LERP(m_src_rect.y, m_dst_rect.y, progress);
	rect.w = (int) LERP(m_src_rect.w, m_dst_rect.w, progress);
	rect.h = (int) LERP(m_src_rect.h, m_dst_rect.h, progress);
	m_widget->SetRect(rect);
}

void WidgetAnimationRect::OnAnimationStop(bool aborted)
{
	m_widget->SetRect(m_dst_rect);
	delete this;
}

// == WidgetsAnimationManager =====================================================

WidgetsAnimationManager widgets_animation_manager;

void WidgetsAnimationManager::Init()
{
	AnimationManager::Init();
	TBGlobalWidgetListener::AddListener(&widgets_animation_manager);
}

void WidgetsAnimationManager::Shutdown()
{
	TBGlobalWidgetListener::RemoveListener(&widgets_animation_manager);
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

void WidgetsAnimationManager::OnWidgetDelete(Widget *widget)
{
	// Kill and delete all animations running for the widget being deleted.
	for (int i = 0; i < widget_animations.GetNumItems(); i++)
	{
		WidgetAnimationObject *wao = widget_animations[i];
		if (wao->m_widget == widget)
		{
			// Abort the animation. This will both autoremove itself
			// and autodelete, so no need to do it here.
			AnimationManager::AbortAnimation(wao);
			i--;
		}
	}
}

void WidgetsAnimationManager::OnWidgetAdded(Widget *widget)
{
	if (TBWindow *window = TBSafeCast(TBWindow, widget))
	{
		// Fade in new windows
		if (AnimationObject *anim = new WidgetAnimationOpacity(window, 0.f, 1.f))
			AnimationManager::StartAnimation(anim);

		// Move in new windows that has a title bar
		if (!window->m_rect.IsEmpty() && (window->GetSettings() & WINDOW_SETTINGS_TITLEBAR))
		{
			TBRect src_rect = window->m_rect.Offset(0, -50);
			TBRect dst_rect = window->m_rect;
			if (AnimationObject *anim = new WidgetAnimationRect(window, src_rect, dst_rect))
				AnimationManager::StartAnimation(anim);
		}
	}
	if (TBDimmer *dimmer = TBSafeCast(TBDimmer, widget))
	{
		// Fade in dim layer
		if (AnimationObject *anim = new WidgetAnimationOpacity(dimmer, 0.f, 1.f))
			AnimationManager::StartAnimation(anim);
	}
}

void WidgetsAnimationManager::OnWidgetRemove(Widget *widget)
{
}

}; // namespace tinkerbell
