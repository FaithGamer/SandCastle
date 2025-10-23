#include "pch.h"
#include "SandCastle/Render/Animator.h"
#include "SandCastle/Core/Assets.h"
namespace SandCastle
{

	void Animator::AddAnimation(String stateName, Animation* animation, String transition)
	{
		AnimationState state;
		state.animation = animation;
		state.looping = true;
		state.transition = transition;
		state.signals = animation->signalsTemplate;
		animations.insert(std::make_pair(stateName, state));
	}

	void Animator::AddAnimation(String stateName, String animation, String transition)
	{
		AddAnimation(stateName, Assets::Get<Animation>(animation), transition);
	}

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