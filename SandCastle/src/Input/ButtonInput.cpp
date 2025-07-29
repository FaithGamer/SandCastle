#include "pch.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/Bindings.h"
#include "SandCastle/Core/Container.h"

namespace SandCastle
{

	ButtonInput::ButtonInput(std::string name) :
		m_name(name), m_sendSignalOnPress(true), m_sendSignalOnRelease(false), m_triggerDeadzone(0.2f), m_lastTriggerValue(0.f)
	{

	}

	void ButtonInput::SetBindings(const ButtonBindings& bindings)
	{
		m_bindings = bindings;
		UpdateEventListened();
	}

	std::string ButtonInput::GetName() const
	{
		return m_name;
	}

	InputType ButtonInput::GetType() const
	{
		return InputType::Button;
	}

	void ButtonInput::RemoveAllBindings()
	{
		m_bindings.buttons.clear();
	}

	void ButtonInput::BindKey(KeyScancode keyButton)
	{
		if (HaveBinding(keyButton))
		{
			LOG_WARN("ButtonInput::BindKey -> binding already exists, no binding added");
			return;
		}
		size_t version = m_bindings.buttons.size();
		m_bindings.buttons.push_back(Button());
		m_bindings.buttons[version].key = keyButton;
		UpdateEventListened();
	}

	void ButtonInput::BindMouse(MouseButton mouseButton)
	{
		if (HaveBinding(mouseButton))
		{
			LOG_WARN("ButtonInput::BindMouse -> binding already exists, no binding added");
			return;
		}
		size_t version = m_bindings.buttons.size();
		m_bindings.buttons.push_back(Button());
		m_bindings.buttons[version].mouse = mouseButton;
		UpdateEventListened();
	}

	void ButtonInput::BindControllerButton(ControllerButton controllerButton)
	{
		if (HaveBinding(controllerButton))
		{
			LOG_WARN("ButtonInput::BindControllerButton -> binding already exists, no binding added");
			return;
		}
		size_t version = m_bindings.buttons.size();
		m_bindings.buttons.push_back(Button());
		m_bindings.buttons[version].controller = controllerButton;
		UpdateEventListened();
	}

	void ButtonInput::BindControllerTrigger(ControllerTrigger trigger)
	{
		if (HaveBinding(trigger))
		{
			LOG_WARN("ButtonInput::BindControllerTrigger -> binding already exists, no binding added");
			return;
		}
		size_t version = m_bindings.buttons.size();
		m_bindings.buttons.push_back(Button());
		m_bindings.buttons[version].trigger = trigger;
		UpdateEventListened();
	}

	void ButtonInput::SetKey(int version, KeyScancode keyButton)
	{
		if (version >= m_bindings.buttons.size())
		{
			LOG_WARN("ButtonInput::SetKey -> Button binding version does not exists.");
			return;
		}
		m_bindings.buttons[version].key = keyButton;
		UpdateEventListened();
	}

	void ButtonInput::SetMouse(int version, MouseButton mouseButton)
	{
		if (version >= m_bindings.buttons.size())
		{
			LOG_WARN("ButtonInput::SetMouse -> Button binding version does not exists.");
			return;
		}
		m_bindings.buttons[version].mouse = mouseButton;
		UpdateEventListened();
	}

	void ButtonInput::SetControllerButton(int version, ControllerButton controllerButton)
	{
		if (version >= m_bindings.buttons.size())
		{
			LOG_WARN("ButtonInput::SetControllerButton -> Button binding version does not exists.");
			return;
		}
		m_bindings.buttons[version].controller = controllerButton;
		UpdateEventListened();
	}

	void ButtonInput::SetControllerTrigger(int version, ControllerTrigger controllerTrigger)
	{
		if (version >= m_bindings.buttons.size())
		{
			LOG_WARN("ButtonInput::SetControllerTrigger -> Button binding version does not exists.");
			return;
		}
		m_bindings.buttons[version].trigger = controllerTrigger;
		UpdateEventListened();
	}

	void ButtonInput::RemoveBinding(int version)
	{
		if (m_bindings.buttons.size() <= version)
		{
			LOG_WARN("ButtonInput::RemoveBinding -> bindings count is under version, no binding removed.");
			return;
		}
		Container::RemoveAt(m_bindings.buttons, version);
	}

	void ButtonInput::SetSendSignalOnPress(bool signalOnPress)
	{
		m_sendSignalOnPress = signalOnPress;
	}

	void ButtonInput::SetSendSignalOnRelease(bool signalOnRelease)
	{
		m_sendSignalOnRelease = signalOnRelease;
	}

