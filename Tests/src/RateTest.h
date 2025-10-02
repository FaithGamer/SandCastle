#pragma once

#include <SandCastle.h>
using namespace SandCastle;

class RateSys : public System
{
public:
	Rate clickRate = Rate(60., 5 );
	Rate testRate;
	float rate = 0.;
	float timer = 0.;
	void Start()
	{
		auto map = Inputs::CreateInputMap();
		auto b = map->CreateButtonInput("Click");
		b->BindMouse(Mouse::Button::Left);
		b->signal.Listen(&RateSys::OnClick, this);
	}
	void Update()
	{
		timer += Time::Delta();
		while (timer >= 1.f/rate)
		{
			timer -= 1.f / rate;
			testRate.Tick(1.);
		}
		clickRate.Update(Time::Delta());
		testRate.Update(Time::Delta());
	}
	void OnClick(InputSignal* signal)
	{
		clickRate.Tick(1.);
	}
	void OnImGui()
	{
		ImGui::Begin("rate");
		ImGui::Value("click per minute", (float)clickRate.rate);
		ImGui::Value("test rate", (float)testRate.rate);
		ImGui::SliderFloat("set test rate", &rate, 0.f, 1000.f);
		ImGui::End();
	}
};
void RateTest()
{
	Engine::Init();
	Systems::Push<RateSys>();
	Engine::Launch();
}
