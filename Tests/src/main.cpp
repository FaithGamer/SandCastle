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


struct UiFrame
{
	int tag;
};
class UiSys : public System
{
	void Update() override
	{
		static float timer = 0.f;
		auto delta = Time::Delta();
		timer += delta;
		float ypos = std::sin(timer*10.f);
		auto view =Entity::View<UiFrame, Transform>();
		view.each([&](UiFrame& fr, Transform& tr)
			{
				tr.SetPosition(0, ypos, 0);
			});
	}
};
void UiTest()
{
	Engine::Init();
	Systems::Push<UiSys>();
	Ui ui;
	//Camera::main->zoom = 0.003f;
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	auto e =ui.MakeFrameSprites(0, Vec2f(140, 140), "frame.png", true);
	e.GetComponent<Transform>()->SetPosition(0, 0, 0);
	e.adc<UiFrame>();

	/*auto tex = Renderer2D::CreateSubTexture(sprite->GetTexture(), sprite->GetTextureRect());
	auto spr = new Sprite(tex);
	auto e = Entity::Create();
	e.adc<Transform>();
	e.adc<SpriteRender>()->SetSprite(spr);*/
	Engine::Launch();
}

/*void TextureFromSubTextureTest()
{
	Engine::Init();
	Camera::main->zoom = 0.01;
	auto frame = Assets::Get<Sprite>("frame.png_0_0");
	Texture* texture = Renderer2D::CreateSubTexture(frame->GetTexture(), frame->GetTextureRect());
	texture->SetWrapping(TextureWrapping::Repeat);
	Sprite* sprite = new Sprite(texture, Rect(0, 0, 1000, 30));
	Entity e = Entity::CreateSprite();
	e.gc<SpriteRender>()->SetSprite(sprite);
	Engine::Launch();
}*/

using namespace SandCastle;
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
//	PixPerfectGame();
	//TextureFromSubTextureTest();
	UiTest();
}
