#pragma once

#include <SandCastle/SandCastle.h>

using namespace SandCastle;

void OnFire(InputSignal* signal)
{
	LOG_INFO("Pew!");
}

void OnRightClickPressed(InputSignal* signal)
{
	if (signal->GetBool())
	{
		LOG_INFO("Right click pressed.");
	}
	else
	{
		LOG_INFO("Right click released.");
	}
}

void OnMiddleClickPressed(InputSignal* signal)
{
	if (signal->GetBool())
	{
		LOG_INFO("Middle click pressed.");
	}
	else
	{
		LOG_INFO("Middle click released.");
	}
}

void OnMove(InputSignal* signal)
{
	auto direction = signal->GetVec2f();

	LOG_INFO("Direction: [{0}, {1}]", direction.x, direction.y);
}

void MouseKeyboard()
{
	Engine::Init();

	//Creating an InputMap "map" that regroup as many inputs as desired.
	//This could be one control scheme among others or Player1 controls in a multiplayer game
	auto map = Inputs::CreateInputMap();

	//Create a button input (the input is kept inside "map", "fire" is a shared pointer)
	auto fire = map->CreateButtonInput("Fire");
	//Bind a mouse button to it.
	fire->BindMouse(MouseButton::Left);
	//Bind a keyboard scancode
	//Scancode means W is the position of the key on US keyboard (will automatically translate to Z for FR)
	fire->BindKey(KeyScancode::Space);
	//What function will be called when the input is triggered.
	fire->signal.AddListener(&OnFire);
	//Could also be listened by an object:
	//fire->signal.AddListener(&Class::OnFire, &object);

	//Create other inputs
	auto rightClick = map->CreateButtonInput("RightCLick");
	rightClick->BindMouse(MouseButton::Right);
	rightClick->signal.AddListener(&OnRightClickPressed);
	//Also triggers the event when releasing the button
	rightClick->SetSendSignalOnRelease(true);

	auto middleClick = map->CreateButtonInput("MiddleCLick");
	middleClick->BindMouse(MouseButton::Middle);
	middleClick->signal.AddListener(&OnMiddleClickPressed);
	middleClick->SetSendSignalOnRelease(true);

	//Create a directional input (it's signal will send a Vec2f direction)
	auto directions = map->CreateDirectionalInput("Directions");

	//Create buttons that each represent a direction
	std::vector<DirectionalButton> dirButtons;

	//Scancode means W is the position of the key on US keyboard (will automatically translate to Z for FR)
	dirButtons.emplace_back(DirectionalButton(Button(KeyScancode::W), Vec2f(0, 1)));
	dirButtons.emplace_back(DirectionalButton(Button(KeyScancode::A), Vec2f(-1, 0)));
	dirButtons.emplace_back(DirectionalButton(Button(KeyScancode::S), Vec2f(0, -1)));
	dirButtons.emplace_back(DirectionalButton(Button(KeyScancode::D), Vec2f(1, 0)));
	directions->BindButtons(dirButtons);

	//Bind a controller stick
	directions->BindStick(ControllerStick::Left);
	directions->signal.AddListener(&OnMove);

	Engine::Launch();
}
