#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Gamepad.h"


namespace SandCastle
{
	namespace Gamepad
	{
		const Stick Stick::None = Stick(Axis::Invalid, Axis::Invalid);
		const Stick Stick::Left = Stick(Axis::LeftX, Axis::LeftY);
		const Stick Stick::Right = Stick(Axis::RightX, Axis::LeftY);

		Stick::Stick()
			: xAxis(Axis::Invalid), yAxis(Axis::Invalid)
		{
		}

		constexpr Stick::Stick(Axis x, Axis y)
			: xAxis(x), yAxis(y)
		{
		}

		std::string ButtonName(Button button)
		{
			if ((int)button < 0 || (int)button >= SDL_GAMEPAD_BUTTON_COUNT)
			{
				LOG_ERROR("Trying to get the name of an incorrect Gamepad Button: " + std::to_string((int)button));
				return "UnknowGamepadButton";
			}
			return std::string(SDL_GetGamepadStringForButton((SDL_GamepadButton)button));
		}

		std::string StickName(Stick stick)
		{
			if (stick == Stick::Left)
			{
				return "LeftStick";
			}
			else if (stick == Stick::Right)
			{
				return "RightStick";
			}
			LOG_ERROR("Gamepad axis left/right mismatch, cannot identify a Stick");
			return "UnknownStick";
		}

		std::string TriggerName(Trigger trigger)
		{
			if (trigger == Trigger::Left)
			{
				return "LeftTrigger";
			}
			else if (trigger == Trigger::Right)
			{
				return "RightTrigger";
			}
			LOG_ERROR("Trying to get the name of an Invalid Gamepad Trigger");
			return "ErrorType";
		}



		SDL_GamepadButton ButtonFromName(std::string name)
		{
			SDL_GamepadButton button = SDL_GetGamepadButtonFromString(name.c_str());
			if (button == SDL_GAMEPAD_BUTTON_INVALID)
			{
				LOG_ERROR("Unknown Gamepad button name:  " + LogSDLError(""));
			}
			return button;
		}


		Stick StickFromName(std::string name)
		{
			if (name == "LeftStick")
			{
				return Stick::Left;
			}
			else if (name == "RightStick")
			{
				return Stick::Right;
			}
			LOG_ERROR("Unknown Gamepad Stick name: " + name);
			return Stick();
		}


		Trigger TriggerFromName(std::string name)
		{
			if (name == "LeftTrigger")
			{
				return Trigger::Left;
			}
			else if (name == "RightTrigger")
			{
				return Trigger::Right;
			}
			LOG_ERROR("Unknown Gamepad Trigger name: " + name);
			return Trigger::Undefined;
		}
	}
}