#pragma once

#include "SandCastle/Audio/Sound.h"
#include "SandCastle/Core/Rate.h"

namespace SandCastle
{
	/// @brief Sound that crossfades between several files based on the
	/// frequency at which Play() is called. Useful for "machine gun" or
	/// "engine" effects where a sustained loop should take over once single
	/// shots become too dense.
	class HighrateSound : public Sound
	{
	public:
		/// @brief One crossfade layer: which file to play, plus the pitch range and the rate range it covers.
		struct Range
		{
			float pitchMin = 1;
			float pitchMax = 1;
			float rateMin = 0;
			float rateMax = 1;
		};
	public:
		HighrateSound(unsigned int channel);
		/// @brief Register a layer that becomes audible when the play rate falls inside its range.
		void AddRate(String filename, Range range);
		void Play(float fadeIn = -1.f, float pitch = -1) override;
		void Loop(float fadeIn = -1.f, float pitch = -1) override;
		void Update() override;
		bool IsPlaying() override;
		/// @brief Current smoothed play rate driving the crossfade.
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