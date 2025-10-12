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
	void OnClick(InputSignal* signal)
	{
		LOG_INFO("Non UI click");
	}
	Uis uis;
	void Start()
	{
		Popup();
		Checkbox();
		Images();
		Right();
		Button();
		Destroy();
		Loca();
	//	JALineR();
		AddElem();


		auto map = Inputs::CreateInputMap();
		auto button = map->CreateButtonInput("Button");
		button->BindMouse(Mouse::Button::Left);
		button->signal.Listen(&TestSys::OnClick, this);
	}
	void Update()
	{
		MemoryUpdt();
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
	}
	bool memoryLeakTest = false;
	void Checkbox()
	{
		Ui::Context("title");
		Ui::SetRootAnchor(CanvasAnchor::TopRight);

		auto r = Ui::Begin();//root
		r->SetPosition(Vec2f(320, 180));
		Ui::Text("Checkboxes");
		Ui::Context("base");
		Ui::SetCanvasSpacing(2.f);
		Ui::SetCanvasPadding(0.f);

		Ui::Begin(0.f, false);//checkboxes
		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);

		Ui::Begin(Vec2f(140.f, 0), false); //Checkbox1
		auto popupCheck = Ui::Checkbox();
		popupCheck->checkSignal.Listen(&TestSys::OnCheck, this);
		Ui::Text("Popup");
		Ui::End(); //checkbox1

		Ui::Begin(Vec2f(140.f, 0), false);//Checkbox 2
		Ui::Checkbox(&memoryLeakTest);
		Ui::Text("Memory leak test");
		Ui::End();//checkbox 2

		Ui::Begin(Vec2f(140.f, 0), false);//Checkbox 3
		static bool ch = true;
		Ui::Checkbox(&ch);
		Ui::Text("Value: {0}", 0.f, &ch);
		Ui::End();//checkbox 3

		Ui::End();//checkboxes

		Ui::End();//root
	}
	void MemoryUpdt()
	{
		if (!memoryLeakTest)
			return;

		static int counter = 0;
		static UiCanvas* r = nullptr;

		if (counter == 0)
		{
			static int k = 42;
			Ui::Context("popup");
			r = Ui::Begin();
			r->SetPosition(Vec2f(-100, 180));
			Ui::Text("Bonjour {0}", 0.f, &k);
			Ui::Begin();
			auto b = Ui::Button("Ok");
			b->ListenClickReleased(&TestSys::DummyCallback, this);
			Ui::Image("yellow.png_0_0");
			static bool c = false;
			Ui::End();
			auto check = Ui::Checkbox(&c);
			check->checkSignal.Listen(&TestSys::DummyCallback2, this);
			Ui::End();
			counter++;
		}
		else if (r != nullptr)
		{
			counter = 0;
			Ui::Destroy(r);
		}

	}
	void DummyCallback(UiElem* signal)
	{
		//nothing
	}
	void DummyCallback2(UiCheckbox* signal)
	{
		//nothing
	}
	UiCanvas* popupChecked = nullptr;
	void OnCheck(UiCheckbox* checkbox)
	{
		if (checkbox->IsChecked() && popupChecked == nullptr)
		{
			Ui::Context("popup");
			Ui::SetRootAnchor(CanvasAnchor::TopRight);
			popupChecked = Ui::Begin(Vec2f(50, 50));
			auto parent = checkbox->GetParent();
			auto ppos = parent->GetPosition();
			auto psize = parent->GetSize();
			popupChecked->SetPosition(Vec2f(
				ppos.x - psize.x,
				ppos.y
			));
			Ui::Text("Pop!");
			Ui::End();

		}
		else if (popupChecked != nullptr)
		{
			Ui::Destroy(popupChecked);
		}
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
		Ui::Image("green.png_0_0");
		Ui::End();
	}
	int clicked = 0;
	void Button()
	{
		Ui::Context("title");
		Ui::SetRootAnchor(CanvasAnchor::BotLeft);
		auto r = Ui::Begin(Vec2f(150, 0));
		r->SetPosition(Vec2f(-320.f, -180.f));
		Ui::TextLoc("clicked_times", 0.f, &clicked);
		Ui::Button("Ok")->ListenClickReleased(&TestSys::OnClickOk, this);
		Ui::End();
	}
	void OnClickOk(UiElem* signal)
	{
		clicked++;
	}
	std::vector<UiElem*> createds;
	void Destroy()
	{
		Ui::Context("base");
		Ui::SetRootAnchor(CanvasAnchor::BotRight);
		Ui::SetCanvasLayoutAlignV(LayoutAlign::End);
		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);
		auto r = Ui::Begin(Vec2f(0, 100));
		r->SetPosition(Vec2f(320, -180));
		auto cbtn = Ui::Button("Create");
		cbtn->SetColor(Color::Green);
		cbtn->ListenClickReleased(&TestSys::OnClickCreate, this);
		auto dbtn = Ui::Button("Destroy");
		dbtn->SetColor(Color::Red);
		dbtn->ListenClickReleased(&TestSys::OnClickDestroy, this);
		Ui::End();
	}
	void OnClickCreate(UiElem* signal)
	{
		Ui::Context("popup");
		auto r = Ui::Begin(Vec2f(50, 50));
		Vec2f pos(Random::Range(-100, 100), Random::Range(-100, 100));
		r->SetPosition(pos);
		Ui::Text("This text is too long to fit in.");
		Ui::End();
		createds.emplace_back(r);
	}
	void OnClickDestroy(UiElem* signal)
	{
		for (auto c : createds)
		{
			Ui::Destroy(c);
		}
		createds.clear();
	}
	void Loca()
	{
		Ui::Context("base");
		Ui::SetRootAnchor(CanvasAnchor::BotCenter);
		Ui::SetCanvasLayoutAlignH(LayoutAlign::Middle);
		auto r = Ui::Begin();
		r->SetPosition(Vec2f(0, -180));
		Ui::TextLoc("ui_test");
		Ui::Image("flag.png_0_0");
		Ui::SetCanvasLayoutDir(LayoutDir::LeftRight);
		
		Ui::Begin(0.f, false);
		Ui::ButtonLoc("fr")->ListenClickReleased(&TestSys::OnFR, this);
		Ui::ButtonLoc("jap")->ListenClickReleased(&TestSys::OnJA, this);
		Ui::End();
		Ui::End();
	}
	void JALineR()
	{
		Ui::Context("base");
		Ui::SetRootAnchor(CanvasAnchor::TopCenter);
		auto r = Ui::Begin(Vec2f(200, 0));
		r->SetPosition(Vec2f(0, 180.f));
		Ui::Text((const char*)u8"母さ供過通換げ済八えか合巡じすっレ女手ネヱイ処国ト雄約スワリ条球ど置期こがて形申ヱカロ投掲ルコチ極供ごこゅ年鮮堪薫け。");
		Ui::End();
	}
	UiCanvas* addelem = nullptr;
	void AddElem()
	{
		Ui::Context("base");
		Ui::SetRootAnchor(CanvasAnchor::TopCenter);
		addelem = Ui::Begin(Vec2f(200, 0));
		addelem->SetPosition(Vec2f(0, 180.f));
		Ui::Text((const char*)u8"You can add elem");
		auto btn = Ui::Button("Add");
		btn->ListenClickReleased(&TestSys::OnAddElem, this);
		Ui::End();
	}
	void OnAddElem(UiElem* signal)
	{
		auto t = Ui::Text("added element");
		addelem->AddElem(t);
	}
	void OnFR(UiElem* signal)
	{
		Assets::SetLang("fr");
	}
	void OnJA(UiElem* signal)
	{
		Assets::SetLang("ja");
	}
};
void UiTest2()
{
	EngineSettings settings;
	settings.appName = "Ui tests";
	settings.defaultLang = "ja";
	settings.textureImport = TextureImportSettings(
		TextureFiltering::Nearest,
		TextureWrapping::Clamp,
		1.f,
		false,
		false
	);
	Versioning::Push("0.1");
	Versioning::Push("0.2");
	
	Engine::Init(settings);
	Systems::Push<TestSys>();
	//Pixel art render
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);

	//Ui assets
	Ui::GetWriter()->SetLineAdjustement(0.2f);
	Ui::MakeFont("Silver.ttf", "p", 38, .5f, .75f, { "ja" });
	Ui::MakeFont("alata-regular.ttf", "p", 10, 1.f, 1.f, { "fr" });
	Ui::MakeFont("Silver.ttf", "h2", 38, .5f, 0.5f, { "ja" });
	Ui::MakeFont("alata-regular.ttf", "h2", 13, 1.f, 1.f, { "fr" });
	Ui::MakeFont("Silver.ttf", "h1", 38, .5f, 0.75f, { "ja" });
	Ui::MakeFont("alata-regular.ttf", "h1", 14, 1.f, 1.f, { "fr" });
	Ui::MakeFrameTemplate("frame.png");
	Ui::MakeFrameTemplate("frame_black.png");
	Ui::MakeFrameTemplate("btn.png");
	Ui::MakeFrameTemplate("btn_hover.png");
	Ui::MakeFrameTemplate("btn_pressed.png");

	//Audio asset (for ui)
	Audio::AddChannel("Master");
	Audio::AddChannel("UI", "Master");
	auto clickPressSound = Audio::MakeSound("click_press.wav", "UI");
	auto clickReleaseSound = Audio::MakeSound("click_release.wav", "UI");
	Ui::SetButtonPressSound(clickPressSound);
	Ui::SetButtonReleaseSound(clickReleaseSound);

	Ui::SetCanvasPadding(6.f);
	Ui::SetRootMargin(2.f);
	Ui::SetCanvasFrame("frame.png");
	Ui::SetCanvasLayoutDir(LayoutDir::TopDown);
	Ui::SetCanvasSpacing(3.f);
	Ui::SetTextColor(Color::Black);
	Ui::SetTextFont("p");
	Ui::SetButtonFont("h2");
	Ui::SetButtonPadding(5.f);
	Ui::SetButtonTextColor(Color::White);
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
