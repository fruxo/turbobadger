// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_ANIMATION_UTILS_H
#define TB_ANIMATION_UTILS_H

#include "tb_animation.h"

namespace tinkerbell {

// TBAnimatedFloat - A animated float value

class TBAnimatedFloat : public AnimationObject
{
public:
	float src_val;
	float dst_val;
	float current_progress;
public:
	TBAnimatedFloat(	float initial_value,
					ANIMATION_CURVE animation_curve = ANIMATION_DEFAULT_CURVE,
					double animation_duration = ANIMATION_DEFAULT_DURATION)
		: src_val(initial_value), dst_val(initial_value), current_progress(0)
	{
		AnimationObject::animation_curve = animation_curve;
		AnimationObject::animation_duration = animation_duration;
	}

	float GetValue() { return src_val + (dst_val - src_val) * current_progress; }
	void SetValueAnimated(float value) { src_val = GetValue(); dst_val = value; AnimationManager::StartAnimation(this, animation_curve, animation_duration); }
	void SetValueImmediately(float value) { AnimationManager::AbortAnimation(this); src_val = dst_val = value; OnAnimationUpdate(1.0f); }

	virtual void OnAnimationStart() { current_progress = 0; }
	virtual void OnAnimationUpdate(float progress) { current_progress = progress; }
	virtual void OnAnimationStop(bool aborted) {}
};

// TBFloatAnimator - Animates a external float value, which address is given in the constructor.

class TBFloatAnimator : public TBAnimatedFloat
{
public:
	float *target_value;
public:
	TBFloatAnimator(	float *target_value,
					ANIMATION_CURVE animation_curve = ANIMATION_DEFAULT_CURVE,
					double animation_duration = ANIMATION_DEFAULT_DURATION)
		: TBAnimatedFloat(*target_value), target_value(target_value) {}

	virtual void OnAnimationStart() { TBAnimatedFloat::OnAnimationStart(); *target_value = GetValue(); }
	virtual void OnAnimationUpdate(float progress) { TBAnimatedFloat::OnAnimationUpdate(progress); *target_value = GetValue(); }
};

}; // namespace tinkerbell

#endif // TB_ANIMATION_UTILS_H
