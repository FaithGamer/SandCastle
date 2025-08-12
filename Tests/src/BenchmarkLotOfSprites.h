#pragma once

#include <SandCastle.h>
using namespace SandCastle;
void OnClick(InputSignal* signal)
{
	Profiling::LogAllClocks();
}
void BenchmarkLotOfSprites()
{
	Engine::Init();

	
	auto map = Inputs::CreateInputMap();
	auto btn = map->CreateButtonInput("click");
	btn->BindMouse(Mouse::Button::Left);
	btn->signal.Listen(&OnClick);

	int spriteCount = 50000;
	Vec2f space(16.f / 9.f * 50.f, 50);
	for (int i = 0; i < spriteCount; i++)
	{
		auto entt = Entity::CreateSprite("trollface.png_0_0");
		entt.GetComponent<Transform>()->SetScale(0.1f);
		Vec3f pos = { Random::Range(-space.x, space.x),
			Random::Range(-space.y, space.y),
			0.f };
		entt.GetComponent<Transform>()->SetPosition(pos);
	}
	Engine::Launch();
}
