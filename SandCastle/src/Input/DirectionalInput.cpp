#include "pch.h"
#include "SandCastle/Input/DirectionalInput.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{
	DirectionalInput::DirectionalInput(std::string name)
		: m_name(name), m_triggerDeadzone(0.1f), m_stickDeadzone(0.1f), m_mouseWheel(false)
	{
	}

	void DirectionalInput::SetBindings(const DirectionalBindings& bindings)
	{
		m_bindings = bindings;
	}

	std::string DirectionalInput::GetName() const
	{
		return m_name;
	}

	InputType DirectionalInput::GetType() const
	{
		return InputType::Directional;
	}

	void DirectionalInput::RemoveAllBindings()
	{
		m_bindings.directions.clear();
	}

	void DirectionalInput::BindStick(Gamepad::Stick stick)
	{
		if (HaveBinding(stick))
		{
			LOG_WARN("DirectionalInput::BindStick -> bindings already have the stick, no bindings added.");
			return;
		}
		m_bindings.directions.push_back(Direction(stick));
		UpdateEventListened();
	}

	void DirectionalInput::BindButtons(std::vector<DirectionalButton> buttons)
	{
		if (HaveBinding(buttons))
		{
			LOG_WARN("DirectionalInput::BindButtons -> bindings already have one or more of the buttons, no bindings added.");
			return;
		}
		m_bindings.directions.push_back(Direction(buttons));
		UpdateEventListened();
	}

	void DirectionalInput::SetMouseWheel(bool mouseWheel)
	{
		m_mouseWheel = mouseWheel;
		UpdateEventListened();
	}

	void DirectionalInput::SetStick(Gamepad::Stick stick, int version)
	{
		if (m_bindings.directions.size() <= version)
		{
			LOG_WARN("DirectionalInput::SetStick -> version does not exists.");
			return;
		}
		m_bindings.directions[version].stick = stick;
		UpdateEventListened();
	}

	void DirectionalInput::SetButtons(std::vector<DirectionalButton> buttons, int version)
	{
		if (m_bindings.directions.size() <= version)
		{
			LOG_WARN("DirectionalInput::SetButtons -> version does not exists.");
			return;
		}
		m_bindings.directions[version].buttons = buttons;
		UpdateEventListened();
	}

	void DirectionalInput::RemoveBinding(int version)
	{
		if (!Container::RemoveAt(m_bindings.directions, version))
		{
			LOG_WARN("DirectionalInput::RemoveBinding -> version does not exists.");
		}
		UpdateEventListened();
	}

	void DirectionalInput::SetStickDeadZone(float deadzone)
	{
		m_stickDeadzone = deadzone;
	}

	void DirectionalInput::SetTriggerDeadZone(float deadzone)
	{
		m_triggerDeadzone = deadzone;
	}

	bool DirectionalInput::HaveBinding(Gamepad::Stick stick)
	{
		for (auto& direction : m_bindings.directions)
		{
			if (direction.stick.stick == stick)
				return true;
		}
		return false;
	}

	bool DirectionalInput::HaveBinding(std::vector<DirectionalButton> buttons)
	{
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				for (auto& buttonCompare : buttons)
				{
					if (buttonCompare.button == button.button)
						return true;
				}
			}
		}
		return false;
	}

	bool DirectionalInput::HaveBinding(Button button)
	{
		for (auto& direction : m_bindings.directions)
		{
			for (auto& buttonCompare : direction.buttons)
			{
				if (buttonCompare.button == button)
					return true;
			}
		}
		return false;
	}

	int DirectionalInput::GetBindingsCount() const
	{
		return (int)m_bindings.directions.size();
	}

	bool DirectionalInput::KeyPressed(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if (e.key.scancode == (SDL_Scancode)button.button.key && !button.pressed)
				{
					button.pressed = true;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}

	bool DirectionalInput::KeyReleased(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if (e.key.scancode == (SDL_Scancode)button.button.key && button.pressed)
				{
					button.pressed = false;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}
	bool DirectionalInput::MouseButtonPressed(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if (e.button.button == (Uint8)button.button.mouse && !button.pressed)
				{
					button.pressed = true;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}
	bool DirectionalInput::MouseButtonReleased(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if (e.button.button == (Uint8)button.button.mouse && button.pressed)
				{
					button.pressed = false;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}
	bool DirectionalInput::MouseWheelMoved(const SDL_Event& e)
	{
		//Do not change state, just send signal right away
		DirectionalInputState state;
		if (e.wheel.y > 0)
		{
			state.direction = Vec2f(0, 1);
		}
		else if (e.wheel.y < 0)
		{
			state.direction = Vec2f(0, -1);
		}
		signal.Send(&state);
		return true;
	}
	bool DirectionalInput::GamepadButtonPressed(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if ((SDL_GamepadButton)e.gbutton.button == (SDL_GamepadButton)button.button.controller && !button.pressed)
				{
					button.pressed = true;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}
	bool DirectionalInput::GamepadButtonReleased(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if ((SDL_GamepadButton)e.gbutton.button == (SDL_GamepadButton)button.button.controller && button.pressed)
				{
					button.pressed = false;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}
	bool DirectionalInput::GamepadTriggerMoved(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				//float value = Math::ScaleRangeTo((float)e.gaxis.value, 0.f, 1.f, 0.f, 32767.f);
				float value = e.gaxis.value / 32767.f;
				float threshold = m_triggerDeadzone;
				if (value >= threshold && button.pressed == false)
				{
					button.pressed = true;
					mustComputeState = true;
				}
				else if (value < threshold && button.pressed == true)
				{
					button.pressed = false;
					mustComputeState = true;
				}
			}
		}
		if (mustComputeState)
		{
			ComputeState();
			return true;
		}
		return false;
	}

	bool DirectionalInput::GamepadStickMoved(const SDL_Event& e)
	{
		bool mustComputeState = false;
		for (auto& direction : m_bindings.directions)
		{
			if ((SDL_GamepadAxis)e.gaxis.axis == (SDL_GamepadAxis)direction.stick.stick.xAxis)
			{
				AxisMove((float)e.gaxis.value, true, direction);
				return true;
			}
			else if ((SDL_GamepadAxis)e.gaxis.axis == (SDL_GamepadAxis)direction.stick.stick.yAxis)
			{
				AxisMove((float)e.gaxis.value, false, direction);
				return true;
			}
		}
		return false;
	}

	void DirectionalInput::AxisMove(float value, bool x, Direction& direction)
	{
		//value = Math::ScaleRangeTo(value, -1.0f, 1.0f, -32768.f, 32767.f);
		value /= 32767.f;
		float absValue = std::fabs(value);
		if (absValue > m_stickDeadzone)
		{
			absValue = Math::ScaleRangeTo(absValue, m_stickDeadzone, 1.0f, 0.0f, 1.0f);

			if (value < 0.f)
				value = -absValue;
			else
				value = absValue;
		}
		else
		{
			value = 0.f;
		}
		if (x)
			direction.stick.currentDirection.x = value;
		else
			direction.stick.currentDirection.y = -value;
		ComputeState();
	}

	void DirectionalInput::UpdateEventListened()
	{
		int newEvents = 0;

		for (auto& direction : m_bindings.directions)
		{
			if (direction.stick.stick != Gamepad::Stick::None)
			{
				newEvents |= Input::GamepadStickFlag;
			}
			for (auto& button : direction.buttons)
			{
				if (button.button.key != Key::Scancode::Unknown)
					newEvents |= Input::KeyButtonFlag;

				if (button.button.mouse != Mouse::Button::Invalid)
					newEvents |= Input::MouseButtonFlag;

				if (button.button.controller != Gamepad::Button::Invalid)
					newEvents |= Input::GamepadButtonFlag;

				if (button.button.trigger != Gamepad::Trigger::Undefined)
					newEvents |= Input::GamepadTriggerFlag;
			}
		}
		if (m_mouseWheel)
		{
			newEvents |= Input::MouseWheelFlag;
		}
		if (m_eventsListened != newEvents)
		{
			m_eventsListened = newEvents;
			OnEventListenedUpdated();
		}
	}

	DirectionalBindings& DirectionalInput::GetBindings()
	{
		return m_bindings;
	}

	void DirectionalInput::ComputeState()
	{
		Vec2f sum(0, 0);

		for (auto& direction : m_bindings.directions)
		{
			for (auto& button : direction.buttons)
			{
				if (button.pressed)
				{
					sum += button.direction;
				}
			}
			if (direction.stick.stick != Gamepad::Stick::None)
			{
				sum += direction.stick.currentDirection;
			}
		}

		if (m_state.direction != sum)
		{
			m_state.direction = sum;
			signal.Send(&m_state);
		}
	}

}