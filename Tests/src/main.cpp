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
#include "PixPerfectGame.h"
#include "SubTexture.h"
#include "FrameTest.h"
#include "DepthBlendTest.h"
#include "UiTest.h"

using namespace SandCastle;
struct Player {
	int tag;
};
class SpriteMove : public System
{
	Vec2f dir;
	float speed = 10.f;
	void Start()override
	{
		auto input = Inputs::CreateInputMap("Player");
		auto dirs = input->CreateDirectionalInput("dirs");
		dirs->BindWASD();
		dirs->signal.Listen(&SpriteMove::OnDir, this);
		auto entt = Entity::CreateSprite("btn.png_0_0");
		auto entt2 = Entity::CreateSprite("btn.png_0_0");
		entt.gtr()->Move(5.f, 2.f, 0.f);
		auto parent= Entity::Create();
		parent.AddComponent<Transform>();
		parent.AddComponent<Player>();
		parent.AddChild(entt);
		parent.AddChild(entt2);
	}
	void Update() override
	{
		auto delta = Time::Delta();
		auto v = Entity::View<Transform, Player>();
		v.each([&](Transform& tr, Player pl)
			{
				Vec2f offset = dir * delta * speed;
				tr.Move(offset);
			});
	}
	void OnDir(InputSignal* signal)
	{
		dir = signal->GetVec2f();

	}
};
void SpriteUiTest()
{
	Engine::Init();
	Systems::Push<SpriteMove>();
	Camera::Constraints cons;
	cons.SetDefault();
	Window::SetClearColor(Vec4f(1, 1, 1, 1));
	Camera::main->SetConstraints(cons);
	Engine::Launch();
}
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
	//PixPerfectGame();
	//SubTexture();
	//FrameTest();
	//DepthBlendTest();
	UiTest();
	//SpriteUiTest();
}
