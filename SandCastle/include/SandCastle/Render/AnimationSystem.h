#pragma once
#include "SandCastle/ECS/System.h"
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Render/Sprite.h"

namespace SandCastle
{
	struct KeyframeSignal
	{
		String stateName;
		int frame = 0;
	};
	struct Keyframe
	{
		Keyframe();
		Sprite* sprite;
		float timeToNext;
		bool sendSignal = false;
	};
	struct Animation : public Serializable
	{
		Animation();
		Animation(Serialized& config);
		float GetTime();
		Serialized Serialize() override;
		void Deserialize(Serialized& config);
		std::vector<SignalSender<KeyframeSignal>> signalsTemplate;
		std::vector<Keyframe> frames;
		float frequency; // 1/fps
	};
	struct AnimationState
	{
		template <typename ListenerType, typename SignalType>
		bool ListenFrame(int frame, void (ListenerType::* callback)(SignalType), ListenerType* const listener, SignalPriority priority = SignalPriority::medium)
		{
			if (signals.size() <= frame)
				return false;
			signals[frame].AddListener(callback, listener, priority);
		}
		template <typename SignalType>
		bool AddListener(int frame, void (*callback)(SignalType), SignalPriority priority = SignalPriority::medium)
		{
			if (signals.size() <= frame)
				return false;
			signals[frame].AddListener(callback, priority);
		}
		Animation* animation = nullptr;
		bool looping = false;
		String transition = "";
		std::vector<SignalSender<KeyframeSignal>> signals;
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
	class AnimationSystem : public System
	{
	public:
		void OnUpdate(Time delta) override;
		int GetUsedMethod() override;

		Keyframe EvaluateTransform(Animator& animator);

	};
}