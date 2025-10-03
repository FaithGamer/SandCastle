#include "pch.h"
#ifndef SandCastle_NO_AUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"
#endif

#include "SandCastle/Audio/Audio.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Audio/Sound.h"
#include "SandCastle/Render/Window.h"

namespace SandCastle
{
	Audio::Audio()
	{
	}

	void Audio::Init()
	{
		//Initialize audio engine
		m_engine = new ma_engine;
		ma_result result = ma_engine_init(NULL, m_engine);
		if (result != MA_SUCCESS)
		{
			LOG_ERROR("Can't initialize audio engine.");
		}

		Window::GetMinimizedSignal()->Listen(&Audio::OnMinimized, this);
		Window::GetFocusSignal()->Listen(&Audio::OnFocus, this);
	}

	void Audio::LoadSound(const String& path)
	{
		m_maSounds.emplace_back(new ma_sound);
		ma_sound_init_from_file(m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, m_maSounds.back());
		size_t i = path.find_last_of("/") + 1;
		String filename = path.substr(i, path.size() - i);
		auto ins_it = m_filenameToPath.insert(std::make_pair(filename, path));
	}

	void Audio::AddChannel(const String& channel, const String& parent)
	{
		auto i = Instance();
		i->m_channels.emplace_back(new ma_sound_group);
		i->m_channelNames.emplace_back(channel);
		ma_sound_group* parentPtr = NULL;
		if (parent != "")
		{
			auto parentIt = i->GetChannel(parent);
			parentPtr = i->m_channels[parentIt];
		}
		auto r = ma_sound_group_init(i->m_engine, 0, parentPtr, i->m_channels.back());
		if (r != MA_SUCCESS)
		{
			LOG_ERROR("Can't initialize audio channel, {0}", channel);
		}
	}

	unsigned int Audio::GetChannel(const String& channel)
	{
		auto ins = Instance();
		for (int i = 0; i < ins->m_channelNames.size(); i++)
		{
			if (ins->m_channelNames[i] == channel)
				return i;
		}
		ASSERT_LOG_ERROR(false, "Audio channel {0}, doesn't exists", channel);
		return 0;
	}

	Sound* Audio::MakeSound(const String& filename, const String& channel)
	{
		auto i = Instance();
		auto& sound = i->m_sounds.emplace_back(Sound(GetChannel(channel)));
		sound.AddVariant(filename);
		sound.m_id = i->m_sounds.size()-1;
		return &sound;
	}

	HighrateSound* Audio::MakeHighrateSound(const String& filename, const String& channel)
	{
		auto i = Instance();
		auto& sound = i->m_hrSounds.emplace_back(HighrateSound(GetChannel(channel)));
		sound.AddVariant(filename);
		sound.m_id = i->m_hrSounds.size() - 1;
		return &sound;
	}

	String Audio::Path(const String& filename)
	{
		auto i = Instance();
		auto f_it = i->m_filenameToPath.find(filename);
		if (f_it == i->m_filenameToPath.end())
		{
			LOG_ERROR("Sound file {0} doesn't exists", filename);
			return "";
		}
		return f_it->second;
	}

	void Audio::SetChannelVolume(const String& channel, float volume)
	{
		auto i = Instance();
		auto ch = i->GetChannel(channel);
		ma_sound_group_set_volume(i->m_channels[ch], volume);
	}

	SoundHandle Audio::MakeHandle(const String& soundPath, bool play)
	{
		auto i = Instance();
		ma_sound* masound = new ma_sound;
		ma_sound_init_from_file(i->m_engine, soundPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, masound);

		if (play)
			ma_sound_start(masound);
		return SoundHandle(masound);
	}

	SoundHandle Audio::MakeHandle(const String& soundPath, unsigned int channel, bool play)
	{
		auto i = Instance();
		if (channel >= i->m_channels.size())
		{
			LOG_WARN("Trying to create sound handle on unexisting channel: {0}", channel);
			return SoundHandle();
		}

		ma_sound* masound = new ma_sound;
		ma_sound_init_from_file(i->m_engine, soundPath.c_str(), MA_SOUND_FLAG_DECODE, i->m_channels[channel], NULL, masound);

		if (play)
			ma_sound_start(masound);
		return SoundHandle(masound);
	}

	Audio::~Audio()
	{
		for (auto& sound : m_maSounds)
		{
			ma_sound_uninit(sound);
			delete sound;
		}
		for (auto& channel : m_channels)
		{
			ma_sound_group_uninit(channel);
			delete channel;
		}
	}

	void Audio::Update()
	{
		SpikeProtectionUpdate();
		for (int i = 0; i < m_sounds.size(); i++)
		{
			m_sounds[i].Update();
		}
		for (int i = 0; i < m_hrSounds.size(); i++)
		{
			m_hrSounds[i].Update();
		}
	}
	void Audio::SpikeProtectionUpdate()
	{
		/*float delta = Time::Delta();
		if (m_spikeMuter > 0.f && delta < 0.3)
		{
			auto settings = Systems::Get<SettingsSystem>();
			float t = 1.f - m_spikeMuter / m_spikeMuterTimeMax;
			t = Math::Clamp01(t);
			float volume = Math::Lerp(0, settings->settings.masterVolume, Easing::QuadIn(t));
			Audio::SetChannelVolume(Audio::GetChannel("Master"), volume);
			m_spikeMuter -= delta;

			if (m_spikeMuter <= 0.f)
			{
				m_spikeMuterTimeMax = 0.f;
			}
		}

		if (delta >= 0.3)
		{
			m_spikeMuter += delta;
			m_spikeMuterTimeMax = m_spikeMuter;
			m_spikeLastVolum
			Audio::SetChannelVolume(Audio::GetChannel("Master"), 0.f);
		}*/

	}

	void Audio::OnMinimized(bool minimized)
	{

	}

	void Audio::OnFocus(bool focus)
	{

	}
}
