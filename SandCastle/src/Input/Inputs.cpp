#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/DirectionalInput.h"
#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Core/Hardware.h"
#include "SandCastle/Core/Math.h"
#include "SandCastle/Render/Window.h"
#include "SandCastle/UI/Ui.h"

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
		// Default to gamepad mode when a controller is plugged in at launch, or
		// when running on a Steam Deck (which has no separate mouse). Any later
		// real input event will switch us appropriately via OnEvent.
		bool initialGamepad = (count > 0) || Hardware::IsSteamDeck();
		if (joystickArray) SDL_free(joystickArray);
		m_gamepadMode = initialGamepad;
		ApplyGamepadMode(m_gamepadMode);
	}

	bool Inputs::OnEvent(SDL_Event& event)
	{
		AutoDetectGamepadModeFromEvent(event);

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

	bool Inputs::IsGamepadMode()
	{
		return Instance()->m_gamepadMode;
	}

	void Inputs::SetGamepadMode(bool gamepad)
	{
		Instance()->SetGamepadModeInternal(gamepad);
	}

	void Inputs::AutoToggleGamepadMode(bool autoToggle)
	{
		Instance()->m_autoToggleGamepadMode = autoToggle;
	}

	bool Inputs::IsAutoToggleGamepadMode()
	{
		return Instance()->m_autoToggleGamepadMode;
	}

	void Inputs::SetHideCursorOnGamepadMode(bool hide)
	{
		Instance()->m_hideCursorOnGamepadMode = hide;
	}

	bool Inputs::IsHideCursorOnGamepadMode()
	{
		return Instance()->m_hideCursorOnGamepadMode;
	}

	void Inputs::AutoDetectGamepadModeFromEvent(const SDL_Event& event)
	{
		if (!m_autoToggleGamepadMode)
			return;
		switch (event.type)
		{
		// Gamepad activity → gamepad mode.
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			SetGamepadModeInternal(true);
			break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			// Ignore noise below the standard 15% deadzone — sticks idle near zero.
			if (Math::Abs(event.gaxis.value / 32767.f) > 0.15f)
				SetGamepadModeInternal(true);
			break;

		// Deliberate mouse/keyboard activity → mouse mode.
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_WHEEL:
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			SetGamepadModeInternal(false);
			break;

		// Mouse motion: ignore zero-delta events (SDL fires one on window focus
		// at launch, which would otherwise flip us out of gamepad mode before
		// the player has done anything).
		case SDL_EVENT_MOUSE_MOTION:
			if (event.motion.xrel != 0.f || event.motion.yrel != 0.f)
				SetGamepadModeInternal(false);
			break;

		default:
			break;
		}
	}

	void Inputs::SetGamepadModeInternal(bool gamepad)
	{
		if (gamepad == m_gamepadMode)
			return;
		m_gamepadMode = gamepad;
		ApplyGamepadMode(gamepad);
		gamepadModeSignal.Send(gamepad);
	}

	void Inputs::ApplyGamepadMode(bool gamepad)
	{
		if (!m_hideCursorOnGamepadMode)
			return;
		Window::ShowCursor(!gamepad);
	}

	void Inputs::CreateDefaultGamepadNav(const String& mapName)
	{
		auto map = Instance()->m_inputMaps.Get(mapName);
		if (!map)
		{
			LOG_ERROR("Inputs::CreateDefaultGamepadNav: input map '{0}' doesn't exist.", mapName);
			return;
		}

		struct Bind
		{
			const char* name;
			Gamepad::Button pad;
			Key::Scancode key;
			bool onPress;
			bool onRelease;
			void (*handler)(InputSignal*);
		};
		const Bind binds[] = {
			{ "ui_left",   Gamepad::Button::PadLeft,  Key::Scancode::Left,   true, false, &Ui::NavigateLeft  },
			{ "ui_right",  Gamepad::Button::PadRight, Key::Scancode::Right,  true, false, &Ui::NavigateRight },
			{ "ui_up",     Gamepad::Button::PadUp,    Key::Scancode::Up,     true, false, &Ui::NavigateUp    },
			{ "ui_down",   Gamepad::Button::PadDown,  Key::Scancode::Down,   true, false, &Ui::NavigateDown  },
			{ "ui_select", Gamepad::Button::South,    Key::Scancode::Return, true, true,  &Ui::Select        },
			{ "ui_cancel", Gamepad::Button::East,     Key::Scancode::Escape, true, true,  &Ui::Cancel        },
		};
		for (const auto& b : binds)
		{
			auto in = map->CreateButtonInput(b.name);
			if (!in) continue;
			in->BindGamepadButton(b.pad);
			in->BindKey(b.key);
			in->SetSignalOnPress(b.onPress);
			in->SetSignalOnRelease(b.onRelease);
			in->signal.Listen(b.handler);
		}
	}
}