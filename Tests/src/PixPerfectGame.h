#pragma once

#include <SandCastle.h>
using namespace SandCastle;

struct Swordman
{
	int t;
};
class SwordmanSys : public System
{
	void Start() override
	{
		auto e = Entity::CreateAnimatedSprite("swordman_walk.anim");
		e.adc<Swordman>();
		Inputs::Get("Player", "Dest")->signal.Listen(&SwordmanSys::OnDest, this);
	}
	void MoveToDest(Transform& tr)
	{
		auto dir = (dest - tr.GetPosition()).Normalized();
		auto delta = Time::Delta();
		auto offset = dir * speed * delta;

		tr.Move(offset);
	}
	void FlipSprite(Transform& tr)
	{
		if (dest.x > tr.GetPosition().x)
			tr.SetScale(-1, 1, 1);
		else
			tr.SetScale(1, 1, 1);
	}
	void Update() override
	{
		auto delta = Time::Delta();
		auto view = Entity::View<Swordman, Transform>();
		view.each([&](Swordman& t, Transform& tr)
			{
				MoveToDest(tr);
			});
	}
	void OnDest(InputSignal* signal)
	{
		dest = Mouse::GetWorldPos();
		auto view = Entity::View<Swordman, Transform>();
		view.each([&](Swordman& t, Transform& tr)
			{
				FlipSprite(tr);
			});
	}
	Vec3f dest;
	float speed = 200.f;
};
void PixPerfectGame()
{
	Engine::Init();
	Window::SetClearColor(Vec4f(0, 0, 0, 1.f));
	Camera::Constraints constraints;
	constraints.pxStep = 360;
	constraints.targetRatio = 16.f / 9.f;
	constraints.cropH = true;
	constraints.cropW = true;
	Camera::main->SetConstraints(constraints);
	auto inputs = Inputs::CreateInputMap("Player");
	auto dir = inputs->CreateButtonInput("Dest");
	dir->BindMouse(Mouse::Button::Left);
	Systems::Push<SwordmanSys>();
	auto e = Entity::CreateSprite("background.png_0_0");
	e.gtr()->SetPosition(0, 0, 100);
	Engine::Launch();
}