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

	void SoundHandle::PlayAt(uint64_t engineFrame)
	{
		if (m_sound == nullptr)
			return;
		ma_sound_set_start_time_in_pcm_frames(m_sound, engineFrame);
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
		if (m_sound == nullptr)
			return false;
		if (ma_sound_is_playing(m_sound))
			return true;
		// Started with a start time still in the future: ma_sound_is_playing reports it
		// stopped until that time comes, but it is live and must not be discarded.
		return ma_node_get_state(m_sound) == ma_node_state_started
			&& ma_node_get_state_time(m_sound, ma_node_state_started) > ma_engine_get_time_in_pcm_frames(ma_sound_get_engine(m_sound));
	}

	uint64_t SoundHandle::GetLengthInFrames()
	{
		if (m_sound == nullptr)
			return 0;
		ma_uint64 length = 0;
		ma_uint32 rate = 0;
		if (ma_sound_get_length_in_pcm_frames(m_sound, &length) != MA_SUCCESS
			|| ma_sound_get_data_format(m_sound, NULL, NULL, &rate, NULL, 0) != MA_SUCCESS
			|| rate == 0)
			return 0;
		const ma_uint32 engineRate = ma_engine_get_sample_rate(ma_sound_get_engine(m_sound));
		if (rate == engineRate)
			return length;
		return (uint64_t)((double)length * engineRate / rate + 0.5);
	}
}