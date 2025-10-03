#pragma once
#include <SandCastle.h>
using namespace SandCastle;

class SoundSys : public System
{
public:
	Sound* hh = nullptr;
	HighrateSound* hr = nullptr;
	double rate = 0.;
	double acc = 0.;
	void Start() override
	{
		hh = Audio::MakeSound("hithat.wav", "Master");
		hh->AddVariant("checkpoint.wav");
		hr = Audio::MakeHighrateSound("clap1.wav", "Master");
		hr->AddVariant("clap2.wav");
		hr->AddVariant("clap3.wav");
		HighrateSound::Range range;
		range.pitchMin = 1.f;
		range.pitchMax = 1.5f;
		range.rateMin = 30.f;
		range.rateMax = 100.f;
		hr->AddRate("clap_merged1.wav", range);
		HighrateSound::Range range2;
		range2.pitchMin = 1.f;
		range2.pitchMax = 3.f;
		range2.rateMin = 100.f;
		range2.rateMax = 300.f;
		hr->AddRate("clap_merged2.wav", range2);
	}
	void Update() override
	{
		acc += Time::Delta();
		double time = 1. / rate;
		while (acc > time)
		{
			hr->Play();
			acc -= time;
		}
	}
	void OnImGui()
	{
		ImGui::Begin("window");
		if (ImGui::Button("play"))
		{
			hh->Play();
		}
		static float rateF = 0.f;
		if (ImGui::SliderFloat("rate", &rateF, 0.f, 300.f))
			rate = (double)rateF;
		ImGui::Value("rate value", (float)hr->GetRate());
		ImGui::End();
	}
};
void SoundTest()
{
	Engine::Init();
	Systems::Push<SoundSys>();
	Audio::AddChannel("Master");
	Engine::Launch();
}