	void ButtonInput::SetTriggerDeadzone(float deadzone)
	{
		deadzone = glm::clamp(deadzone, 0.0f, 1.0f);
		m_triggerDeadzone = deadzone;
	}

	int ButtonInput::GetBindingsCount() const
	{
		return (int)m_bindings.buttons.size();
	}

	ButtonBindings& ButtonInput::GetBindings()
	{
		return m_bindings;
	}

	bool ButtonInput::GetSendSignalOnPress()
	{
		return m_sendSignalOnPress;
	}

	bool ButtonInput::GetSendSignalOnRelease()
	{
		return m_sendSignalOnRelease;
	}

	bool ButtonInput::HaveBinding(KeyScancode key)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (button.key == key)
				return true;
		}
		return false;
	}

	bool ButtonInput::HaveBinding(MouseButton mouse)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (button.mouse == mouse)
				return true;
		}
		return false;
	}

	bool ButtonInput::HaveBinding(ControllerButton controllerButton)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (button.controller == controllerButton)
				return true;
		}
		return false;
	}

	bool ButtonInput::HaveBinding(ControllerTrigger trigger)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (button.trigger == trigger)
				return true;
		}
		return false;
	}

	bool ButtonInput::KeyPressed(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (e.key.scancode == (SDL_Scancode)button.key)
			{
				PressButton();
				return true;
			}
		}
		return false;
	}

	bool ButtonInput::KeyReleased(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (e.key.scancode == (SDL_Scancode)button.key)
			{
				return ReleaseButton();
			}
		}
		return false;
	}

	bool ButtonInput::MouseButtonPressed(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (e.button.button == (Uint8)button.mouse)
			{
				return PressButton();
			}
		}
		return false;
	}

	bool ButtonInput::MouseButtonReleased(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (e.button.button == (Uint8)button.mouse)
			{
				return ReleaseButton();
			}
		}
		return false;
	}

	bool ButtonInput::ControllerButtonPressed(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if ((SDL_GamepadButton)e.gbutton.button == (SDL_GamepadButton)button.controller)
			{
				return PressButton();
			}
		}
		return false;
	}

	bool ButtonInput::ControllerButtonReleased(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if ((SDL_GamepadButton)e.gbutton.button == (SDL_GamepadButton)button.controller)
			{
				return ReleaseButton();
			}
		}
		return false;
	}

	bool ButtonInput::ControllerTriggerMoved(const SDL_Event& e)
	{
		for (auto& button : m_bindings.buttons)
		{
			if (e.gaxis.axis == (Uint8)button.trigger)
			{
				//Scale trigger axis value from 0 to 1
				//float value = Math::ScaleRangeTo((float)e.gaxis.value,  0.0f, 32767.0f, 0.0f, 1.0f);
				float value = (float)e.gaxis.value / 32767.0f;
				float threshold = m_triggerDeadzone;
				if (value >= threshold)
				{
					m_state.pressed = true;
					value = Math::ScaleRangeTo(value, threshold, 1.0f, 0.0f, 1.0f);
					return SetPressedAmount(value);
				}
				else
				{
					m_state.pressed = false;
					return SetPressedAmount(0.0f);
				}
			}
		}
		return false;
	}

	void ButtonInput::UpdateEventListened()
	{
		int newEvents = 0;

		for (auto& button : m_bindings.buttons)
		{
			if (button.key != KeyScancode::Unknown)
				newEvents |= Input::KeyButtonFlag;

			if (button.mouse != MouseButton::Invalid)
				newEvents |= Input::MouseButtonFlag;

			if (button.controller != ControllerButton::Invalid)
				newEvents |= Input::ControllerButtonFlag;

			if (button.trigger != ControllerTrigger::Undefined)
				newEvents |= Input::ControllerTriggerFlag;
		}
		if (m_eventsListened != newEvents)
		{
			m_eventsListened = newEvents;
			OnEventListenedUpdated();
		}
	}

	bool ButtonInput::PressButton()
	{
		if (!m_state.pressed)
		{
			m_state.pressed = true;
			if (m_sendSignalOnPress)
			{
				signal.SendSignal(&m_state);
				return true;
			}
		}
		return false;
	}

	bool ButtonInput::ReleaseButton()
	{
		if (m_state.pressed)
		{
			m_state.pressed = false;
			if (m_sendSignalOnRelease)
			{
				signal.SendSignal(&m_state);
				return true;
			}
		}
		return false;
	}

	bool ButtonInput::SetPressedAmount(float amount)
	{
		if (amount == m_state.pressedAmount)
			return false;

		m_state.pressedAmount = amount;
		signal.SendSignal(&m_state);

		return true;
	}

}