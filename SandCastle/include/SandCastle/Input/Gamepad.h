#pragma once
#include <SDL3/SDL.h>

namespace SandCastle
{
	namespace Gamepad
	{
		enum class Axis
		{

			Invalid = SDL_GAMEPAD_AXIS_INVALID,
			LeftX = SDL_GAMEPAD_AXIS_LEFTX,
			LeftY = SDL_GAMEPAD_AXIS_LEFTY,
			RightX = SDL_GAMEPAD_AXIS_RIGHTX,
			RightY = SDL_GAMEPAD_AXIS_RIGHTY,
			TriggerLeft = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
			TriggerRight = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
			AxisMax = SDL_GAMEPAD_AXIS_COUNT,

		};

		typedef enum
		{
			Invalid = SDL_GAMEPAD_BUTTON_INVALID,
			South = SDL_GAMEPAD_BUTTON_SOUTH,
			East = SDL_GAMEPAD_BUTTON_EAST,
			West = SDL_GAMEPAD_BUTTON_WEST,
			North = SDL_GAMEPAD_BUTTON_NORTH,
			Back = SDL_GAMEPAD_BUTTON_BACK,
			Guide = SDL_GAMEPAD_BUTTON_GUIDE,
			Start = SDL_GAMEPAD_BUTTON_START,
			LeftStick = SDL_GAMEPAD_BUTTON_LEFT_STICK,
			RightStick = SDL_GAMEPAD_BUTTON_RIGHT_STICK,
			LeftShoulder = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
			RightShoulder = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
			PadUp = SDL_GAMEPAD_BUTTON_DPAD_UP,
			PadDown = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
			PadLeft = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
			PadRight = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
			Misc1 = SDL_GAMEPAD_BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
			Paddle1 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,  /* Xbox Elite paddle P1 */
			Paddle2 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,  /* Xbox Elite paddle P3 */
			Paddle3 = SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,  /* Xbox Elite paddle P2 */
			Paddle4 = SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,  /* Xbox Elite paddle P4 */
			TouchPad = SDL_GAMEPAD_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
			ButtonMax = SDL_GAMEPAD_BUTTON_COUNT
		} Button;

		enum class Trigger
		{
			Undefined = SDL_GAMEPAD_AXIS_INVALID,
			Left = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
			Right = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
		};

		/// @brief Two controller axis to identify a Stick
		struct Stick
		{
			Stick();
			constexpr Stick(Axis x, Axis y);

			constexpr bool operator==(const Stick& other) const
			{
				return xAxis == other.xAxis && yAxis == other.yAxis;
			}

			Axis xAxis;
			Axis yAxis;

			static const Stick None;
			static const Stick Left;
			static const Stick Right;
		};

		std::string ButtonName(Button button);
		std::string StickName(Stick stick);
		std::string TriggerName(Trigger trigger);

		SDL_GamepadButton ButtonFromName(std::string name);
		Stick StickFromName(std::string name);
		Trigger TriggerFromName(std::string name);
	}
}