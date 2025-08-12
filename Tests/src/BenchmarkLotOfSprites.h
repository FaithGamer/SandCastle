#pragma once

#include <SandCastle.h>
#include <imgui/imgui.h>
using namespace SandCastle;
void OnClick(InputSignal* signal)
{
	Profiling::LogAllClocks();
}

struct Trollface
{
	float timer = 0.f;
};
class Benchmark0 : public System
{
public:
	void OnStart() override
	{
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
	}
};
class Benchmark1 : public System
{
public:
	void OnUpdate(Time delta) override
	{
		faceCount = 0;
		ForeachEntities<Trollface>([&](Entity entity, Trollface& face)
			{
				face.timer -= delta;
				if (face.timer <= 0.f)
					entity.Destroy();
				else
					faceCount++;
			});

		accumulator += delta;
		Vec2f space(16.f / 9.f * 23.f, 23);
		float instancePerSecond = (float)max / time;
		int count = instancePerSecond * delta;
		count = Math::Min(count, 10000);
		for (int i = 0; i < count; i++)
		{
			auto entt = Entity::CreateSprite("trollface.png_0_0");
			entt.GetComponent<Transform>()->SetScale(0.1f);
			entt.AddComponent<Trollface>()->timer = Random::Range(0.1f, time * 2.f);
			Vec3f pos = { Random::Range(-space.x, space.x),
				Random::Range(-space.y, space.y),
				0.f };
			entt.GetComponent<Transform>()->SetPosition(pos);
		}
	}
public:
	int faceCount = 0;
private:
	int max = 50000;
	float time = 3.f;
	float accumulator = 0.f;
};

class SysIm : public System
{
public:
	void OnImGui() override
	{
		ImGui::Begin("window");
		ImGui::Value("face count", Systems::Get<Benchmark1>()->faceCount);
		ImGui::End();
	}
};

void BenchmarkLotOfSprites()
{
	Engine::Init();
	Systems::Push<SysIm>();
	Systems::Push<Benchmark0>();
	Systems::Push<Benchmark1>();

	auto map = Inputs::CreateInputMap();
	auto btn = map->CreateButtonInput("click");
	btn->BindMouse(Mouse::Button::Left);
	btn->signal.Listen(&OnClick);

	Engine::Launch();
}
