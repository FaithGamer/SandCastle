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

		//Ui::GetWriter()->Write("Hello!");

		Ui::SetCanvasFrame("frame.png");
		Ui::SetFont("alata");

		//Nested:

		/*auto canvas = Ui::BeginCanvas(Vec2f(150.f, 0));
		canvas->layoutDir = Ui::LayoutDir::LeftRight;
		canvas->SetSpacing(2.f);
		//canvas->SetAnchor(Ui::Anchor::TopRight);
		//canvas->SetPosition(Vec2f(320, 180));
		//Ui::SetTextAlign(TextAlign::Center);
		//Ui::SetPadding(Vec2f(5, 5));
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::BeginCanvas();
		Ui::Text("Bonjour a tous");
		auto nested2 = Ui::BeginCanvas();
		nested2->layoutDir = Ui::LayoutDir::LeftRight;
		Ui::Image("green30.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::BeginCanvas();
		Ui::Text("nested3");
		Ui::EndCanvas();
		Ui::EndCanvas();
		Ui::EndCanvas();
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("yellow10-20.png_0_0");
		Ui::EndCanvas();*/

		//Spacing:

		//Ui::SetCanvasMargin(4.f);
		auto canvas = Ui::BeginCanvas(Vec2f(150.f, 0));
		canvas->layoutDir = Ui::LayoutDir::LeftRight;
		canvas->SetSpacing(2.f);
		canvas->SetMargin(4.f);
		//canvas->SetBorder(2.f);
		canvas->SetPosition({ -320.f, 180.f });
		Ui::SetMargin(4.f);
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("yellow10-20.png_0_0");
		Ui::EndCanvas();

		//Top Down VCenter
		/*auto canvas = Ui::BeginCanvas(Vec2f(0, 150));
		canvas->layoutDir = Ui::LayoutDir::TopDown;
		canvas->layoutAlignV = Ui::LayoutAlignV::Center;
	
		Ui::SetPadding(Vec2f(5, 5));
		Ui::Text("1\n2\n3");
		Ui::Text("Hello");
		Ui::Text("SALUT");
		Ui::Text("oui");
		Ui::Text("j'adore\ndes");
		Ui::Text("les frites");
		Ui::EndCanvas();*/

		//Top Down VCenter HRight
		/*auto canvas = Ui::BeginCanvas(Vec2f(300, 150));
		canvas->layoutDir = Ui::LayoutDir::TopDown;
		canvas->layoutAlignV = Ui::LayoutAlignV::Center;
		canvas->layoutAlignH = Ui::LayoutAlignH::Right;
		Ui::SetPadding(Vec2f(5, 5));
		Ui::Text("1\n2\n3");
		Ui::Text("Hello");
		Ui::Text("SALUT");
		Ui::Text("oui");
		Ui::Text("j'adore\ndes");
		Ui::Text("les frites");
		Ui::EndCanvas();*/

		//Left Right CenterV CenterH
		/*auto canvas = Ui::BeginCanvas(Vec2f(150, 150));
		canvas->layoutDir = Ui::LayoutDir::LeftRight;
		canvas->layoutAlignV = Ui::LayoutAlignV::Center;
		canvas->layoutAlignH = Ui::LayoutAlignH::Center;
		Ui::SetPadding(Vec2f(5, 5));
		Ui::Text("1\n2\n3");
		Ui::Text("Hello");
		Ui::Text("SALUT");
		Ui::Text("oui");
		Ui::Text("j'adore\ndes");
		Ui::Text("les frites");
		Ui::EndCanvas();*/

		/*auto canvas = Ui::BeginCanvas(Vec2f(150, 150));
		canvas->layoutDir = Ui::LayoutDir::LeftRight;
		canvas->layoutAlignV = Ui::LayoutAlignV::Bot;
		canvas->layoutAlignH = Ui::LayoutAlignH::Right;
		Ui::SetTextAlign(TextAlign::Center);
		Ui::SetPadding(Vec2f(5, 5));
		Ui::Text("1\n2\n3");
		Ui::Text("Hello");
		Ui::Text("SALUT");
		Ui::Text("oui");
		Ui::Text("j'adore\ndes");
		Ui::Text("les frites");
		Ui::EndCanvas();*/

		//Images:
		/*auto canvas = Ui::BeginCanvas(Vec2f(640.f, 0));
		canvas->layoutDir = Ui::LayoutDir::LeftRight;
		canvas->layoutAlignV = Ui::LayoutAlignV::Top;
		canvas->layoutAlignH = Ui::LayoutAlignH::Left;
		canvas->SetAnchor(Ui::Anchor::TopRight);
		canvas->SetPosition(Vec2f(320, 180));
		Ui::SetTextAlign(TextAlign::Center);
		Ui::SetPadding(Vec2f(5, 5));
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("yellow10-20.png_0_0");
		Ui::Image("yellow10-20.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::EndCanvas();*/


	}
	void Start() override
	{
		auto worldLayer = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = worldLayer;

		//Init UI
		Ui::MakeFrameTemplate("frame.png", false);
		//Optionally change the ppu before making font
		//This can help to make the font px size 
		//to be 1:1 for a specific screen resolution
		Ui::GetWriter()->SetPPU(2.f); //Will be native at 1080p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		//Ui::GetWriter()->SetPPU(1.f); //Will be native at 360p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		Ui::MakeFont("alata-regular.ttf", "alata", 14, .2f);

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
