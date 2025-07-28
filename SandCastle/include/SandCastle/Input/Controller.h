#pragma once
#include <SDL3/SDL.h>

namespace SandCastle
{
	enum class ControllerAxis
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
		A = SDL_GAMEPAD_BUTTON_SOUTH,
		B = SDL_GAMEPAD_BUTTON_EAST,
		X = SDL_GAMEPAD_BUTTON_WEST,
		Y = SDL_GAMEPAD_BUTTON_NORTH,
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
	} ControllerButton;

	enum class ControllerTrigger
	{
		Undefined = SDL_GAMEPAD_AXIS_INVALID,
		Left = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
		Right = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
	};

	/// @brief Two controller axis to identify a Stick
	struct ControllerStick
	{
		ControllerStick();
		constexpr ControllerStick(ControllerAxis x, ControllerAxis y);

		constexpr bool operator==(const ControllerStick& other) const
		{
			return xAxis == other.xAxis && yAxis == other.yAxis;
		}

		ControllerAxis xAxis;
		ControllerAxis yAxis;

		static const ControllerStick None;
		static const ControllerStick Left;
		static const ControllerStick Right;
	};

	std::string ControllerButtonName(ControllerButton button);
	std::string ControllerStickName(ControllerStick stick);
	std::string ControllerTriggerName(ControllerTrigger trigger);

	SDL_GamepadButton ControllerButtonFromName(std::string name);
	ControllerStick ControllerStickFromName(std::string name);
	ControllerTrigger ControllerTriggerFromName(std::string name);
}