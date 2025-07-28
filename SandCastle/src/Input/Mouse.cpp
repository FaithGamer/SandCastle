#include "pch.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Mouse.h"

namespace SandCastle
{

	std::string MouseButtonName(MouseButton mouseButton)
	{
		switch (mouseButton)
		{
		case MouseButton::Left:			return "LeftClick";		break;
		case MouseButton::Right:		return "RightClick";	break;
		case MouseButton::Middle:		return "MiddleClick";	break;
		case MouseButton::X1:			return "X1Click";		break;
		case MouseButton::X2:			return "X2Click";		break;
		default:
			LOG_ERROR("Trying to get the name of an incorrect Mouse Button: " + std::to_string((int)mouseButton));
			return "UnknownMouseButton";
			break;
		}
	}

	MouseButton MouseButtonFromName(std::string name)
	{
		if (name == "LeftClick")
			return MouseButton::Left;
		if (name == "RightClick")
			return MouseButton::Right;
		if (name == "MiddleClick")
			return MouseButton::Middle;
		if (name == "X1Click")
			return MouseButton::X1;
		if (name == "X2Click")
			return MouseButton::X2;

		LOG_ERROR("Unknown Mouse Button name: " + name);
		return MouseButton::Invalid;
	}

	Vec2f GetMousePosition()
	{
		float x;
		float y;
		SDL_GetMouseState(&x, &y);
		return Vec2f(x, y);
	}
}