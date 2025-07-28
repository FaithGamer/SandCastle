#pragma once

#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Input/Controller.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Serialization.h"

/// @brief Every possible bindings, can be used to set binding on an Input wich will ignore the adequate fields
namespace SandCastle
{
	//To do: serialization

	struct Button : Serializable
	{
		Button() {}
		Button(KeyScancode Key) : key(Key) {}
		Button(MouseButton Mouse) : mouse(Mouse) {}
		Button(ControllerButton Controller) : controller(Controller) {}
		Button(ControllerTrigger Trigger) : trigger(Trigger) {}

		//Serializable
		void Deserialize(Serialized& config) override;
		Serialized Serialize() override;

		MouseButton mouse = MouseButton::Invalid;
		KeyScancode key = KeyScancode::Unknown;// scancode is a physical position on the keyboard.
		// to retreive it's Keycode on the current keyboard layout, use the macro SDL_SCANCODE_TO_KEYCODE
		ControllerButton controller = ControllerButton::Invalid;
		ControllerTrigger trigger = ControllerTrigger::Undefined;

		bool operator==(const Button& obj) const
		{
			return mouse == obj.mouse && key == obj.key && controller == obj.controller && trigger == obj.trigger;
		}
	};

	/// @brief Hold the current state of a directional button
	struct DirectionalButton
	{
		DirectionalButton() {}
		DirectionalButton(Button Button, Vec2f Direction) : button(Button), direction(Direction) {}

		Button button;
		Vec2f direction = Vec2f(0, 0);
		bool pressed = false;
	};

	/// @brief Hold the current state of a directional stick
	struct DirectionalStick
	{
		DirectionalStick(){}
		DirectionalStick(ControllerStick Stick) : stick(Stick) {}

		ControllerStick stick;
		Vec2f currentDirection = Vec2f(0, 0);
	};

	struct Direction
	{
		Direction() {}
		Direction(std::vector<DirectionalButton> Buttons) : buttons(Buttons) {}
		Direction(ControllerStick Stick) : stick(Stick) {}

		std::vector<DirectionalButton> buttons;
		DirectionalStick stick;
	};

	////////////////
	/// Bindings ///
	////////////////

	struct DirectionalBindings
	{	
		//To do: optimize by having the bindings sorted by their event type
		std::vector<Direction> directions;
	};

	struct ButtonBindings : Serializable
	{
		//Serializable
		void Deserialize(Serialized& config) override;
		Serialized Serialize() override;

		std::vector<Button> buttons;
	};
}