#include "pch.h"
#include "SandCastle/Render/Animation.h"
#include "SandCastle/Core/Assets.h"

namespace SandCastle
{
	//---Keyframe---

	Keyframe::Keyframe() : timeToNext(0)
	{
	}

	Animation::Animation() : frequency(0.1666f)
	{
	}

	Animation::Animation(Serialized& config)
	{
		Deserialize(config);
	}

	float Animation::GetTime()
	{
		return frequency * frames.size();
	}

	//---Animation---

	Serialized Animation::Serialize()
	{
		LOG_WARN("Cannot serialize animation.");
		return Serialized();
	}

	void Animation::Deserialize(Serialized& config)
	{
		frequency = 1.f / config["frame_per_second"].get<float>();

		int currentFrame = 0;
		bool signal = false;
		for (auto& frame : config["frames"])
		{
			Sprite* sprite = nullptr;
			//Set sprite if any
			auto sprite_it = frame.find("sprite");
			if (sprite_it != frame.end())
			{
				sprite = Assets::Get<Sprite>(sprite_it->get<String>());
			}

			float timeToNext = 1.f * frequency;

			//Set custom time if any
			auto time_it = frame.find("time");
			if (time_it != frame.end())
			{
				timeToNext = time_it->get<float>() * frequency;
			}

			Keyframe keyframe;
			keyframe.sprite = sprite;
			keyframe.timeToNext = timeToNext;
			keyframe.sendSignal = frame.find("signal") != frame.end() ? frame.at("signal").get<bool>() : false;
			if (keyframe.sendSignal)
				signal = true;
			frames.push_back(keyframe);
		}
		if (signal)
		{
			signalsTemplate.resize(frames.size());
		}
	}
}