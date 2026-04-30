#pragma once
#include <SDL3/SDL.h>
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	/// @brief Mouse-related enums and helpers.
	namespace Mouse
	{
		/// @brief Identifier for each mouse button supported by SDL.
		enum class Button : Uint8
		{
			Invalid = 0,
			Left = SDL_BUTTON_LEFT,
			Middle = SDL_BUTTON_MIDDLE,
			Right = SDL_BUTTON_RIGHT,
			X1 = SDL_BUTTON_X1,
			X2 = SDL_BUTTON_X2
		};

		/// @brief Human-readable name of a mouse button (e.g. "Left").
		std::string ButtonName(Button mouseButton);
		/// @brief Reverse of ButtonName.
		Button ButtonFromName(std::string name);
		/// @brief Get the mouse position in window coordinates
		Vec2f GetPosition();
		/// @brief Mouse position projected into world space using Camera::main.
		Vec3f GetWorldPos();
	}
}
