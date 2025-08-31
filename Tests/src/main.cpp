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

struct Txt
{
	int tag;
};
class UiSys : public System
{
public:

	void Start() override
	{
	
		auto uiLayer = Renderer2D::AddLayer("ui");
		auto worldLayer = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = worldLayer;

		auto fSys = Ui::GetWriter();
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->SetFloat("uDpi", 1.f / 360.f);
		fSys->SetLayer(uiLayer);
		fSys->SetMaterial(uiMat);
		fSys->SetPPU(1.f);  
		auto font = fSys->MakeFont("alata-regular.ttf", 50, 2);
		Camera::main->SetPxZoom(2);
		fSys->UseFont(font);

		//Create some world entities
		for (int i = 0; i < 400; i++)
		{
			float x = Random::Range(-300, 300);
			float y = Random::Range(-180, 180);
			auto entt = Entity::CreateSprite("swordman.png_0_0");
			entt.gtr()->SetPosition(x, y, -5);
		}

	
		auto se2 = fSys->Write((const char*)u8"Hello World");
		se2.root.adc<Txt>();
	}
	void Update() override
	{
		static float timer = 0.f;
		auto delta = Time::Delta();
		timer += delta;
		float ypos = std::sin(timer) * 1000.f;
		float xpos = ypos;

		auto view = Entity::View<Txt, Transform>();
		view.each([&](Txt& t, Transform& tr)
			{
				tr.SetPosition(xpos, 0, 0);
			});

	}

};
void UiTest()
{
	Engine::Init();
	Systems::Push<UiSys>();
	Camera::Constraints cons;
	Window::SetClearColor(Vec4f(1, 0, 0, 1));
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	Engine::Launch();
}

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
	//PixPerfectGame();
	//SubTexture();
	//FrameTest();
	UiTest();
}
