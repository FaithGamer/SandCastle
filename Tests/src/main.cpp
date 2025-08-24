#include "Launch.h"
#include "DrawSprite.h"
#include "DrawAnimation.h"
#include "BasicInputs.h"
#include "Rotation.h"
#include "WindowEvents.h"
#include "Serialization.h"
#include "Benchmark1.h"
#include "Benchmark2.h"
#include "Delegates.h"
#include "Signals.h"
#include "FontTest.h"

using namespace SandCastle;

class LayerTest : public System
{
public:
	void Start() override
	{
		auto l1 = Renderer2D::AddLayer("l1");
		auto l2 = Renderer2D::AddLayer("l2");
		auto l3 = Renderer2D::AddLayer("l3");

		float scale = 0.5;

		auto t1 = Entity::CreateSprite("trollface.png_0_0");
		t1.gtr()->SetPosition(-6, 0, -5);
		t1.gtr()->SetScale(scale);
		t1.gc<SpriteRender>()->SetLayer(l3);
		t1.gc<SpriteRender>()->color = Vec4f(0, 0, 1, 1);

		auto t2 = Entity::CreateSprite("trollface.png_0_0");
		t2.gtr()->SetPosition(-0, 0, 0);
		t2.gtr()->SetScale(scale);
		t2.gc<SpriteRender>()->SetLayer(l2);
		t2.gc<SpriteRender>()->color = Vec4f(0, 1, 0, 1);

		auto t3 = Entity::CreateSprite("trollface.png_0_0");
		t3.gtr()->SetPosition(6, 0, 5);
		t3.gtr()->SetScale(scale);
		t3.gc<SpriteRender>()->SetLayer(l1);
		t3.gc<SpriteRender>()->color = Vec4f(1, 0, 0, 1);
	}

};
int main()
{
	//Launch();
	//DrawSprite();
	//DrawAnimation();
	//BasicInputs();
	//Rotation();
	//WindowEvents();
	//Serialization();
	Benchmark2();
	//Delegates();
	//Signals();
	//FontTest();

	/*Engine::Init();
	Systems::Push<LayerTest>();
	Engine::Launch();*/
}
