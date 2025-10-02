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
			float minVol = 0;
			float maxVol = 1;
			float minRate = 0;
			float maxRate = 100;
			float pitchMin = 1;
			float pitchMax = 1;
			float pitchRateMin = 0;
			float pitchRateMax = 1;
		};
	public:
		HighrateSound();
		void AddMerged(String filename, Range range);
		void Play(float fadeIn = -1.f, float pitch = -1) override;
		void Loop(float fadeIn = -1.f, float pitch = -1) override;
		void Update(float delta) override;
		bool IsPlaying() override;
		virtual float GetRate() override;
		void SetRate(int rate) override;
		void SetPitch(float pitch) override;

	private:
		void PlayMerged(int i);
		void StopMerged(int i);
		float ComputeMergeVolume(int i);
		float ComputeMergePitch(int i);
	public:

		double(*easing)(double);
		Range oneShotRange;
		float fixedPitch = -1;
	private:
		Rate m_rate;

		std::vector<Range> m_mergeRange;
		std::vector<String> m_mergePath;
		std::vector<SoundHandle> m_mergeSound;
		std::vector<float> m_mergeVolume;
		std::vector<float> m_mergeVolumeTarget;
		std::vector<float> m_mergePitch;
		std::vector<float> m_mergePitchTarget;

		float m_mergePlaybackSpeed = 1;
		bool m_playingMerged = false;
		int m_playOverPeriod = 0;

		bool m_manualRate = false;


	};
}