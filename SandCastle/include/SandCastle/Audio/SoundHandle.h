#pragma once

#include <cstdint>
#include "miniaudio/miniaudio.h"
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief ma_sound* wrapper, can be safely copied.
	/// Must be created with the Audio singleton.
	class SoundHandle
	{
	public:
		SoundHandle();
		SoundHandle(ma_sound* sound);
		~SoundHandle();
		SoundHandle(const SoundHandle& obj);
		SoundHandle& operator=(const SoundHandle& obj);
		SoundHandle(const SoundHandle&& obj);
		SoundHandle& operator=(const SoundHandle&& obj);
		void Play();
		/// @brief Start at an absolute audio-clock time (see Audio::GetTime). Sample-accurate
		/// relative to every other sound scheduled on the same clock.
		void PlayAt(uint64_t engineFrame);
		void Stop();
		void SetLoop(bool loop);
		/// @brief Set the sound volume.
		/// @param volume linear scale between 0.f and 1.f. Above 1.f amplify original sound.
		void SetVolume(float volume);
		void SetPitch(float pitch);
		/// @brief True while playing, or started with PlayAt and still waiting for its time.
		bool IsPlaying();
		/// @brief Length of the sound in audio-clock frames (0 if unknown).
		uint64_t GetLengthInFrames();
	private:
		int* m_refcount = nullptr;
		ma_sound* m_sound = nullptr;

	};
}