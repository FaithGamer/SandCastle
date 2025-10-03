#pragma once

#include "SandCastle/Audio/Sound.h"
#include "SandCastle/Core/Rate.h"

namespace SandCastle
{
	class HighrateSound : public Sound
	{
	public:
		struct Range
		{
			float pitchMin = 1;
			float pitchMax = 1;
			float rateMin = 0;
			float rateMax = 1;
		};
	public:
		HighrateSound(unsigned int channel);
		void AddRate(String filename, Range range);
		void Play(float fadeIn = -1.f, float pitch = -1) override;
		void Loop(float fadeIn = -1.f, float pitch = -1) override;
		void Update() override;
		bool IsPlaying() override;
		double GetRate();
		void SetPitch(float pitch) override;

	private:
		void PlayMerged(int i);
		void StopMerged(int i);
		float ComputeMergeVolume(int i);
		float ComputeMergePitch(int i);
	public:

		double(*easing)(double);
	private:
		Rate m_rate;

		std::vector<Range> m_mergeRange;
		std::vector<String> m_mergePath;
		std::vector<SoundHandle> m_mergeSounds;
		std::vector<float> m_mergeVolume;
		std::vector<float> m_mergeVolumeTarget;
		std::vector<float> m_mergePitch;
		std::vector<float> m_mergePitchTarget;
		float m_fixedPitch = -1;
		bool m_playingMerged = false;
		bool m_manualRate = false;
	};
}