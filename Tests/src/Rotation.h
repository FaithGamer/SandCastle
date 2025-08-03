#pragma once

#include <SandCastle/SandCastle.h>
using namespace SandCastle;

/*

This test is made to check if rotations works within the intended paradigm.

In SandCastle, all rotations are represented in degrees.

-Rotating +X° is clock-wise, -X° counter-clockwise.
-0° is 12:00, 90° is 15:00, 180° is 18:00.
-Converting a 0° angle to a vector is [0, 1] (up)
-Converting a 90° angle to a vector is [1, 0] (right)
-Converting a -90° angle to a vector is [-1, 0] (left)
-Converting a -180 or 180° angle to a vector is [0, -1] (down)
-460° == 100° (when applied to a Transform, or converted to a Vec2)

-Conversions from vector to angle is consistent with the above example.

*/

struct Rotate
{
	int dummy;
};

class RotateSystem : public System
{
public:
	void OnStart() override
	{
		auto dirs = Inputs::GetInputMap("map")->GetInput("Dirs");
		dirs->signal.AddListener(&RotateSystem::OnDir, this);
	}
	void OnUpdate(Time time) override
	{
		ForeachEntities<Rotate>([&](Entity entity, Rotate& rotate)
			{
				auto trans = entity.GetComponent<Transform>();

				trans->RotateZ(_dir*_speed);
				float rot = trans->GetRotation().z;

				auto vec = Math::AngleToVec(rot);

				if (Math::Abs(_dir) > 0.01f)
				{
					LOG_INFO("Degrees: {0}", rot);
					LOG_INFO("Vector: [{0}, {1}]", vec.x, vec.y);
					LOG_INFO("Degrees: {0}", Math::VecToAngle(vec));
				}
			});
	}
	void OnDir(InputSignal* signal)
	{
		_dir = signal->GetVec2f().x;
	}
private:
	float _speed = 0.2f;
	float _dir = 0.f;
};

void Rotation()
{
	Engine::Init();

	Systems::Push<RotateSystem>();

	auto map = Inputs::CreateInputMap("map");
	auto dir = map->CreateDirectionalInput("Dirs");

	std::vector<DirectionalButton> buttons;
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::Left), Vec2f(-1.f, 0)));
	buttons.emplace_back(DirectionalButton(Button(Key::Scancode::Right), Vec2f(1.f, 0)));
	dir->BindButtons(buttons);

	auto entity = Entity::CreateSprite("trollface.png_0_0");
	entity.AddComponent<Rotate>();

	Engine::Launch();
}
