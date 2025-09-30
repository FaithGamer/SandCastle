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
#include "PixPerfectGame.h"
#include "SubTexture.h"
#include "FrameTest.h"
#include "DepthBlendTest.h"
#include "UiPosTest.h"
#include "UiTest2.h"

using namespace SandCastle;

int main()
{
	//Launch();
	//DrawSprite();
	//DrawAnimation();
	//BasicInputs();
	//Rotation();
	//WindowEvents();
	//Serialization();
	//Benchmark1();
	//Benchmark2(); //~190 -> 210
	//Delegates();
	//Signals();
	//FontTest();
	//LayerTest();
	//PixPerfectGame();
	//SubTexture();
	//FrameTest();
	//DepthBlendTest();
	//UiTest();
	//UiTest2();

	Engine::Init();

	auto wri = Ui::GetWriter();
	auto fontFR = wri->MakeFont("alata-regular.ttf", 10);
	auto fontJA = wri->MakeFont("NotoSansJP-Regular.ttf", 10);

	wri->NameFont(fontFR, "name", { "fr" });
	wri->NameFont(fontJA, "name", { "ja" });

	wri->UseFont("name", "ja");
	auto s = wri->Write("Salut");
	s.root.gtr()->Move(0, -50, 0);
	wri->UseFont("name", "fr");
	wri->Write("Salut");

	Engine::Launch();
}
