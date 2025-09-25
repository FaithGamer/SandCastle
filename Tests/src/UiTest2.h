#pragma once

#include <SandCastle.h>

using namespace SandCastle;

class TestSys : public System
{
public:
	struct Uis
	{
		UiCanvas* hoverable = nullptr;
		UiCanvas* popup = nullptr;
	};
	Uis uis;
	void Start()
	{
		Popup();
		Checkbox();
	}
	bool ch = false;
	void Checkbox()
	{

		auto r = Ui::Begin();

		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);
		Ui::Begin(Vec2f(140.f, 0), false);
		auto check = Ui::Checkbox();
		check->SetChecked(ch);
		check->checkSignal.Listen(&TestSys::OnCheck, this);
		Ui::Text("Checked: {0}", 0.f, &ch);
		Ui::EndCanvas();

		Ui::Begin(Vec2f(140.f, 0), false);
		Ui::Checkbox();
		Ui::Text("Normal");
		Ui::EndCanvas();

		Ui::EndCanvas();
	}
	void OnCheck(bool checked)
	{
		ch = checked;
	}
	void Popup()
	{
		Ui::Context("base");
		uis.hoverable = Ui::Begin(Vec2f(100.f, 0.f));
		uis.hoverable->ListenHover(&TestSys::OnHoverHoverable, this);
		uis.hoverable->ListenUnhover(&TestSys::OnUnhoverHoverable, this);
		uis.hoverable->SetPosition(Vec2f(-320.f, 180.f));

		Ui::Context("title");
		Ui::Text("Title on multiple lines");
		Ui::Context("base");
		Ui::Text("Hover this canvas to show a popup. Do it.");
		Ui::EndCanvas();
	}
	void MakePopup()
	{
		Ui::Context("popup");
		uis.popup = Ui::Begin(Vec2f(100.f, 0.f));
		Vec2f pos(
			uis.hoverable->GetPosition().x + uis.hoverable->GetSize().x,
			uis.hoverable->GetPosition().y
		);
		uis.popup->SetPosition(pos);
		Ui::Text("This window has a popup appearing when you hover it.");
		Ui::EndCanvas();
	}
	void OnHoverHoverable(UiElem* signal)
	{
		if (uis.popup != nullptr)
			return;
		MakePopup();
	}
	void OnUnhoverHoverable(UiElem* signal)
	{
		if (uis.popup == nullptr)
			return;
		Ui::Destroy(uis.popup);
		uis.popup = nullptr;
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
	Ui::MakeFont("ark-pixel-12px-proportional-latin.ttf", "h2", 12);
	Ui::MakeFont("ark-pixel-16px-proportional-latin.ttf", "h1", 16);
	Ui::MakeFrameTemplate("frame.png");
	Ui::MakeFrameTemplate("frame_black.png");
	Ui::MakeFrameTemplate("btn.png");
	Ui::MakeFrameTemplate("btn_hover.png");
	Ui::MakeFrameTemplate("btn_pressed.png");

	//Context
	Ui::SetCanvasPadding(5.f);
	Ui::SetRootMargin(2.f);
	Ui::SetCanvasFrame("frame.png");
	Ui::SetCanvasLayoutDir(LayoutDir::TopDown);
	Ui::SetCanvasSpacing(3.f);
	Ui::SetTextColor(Color::Black);
	Ui::SetTextFont("p");
	Ui::SetButtonFrame("btn.png");
	Ui::SetButtonFrameHover("btn_hover.png");
	Ui::SetButtonFramePressed("btn_pressed.png");
	Ui::SetCheckboxSprites("checkbox.png");
	Ui::SnapshotContext("base");

	Ui::SetCanvasFrame("frame_black.png");
	Ui::SetTextColor(Color::White);
	Ui::SnapshotContext("popup");

	Ui::SetTextColor(Color::Black);
	Ui::SetCanvasFrame("frame.png");
	Ui::SetTextFont("h2");
	Ui::SetTextColor(Color::Red);
	Ui::SetTextAlign(TextAlign::Center);
	Ui::SetCanvasLayoutAlignH(LayoutAlign::Middle);
	Ui::SnapshotContext("title");




	Engine::Launch();
}
