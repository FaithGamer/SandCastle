#pragma once

#include <SandCastle.h>
using namespace SandCastle;

struct Txt
{
	int tag;
};
struct Control
{
	int tag;
};
class UiSys : public System
{
public:
	struct Oscillate
	{
		int tag;
	};

	void CreateSomeUi()
	{

		Ui::SetCanvasFrame("frame.png");
		Ui::SetFont("p");

		//Spacing:

		//Ui::SetCanvasMargin(4.f);
		auto canvas = Ui::BeginCanvas(Vec2f(150.f, 0));
		canvas->layoutDir = UiCanvas::LayoutDir::LeftRight;
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
		Ui::SetTextAlign(TextAlign::Center);
		//Center the canvas on screen
		root->SetPosition(Vec2f(0.f, 0.f));
		root->SetAnchor(UiCanvas::Anchor::MiddleCenter);
		//Elements inside this canvas will be added from top to bot and wrap from left to right.
		root->layoutDir = UiCanvas::LayoutDir::TopDown;
		//The elements in this canvas will horizontally align to the center 
		root->layoutAlignH = UiCanvas::LayoutAlignH::Center;
		//The space between the canvas border and its content
		root->SetBorder(5.f);
		//The space between elements inside the canvas.
		root->SetSpacing(20.f);
		//Use the font for titles.
		Ui::SetFont("h1");
		//Draw the text.
		//Ui::Image("green30.png_0_0");
		Ui::Text("Square window.");

		//This canvas has no frame, only it's content is visible (false)
		auto squares = Ui::BeginCanvas(Vec2f(0, 0), false);
		squares->layoutDir = UiCanvas::LayoutDir::LeftRight;
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
		Ui::SetFont("p");
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

			upRoot->layoutDir = UiCanvas::LayoutDir::LeftRight;
			upRoot->layoutAlignV = UiCanvas::LayoutAlignV::Center;
			auto upTitle = Ui::BeginCanvas(Vec2f(95, 0), false);
			upTitle->layoutAlignH = UiCanvas::LayoutAlignH::Left;
			Ui::Text(upgrades[i].name);
			Ui::EndCanvas();
			auto upButton = Ui::BeginCanvas(Vec2f(95, 0), false);
			upButton->layoutAlignH = UiCanvas::LayoutAlignH::Right;
			Ui::Image("blue40-20.png_0_0");
			Ui::EndCanvas();

			Ui::EndCanvas();
		}

