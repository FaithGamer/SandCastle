#pragma once

#include <SandCastle.h>
using namespace SandCastle;

class LayerTestSys : public System
{
public:
	void Start() override
	{
		
		auto l1 = Renderer2D::AddLayer("l1");
		auto l2 = Renderer2D::AddLayer("l2");
		auto l3 = Renderer2D::AddLayer("l3");

		float scale = .5f;

		auto t1 = Entity::CreateSprite("trollface.png_0_0");
		t1.gtr()->SetPosition(-5, 0, 5);
		t1.gtr()->SetScale(scale);
		t1.gc<SpriteRender>()->SetLayer(l1);
		t1.gc<SpriteRender>()->color = Color(255, 0, 0, 255);

		auto t2 = Entity::CreateSprite("trollface.png_0_0");
		t2.gtr()->SetPosition(-0, 0, 10);
		t2.gtr()->SetScale(scale); 
		t2.gc<SpriteRender>()->SetLayer(l2);
		t2.gc<SpriteRender>()->color = Vec4f(0, 1, 0, 1);

		auto t3 = Entity::CreateSprite("trollface.png_0_0");
		t3.gtr()->SetPosition(5, 0, -5);
		t3.gtr()->SetScale(scale);
		t3.gc<SpriteRender>()->SetLayer(l3);
		t3.gc<SpriteRender>()->color = Vec4f(0, 0, 1, 1);
	}
};

void LayerTest()
{
	Engine::Init();
	Systems::Push<LayerTestSys>();
	Engine::Launch();
}
