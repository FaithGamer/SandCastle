#include "pch.h"
#include "SandCastle/Render/Animator.h"

namespace SandCastle
{
	void Animator::SetAnimation(String animation)
	{
		auto find_it = animations.find(animation);
		if (find_it != animations.end())
		{
			currentStateName = animation;
			currentState = &find_it->second;
			accumulator = 0;
			currentKeyFrame = 0;
		}
		else
		{
			LOG_WARN("Cannot find animation with name: {0}", animation);
		}
	}
}