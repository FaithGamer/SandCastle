#pragma once
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Sprite;
	/// @brief Payload sent by an Animator when entering a frame flagged with sendSignal.
	struct KeyframeSignal
	{
		String stateName;
		int frame = 0;
	};
	/// @brief One frame of an Animation: a sprite, how long it should display, and an optional signal flag.
	struct Keyframe
	{
		Keyframe();
		Sprite* sprite;
		float timeToNext;
		bool sendSignal = false;
	};

	/// @brief Sprite animation asset: an ordered list of Keyframes with a base frequency.
	/// Loaded by Assets from `.anim` files. Played back by the Animator/AnimationSystem.
	struct Animation : public Serializable
	{
		Animation();
		Animation(Serialized& config);
		/// @brief Total duration of the animation in seconds (sum of every keyframe's timeToNext).
		float GetTime();
		Serialized Serialize() override;
		void Deserialize(Serialized& config);
		std::vector<Signal<KeyframeSignal>> signalsTemplate;
		std::vector<Keyframe> frames;
		float frequency; // 1/fps
	};
}