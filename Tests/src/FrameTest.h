#pragma once
#include <SandCastle.h>

using namespace SandCastle;

void FrameTest()
{
	Engine::Init();

	Ui ui;
	auto e = ui.MakeFrameSprites(0, Vec2f(140, 140), "frame.png", true);
	e.GetComponent<Transform>()->SetPosition(0, 0, 10);

	Engine::Launch();
}

