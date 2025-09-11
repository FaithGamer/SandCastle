#pragma once
#include <SandCastle.h>
using namespace SandCastle;

class DepthSys : public System
{
public:
	Entity red;
	Entity green;
	void Start() override
	{
		auto world = Renderer2D::AddLayer("world");
		SpriteRender::defaultLayer = world;

		red = Entity::CreateSprite("red_depth.png_0_0");
		green = Entity::CreateSprite("green_depth.png_0_0");

		green.gtr()->Move(3, 0, 0);
		red.gtr()->Move(-3, 0, 0);

	}
	void OnImGui() override
	{
		static bool sort = false;
		ImGui::Begin("window");
		if (ImGui::Checkbox("z sort", &sort))
		{
			Renderer2D::SetLayerSortZ(Renderer2D::GetLayerId("world"), sort);
		}
		static float greenZ = 0.f;
		if (ImGui::SliderFloat("green z", &greenZ, -5, 5))
		{
			green.gtr()->SetPosition(3, 0, greenZ);
		}
		static float redZ = 0.f;
		if (ImGui::SliderFloat("red z", &redZ, -5, 5))
		{
			red.gtr()->SetPosition(-3, 0, redZ);
		}
		static float alpha = 0.f;
		if (ImGui::SliderFloat("alpha discard", &alpha, 0.f, 1.f))
		{
			Renderer2D::GetDefaultQuadMaterial()->SetFloat("uDiscardAlpha", alpha);
		}
		ImGui::End();
	}
};
void DepthBlendTest()
{
	Engine::Init();
	Systems::Push<DepthSys>();
	Engine::Launch();
}
