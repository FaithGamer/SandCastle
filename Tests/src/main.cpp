#include "Launch.h"
#include "DrawSprite.h"
#include "DrawAnimation.h"
#include "BasicInputs.h"
#include "Rotation.h"
#include "WindowEvents.h"
#include "Serialization.h"
#include "Benchmark1.h"
#include "Benchmark2.h"
#include "Delegates.h"
#include "Signals.h"
#include "FontTest.h"
#include "LayerTest.h"

using namespace SandCastle;


struct Tag
{
	int t;
};
class MoveS : public System
{
	void Start() override
	{
		auto e = Entity::CreateAnimatedSprite("swordman_walk.anim");
		e.adc<Tag>();

		Inputs::Get("Player", "Dest")->signal.Listen(&MoveS::OnDest, this);
	}
	void MoveToDest(Transform& tr)
	{
		auto dir = (dest - tr.GetPosition()).Normalized();
		auto delta = Time::Delta();
		auto offset = dir * speed * delta;

		tr.Move(offset);
	}
	void Update() override
	{
		auto delta = Time::Delta();
		auto view = Entity::View<Tag, Transform>();
		view.each([&](Tag& t, Transform& tr)
			{
				MoveToDest(tr);
			});
	}
	void OnDest(InputSignal* signal)
	{
		dest = Mouse::GetWorldPos();
	}
	Vec3f dest;
	float speed = 5.f;
};
int main()
{
	//Launch();
	//DrawSprite();
	//DrawAnimation();
	//BasicInputs();
	//Rotation();
	//WindowEvents();
	//Serialization();
	//Benchmark2();
	//Delegates();
	//Signals();
	//FontTest();
	//LayerTest();

	Engine::Init();
	Window::SetClearColor(Vec4f(0.6, 0.6, 0.6, 1.f));
	Camera::main->zoom = 0.002777777f; //pixel perfect for 1.f ppu texture
	auto inputs = Inputs::CreateInputMap("Player");
	auto dir = inputs->CreateButtonInput("Dest");
	dir->BindMouse(Mouse::Button::Left);
	Systems::Push<MoveS>();
	Engine::Launch();
}
