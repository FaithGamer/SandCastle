#pragma once
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Render/Animation.h"

namespace SandCastle
{
	struct AnimationState
	{
		template <typename ListenerType, typename SignalType>
		bool ListenFrame(int frame, void (ListenerType::* callback)(SignalType), ListenerType* const listener, SignalPriority priority = SignalPriority::medium)
		{
			if (signals.size() <= frame)
				return false;
			signals[frame].Listen(callback, listener, priority);
		}
		template <typename SignalType>
		bool Listen(int frame, void (*callback)(SignalType), SignalPriority priority = SignalPriority::medium)
		{
			if (signals.size() <= frame)
				return false;
			signals[frame].Listen(callback, priority);
		}
		Animation* animation = nullptr;
		bool looping = false;
		String transition = "";
		std::vector<Signal<KeyframeSignal>> signals;
	};
	struct Animator
	{
		void SetAnimation(String animation);
		void AddAnimation(String stateName, Animation* animation, String transition = "");
		void AddAnimation(String stateName, String animation, String transition = "");
		std::map<String, AnimationState> animations;
		AnimationState* currentState = nullptr;
		String currentStateName;
		float accumulator = 0;
		int currentKeyFrame = 0;
		float nextFrame = 0;
		float speed = 1;
		bool loop = true;
	};
}