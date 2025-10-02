#pragma once
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Audio/SoundHandle.h"

namespace SandCastle
{
	/// @brief High level API for sound
	class Sound
	{
		typedef size_t ID;
	public:
		virtual ~Sound() = default;
		void AddVariant(String path);
		virtual void Play(float fadeIn = 0.f, float pitch = -1);
		virtual void Loop(float fadeIn = 0.f, float pitch = -1);
		virtual void Stop(float fadeOut = -1.f);
		virtual void Update(float delta);
		virtual void SetPitch(float pitch);
		virtual bool IsPlaying();
		void SetVolume(float volume);
	protected:
		void PlayPrivate(bool loop = false, float fadeIn = 0.f, float pitch = -1);
		Sound(unsigned int channel);
	public:
		float pitchVariation = 0.f;
		std::pair<float, float> timeBetweenPlay = std::make_pair(0.02f, 0.06f);
	protected:
		ID m_id = 0;
		String m_assetPath;
		unsigned int m_channel = 0;
		std::list<SoundHandle> m_instances;
		std::vector<float> m_timers;
		std::vector<String> m_variants;
		float m_fadeOut = -1.f;
		float m_fadeIn = -1.f;
		float m_fadeVolume = 0.f;
		float m_volume = 1.f;
		float m_timeBetweenPlay = 0.05f;
		bool m_playedThisIteration = false;
	};
}