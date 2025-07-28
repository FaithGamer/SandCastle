#include "pch.h"
#include "Sound.h"

namespace SandCastle
{
#ifndef SandCastle_NO_AUDIO

	Sound::Sound() : m_sound(nullptr)
	{

	}
	Sound::Sound(ma_sound* sound) : m_sound(sound)
	{
		m_refcount = new int(1);
	}
	Sound::~Sound()
	{
		if (m_sound != nullptr)
		{
			*m_refcount = *m_refcount - 1;
			if (*m_refcount <= 0)
			{

				ma_sound_uninit(m_sound);
				delete m_sound;

				delete m_refcount;
			}
		}
	}

	Sound::Sound(Sound& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
	}

	Sound& Sound::operator=(Sound& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
		return *this;

	}

	Sound::Sound(Sound&& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
	}

	Sound& Sound::operator=(Sound&& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
		return *this;
	}

	void Sound::Play()
	{
		if (m_sound != nullptr)
			ma_sound_start(m_sound);
	}

	void Sound::Stop()
	{
		if (m_sound != nullptr)
			ma_sound_stop(m_sound);
	}

	void Sound::SetLoop(bool loop)
	{
		if (m_sound != nullptr)
			ma_sound_set_looping(m_sound, (ma_bool32)loop);
	}

	void Sound::SetVolume(float volume)
	{
		if (m_sound != nullptr)
			ma_sound_set_volume(m_sound, volume);
	}

	void Sound::SetPitch(float pitch)
	{
		if (m_sound != nullptr)
			ma_sound_set_pitch(m_sound, pitch);
	}

	bool Sound::IsPlaying()
	{
		if (m_sound != nullptr)
			return ma_sound_is_playing(m_sound);
		return false;
	}

#else
	Sound::Sound(ma_sound* sound) : m_sound(sound)
	{


	}


	Sound::~Sound()
	{

	}

	Sound::Sound(Sound& obj)
	{

	}

	Sound& Sound::operator=(Sound& obj)
	{

		return *this;

	}

	void Sound::Play()
	{

	}

	void Sound::Stop()
	{

	}

	void Sound::SetLoop(bool loop)
	{

	}

	void Sound::SetVolume(float volume)
	{

	}

	bool Sound::IsPlaying()
	{

		return false;
	}


#endif

	}