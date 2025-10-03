#pragma once
#include <SandCastle.h>
using namespace SandCastle;

class SoundSys : public System
{
public:
	Sound* hh = nullptr;
	void Start() override
	{
		hh = Audio::MakeSound("hithat.wav", "Master");
	}
	void OnImGui()
	{
		ImGui::Begin("window");
		if (ImGui::Button("play"))
		{
			hh->Play();
		}
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
