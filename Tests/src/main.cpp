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
#include "LayerTest.h"

using namespace SandCastle;


struct Tag
{
	int t;
};
class MoveS : public System
{
	void Start() override
	{

	}
	void Update() override
	{
		auto delta = Time::Delta();

		static float timer = 0.f;
		timer += delta*0.1f;

		float x = std::sin(timer) * 100.f;

		auto view = Entity::View<Tag, Transform>();
		view.each([&](Tag& t, Transform& tr)
			{
				tr.SetPosition(x, 0, 0);
			});
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
	//Benchmark2();
	//Delegates();
	//Signals();
	//FontTest();
	//LayerTest();

	Engine::Init();
	Systems::GetMainCamera()->zoom = 0.00555555f;
	auto e = Entity::CreateSprite("360.png_0_0");
	e.adc<Tag>();
	Systems::Push<MoveS>();
	Engine::Launch();
}
