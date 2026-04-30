#pragma once
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Audio/SoundHandle.h"

namespace SandCastle
{
	class Audio;
	/// @brief High-level audio asset built on top of miniaudio.
	/// Holds one or more variants randomly picked at Play time, applies a
	/// random pitch within `pitchVariation`, and enforces a minimum interval
	/// between successive Plays so spammy triggers don't pile up.
	class Sound
	{
		typedef size_t ID;
	public:
		virtual ~Sound() = default;
		/// @brief Add an alternate file picked randomly at Play() time.
		void AddVariant(String filename);
		/// @brief Play once. Optional fade-in and pitch override (-1 = use defaults).
		virtual void Play(float fadeIn = 0.f, float pitch = -1);
		/// @brief Start playing in a loop.
		virtual void Loop(float fadeIn = 0.f, float pitch = -1);
		/// @brief Stop active instances with optional fade-out (-1 = immediate).
		virtual void Stop(float fadeOut = -1.f);
		/// @brief Internal per-frame update (fade timers, instance cleanup).
		virtual void Update();
		/// @brief Override pitch for currently playing instances. Negative = unchanged.
		virtual void SetPitch(float pitch);
		/// @brief True if at least one instance is currently playing.
		virtual bool IsPlaying();
		/// @brief Set per-sound linear volume (combined with channel volume).
		void SetVolume(float volume);
	protected:
		friend Audio;
		void PlayPrivate(bool loop = false, float fadeIn = 0.f, float pitch = -1);
		Sound(unsigned int channel);
	public:
		/// @brief Random pitch jitter range (in semitones-equivalent) added to each Play.
		float pitchVariation = 0.f;
		/// @brief Min/max time (seconds) between two successive Play calls; calls within this window are throttled.
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