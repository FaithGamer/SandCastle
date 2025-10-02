#pragma once

#include <miniaudio/miniaudio.h>
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Audio/SoundHandle.h"
#include "SandCastle/Audio/Sound.h"
#include "SandCastle/Audio/HighrateSound.h"

namespace SandCastle
{
	class Assets;
	class Engine;
	class Systems;
	class Audio : public Singleton<Audio>
	{
	public:

		Audio();
		~Audio();


		/// @brief Add an audio channel, example "Music", "FX"...
		/// @param channel The name of the channel
		/// @param parent The name of the parent audio channel, example "Master"
		/// @return a unique identifier. (low level API only)
		static unsigned int AddChannel(String channel, String parent = "");
		static void SetChannelVolume(String channel, float volume);
		static unsigned int GetChannel(String channel);
		//High level API
		/// @brief Every sound used in game must first be created with this method.
		/// @param filename the name of the file (not the full path)
		/// @return The sound.
		static Sound* MakeSound(String filename, String channel);
		/// @brief Create a sound that can switch to playing a loop when the
		/// rate of playing (call to Play()) it is over a certain threshold.
		/// There can be multiple loops, each representing a given rate.
		/// The different sound will fade in and out of each other depending
		/// On the rate that is set.
		/// @param path 
		/// @param channel 
		/// @return 
		static HighrateSound* MakeHighrateSound(String path, String channel);

		/// @brief Low level API, most likely you don't need this
		static SoundHandle MakeHandle(String path, bool play = true);
		/// @brief Low level API, most likely you don't need this
		static SoundHandle MakeHandle(String path, unsigned int channel, bool play = true);

	private:
		friend Engine;
		friend Systems;
		friend Assets;
		void Init();
		void Update();
		void LoadSound(String path);
		void SpikeProtectionUpdate();
		void OnMinimized(bool minimized);
		void OnFocus(bool focus);
	private:
		friend sptr<Audio> Singleton<Audio>::Instance();
		friend void Singleton<Audio>::Kill();

		//Backend
		ma_engine* m_engine = nullptr;
		std::vector<ma_sound*> m_maSounds;
		std::vector<ma_sound_group*> m_channels;

		//collection
		std::unordered_map<String, String> m_filenameToPath;
		std::unordered_map<String, Sound*> m_filenameToSound;
		std::vector<String> m_channelNames;
		std::vector<Sound*> m_sounds;
		std::vector<HighrateSound*> m_sounds;
	
		float m_spikeDetector = 0.f;
		std::vector<float> m_spikeLastVolumes;
		float m_spikeMuter = 0.f;
		float m_spikeMuterTimeMax = 0.f;
	};
}