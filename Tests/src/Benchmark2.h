#pragma once

#include <SandCastle.h>
#include <imgui/imgui.h>
using namespace SandCastle;

void OnClick2(InputSignal* signal)
{
	Profiling::LogAllClocks();
}

struct Troll
{
	Vec2f dir;
	float timer;
	float speed;
};

class TrollSys : public System
{
public:
	void Update() override
	{
		trollCount = 0;
		auto mouse = Mouse::GetWorldPos();
		auto delta = Time::Delta();
		Entity::View<Troll>().each([&](Entity entity, Troll& troll)
			{
				troll.timer -= delta;
				if (troll.timer <= 0.f)
				{
					entity.Destroy();
					return;
				}

				trollCount++;

				auto trans = entity.GetComponent<Transform>();
				Vec2f offset = troll.dir * delta * troll.speed;
				trans->Move(offset);

				float prox = Math::Clamp01(1.f - (trans->GetPosition().Distance(mouse) / 4.f));
				float target = Math::VecToAngle(mouse - trans->GetPosition());
				float newAngle = Math::MoveTowardsAngle(Math::VecToAngle(troll.dir), target, 360 * prox * delta * troll.speed);
				float newSpeed = Math::MoveTowards(troll.speed, Math::Lerp(2.f, 100.f, prox), 100 * delta);
				troll.speed = newSpeed;
				troll.dir = Math::AngleToVec(newAngle);

			});

		//Instancing trolls
		Vec2f space(16.f / 9.f * 19.f, 19);
		accumulator += tps * delta;
		accumulator = Math::Min(accumulator, 10000.f);
		for (int i = 0; i < (int)accumulator; i++)
		{
			auto entt = Entity::CreateAnimatedSprite();
			entt.GetComponent<Transform>()->SetScale(0.1f * scale);
			auto troll = entt.AddComponent<Troll>();
			troll->timer = Random::Range(0.1f, time * 2.f);
			troll->dir = Math::AngleToVec(Random::Range(0, 359));
			troll->speed = Random::Range(1.f, 10.f);
			Vec3f pos = { Random::Range(-space.x, space.x),
				Random::Range(-space.y, space.y),
				0.f };
			entt.GetComponent<Transform>()->SetPosition(pos);

		}
		accumulator -= (int)accumulator;
	}
	void OnImGui() override
	{
		ImGui::Begin("Parameters");
		ImGui::DragFloat("Troll/s", &tps, 1.f, 100000.f);
		ImGui::DragFloat("scale", &scale, .1f, 5.f);
		ImGui::Value("Trolls Count", trollCount);
		ImGui::End();
	}
public:
	int trollCount = 0;
private:
	int max = 50000;
	float time = 3.f;
	float tps = 10000;
	float scale = 1.f;
	float accumulator = 0;
};

void Benchmark2()
{
	Engine::Init();
	auto map = Inputs::CreateInputMap();
	auto l = Renderer2D::GetLayerId("Window");
	SpriteRender::defaultLayer = l;
	Renderer2D::SetLayerSortZ(l, true);
	auto btn = map->CreateButtonInput("click");
	btn->BindMouse(Mouse::Button::Left);
	btn->signal.Listen(&OnClick2);
	Systems::Push<TrollSys>();
	Engine::Launch();
}