		Ui::EndCanvas();
	}
	UiCanvas* btnCanvas;
	UiTxt* updateText;
	UiBtn* btn;
	String someStr = "Wakanda!";
	int btnCounter = 0;
	void CreateButtonWindow()
	{
		btnCanvas = Ui::BeginCanvas(0);
		btnCanvas->SetBorder(5.f);
		btnCanvas->SetSpacing(6.f);
		btnCanvas->SetPosition(Vec2f(33, 70));
		btnCanvas->SetAnchor(UiCanvas::Anchor::MiddleCenter);
		btnCanvas->SetPosition(Vec2f(0, 0));
		btnCanvas->layoutAlignH = UiCanvas::LayoutAlignH::Center;
		Ui::SetFont("h1");
		Ui::SetTextColor(Color::Black);
		updateText = Ui::Text("There is a button");
		Ui::SetTextColor(Color::White);
		Ui::SetFont("h2");
		btn = Ui::Button("Ok", Vec2f(8.f));
		btn->ListenClickReleased(&UiSys::OnClickBtn, this);
		btn->SetColor(Color(50, 243, 60, 255));
		Ui::SetFont("p");
		Ui::SetTextColor(Color::Black);
		Ui::SetTextAlign(TextAlign::Center);
		Ui::Text("{0}", 100.f, &someStr);

		Ui::EndCanvas();
		btnCanvas->root.AddComponent<Control>();
	}
	void OnClickBtn(UiElem* signal)
	{
		btnCounter++;
		int i = Random::Range(0, 3);
		switch (i)
		{
		case 0:
			someStr = "POPPOPOPOPOPOPO";
			break;
		case 1:
			someStr = "lol";
			break;
		case 2:
			someStr = "Mouem";
			break;
		}
		Ui::UpdateText(updateText, "You clicked " + std::to_string(btnCounter) + " times.");
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

		Ui::SetButtonFrame("btn.png");
		Ui::SetButtonFrameHover("btn_hover.png");
		Ui::SetButtonFramePressed("btn_pressed.png");
		Ui::SetCanvasFrame("frame.png");

		//Optionally change the ppu before making font
		//This can help to make the font px size 
		//to be 1:1 for a specific screen resolution
		Ui::GetWriter()->SetPPU(1.f); //Will be native at 1080p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		//Ui::GetWriter()->SetPPU(1.f); //Will be native at 360p with the default settings (ui dpi 1.f/360.f for pixel perfect game)
		/*Ui::MakeFont("NotoSansJP-Regular.ttf", "p", 10, 0);
		Ui::MakeFont("NotoSansJP-Regular.ttf", "h2", 12, 0);
		Ui::MakeFont("NotoSansJP-Regular.ttf", "h1", 16, 0);*/
		Ui::MakeFont("ark-pixel-10px-proportional-latin.ttf", "p", 10, 0);
		Ui::MakeFont("ark-pixel-12px-proportional-latin.ttf", "h2", 12, 0);
		Ui::MakeFont("ark-pixel-16px-proportional-latin.ttf", "h1", 16);
		//Create ui stuff
		//CreateSomeUi();
		//CreateSquareWindow();
	//	CreateButtonWindow();
	//	MakeUpgradeUi();
		//DeleteUi();
		//BenchmarkUi();
		//BenchmarkUi2();
		SpriteBenchmark();
	}
	void BenchmarkUi()
	{
		int count = 30;
		for (int i = 0; i < count; i++)
		{
			Vec2f p;
			p.x = Random::Range(-320, 320);
			p.y = Random::Range(-180, 180);
			auto r = Ui::BeginCanvas();
			r->SetPosition(p);
			Ui::Text("This is an average UI with a couple stuff inside.Some text of course, this is the most important, because there will be quite a lot of text in most of UI.", 200.f);
			Ui::EndCanvas();
		}
		LOG_INFO("entities: {0}", Entity::Count());
	}
	void BenchmarkUi2()
	{
		int count = 30;
		for (int i = 0; i < count; i++)
		{
			Vec2f p;
			p.x = Random::Range(-320, 320);
			p.y = Random::Range(-180, 180);
			auto r = Ui::BeginCanvas();
			r->SetPosition(p);
			Ui::BeginCanvas();
			Ui::Text("This is an average UI with a couple stuff inside.Some text of course, this is the most important, because there will be quite a lot of text in most of UI.", 200.f);
			Ui::EndCanvas();
			Ui::EndCanvas();
		}
		LOG_INFO("entities: {0}", Entity::Count());
	}
	void SpriteBenchmark()
	{
		int count = 3800;
		for (int i = 0; i < count; i++)
		{
			Vec3f p;
			p.x = Random::Range(-320, 320);
			p.y = Random::Range(-180, 180);
			auto parent = Entity::Create();
			auto tr = parent.AddComponent<Transform>();
			tr->Move(p);
			auto e = Entity::CreateSprite("trollface.png_0_0");
			parent.AddChild(e);
			//e.gtr()->SetPosition(p);
		}
	}
	void DeleteUi()
	{
		auto root = Ui::BeginCanvas();
		root->layoutAlignH = UiCanvas::LayoutAlignH::Center;
		root->SetPosition(Vec2f(-320, 180));
		root->SetBorder(5.f);
		Ui::SetTextColor(Color::Black);
		Ui::Text("Click the buttons.");
		auto btnCan = Ui::BeginCanvas();
		btnCan->layoutDir = UiCanvas::LayoutDir::LeftRight;
		Ui::SetTextColor(Color::White);
		auto btn1 = Ui::Button("Create", Vec2f(5, 5));
		btn1->ListenClickReleased(&UiSys::OnClickCreate, this);
		auto btn2 = Ui::Button("Delete", Vec2f(5, 5));
		btn2->ListenClickReleased(&UiSys::OnClickDelete, this);
		Ui::EndCanvas();
		Ui::EndCanvas();
	}
	void OnClickDelete(UiElem* signal)
	{
		for (auto& created : createds)
		{
			auto elem = (UiElem*)created;
			Ui::Destroy(elem);
		}
		createds.clear();
	}
	std::list<UiCanvas*> createds;
	void OnClickCreate(UiElem* signal)
	{
		auto created = Ui::BeginCanvas();
		Ui::SetTextColor(Color::Black);
		created->SetPosition(Vec2f(Random::Range(-200, 200), Random::Range(-100, 100)));
		Ui::Text((const char*)u8"This canvas ô was created!");
		auto btn = Ui::Button((const char*)u8"無量大数 Delete This", Vec2f(5));
		btn->ListenClickReleased(&UiSys::DeleteThis, this);
		Ui::EndCanvas();

		createds.emplace_back(created);
	}
	void DeleteThis(UiElem* elem)
	{
		Ui::Destroy(elem);
	}
	void Update() override
	{
		OscillateUpdt();
		TextUpdate();
		//CreateDelete();
	}
	void CreateDelete()
	{
		//No memory leak has been reported with this test
		auto c = Ui::BeginCanvas();
		Ui::Text("Salutations dceci va disparaître aussi tôt qu'apparut");
		Ui::EndCanvas();
		//createds.emplace_back(c);
		Ui::Destroy(c);
	}
	void TextUpdate()
	{
		static float timer = 0.f;
		static int T = 0;
		timer += Time::Delta();
		static String complete = "This is a long story that will unravel with the flow of life. Slowly, but surely, it tells you something you didn't expected.";
		if (timer >= 0.1f)
		{
			timer -= 0.1f;
			T++;
			if (T >= complete.size())
			{
				T = 0;
			}
		}
		someStr = complete.substr(0, T);
	}
	void OscillateUpdt()
	{
		static float timer = 0.f;
		auto delta = Time::Delta();
		timer += delta;
		float ypos = std::sin(timer) * 100.f;
		float xpos = ypos;

		auto view = Entity::View<Oscillate, Transform>();
		view.each([&](Oscillate& t, Transform& tr)
			{
				tr.SetPosition(xpos, 0, 0);
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
		/*float time = Time::Delta();
		if (dir.Magnitude() > 0.01f)
		{
			Vec2f offset = dir * time * 10.f;
			Camera::main->MoveWorld(offset);
		}*/
		float speed = 10.f;
		auto view = Entity::View<Transform, Control>();
		view.each([&](Transform& tr, Control& cn)
			{
				Vec2f offset = dir * Time::Delta() * speed;
				tr.Move(offset);
			});

	}
	void OnImGui()
	{

		return;
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
