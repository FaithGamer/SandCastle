#include "SpriteExportTool.h"
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
#include "RateTest.h"
#include "SoundTest.h"
#include "MaskTest.h"
#include "PhysicsTest.h"
#include "PhysicsPlayground.h"
#include "PhysicsBench.h"

using namespace SandCastle;

int main()
{
	//SpriteExportTool();
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
	//RateTest();
	//SoundTest();
	//MaskTest();
	PhysicsTest(); //headless assertions
	//PhysicsPlayground(); //interactive visual test
	PhysicsBench(); //performance / limitations stress test
}
