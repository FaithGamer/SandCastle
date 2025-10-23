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
		/// @brief Add an animation to be played later at any time.
		/// @param stateName The state name to reference this animation
		/// @param animation the animation pointer
		/// @param transition The state to transition to when the animation end, leave empty to loop.
		void AddAnimation(String stateName, Animation* animation, String transition = "");
		/// @brief Add an animation to be played later at any time.
		/// @param stateName The state name to reference this animation
		/// @param animation The asset file name
		/// @param transition The state to transition to when the animation end, leave empty to loop.
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