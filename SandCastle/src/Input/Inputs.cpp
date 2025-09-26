#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/DirectionalInput.h"

namespace SandCastle
{
	/////////////////////////
	/// InputMapContainer ///
	/////////////////////////

	sptr<InputMap> InputMapContainer::Add(std::string name)
	{
		int64_t alreadyExists = Container::FindIndex(names, name);
		if (alreadyExists != -1)
		{
			LOG_WARN("An input map with the name " + name + " already exists, no input map has been added. Nullptr returned.");
			return nullptr;
		}
		inputs.push_back(makesptr<InputMap>(name));
		names.push_back(name);
		return inputs.back();
	}

	sptr<InputMap> InputMapContainer::Get(std::string name)
	{
		int64_t index = Container::FindIndex(names, name);
		if (index == -1)
		{
			LOG_WARN("No input map with the name " + name + " nullptr returned.");
			return nullptr;
		}
		return inputs[index];
	}

	void InputMapContainer::Remove(std::string name)
	{
		int64_t index = Container::FindIndex(names, name);
		if (index == -1)
		{
			LOG_WARN("No input map with the name " + name + " no input map removed.");
			return;
		}

		Container::RemoveAt(names, index);
		Container::RemoveAt(inputs, index);

	}

	//////////////
	/// Inputs ///
	//////////////

	Inputs::Inputs()
	{
		//Open gamepad if relevant
		int count = 0;
		SDL_JoystickID* joystickArray = SDL_GetGamepads(&count);
		for (int i = 0; i < count; i++)
		{
			InitGamepad(joystickArray[i]);
		}
	}

	bool Inputs::OnEvent(SDL_Event& event)
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
			for (auto& inputMap : m_inputMaps.inputs)
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

	void Inputs::StartRebind(sptr<Input> input, int peripherals, int version)
	{
		auto i = Instance();
		i->m_forbiddenKeys.clear();
		i->m_forbiddenButtons.clear();
		i->m_forbiddenTriggers.clear();
		i->m_forbiddenMouses.clear();

		i->m_rebind = input;
		i->m_rebindVersion = version;
		i->m_rebindPeripherals = peripherals;

		if (i->m_rebindPeripherals == 0)
		{
			LOG_WARN("Rebind peripheral flag no set.");
		}
	}

	void Inputs::EndRebind()
	{
		auto i = Instance();
		i->m_forbiddenKeys.clear();
		i->m_forbiddenButtons.clear();
		i->m_forbiddenTriggers.clear();
		i->m_forbiddenMouses.clear();

		i->m_rebind = nullptr;
		i->m_rebindVersion = 0;
		i->m_rebindPeripherals = 0;
	}
	void Inputs::AddForbiddenBinding(Key::Scancode key)
	{
		Instance()->m_forbiddenKeys.push_back(key);
	}
	void Inputs::AddForbiddenBinding(Gamepad::Button button)
	{
		Instance()->m_forbiddenButtons.push_back(button);
	}
	void Inputs::AddForbiddenBinding(Gamepad::Trigger trigger)
	{
		Instance()->m_forbiddenTriggers.push_back(trigger);
	}
	void Inputs::AddForbiddenBinding(Mouse::Button mouse)
	{
		Instance()->m_forbiddenMouses.push_back(mouse);
	}

	bool Inputs::IsRebinding()
	{
		return !(Instance()->m_rebind == nullptr);
	}
	sptr<Input> Inputs::GetRebindingInput()
	{
		return Instance()->m_rebind;
	}
	bool Inputs::Rebind(SDL_Event& e)
	{
		auto i = Instance();
		switch (m_rebind->GetType())
		{
		case InputType::Button:
		{
			auto buttonInput = static_pointer_cast<ButtonInput>(i->m_rebind);
			int version = i->m_rebindVersion;
			if (i->m_rebindVersion >= buttonInput->GetBindingsCount())
			{
				version = -1;
			}

			if ((i->m_rebindPeripherals & PeripheralFlag::Mouse) == PeripheralFlag::Mouse)
			{
				//Mouse
				switch (e.type)
				{
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (Container::Contains(i->m_forbiddenMouses, (Mouse::Button)e.button.button))
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

			if ((i->m_rebindPeripherals & PeripheralFlag::Keyboard) == PeripheralFlag::Keyboard)
			{
				//Keyboard
				switch (e.type)
				{
				case SDL_EVENT_KEY_DOWN:
					if (Container::Contains(i->m_forbiddenKeys, (Key::Scancode)e.key.scancode))
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

			if ((i->m_rebindPeripherals & PeripheralFlag::Gamepad) == PeripheralFlag::Gamepad)
			{
				//Gamepad
				switch (e.type)
				{
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					if (Container::Contains(i->m_forbiddenButtons, (Gamepad::Button)e.gbutton.button))
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
					if (Container::Contains(i->m_forbiddenTriggers, Gamepad::Trigger::Left))
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
					if (Container::Contains(i->m_forbiddenTriggers, Gamepad::Trigger::Right))
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
			LOG_WARN("Rebind of input type {0} not implemented", InputTypeName(i->m_rebind->GetType()));
			break;
		}
		return false;
	}
	void Inputs::InitGamepad(SDL_JoystickID id)
	{
		SDL_Gamepad* controller = SDL_OpenGamepad(id);
		if (controller == NULL)
		{
			LOG_INFO("Could not open game controller: {0}\n", SDL_GetError());
		}
	}

	sptr<InputMap> Inputs::CreateInputMap()
	{
		auto instance = Inputs::Instance();
		std::string name = "InputMap_" + std::to_string(instance->m_inputMaps.names.size());

		return instance->m_inputMaps.Add(name);
	}

	sptr<InputMap> Inputs::CreateInputMap(std::string name)
	{
		auto instance = Inputs::Instance();
		return instance->m_inputMaps.Add(name);
	}

	void Inputs::DestroyInputMap(std::string name)
	{
		auto instance = Inputs::Instance();
		instance->m_inputMaps.Add(name);
	}

	sptr<Input> Inputs::Get(String mapName, String inputName)
	{
		auto ins = Instance();
		auto map = ins->m_inputMaps.Get(mapName);
		if (map == nullptr)
			return nullptr;
		return map->GetInput(inputName);
	}

	std::vector<sptr<InputMap>>& Inputs::GetInputMaps()
	{
		return Inputs::Instance()->m_inputMaps.inputs;
	}

	sptr<InputMap> Inputs::GetInputMap(std::string name)
	{
		auto instance = Inputs::Instance();
		return instance->m_inputMaps.Get(name);
	}

	std::vector<std::string> Inputs::GetInputMapNameList()
	{
		auto instance = Inputs::Instance();

		return instance->m_inputMaps.names;
	}
}