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
	HighrateSound::HighrateSound()
	{
		easing = &Easing::QuadOut;
	}
	void HighrateSound::AddMerged(Range range, String soundPath)
	{
		m_mergeRange.push_back(range);
		m_mergePath.push_back(soundPath);
		m_mergeSound.push_back(SoundHandle());
		m_mergeVolume.push_back(0);
		m_mergeVolumeTarget.push_back(0);
		m_mergePitch.push_back(1);
		m_mergePitchTarget.push_back(1);
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

	void HighrateSound::Update(float delta)
	{
		Sound::Update(delta);


		m_rate.Update(Time::UnscaledDelta());
		auto rate = m_rate.rate;

		m_volume = Math::Lerp(oneShotRange.minVol, oneShotRange.maxVol, Easing::QuadIn(Math::Clamp01(1.f - rate / oneShotRange.maxRate)));
		for (int i = 0; i < m_mergeSound.size(); i++)
		{
			if (!m_mergeSound[i].IsPlaying() && rate >= m_mergeRange[i].minRate)
			{
				PlayMerged(i);
			}

			if (m_mergeSound[i].IsPlaying())
			{
				m_mergeVolumeTarget[i] = ComputeMergeVolume(i);
				m_mergeVolume[i] = Math::Lerp(m_mergeVolume[i], m_mergeVolumeTarget[i], delta * 10);
				m_mergeSound[i].SetVolume(m_mergeVolume[i]);

				if (fixedPitch < 0)
				{
					m_mergePitchTarget[i] = ComputeMergePitch(i);
					m_mergePitch[i] = Math::Lerp(m_mergePitch[i], m_mergePitchTarget[i], delta * 10);
					m_mergeSound[i].SetPitch(m_mergePitch[i]);
				}
				else
				{
					m_mergeSound[i].SetPitch(fixedPitch);
				}

				if (m_mergeVolume[i] < 0.02f && rate < m_mergeRange[i].minRate)
				{
					m_mergeSound[i].Stop();
				}

			}

		}
	}

	bool HighrateSound::IsPlaying()
	{
		return false; // to do
	}


	float HighrateSound::GetRate()
	{
		return (float)m_rate.rate;
	}

	void HighrateSound::SetRate(int rate)
	{
		m_playOverPeriod = (double)rate / 10.f;
	}

	void HighrateSound::SetPitch(float pitch)
	{
		fixedPitch = pitch;
	}

	void HighrateSound::PlayMerged(int i)
	{
		m_playingMerged = true;

		m_mergeSound[i] = Audio::MakeHandle(m_mergePath[i], m_channel, false);
		m_mergeVolume[i] = 0.f;
		m_mergeSound[i].SetVolume(m_mergeVolume[i]);
		m_mergeSound[i].Play();
		m_mergeSound[i].SetLoop(true);
	}

	void HighrateSound::StopMerged(int i)
	{
		m_mergeSound[i].Stop();
	}

	float HighrateSound::ComputeMergeVolume(int i)
	{
		float minus = 0;
		if (i + 1 < m_mergeRange.size())
			minus = -(Math::Clamp01((m_rate.rate - m_mergeRange[i + 1].minRate) / m_mergeRange[i + 1].maxRate));
		return Math::Lerp(m_mergeRange[i].minVol, m_mergeRange[i].maxVol,
			minus + easing(Math::Clamp01((m_rate.rate - m_mergeRange[i].minRate) / m_mergeRange[i].maxRate)));
	}

	float HighrateSound::ComputeMergePitch(int i)
	{
		return Math::Lerp(m_mergeRange[i].pitchMin, m_mergeRange[i].pitchMax,
			Math::Clamp01((m_rate.rate - m_mergeRange[i].pitchRateMin) / m_mergeRange[i].pitchRateMax));
	}

	//Music
	LoveMusic::LoveMusic()
	{
		m_channel = Audio::GetChannel("Music");
		m_assetPath = "assets/audio/music/";
	}
}