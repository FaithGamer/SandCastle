#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Mouse.h"

namespace SandCastle
{
	namespace Mouse
	{
		std::string ButtonName(Button mouseButton)
		{
			switch (mouseButton)
			{
			case Button::Left:			return "LeftClick";		break;
			case Button::Right:		return "RightClick";	break;
			case Button::Middle:		return "MiddleClick";	break;
			case Button::X1:			return "X1Click";		break;
			case Button::X2:			return "X2Click";		break;
			default:
				LOG_ERROR("Trying to get the name of an incorrect Mouse Button: " + std::to_string((int)mouseButton));
				return "UnknownButton";
				break;
			}
		}

		Button ButtonFromName(std::string name)
		{
			if (name == "LeftClick")
				return Button::Left;
			if (name == "RightClick")
				return Button::Right;
			if (name == "MiddleClick")
				return Button::Middle;
			if (name == "X1Click")
				return Button::X1;
			if (name == "X2Click")
				return Button::X2;

			LOG_ERROR("Unknown Mouse Button name: " + name);
			return Button::Invalid;
		}

		Vec2f GetPosition()
		{
			float x;
			float y;
			SDL_GetMouseState(&x, &y);
			return Vec2f(x, y);
		}
	}
}