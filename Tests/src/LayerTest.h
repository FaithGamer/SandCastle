#pragma once

#include <SandCastle.h>
using namespace SandCastle;

class LayerTestSys : public System
{
public:
	void Start() override
	{
		auto uiMat = Renderer2D::CreateMaterial(Assets::Get<Shader>("ui.shader"));
		uiMat->SetFloat("uDpi", 1 / 360.f);
		Camera::Constraints cons;
		Window::SetClearColor(Vec4f(1, 0, 0, 1));
		cons.SetDefault();
		Camera::main->SetConstraints(cons);
		auto l1 = Renderer2D::AddLayer("l1");
		//auto l3 = Renderer2D::AddLayer("l3");
		auto l2 = Renderer2D::AddLayer("l2");

		float scale = 3.f;

		auto t1 = Entity::CreateSprite("trollface.png_0_0");
		t1.gtr()->SetPosition(-50, 0, 5);
		t1.gtr()->SetScale(scale);
		t1.gc<SpriteRender>()->SetLayer(l2);
	//	t1.gc<SpriteRender>()->SetMaterial(uiMat->GetID());
		//t1.gc<SpriteRender>()->color = Vec4f(0, 0, 1, 1);

		auto t2 = Entity::CreateSprite("trollface.png_0_0");
		t2.gtr()->SetPosition(-0, 0, 0);
		t2.gtr()->SetScale(scale); 
		//t2.gc<SpriteRender>()->SetLayer(l2);
		//t2.gc<SpriteRender>()->color = Vec4f(0, 1, 0, 1);

	/*	auto t3 = Entity::CreateSprite("trollface.png_0_0");
		t3.gtr()->SetPosition(6, 0, 5);
		t3.gtr()->SetScale(scale);
		t3.gc<SpriteRender>()->SetLayer(l2);*/
		//t3.gc<SpriteRender>()->color = Vec4f(1, 0, 0, 1);
	}

};

void LayerTest()
{
	Engine::Init();
	Systems::Push<LayerTestSys>();
	Engine::Launch();
}
