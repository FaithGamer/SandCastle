#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Controller.h"


namespace SandCastle
{
	const ControllerStick ControllerStick::None = ControllerStick(ControllerAxis::Invalid, ControllerAxis::Invalid);
	const ControllerStick ControllerStick::Left = ControllerStick(ControllerAxis::LeftX, ControllerAxis::LeftY);
	const ControllerStick ControllerStick::Right = ControllerStick(ControllerAxis::RightX, ControllerAxis::LeftY);

	ControllerStick::ControllerStick()
		: xAxis(ControllerAxis::Invalid), yAxis(ControllerAxis::Invalid)
	{
	}

	constexpr ControllerStick::ControllerStick(ControllerAxis x, ControllerAxis y)
		: xAxis(x), yAxis(y)
	{
	}

	std::string ControllerButtonName(ControllerButton button)
	{
		if ((int)button < 0 || (int)button >= SDL_GAMEPAD_BUTTON_COUNT)
		{
			LOG_ERROR("Trying to get the name of an incorrect Controller Button: " + std::to_string((int)button));
			return "UnknowControllerButton";
		}
		return std::string(SDL_GetGamepadStringForButton((SDL_GamepadButton)button));
	}

	std::string ControllerStickName(ControllerStick stick)
	{
		if (stick == ControllerStick::Left)
		{
			return "LeftStick";
		}
		else if (stick == ControllerStick::Right)
		{
			return "RightStick";
		}
		LOG_ERROR("Controller axis left/right mismatch, cannot identify a ControllerStick");
		return "UnknownStick";
	}

	std::string ControllerTriggerName(ControllerTrigger trigger)
	{
		if (trigger == ControllerTrigger::Left)
		{
			return "LeftTrigger";
		}
		else if (trigger == ControllerTrigger::Right)
		{
			return "RightTrigger";
		}
		LOG_ERROR("Trying to get the name of an Invalid Controller Trigger");
		return "ErrorType";
	}



	SDL_GamepadButton ControllerButtonFromName(std::string name)
	{
		SDL_GamepadButton button = SDL_GetGamepadButtonFromString(name.c_str());
		if (button == SDL_GAMEPAD_BUTTON_INVALID)
		{
			LOG_ERROR("Unknown Controller button name:  " + LogSDLError(""));
		}
		return button;
	}


	ControllerStick ControllerStickFromName(std::string name)
	{
		if (name == "LeftStick")
		{
			return ControllerStick::Left;
		}
		else if (name == "RightStick")
		{
			return ControllerStick::Right;
		}
		LOG_ERROR("Unknown Controller Stick name: " + name);
		return ControllerStick();
	}


	ControllerTrigger ControllerTriggerFromName(std::string name)
	{
		if (name == "LeftTrigger")
		{
			return ControllerTrigger::Left;
		}
		else if (name == "RightTrigger")
		{
			return ControllerTrigger::Right;
		}
		LOG_ERROR("Unknown Controller Trigger name: " + name);
		return ControllerTrigger::Undefined;
	}

}