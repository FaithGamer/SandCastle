#include "pch.h"

#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Input/InputSystem.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/DirectionalInput.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Container.h"
namespace SandCastle
{
	InputSystem::InputSystem()
	{
		SetPriority(-10000);
	}

	void InputSystem::OnStart()
	{
		//Open one game controller

		int count = 0;
		SDL_JoystickID* joystickArray = SDL_GetGamepads(&count);
		for (int i = 0; i < count ; i++)
		{
			InitGamepad(joystickArray[i]);
		}
	}

	bool InputSystem::OnEvent(SDL_Event& event)
	{
		bool eventConsumed = false;
		if (m_rebind != nullptr)
		{
			//Rebinding is occuring
			eventConsumed = Rebind(event);
		}
		if (!eventConsumed)
		{
			//Normal usage of inputs
			for (auto& inputMap : Inputs::GetInputMaps())
			{
				if (!inputMap->IsActive())
					continue;

				eventConsumed = inputMap->OnEvent(event);
			}
		}

		//controller plug in/out
		switch (event.type)
		{
		case SDL_EVENT_GAMEPAD_ADDED:
		{
			int joystickCount = 0;
			SDL_GetGamepads(&joystickCount);
			if (joystickCount > 0)
			{
				InitGamepad(event.gdevice.which);
			}
			else
			{
				LOG_WARN("Gamepad added but joystickCount is 0.");
			}
			break;
		}
		case SDL_EVENT_GAMEPAD_REMOVED:
		{
			SDL_Gamepad* controller = SDL_GetGamepadFromID(event.gdevice.which);
			SDL_CloseGamepad(controller);
		}
		break;
		default:
			break;
		}

		return eventConsumed;
	}

	void InputSystem::StartRebind(sptr<Input> input, int peripherals, int version)
	{
		m_forbiddenKeys.clear();
		m_forbiddenButtons.clear();
		m_forbiddenTriggers.clear();
		m_forbiddenMouses.clear();

		m_rebind = input;
		m_rebindVersion = version;
		m_rebindPeripherals = peripherals;

		if (m_rebindPeripherals == 0)
		{
			LOG_WARN("Rebind peripheral flag no set.");
		}
	}

	void InputSystem::EndRebind()
	{
		m_forbiddenKeys.clear();
		m_forbiddenButtons.clear();
		m_forbiddenTriggers.clear();
		m_forbiddenMouses.clear();

		m_rebind = nullptr;
		m_rebindVersion = 0;
		m_rebindPeripherals = 0;
	}
	void InputSystem::AddForbiddenBinding(Key::Scancode key)
	{
		m_forbiddenKeys.push_back(key);
	}
	void InputSystem::AddForbiddenBinding(Gamepad::Button button)
	{
		m_forbiddenButtons.push_back(button);
	}
	void InputSystem::AddForbiddenBinding(Gamepad::Trigger trigger)
	{
		m_forbiddenTriggers.push_back(trigger);
	}
	void InputSystem::AddForbiddenBinding(Mouse::Button mouse)
	{
		m_forbiddenMouses.push_back(mouse);
	}

	int InputSystem::GetUsedMethod()
	{
		return System::Method::Event;
	}

	bool InputSystem::IsRebinding()
	{
		return !(m_rebind == nullptr);
	}
	sptr<Input> InputSystem::GetRebindingInput()
	{
		return m_rebind;
	}
	bool InputSystem::Rebind(SDL_Event& e)
	{
		switch (m_rebind->GetType())
		{
		case InputType::Button:
		{
			auto buttonInput = static_pointer_cast<ButtonInput>(m_rebind);
			int version = m_rebindVersion;
			if (m_rebindVersion >= buttonInput->GetBindingsCount())
			{
				version = -1;
			}

			if ((m_rebindPeripherals & PeripheralFlag::Mouse) == PeripheralFlag::Mouse)
			{
				//Mouse
				switch (e.type)
				{
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (Container::Contains(m_forbiddenMouses, (Mouse::Button)e.button.button))
						return false;

					if (version == -1)
					{
						buttonInput->BindMouse((Mouse::Button)e.button.button);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetKey(version, Key::Scancode::Unknown);
						buttonInput->SetMouse(version, (Mouse::Button)e.button.button);
						EndRebind();
						return true;
					}
					break;
				default:
					break;
				}
			}

			if ((m_rebindPeripherals & PeripheralFlag::Keyboard) == PeripheralFlag::Keyboard)
			{
				//Keyboard
				switch (e.type)
				{
				case SDL_EVENT_KEY_DOWN:
					if(Container::Contains(m_forbiddenKeys, (Key::Scancode)e.key.scancode))
						return false;

					if (version == -1)
					{
						buttonInput->BindKey((Key::Scancode)e.key.scancode);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetMouse(version, Mouse::Button::Invalid);
						buttonInput->SetKey(version, (Key::Scancode)e.key.scancode);
						EndRebind();
						return true;
					}
					break;
				default:
					break;
				}
			}

			if ((m_rebindPeripherals & PeripheralFlag::Gamepad) == PeripheralFlag::Gamepad)
			{
				//Gamepad
				switch (e.type)
				{
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					if (Container::Contains(m_forbiddenButtons, (Gamepad::Button)e.gbutton.button))
						return false;

					if (version == -1)
					{
						buttonInput->BindGamepadButton((Gamepad::Button)e.gbutton.button);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetGamepadButton(version, (Gamepad::Button)e.gbutton.button);
						EndRebind();
						return true;
					}
					break;
				case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
					if (Container::Contains(m_forbiddenTriggers, Gamepad::Trigger::Left))
						return false;

					if (version == -1)
					{
						buttonInput->BindGamepadTrigger(Gamepad::Trigger::Left);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetGamepadTrigger(version, Gamepad::Trigger::Left);
						EndRebind();
						return true;
					}
					break;
				case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
					if (Container::Contains(m_forbiddenTriggers, Gamepad::Trigger::Right))
						return false;

					if (version == -1)
					{
						buttonInput->BindGamepadTrigger(Gamepad::Trigger::Right);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetGamepadTrigger(version, Gamepad::Trigger::Right);
						EndRebind();
						return true;
					}
					break;
				default:
					break;
				}
			}
		}
		break;
		default:
			LOG_WARN("Rebind of input type {0} not implemented", InputTypeName(m_rebind->GetType()));
			break;
		}
		return false;
	}
	void InputSystem::InitGamepad(SDL_JoystickID id)
	{
		SDL_Gamepad* controller = SDL_OpenGamepad(id);
		if (controller == NULL)
		{
			LOG_INFO("Could not open game controller: {0}\n", SDL_GetError());
		}
	}
}