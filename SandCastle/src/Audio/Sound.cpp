#include "pch.h"
#include "SandCastle/Audio/Sound.h"
#include "SandCastle/Audio/Audio.h"
#include "SandCastle/Core/Random.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Core/Easing.h"
#include "SandCastle/Core/Time.h"

namespace SandCastle
{
	
	Sound::Sound(unsigned int channel) : m_channel(channel)
	{
	}

	void Sound::AddVariant(String path)
	{
		m_variants.emplace_back(path);
		m_timers.emplace_back(100.f);
	}
	void Sound::PlayPrivate(bool loop, float fadeIn, float pitch)
	{
		//Avoid having same sound stacking upon itslef 
		if (m_playedThisIteration)
			return;

		if (m_variants.size() < 1)
		{
			LOG_WARN("Trying to play a sound with no variant.");
			return;
		}
		int r = 0;
		if (m_variants.size() > 1)
			r = Random::Range(0, (int)m_variants.size() - 1);

		if (m_timers[0] < m_timeBetweenPlay)
			return;

		if (loop)
		{
			m_instances.clear();
		}

		m_fadeOut = -1.f;
		m_timeBetweenPlay = Random::Range(timeBetweenPlay.first, timeBetweenPlay.second);
		m_timers[0] = 0.f;
		m_instances.emplace_back(Audio::MakeHandle(m_variants[r], m_channel, true));

		m_instances.back().SetLoop(loop);
		if (pitch <= 0.0001)
			m_instances.back().SetPitch(Random::Range(1.f - pitchVariation, 1.f + pitchVariation));
		else
			m_instances.back().SetPitch(pitch);

		if (fadeIn > 0)
		{
			m_fadeIn = m_volume / fadeIn;
			m_fadeVolume = 0;
			m_instances.back().SetVolume(0.01f);
		}
		else
		{
			m_fadeVolume = m_volume;
			m_instances.back().SetVolume(m_volume);
		}

		m_playedThisIteration = true;
	}
	void Sound::Play(float fadeIn, float pitch)
	{
		PlayPrivate(false, fadeIn, pitch);
	}

	void Sound::Loop(float fadeIn, float pitch)
	{
		PlayPrivate(true, fadeIn, pitch);
	}

	void Sound::Stop(float fadeOut)
	{
		if (m_instances.empty())
			return;

		m_fadeIn = -1.f;
		if (fadeOut > 0)
		{
			m_fadeOut = m_volume / fadeOut;
		}
		else
		{
			for (auto& ins : m_instances)
			{
				ins.Stop();
			}
			m_instances.clear();
		}
	}

	void Sound::Update(float delta)
	{
		//Clean instances not playing
		for (auto it = m_instances.begin(); it != m_instances.end();)
		{
			if (!it->IsPlaying())
			{
				m_instances.erase(it++);
			}
			else
			{
				if (m_fadeOut > 0.f)
				{
					float min = m_fadeOut * (float)Time::UnscaledDelta();
					m_fadeVolume -= min;
					if (m_fadeVolume > 0)
						it->SetVolume(m_fadeVolume);
					else
						it->Stop();
				}
				else if (m_fadeIn > 0.f && m_fadeVolume < m_volume)
				{
					float plus = m_fadeIn * (float)Time::UnscaledDelta();
					m_fadeVolume += plus;
					if (m_fadeVolume >= m_volume)
					{
						it->SetVolume(m_volume);
						m_fadeIn = -1.f;
					}
					else
					{
						it->SetVolume(m_fadeVolume);
					}

				}
				it++;
			}
		}
		for (int i = 0; i < m_timers.size(); i++)
		{
			m_timers[i] += Time::UnscaledDelta();
		}
		m_playedThisIteration = false;
	}

	void Sound::SetPitch(float pitch)
	{
		for (auto& in : m_instances)
		{
			in.SetPitch(pitch);
		}
	}

	void Sound::SetVolume(float volume)
	{
		m_volume = volume;
	}

	bool Sound::IsPlaying()
	{
		if (m_instances.empty())
			return false;
		return true;
	}
}