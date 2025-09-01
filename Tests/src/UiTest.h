#pragma once

#include <SandCastle.h>
using namespace SandCastle;

struct Txt
{
	int tag;
};
class UiSys : public System
{
public:

	/*
	Research for API
	void MakeUiX()
	{
		Ui::BeginFrame(&m_xId, texture, anchor, size);
		//Ui::PushFrame(frame);
		Ui::SetFrameAnchor(anchor);
		Ui::SetFrameSize(size);
		Ui::SetFrameStretch(StretchHorizontal | StretchVertical)
		Ui::Text("Hello World!");
		Ui::Button("Continue");
		Ui::EndFrame();
	}*/
	void Start() override
	{
		//auto uiLayer = Renderer2D::AddLayer("ui");
		auto worldLayer = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = worldLayer;
		Ui::MakeFrameTemplate("frame.png", true);
		Ui::Instance()->InstanceFrame(0, "frame.png", Vec2f(140, 140));

		/*auto fSys = Ui::GetWriter();
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->SetFloat("uDpi", 1.f / 360.f);
		fSys->SetLayer(uiLayer);
		fSys->SetMaterial(uiMat);
		fSys->SetPPU(1.f);
		auto font = fSys->MakeFont("alata-regular.ttf", 50, 2);
		Camera::main->SetPxZoom(2);
		fSys->UseFont(font);*/

		//Create some world entities
		/*for (int i = 0; i < 400; i++)
		{
			float x = Random::Range(-300, 300);
			float y = Random::Range(-180, 180);
			auto entt = Entity::CreateSprite("swordman.png_0_0");
			entt.gtr()->SetPosition(x, y, -5);
		}*/


		/*auto se2 = fSys->Write((const char*)u8"Hello World");
		se2.root.adc<Txt>();*/
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
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	Engine::Launch();
}
