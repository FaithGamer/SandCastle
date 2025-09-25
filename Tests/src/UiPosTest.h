#pragma once

#include <SandCastle.h>
#include <imgui/imgui.h>
using namespace SandCastle;


struct Control
{
	int tag;
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

void UiPosTest()
{
	Engine::Init();
	Camera::Constraints cons;
	cons.SetDefault();
	Camera::main->SetConstraints(cons);
	Systems::Push<PosSys>();
	Engine::Launch();
}
