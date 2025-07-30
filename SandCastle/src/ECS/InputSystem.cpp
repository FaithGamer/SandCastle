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
			InitController(joystickArray[i]);
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
				InitController(event.gdevice.which);
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
	void InputSystem::AddForbiddenBinding(KeyScancode key)
	{
		m_forbiddenKeys.push_back(key);
	}
	void InputSystem::AddForbiddenBinding(ControllerButton button)
	{
		m_forbiddenButtons.push_back(button);
	}
	void InputSystem::AddForbiddenBinding(ControllerTrigger trigger)
	{
		m_forbiddenTriggers.push_back(trigger);
	}
	void InputSystem::AddForbiddenBinding(MouseButton mouse)
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
					if (Container::Contains(m_forbiddenMouses, (MouseButton)e.button.button))
						return false;

					if (version == -1)
					{
						buttonInput->BindMouse((MouseButton)e.button.button);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetKey(version, KeyScancode::Unknown);
						buttonInput->SetMouse(version, (MouseButton)e.button.button);
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
					if(Container::Contains(m_forbiddenKeys, (KeyScancode)e.key.scancode))
						return false;

					if (version == -1)
					{
						buttonInput->BindKey((KeyScancode)e.key.scancode);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetMouse(version, MouseButton::Invalid);
						buttonInput->SetKey(version, (KeyScancode)e.key.scancode);
						EndRebind();
						return true;
					}
					break;
				default:
					break;
				}
			}

			if ((m_rebindPeripherals & PeripheralFlag::Controller) == PeripheralFlag::Controller)
			{
				//Controller
				switch (e.type)
				{
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					if (Container::Contains(m_forbiddenButtons, (ControllerButton)e.gbutton.button))
						return false;

					if (version == -1)
					{
						buttonInput->BindControllerButton((ControllerButton)e.gbutton.button);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetControllerButton(version, (ControllerButton)e.gbutton.button);
						EndRebind();
						return true;
					}
					break;
				case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
					if (Container::Contains(m_forbiddenTriggers, ControllerTrigger::Left))
						return false;

					if (version == -1)
					{
						buttonInput->BindControllerTrigger(ControllerTrigger::Left);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetControllerTrigger(version, ControllerTrigger::Left);
						EndRebind();
						return true;
					}
					break;
				case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
					if (Container::Contains(m_forbiddenTriggers, ControllerTrigger::Right))
						return false;

					if (version == -1)
					{
						buttonInput->BindControllerTrigger(ControllerTrigger::Right);
						EndRebind();
						return true;
					}
					else
					{
						buttonInput->SetControllerTrigger(version, ControllerTrigger::Right);
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
	void InputSystem::InitController(SDL_JoystickID id)
	{
		SDL_Gamepad* controller = SDL_OpenGamepad(id);
		if (controller == NULL)
		{
			LOG_INFO("Could not open game controller: {0}\n", SDL_GetError());
		}
	}
}