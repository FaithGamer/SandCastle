#include "pch.h"
#include "SandCastle/Audio/SoundHandle.h"

namespace SandCastle
{
	SoundHandle::SoundHandle() : m_sound(nullptr)
	{

	}
	SoundHandle::SoundHandle(ma_sound* sound) : m_sound(sound)
	{
		m_refcount = new int(1);
	}
	SoundHandle::~SoundHandle()
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

	SoundHandle::SoundHandle(const SoundHandle& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
	}

	SoundHandle& SoundHandle::operator=(const SoundHandle& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
		return *this;

	}

	SoundHandle::SoundHandle(const SoundHandle&& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
	}

	SoundHandle& SoundHandle::operator=(const SoundHandle&& obj)
	{
		m_sound = obj.m_sound;
		m_refcount = obj.m_refcount;
		if (m_sound != nullptr)
			*m_refcount = *m_refcount + 1;
		return *this;
	}

	void SoundHandle::Play()
	{
		if (m_sound != nullptr)
			ma_sound_start(m_sound);
	}

	void SoundHandle::Stop()
	{
		if (m_sound != nullptr)
			ma_sound_stop(m_sound);
	}

	void SoundHandle::SetLoop(bool loop)
	{
		if (m_sound != nullptr)
			ma_sound_set_looping(m_sound, (ma_bool32)loop);
	}

	void SoundHandle::SetVolume(float volume)
	{
		if (m_sound != nullptr)
			ma_sound_set_volume(m_sound, volume);
	}

	void SoundHandle::SetPitch(float pitch)
	{
		if (m_sound != nullptr)
			ma_sound_set_pitch(m_sound, pitch);
	}

	bool SoundHandle::IsPlaying()
	{
		if (m_sound != nullptr)
			return ma_sound_is_playing(m_sound);
		return false;
	}
}