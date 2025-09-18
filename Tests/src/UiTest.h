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
	void CreateSomeUi()
	{
		Ui::SetCanvasFrame("frame.png");
		Ui::SetFont("alata");

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

	}
	void CreateSquareWindow()
	{
		//The visual frame to use for every subsequent canvas
		Ui::SetCanvasFrame("frame.png");

		//Start a canvas:
		//By setting a dimension to non-zero, this makes it unstretchable in this dimension.
		//User must be aware that a canvas size that doesn't match the possible frame size (see fixedStep and minimumSize)
		//May result in some unexpected visual result.
		//Long story short: if you use a fixed step frame, use a canvas size that is a round multiple of that step.
		//If not, use a canvas size that is a minimum of double the sprite corner size of the frame.
		auto root = Ui::BeginCanvas(Vec2f(200, 0));
		//Center the canvas on screen
		root->SetPosition(Vec2f(0.f, 0.f));
		root->SetAnchor(Ui::Canvas::Anchor::MiddleCenter);
		//Elements inside this canvas will be added from top to bot and wrap from left to right.
		root->layoutDir = Ui::LayoutDir::TopDown;
		//The elements in this canvas will horizontally align to the center 
		root->layoutAlignH = Ui::LayoutAlignH::Center;
		//The space between the canvas border and its content
		root->SetBorder(5.f);
		//The space between elements inside the canvas.
		root->SetSpacing(20.f);
		//Use the font for titles.
		Ui::SetFont("title");
		//Draw the text.
		//Ui::Image("green30.png_0_0");
		Ui::Text("Square window.");

		//This canvas has no frame, only it's content is visible (false)
		auto squares = Ui::BeginCanvas(Vec2f(0, 0), false);
		squares->layoutDir = Ui::LayoutDir::LeftRight;
		squares->SetSpacing(2.f);
		Ui::Image("yellow10-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("blue40-20.png_0_0");
		Ui::Image("green30.png_0_0");
		Ui::Image("green30.png_0_0");

		Ui::Image("blue40-20.png_0_0");
		Ui::EndCanvas();

		//Ui::Image("blue40-20.png_0_0");
		Ui::BeginCanvas(Vec2f(0.f), false);
		Ui::SetFont("content");
		Ui::Text("Thanks for watching ");
		Ui::EndCanvas();
		//End the root canvas
		Ui::EndCanvas();
	}
	void MakeUpgradeUi()
	{
		Ui::SetCanvasFrame("frame.png");
		auto root = Ui::BeginCanvas(Vec2f(0, 0));
		root->SetSpacing(3.f);
		root->SetBorder(5.f);
		struct Upgrade
		{
			String name = "";
		};
		std::vector<Upgrade> upgrades;
		upgrades.push_back(Upgrade("Boost"));
		upgrades.push_back(Upgrade("Super boost"));
		upgrades.push_back(Upgrade("Stuff"));
		upgrades.push_back(Upgrade("Little stuff"));
		for (int i = 0; i < upgrades.size(); i++)
		{
			auto upRoot = Ui::BeginCanvas(Vec2f(190, 0), true);

			upRoot->layoutDir = Ui::LayoutDir::LeftRight;
			upRoot->layoutAlignV = Ui::LayoutAlignV::Center;
			auto upTitle = Ui::BeginCanvas(Vec2f(95, 0), false);
			upTitle->layoutAlignH = Ui::LayoutAlignH::Left;
			Ui::Text(upgrades[i].name);
			Ui::EndCanvas();
			auto upButton = Ui::BeginCanvas(Vec2f(95, 0), false);
			upButton->layoutAlignH = Ui::LayoutAlignH::Right;
			Ui::Image("blue40-20.png_0_0");
			Ui::EndCanvas();

			Ui::EndCanvas();
		}

		Ui::EndCanvas();
	}
	void CreateButtonWindow()
	{
		Ui::SetButtonFrame("btn.png");
		Ui::SetButtonFrameHover("btn_hover.png");
		Ui::SetButtonFramePressed("btn_pressed.png");
		Ui::SetCanvasFrame("frame.png");

		auto root = Ui::BeginCanvas(0, false);
		root->SetSpacing(6.7f);
		root->SetPosition(Vec2f(33, 70));
		Ui::Text("There is a button");
		auto btn = Ui::Button("Ok", Vec2f(8.f));
		btn->signal.Listen(&UiSys::OnClickBtn, this);
		Ui::EndCanvas();
	}
	void OnClickBtn(Ui::Elem* signal)
	{
		LOG_INFO("You clicked the button");
	}
	void Start() override
	{

		auto worldLayer = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = worldLayer;

		//Init UI
		Ui::MakeFrameTemplate("frame.png", false);
		Ui::MakeFrameTemplate("btn.png", false);
		Ui::MakeFrameTemplate("btn_pressed.png", false);
		Ui::MakeFrameTemplate("btn_hover.png", false);
		//Optionally change the ppu before making font
		//This can help to make the font px size 
		//to be 1:1 for a specific screen resolution
		Ui::GetWriter()->SetPPU(2.f); //Will be native at 1080p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		//Ui::GetWriter()->SetPPU(1.f); //Will be native at 360p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		Ui::MakeFont("alata-regular.ttf", "content", 12, .2f);
		Ui::MakeFont("alata-regular.ttf", "title", 20, .2f);

		//Create ui stuff
		//CreateSomeUi();
		//CreateSquareWindow();
		CreateButtonWindow();
		//MakeUpgradeUi();
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

class PosSys : public System
{
public:
	Vec2f dir;
	void Start()
	{
		auto inputs = Inputs::CreateInputMap("Player");
		auto cam = inputs->CreateDirectionalInput("Cam");
		cam->BindWASD();
		cam->signal.Listen(&PosSys::OnMoveCam, this);

		auto entt = Entity::CreateSprite();
		entt.gtr()->Move(-100, 100, 0);
	}
	void OnMoveCam(InputSignal* signal)
	{
		dir = signal->GetVec2f();

	}
	void Update()
	{
		float time = Time::Delta();
		if (dir.Magnitude() > 0.01f)
		{
			Vec2f offset = dir * time * 10.f;
			Camera::main->MoveWorld(offset);
		}
	}
	void OnImGui()
	{
		auto mouseUi = Ui::MousePos();
		auto mouseWorld = Mouse::GetWorldPos();
		auto mouseScreen = Mouse::GetPosition();
		auto camPos = Camera::main->GetPosition();
		auto worldToUi = Ui::WorldToUi(mouseWorld);
		auto uiToWorld = Ui::UiToWorld(mouseUi);
		auto drawPos = [](String label, Vec2f pos)
			{
				ImGui::Text(label.c_str());
				ImGui::Value("X", pos.x);
				ImGui::SameLine();
				ImGui::Text(" ");
				ImGui::SameLine();
				ImGui::Value("Y", pos.y);
				ImGui::Spacing();
			};

		ImGui::Begin("Positions");
		drawPos("Mouse Screen", mouseScreen);
		drawPos("Mouse UI", mouseUi);
		drawPos("Mouse World", mouseWorld);
		drawPos("World to UI", worldToUi);
		drawPos("UI to world", uiToWorld);
		drawPos("Camera", camPos);
		ImGui::End();
	}

};

void UiTest()
{
	Engine::Init();
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	Systems::Push<UiSys>();
	Systems::Push<PosSys>();
	Engine::Launch();
}
