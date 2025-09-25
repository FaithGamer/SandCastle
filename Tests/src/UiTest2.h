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
		Images();
		Right();
	}
	void Popup()
	{
		Ui::Context("base");
		Ui::SetRootAnchor(CanvasAnchor::TopLeft);
		uis.hoverable = Ui::Begin(Vec2f(100.f, 0.f));
		uis.hoverable->ListenHover(&TestSys::OnHoverHoverable, this);
		uis.hoverable->ListenUnhover(&TestSys::OnUnhoverHoverable, this);
		uis.hoverable->SetPosition(Vec2f(-320.f, 180.f));

		Ui::Context("title");
		Ui::Text("Title on multiple lines");
		Ui::Context("base");
		Ui::Text("Hover this canvas to show a popup. Do it.");
		Ui::End();
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
		Ui::End();
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
	bool ch = false;
	void Checkbox()
	{
		Ui::Context("title");
		Ui::SetRootAnchor(CanvasAnchor::TopRight);

		auto r = Ui::Begin();//Begin root
		r->SetPosition(Vec2f(320, 180));
		Ui::Text("Checkboxes");
		Ui::Context("base");
		Ui::SetCanvasSpacing(2.f);
		Ui::SetCanvasPadding(0.f);

		Ui::Begin(0.f, false);//Begin checkboxes
		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);

		Ui::Begin(Vec2f(140.f, 0), false); //Checkbox1
		auto check = Ui::Checkbox();
		check->SetChecked(ch);
		check->checkSignal.Listen(&TestSys::OnCheck, this);
		Ui::Text("Checked: {0}", 0.f, &ch);
		Ui::End(); //End checkbox1

		Ui::Begin(Vec2f(140.f, 0), false);//Checkbox 2
		Ui::Checkbox();
		Ui::Text("Normal");
		Ui::End();//End checkbox 2

		Ui::End();//End checkboxes

		Ui::End();//End root
	}
	void OnCheck(bool checked)
	{
		ch = checked;
	}
	void Images()
	{
		Ui::Context("title");
		Ui::SetRootAnchor(CanvasAnchor::MiddleCenter);
		auto c = Ui::Begin(Vec2f(200, 0));
		c->SetPosition(Vec2f(0, 0));
		Ui::Text("Square canvas.");
		Ui::Context("base");
		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);
		Ui::Begin(0.f, false);
		Ui::Image("blue.png_0_0");
		Ui::Image("yellow.png_0_0");
		Ui::Image("green.png_0_0");
		Ui::Image("yellow.png_0_0");
		Ui::Image("green.png_0_0");
		Ui::Image("blue.png_0_0");
		Ui::Image("yellow.png_0_0");
		Ui::Image("blue.png_0_0");
		Ui::Image("green.png_0_0");
		Ui::End();
		Ui::Text("I hope you enjoyed.");
		Ui::End();
	}
	void Right()
	{
		Ui::Context("right");
		Ui::SetRootAnchor(CanvasAnchor::MiddleRight);
		auto r = Ui::Begin(Vec2f(150, 0));
		r->SetPosition(Vec2f(320, 0));
		Ui::Text("This canvas has everything right align", 100.f);
		Ui::Context("base");
		Ui::Begin();
		Ui::Text("But inside this nested canvas, it's aligned left again. Isn't that cool ?", 100.f);
		Ui::Image("blue.png_0_0");
		Ui::End();
		Ui::End();
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
	Ui::SetCanvasPadding(6.f);
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
	Ui::SetRootAnchor(CanvasAnchor::TopLeft);
	Ui::SnapshotContext("popup");

	Ui::SetTextColor(Color::Black);
	Ui::SetCanvasFrame("frame.png");
	Ui::SetTextFont("h1");
	Ui::SetTextColor(Color::Red);
	Ui::SetTextAlign(TextAlign::Center);
	Ui::SetCanvasLayoutAlignH(LayoutAlign::Middle);
	Ui::SnapshotContext("title");

	Ui::SetTextColor(Color::Black);
	Ui::SetTextFont("p");
	Ui::SetTextAlign(TextAlign::Right);
	Ui::SetCanvasLayoutAlignH(LayoutAlign::End);
	Ui::SnapshotContext("right");

	Engine::Launch();
}
