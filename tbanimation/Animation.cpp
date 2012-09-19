// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "Animation.h"
#include "tb_system.h"

namespace tinkerbell {

// == Helpers =======================================================

#define SMOOTHSTEP(x) ((x) * (x) * (3.0f - 2.0f * (x)))

static float sc(float x)
{
	float s = x < 0 ? -1.f : 1.f;
	x = ABS(x);
	if (x >= 1)
		return s;
	return s * (x < 0 ? x / 0.5f : (x / (1 + x * x)) / 0.5f);
}

static float SmoothCurve(float x, float a)
{
	float r = a * x / (2 * a * x - a - x + 1);
	r = (r - 0.5f) * 2;
	return sc(r) * 0.5f + 0.5f;
}

// == AnimationObject ===============================================

// == AnimationManager ==============================================

TBLinkListOf<AnimationObject> AnimationManager::animating_objects;

void AnimationManager::Init()
{
}

void AnimationManager::Shutdown()
{
	while (AnimationObject *obj = animating_objects.GetFirst())
		AbortAnimation(obj);
}

void AnimationManager::Update()
{
	double time_now = TBSystem::GetTimeMS();

	TBLinkListOf<AnimationObject>::Iterator iter = animating_objects.IterateForward();
	while (AnimationObject *obj = iter.GetAndStep())
	{
		// Adjust the start time if it's the first update time for this object.
		if (obj->adjust_start_time)
		{
			obj->animation_start_time = time_now;
			obj->adjust_start_time = false;
		}

		// Calculate current progress
		float progress = (float)(time_now - obj->animation_start_time) / (float)obj->animation_duration;
		progress = MIN(progress, 1.0f);

		// Apply animation curve
		float tmp;
		switch (obj->animation_curve)
		{
		case ANIMATION_CURVE_SLOW_DOWN:
			tmp = 1 - progress;
			progress = 1 - tmp * tmp * tmp;
			break;
		case ANIMATION_CURVE_SPEED_UP:
			progress = progress * progress * progress;
			break;
		case ANIMATION_CURVE_BEZIER:
			progress = SMOOTHSTEP(progress);
			break;
		case ANIMATION_CURVE_SMOOTH:
			progress = SmoothCurve(progress, 0.6f);
			break;
		default: // linear (progress is already linear)
			break;
		};

		// Update animation
		obj->OnAnimationUpdate(progress);

		// Remove completed animations
		if (progress == 1.0f)
		{
			animating_objects.Remove(obj);
			obj->OnAnimationStop(false);
		}
	}
}

bool AnimationManager::HasAnimationsRunning()
{
	return animating_objects.HasLinks();
}

void AnimationManager::StartAnimation(AnimationObject *obj, ANIMATION_CURVE animation_curve, double animation_duration, ANIMATION_TIME animation_time)
{
	if (obj->IsAnimating())
		AbortAnimation(obj);
	obj->adjust_start_time = (animation_time == ANIMATION_TIME_FIRST_UPDATE ? true : false);
	obj->animation_start_time = TBSystem::GetTimeMS();
	obj->animation_duration = MAX(animation_duration, 1);
	obj->animation_curve = animation_curve;
	obj->OnAnimationStart();
	animating_objects.AddLast(obj);
}

void AnimationManager::AbortAnimation(AnimationObject *obj)
{
	if (obj->IsAnimating())
	{
		animating_objects.Remove(obj);
		obj->OnAnimationStop(true);
	}
}

}; // namespace tinkerbell
