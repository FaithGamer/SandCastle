#include "pch.h"
#include "SandCastle/Audio/HighrateSound.h"
#include "SandCastle/Audio/Audio.h"
#include "SandCastle/Core/Random.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Core/Easing.h"
#include "SandCastle/Core/Time.h"

namespace SandCastle
{
	HighrateSound::HighrateSound(unsigned int channel) : Sound(channel)
	{
		easing = &Easing::QuadOut;
	}
	void HighrateSound::AddMerged(String path, Range range)
	{
		m_mergeRange.emplace_back(range);
		m_mergePath.emplace_back(path);
		m_mergeSounds.emplace_back(SoundHandle());
		m_mergeVolume.emplace_back(0);
		m_mergeVolumeTarget.emplace_back(0);
		m_mergePitch.emplace_back(1);
		m_mergePitchTarget.emplace_back(1);
	}
	void HighrateSound::Play(float fadeIn, float pitch)
	{
		m_rate.Tick(1.);
		for (int i = 0; i < m_mergeVolume.size(); i++)
		{
			if (m_mergeVolume[i] > 0.7f)
				return;
		}
		Sound::PlayPrivate(false, fadeIn, pitch);
	}

	void HighrateSound::Loop(float fadeIn, float pitch)
	{
		LOG_ERROR("Cannot loop a highrate sound.");
	}

	void HighrateSound::Update()
	{
		Sound::Update();

		float delta = Time::UnscaledDelta();
		m_rate.Update(delta);
		auto rate = m_rate.rate;

		for (int i = 0; i < m_mergeSounds.size(); i++)
		{
			if (!m_mergeSounds[i].IsPlaying() && rate >= m_mergeRange[i].volRateMin)
			{
				PlayMerged(i);
			}

			if (m_mergeSounds[i].IsPlaying())
			{
				m_mergeVolumeTarget[i] = ComputeMergeVolume(i);
				m_mergeVolume[i] = Math::Lerp(m_mergeVolume[i], m_mergeVolumeTarget[i], delta * 10);
				m_mergeSounds[i].SetVolume(m_mergeVolume[i]);

				if (m_fixedPitch < 0)
				{
					m_mergePitchTarget[i] = ComputeMergePitch(i);
					m_mergePitch[i] = Math::Lerp(m_mergePitch[i], m_mergePitchTarget[i], delta * 10);
					m_mergeSounds[i].SetPitch(m_mergePitch[i]);
				}
				else
				{
					m_mergeSounds[i].SetPitch(m_fixedPitch);
				}

				if (m_mergeVolume[i] < 0.02f && rate < m_mergeRange[i].volRateMin)
				{
					m_mergeSounds[i].Stop();
				}
			}
		}
	}

	bool HighrateSound::IsPlaying()
	{
		for (int i = 0; i < m_mergeSounds.size(); i++)
		{
			if (m_mergeSounds[i].IsPlaying())
				return true;
		}
		return Sound::IsPlaying();
	}

	double HighrateSound::GetRate()
	{
		return m_rate.rate;
	}

	void HighrateSound::SetPitch(float pitch)
	{
		m_fixedPitch = pitch;
	}

	void HighrateSound::PlayMerged(int i)
	{
		m_playingMerged = true;

		m_mergeSounds[i] = Audio::MakeHandle(m_mergePath[i], m_channel, false);
		m_mergeVolume[i] = 0.f;
		m_mergeSounds[i].SetVolume(m_mergeVolume[i]);
		m_mergeSounds[i].Play();
		m_mergeSounds[i].SetLoop(true);
	}

	void HighrateSound::StopMerged(int i)
	{
		m_mergeSounds[i].Stop();
	}

	float HighrateSound::ComputeMergeVolume(int i)
	{
		float minus = 0;
		if (i + 1 < m_mergeRange.size())
			minus = -(Math::Clamp01((m_rate.rate - m_mergeRange[i + 1].volRateMin) / m_mergeRange[i + 1].volRateMax));
		return Math::Lerp(m_mergeRange[i].volMin, m_mergeRange[i].volMax,
			minus + easing(Math::Clamp01((m_rate.rate - m_mergeRange[i].volRateMin) / m_mergeRange[i].volRateMax)));
	}

	float HighrateSound::ComputeMergePitch(int i)
	{
		return Math::Lerp(m_mergeRange[i].pitchMin, m_mergeRange[i].pitchMax,
			Math::Clamp01((m_rate.rate - m_mergeRange[i].pitchRateMin) / m_mergeRange[i].pitchRateMax));
	}
}