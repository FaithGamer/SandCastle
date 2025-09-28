#pragma once
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Sprite;
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
		std::vector<Signal<KeyframeSignal>> signalsTemplate;
		std::vector<Keyframe> frames;
		float frequency; // 1/fps
	};
}