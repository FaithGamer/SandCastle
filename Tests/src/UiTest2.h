#pragma once

#include <SandCastle.h>

using namespace SandCastle;

class TestSys : public System
{
public:
	struct Uis
	{
		UiCanvas* main = nullptr;
		UiCanvas* hint = nullptr;
	};
	Uis uis;
	void Start()
	{
		MakeMain();
	}
	void MakeMain()
	{
		uis.main = Ui::BeginCanvas(Vec2f(100.f, 0.f));
		uis.main->ListenHover(&TestSys::OnHoverMain, this);
		uis.main->ListenUnhover(&TestSys::OnUnhoverMain, this);
		Ui::SetFont("p");
		Ui::SetTextColor(Color::Black);
		Ui::Text("The main window consist of a simple block of text. Nothing crazy to show up here!");
		Ui::EndCanvas();
	}
	void MakeHint()
	{
		uis.hint = Ui::BeginCanvas(Vec2f(100.f, 0.f));
		Vec2f pos(
			uis.main->GetPosition().x - uis.hint->GetSize().x,
			0.f
		);
		uis.hint->SetPosition(pos);
		Ui::SetFont("p");
		Ui::SetTextColor(Color::Black);
		Ui::Text("This is a hint for you, thank me later.");
		Ui::EndCanvas();
	}
	void OnHoverMain(UiElem* signal)
	{
		if (uis.hint != nullptr)
			return;
		MakeHint();
	}
	void OnUnhoverMain(UiElem* signal)
	{
		if (uis.hint == nullptr)
			return;
		Ui::Destroy(uis.hint);
		uis.hint = nullptr;
	}
};
void UiTest2()
{
	Engine::Init();
	Systems::Push<TestSys>();

	//Pixel art render
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);

	//Ui assets
	Ui::MakeFont("ark-pixel-10px-proportional-latin.ttf", "p", 10);
	Ui::MakeFrameTemplate("frame.png");
	Ui::MakeFrameTemplate("btn.png");
	Ui::MakeFrameTemplate("btn_hover.png");
	Ui::MakeFrameTemplate("btn_pressed.png");
	//Globally use these settings
	Ui::SetCanvasPadding(5.f);
	Ui::SetCanvasFrame("frame.png");
	Ui::SetButtonFrame("btn.png");
	Ui::SetButtonFrameHover("btn_hover.png");
	Ui::SetButtonFramePressed("btn_pressed.png");

	Engine::Launch();
}
