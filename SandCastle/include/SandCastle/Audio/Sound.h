#pragma once

#ifndef SandCastle_NO_AUDIO
#include "miniaudio/miniaudio.h"
#endif
#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
	/// @brief Sound handle, can be safely copied.
	/// Must be created with the Audio singleton.
	class Sound
	{
	public:
		Sound();
		Sound(ma_sound* sound);
		~Sound();
		Sound(Sound& obj);
		Sound& operator=(Sound& obj);
		Sound(Sound&& obj);
		Sound& operator=(Sound&& obj);
		void Play();
		void Stop();
		void SetLoop(bool loop);
		/// @brief Set the sound volume.
		/// @param volume linear scale between 0.f and 1.f. Above 1.f amplify original sound.
		void SetVolume(float volume);
		void SetPitch(float pitch);
		bool IsPlaying();
	private:
#ifndef SandCastle_NO_AUDIO
		int* m_refcount;
		ma_sound* m_sound;
#endif

	};
}