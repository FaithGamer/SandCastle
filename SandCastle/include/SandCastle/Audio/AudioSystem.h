#pragma once

#include "SandCastle/ECS/System.h"
#include "SandCastle/Audio/LoveSound.h"

namespace SandCastle
{
	enum AmbianceSound : int
	{

	};
	class AudioSystem : public System
	{
	public:
		AudioSystem();
		void OnStart() override;
		void OnUpdate(Time delta) override;
		void OnImGui() override;

		void SetRate(OneShot sound, int rate);
		void SetPitch(OneShot sound, float pitch);
		void PlaySound(OneShot sound, float fadeIn = -1.f, bool loop = false, float pitch = 0.f);
		void StopSound(OneShot sound, float fadeOut = -1.f);
		void StartAmbiance(AmbianceSound sound);
		void StopAmbiance(AmbianceSound sound);
		void OnMinimized(bool minimized);
		void OnFocus(bool focus);
		bool IsSoundPlaying(OneShot sound);
	private:
		void SpikeProtectionUpdate();
		void Test(float delta);
		float testRate = 0;
		float testTimer = 0;
		float rateTimer = 0;
		float testTimer2 = 1.f;

		//runtime
		
	};
}