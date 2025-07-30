#pragma once

#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Input/Gamepad.h"
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
		Button(Key::Scancode Key) : key(Key) {}
		Button(Mouse::Button Mouse) : mouse(Mouse) {}
		Button(Gamepad::Button Gamepad) : controller(Gamepad) {}
		Button(Gamepad::Trigger Trigger) : trigger(Trigger) {}

		//Serializable
		void Deserialize(Serialized& config) override;
		Serialized Serialize() override;

		Mouse::Button mouse = Mouse::Button::Invalid;
		Key::Scancode key = Key::Scancode::Unknown;// scancode is a physical position on the keyboard.
		// to retreive it's Keycode on the current keyboard layout, use the macro SDL_SCANCODE_TO_KEYCODE
		Gamepad::Button controller = Gamepad::Button::Invalid;
		Gamepad::Trigger trigger = Gamepad::Trigger::Undefined;

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
		DirectionalStick(Gamepad::Stick Stick) : stick(Stick) {}

		Gamepad::Stick stick;
		Vec2f currentDirection = Vec2f(0, 0);
	};

	struct Direction
	{
		Direction() {}
		Direction(std::vector<DirectionalButton> Buttons) : buttons(Buttons) {}
		Direction(Gamepad::Stick Stick) : stick(Stick) {}

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