#pragma once

#include "SandCastle/Internal/Singleton.h"
#include <miniaudio/miniaudio.h>
#include "SandCastle/Audio/Sound.h"

namespace SandCastle
{
	class Audio : public Singleton<Audio>
	{
	public:

		Audio();
		~Audio();
		void Init();
		/// @brief For internal use only. (Assets loading)
		/// @param path 
		void LoadSound(String path);
		/// @brief Instance handle for the sound located at path.
		/// The returned handle must be kept alive as long as you want the sound to play.
		/// The sound has already been loaded, path is just used as a hash key in the memory manager.
		/// To play multiple times the same sound at the same time, call this function multiple times.
		/// @param path The full path of the sound
		/// @play Play the sound right away or not.
		/// @return Handle to the sound being played.
		static Sound MakeSound(String path, bool play = true);
		static Sound MakeSound(String path, unsigned int channel, bool play = true);
		static unsigned int AddChannel(String channel, String parent = "");
		static unsigned int GetChannel(String channel);
		static void SetChannelVolume(unsigned int channel, float volume);

	private:
		friend sptr<Audio> Singleton<Audio>::Instance();
		friend void Singleton<Audio>::Kill();
#ifndef SandCastle_NO_AUDIO
		ma_engine* m_engine = nullptr;
		std::vector<ma_sound*> m_sounds;
		std::vector<String> m_channelNames;
		std::vector<ma_sound_group*> m_channels;
#endif
	};
}