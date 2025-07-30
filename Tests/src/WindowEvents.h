#pragma once

#include <SandCastle/SandCastle.h>
using namespace SandCastle;

void OnResize(Vec2u size)
{
	LOG_INFO("Window resized. {0}, {1}", size.x, size.y);
}
void OnMinimize(bool minimized)
{
	LOG_INFO("Window minimized: {0}", minimized);
}
void OnFocus(bool focus)
{
	LOG_INFO("Window focus: {0}", focus);
}

void WindowEvents()
{
	Engine::Init();
	
	Window::GetResizeSignal()->AddListener(&OnResize);
	Window::GetMinimizedSignal()->AddListener(&OnMinimize);
	Window::GetFocusSignal()->AddListener(&OnFocus);

	Engine::Launch();
}
