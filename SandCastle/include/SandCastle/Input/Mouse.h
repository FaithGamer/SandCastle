#pragma once
#include <SDL3/SDL.h>
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	enum class MouseButton : Uint8
	{
		Invalid = 0,
		Left = SDL_BUTTON_LEFT,
		Middle = SDL_BUTTON_MIDDLE,
		Right = SDL_BUTTON_RIGHT,
		X1 = SDL_BUTTON_X1,
		X2 = SDL_BUTTON_X2
	};

	std::string MouseButtonName(MouseButton mouseButton);
	MouseButton MouseButtonFromName(std::string name);
	/// @brief Get the mouse position in window coordinates
	Vec2f GetMousePosition();
}
