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
	/// @brief Audio backend singleton wrapping miniaudio.
	/// Manages a hierarchy of named channels (with per-channel volume), creates
	/// Sound and HighrateSound assets, and exposes a low-level SoundHandle API
	/// for direct playback. Built-in spike-protection ducks output if a runaway
	/// volume spike is detected.
	class Audio : public Singleton<Audio>
	{
	public:

		Audio();
		~Audio();

		/// @brief Add an audio channel, example "Music", "FX"...
		/// @param channel The name of the channel
		/// @param parent The name of the parent audio channel, example "Master"
		/// @return a unique identifier. (low level API only)
		static void AddChannel(const String& channel, const String& parent = "");
		/// @brief Set the volume of a named channel; cascades to its children.
		static void SetChannelVolume(const String& channel, float volume);
		/// @brief When enabled, the named channel is muted while the window is minimized
		/// and restored to its previous volume when the window is un-minimized.
		static void SetMuteWhenMinmized(const String& channel, bool mute = true);
		/// @brief Resolve a channel name to its underlying numeric id.
		static unsigned int GetChannel(const String& channel);
		//High level API
		/// @brief Every sound used in game must first be created with this method.
		/// @param filename the name of the file (not the full path)
		/// @return The sound.
		static Sound* MakeSound(const String& filename, const String& channel);
		/// @brief Create a sound that can switch to playing a loop when the
		/// rate of playing (call to Play()) it is over a certain threshold.
		/// There can be multiple loops, each representing a given rate.
		/// The different sound will fade in and out of each other depending
		/// On the rate that is set.
		/// @param path 
		/// @param channel 
		/// @return 
		static HighrateSound* MakeHighrateSound(const String& filename, const String& channel);
		/// @brief Resolve a sound's filename to its absolute path on disk.
		static String Path(const String& filename);
		/// @brief Low level API, most likely you don't need this
		static SoundHandle MakeHandle(const String& path, bool play = true);
		/// @brief Low level API, most likely you don't need this
		static SoundHandle MakeHandle(const String& path, unsigned int channel, bool play = true);

	private:
		friend Engine;
		friend Systems;
		friend Assets;
		void Init();
		void PostAssetInit();
		void Update();
		void LoadSound(const String& path);
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
		std::vector<String> m_channelNames;
		std::vector<bool> m_muteWhenMinimized;
		std::vector<float> m_preMinimizeVolumes;
		std::vector<Sound> m_sounds;
		std::vector<HighrateSound> m_hrSounds;
	
		float m_spikeDetector = 0.f;
		std::vector<float> m_spikeLastVolumes;
		float m_spikeMuter = 0.f;
		float m_spikeMuterTimeMax = 0.f;
		size_t m_soundCount = 0;
	};
}