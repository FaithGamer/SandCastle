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

	void CreateSomeUi()
	{
		/*Ui::SetCanvasFrame("frame.png");
		Ui::SetFont("alata");
		UiElemID canvasId = Ui::StartCanvas();
		Ui::SetCanvasSize(Vec2f(100, 360));
		Ui::SetCanvasPos(Vec2f(0, 180));
		Ui::Text("Hello World");
		Ui::EndCanvas();*/

		//Select t
		Ui::SetCanvasFrame("frame.png");
		Ui::SetFont("alata");

		Ui::BeginCanvas();
		Ui::Text("Hello World and everybody in this planet");
		Ui::EndCanvas();

	}
	void Start() override
	{
		auto worldLayer = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = worldLayer;

		//Init UI
		Ui::MakeFrameTemplate("frame.png", true);
		//Optionally change the ppu before making font
		//This can help to make the font px size 
		//to be 1:1 for a specific screen resolution
		Ui::GetWriter()->SetPPU(3.f); //Will be native at 1080p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		//Ui::GetWriter()->SetPPU(1.f); //Will be native at 360p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		Ui::MakeFont("alata-regular.ttf", "alata", 14);
	
		//Create ui stuff
		CreateSomeUi();
	}
	void Update() override
	{
		TextOscillate();
	}
	void TextOscillate()
	{
		static float timer = 0.f;
		auto delta = Time::Delta();
		timer += delta;
		float ypos = std::sin(timer) * 100.f;
		float xpos = ypos;

		auto view = Entity::View<Txt, Transform>();
		view.each([&](Txt& t, Transform& tr)
			{
				tr.SetPosition(0, xpos, 0);
			});
	}
};
void UiTest()
{
	Engine::Init();
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	Systems::Push<UiSys>();
	Engine::Launch();
}
