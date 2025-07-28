#include "pch.h"
#ifndef SandCastle_NO_AUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"
#endif

#include "SandCastle/Audio/Audio.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Audio/Sound.h"

namespace SandCastle
{
	Audio::Audio()
	{
	}

	void Audio::Init()
	{
#ifndef SandCastle_NO_AUDIO
		//Initialize audio engine
		m_engine = new ma_engine;
		ma_result result = ma_engine_init(NULL, m_engine);
		if (result != MA_SUCCESS)
		{
			LOG_ERROR("Can't initialize audio engine.");
		}
#endif
	}

	void Audio::LoadSound(String path)
	{
#ifndef SandCastle_NO_AUDIO
		m_sounds.push_back(new ma_sound);
		ma_sound_init_from_file(m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, m_sounds.back());
#endif
	}

	Sound Audio::MakeSound(String soundPath, bool play)
	{
#ifndef SandCastle_NO_AUDIO
		auto i = Instance();
		ma_sound* masound = new ma_sound;
		ma_sound_init_from_file(i->m_engine, soundPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, masound);

		if (play)
			ma_sound_start(masound);
		return Sound(masound);
#else
		return Sound();
#endif
	}

	Sound Audio::MakeSound(String soundPath, unsigned int channel, bool play)
	{
#ifndef SandCastle_NO_AUDIO
		auto i = Instance();
		if (channel >= i->m_channels.size())
		{
			LOG_WARN("Trying to play audio on unexisting channel, {0}", channel);
			return Sound();
		}

		ma_sound* masound = new ma_sound;
		ma_sound_init_from_file(i->m_engine, soundPath.c_str(), MA_SOUND_FLAG_DECODE, i->m_channels[channel], NULL, masound);

		if (play)
			ma_sound_start(masound);
		return Sound(masound);
#else
		return Sound();
#endif
	}

	unsigned int Audio::AddChannel(String channel, String parent)
	{
		auto i = Instance();
		i->m_channels.push_back(new ma_sound_group);
		i->m_channelNames.push_back(channel);
		ma_sound_group* parentPtr = NULL;
		if (parent != "")
		{
			auto parentIt = i->GetChannel(parent);
			if (i->m_channels.size() <= parentIt)
			{
				LOG_ERROR("Cannot add audio channel {0}, because parent {1}, doesn't exist.", channel, parent);
			}
			parentPtr = i->m_channels[parentIt];
		}
		auto r = ma_sound_group_init(i->m_engine, 0, parentPtr, i->m_channels.back());
		if (r != MA_SUCCESS)
		{
			LOG_ERROR("Can't initialize audio channel, {0}", channel);
			return 0;
		}
		return 0;
	}

	unsigned int Audio::GetChannel(String channel)
	{
		auto ins = Instance();
		for (int i = 0; i < ins->m_channelNames.size(); i++)
		{
			if (ins->m_channelNames[i] == channel)
				return i;
		}
		LOG_WARN("Cannot find audio channel {0}, channel 99 returned", channel);
		return 99;
	}

	void Audio::SetChannelVolume(unsigned int channel, float volume)
	{
		auto i = Instance();
		if (channel >= i->m_channels.size())
		{
			LOG_WARN("Trying to set the volume of unexisting channel, {0}", channel);
			return;
		}
		ma_sound_group_set_volume(i->m_channels[channel], volume);
	}

	Audio::~Audio()
	{
#ifndef SandCastle_NO_AUDIO
		for (auto& sound : m_sounds)
		{
			ma_sound_uninit(sound);
			delete sound;
		}
		for (auto& channel : m_channels)
		{
			ma_sound_group_uninit(channel);
			delete channel;
		}
#endif
	}
}

/*
* init from file in assets.
* 
*/