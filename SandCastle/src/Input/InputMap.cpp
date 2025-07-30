#include "pch.h"
#include "SandCastle/Input/InputMap.h"
#include "SandCastle/Core/Container.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Input/DirectionalInput.h"
#include "SandCastle/Input/Inputs.h"
namespace SandCastle
{
	InputMap::InputMap(std::string name) : m_name(name), m_mustUpdate(false), m_isActive(true), m_passThroughImGui(false)
	{
		m_byEvents.resize((int)EventType::EventTypeCount);
	}

	InputMap::~InputMap()
	{

	}

	void InputMap::Deserialize(Serialized& config)
	{
		String name = config.GetString("name");
		m_passThroughImGui = config.GetBool("pass_through_imgui");
		if (name != m_name)
		{
			LOG_WARN("InputMap deserialization with a different name. Maybe you loaded the wrong InputMap? The previous name has been kept.");
		}

		//Load buttons inputs
		std::vector<Json> jbuttonInputs = config.GetArray<Json>("button_inputs");
		for (auto& jinput : jbuttonInputs)
		{
			Serialized cinput;
			cinput.SetJson(jinput);
			String name = cinput.GetString("name");

			//Load bindings
			ButtonBindings bindings;
			Serialized cbindings = cinput.GetObj("bindings");
			bindings.Deserialize(cbindings);

			//Look if input already exists
			auto input_it = m_byNames.find(name);
			if (input_it != m_byNames.end())
			{
				//In which case we override it's bindings
				if (input_it->second->GetType() != InputType::Button)
				{
					//An input of another type exists with the same name
					LOG_ERROR("InputMap Deserialization: input of type other than button already have the name {0}", name);
					continue;
				}

				//Override
				auto buttonInput = static_pointer_cast<ButtonInput>(input_it->second);
				buttonInput->SetBindings(bindings);
				m_modified.insert(buttonInput);
				m_mustUpdate = true;
			}
			else
			{
				//Otherwise create a new input
				auto buttonInput = CreateButtonInput(name);
				buttonInput->SetBindings(bindings);
			}
		}
	}

	Serialized InputMap::Serialize()
	{
		Serialized config;

		//Input map settings
		config["name"] = m_name;
		config["pass_through_imgui"] = m_passThroughImGui;

		std::vector<Json> jbuttonInputs;

		//Write all the inputs
		for (auto& inputKvp : m_byNames)
		{
			Serialized cInput;
			String name = inputKvp.first;
			sptr<Input> input = inputKvp.second;
			switch (input->GetType())
			{
			case InputType::Button:
			{
				//Add a button input, it's bindings and parameters
				auto buttonInput = static_pointer_cast<ButtonInput>(input);
				cInput["name"] = name;
				cInput["send_signal_on_press"] = buttonInput->GetSendSignalOnPress();
				cInput["send_signal_on_release"] = buttonInput->GetSendSignalOnRelease();
				cInput.AddObj("bindings", buttonInput->GetBindings().Serialize());
				jbuttonInputs.push_back(cInput);
			}
			break;
			//To do consider other input type (directional)
			default:
				break;
			}
		
		}

		config["button_inputs"] = jbuttonInputs;

		return config;
	}

	sptr<ButtonInput> InputMap::CreateButtonInput(std::string name)
	{
		if (CheckHaveInputAndDisplayWarning(name))
		{
			return nullptr;
		}
		auto button = makesptr<ButtonInput>(name);
		AddInput(button);
		return button;
	}

	sptr<DirectionalInput> InputMap::CreateDirectionalInput(std::string name)
	{
		if (CheckHaveInputAndDisplayWarning(name))
		{
			return nullptr;
		}
		auto button = makesptr<DirectionalInput>(name);
		AddInput(button);
		return button;
	}

	void InputMap::SetActive(bool active)
	{
		m_isActive = active;
	}

	void InputMap::SetPassThroughImGui(bool passThrough)
	{
		m_passThroughImGui = passThrough;
	}

	void InputMap::DestroyInput(std::string name)
	{
		auto input = m_byNames.find(name);
		if (input == m_byNames.end())
		{
			LOG_WARN("Cannot destroy input \"" + name + "\", because it doesn't exists.");
		}
		else
		{
			m_modified.erase(input->second);
			m_toDelete.emplace_back(input->second);
			m_mustUpdate = true;
		}
	}

	bool InputMap::OnEvent(const SDL_Event& e)
	{
		if (m_mustUpdate)
			UpdateInputsEvents();

		bool eventHandled = false;
		switch (e.type)
		{
		case SDL_EVENT_KEY_DOWN:
			for (auto& input : m_byEvents[(int)EventType::Key])
			{
				if (input->KeyPressed(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_KEY_UP:
			for (auto& input : m_byEvents[(int)EventType::Key])
			{
				if (input->KeyReleased(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			for (auto& input : m_byEvents[(int)EventType::MouseBtn])
			{
				if (input->MouseButtonPressed(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			for (auto& input : m_byEvents[(int)EventType::MouseBtn])
			{
				if (input->MouseButtonReleased(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			for (auto& input : m_byEvents[(int)EventType::MouseWheel])
			{
				if (input->MouseWheelMoved(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			for (auto& input : m_byEvents[(int)EventType::MouseMove])
			{
				if (input->MouseMoved(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = false;
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			for (auto& input : m_byEvents[(int)EventType::GamepadBtn])
			{
				if (input->GamepadButtonPressed(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = true;
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			for (auto& input : m_byEvents[(int)EventType::GamepadBtn])
			{
				if (input->GamepadButtonReleased(e))
					eventHandled = true;
			}
			Inputs::Instance()->controllerUsedLast = true;
			break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:

			if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX
				|| e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY
				|| e.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTX
				|| e.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHTY)
			{
				for (auto& input : m_byEvents[(int)EventType::GamepadStick])
				{
					if (input->GamepadStickMoved(e))
						eventHandled = true;
				}
				if (e.gaxis.value / 32767.f > 0.15f)
				{
					Inputs::Instance()->controllerUsedLast = true;
				}
			}
			else
			{
				for (auto& input : m_byEvents[(int)EventType::GamepadStick])
				{
					if (input->GamepadTriggerMoved(e))
						eventHandled = true;
				}
				if (e.gaxis.value / 32767.f > 0.15f)
				{
					Inputs::Instance()->controllerUsedLast = true;
				}
			}
			break;
		case SDL_EVENT_TEXT_INPUT:
			for (auto& input : m_byEvents[(int)EventType::Text])
			{
				if (input->TextEntered(e))
					eventHandled = true;
			}
		
			Inputs::Instance()->controllerUsedLast = false;
			
			break;

		case SDL_EVENT_TEXT_EDITING:
			break;
		}
		return eventHandled;
	}

	void InputMap::OnInputEventModified(sptr<Input> input)
	{
		m_modified.insert(input);
		m_mustUpdate = true;
	}

	bool InputMap::IsActive() const
	{
		return m_isActive;
	}

	sptr<Input> InputMap::GetInput(std::string name)
	{
		auto input_it = m_byNames.find(name);
		if (input_it == m_byNames.end())
		{
			LOG_WARN("Cannot find the input \"" + name + "\" in the InputMap \"" + GetName() + "\"");
			return nullptr;
		}
		return input_it->second;
	}

	std::unordered_map<std::string, sptr<Input>> InputMap::GetInputs()
	{
		return m_byNames;
	}

	std::string InputMap::GetName() const
	{
		return m_name;
	}

	bool InputMap::HaveInput(std::string name) const
	{
		auto input_it = m_byNames.find(name);
		if (input_it == m_byNames.end())
		{
			return false;
		}
		return true;
	}

	void InputMap::UpdateInputsEvents()
	{
		for (auto& input : m_toDelete)
		{
			for (auto& inputs : m_byEvents)
			{
				Container::Remove(inputs, input);
			}
			m_byNames.erase(input->GetName());
		}
		m_toDelete.clear();
		for (auto& input : m_modified)
		{
			const int& events = input->m_eventsListened;
			const std::string& name = input->GetName();

			//Key button
			if ((events & Input::KeyButtonFlag) == Input::KeyButtonFlag && !Container::Contains(m_byEvents[(int)EventType::Key], input))
			{
				m_byEvents[(int)EventType::Key].push_back(input);
			}
			else if ((events & Input::KeyButtonFlag) != Input::KeyButtonFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::Key], input);
			}
			//Textual
			if ((events & Input::KeyTextFlag) == Input::KeyTextFlag && !Container::Contains(m_byEvents[(int)EventType::Text], input))
			{
				m_byEvents[(int)EventType::Text].push_back(input);
			}
			else if ((events & Input::KeyTextFlag) != Input::KeyTextFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::Text], input);
			}
			//Mouse button
			if ((events & Input::MouseButtonFlag) == Input::MouseButtonFlag && !Container::Contains(m_byEvents[(int)EventType::MouseBtn], input))
			{
				m_byEvents[(int)EventType::MouseBtn].push_back(input);
			}
			else if ((events & Input::MouseButtonFlag) != Input::MouseButtonFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::MouseBtn], input);
			}
			//Mouse movement
			if ((events & Input::MouseMovementFlag) == Input::MouseMovementFlag && !Container::Contains(m_byEvents[(int)EventType::MouseMove], input))
			{
				m_byEvents[(int)EventType::MouseMove].push_back(input);
			}
			else if ((events & Input::MouseMovementFlag) != Input::MouseMovementFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::MouseMove], input);
			}
			//Mouse wheel
			if ((events & Input::MouseWheelFlag) == Input::MouseWheelFlag && !Container::Contains(m_byEvents[(int)EventType::MouseWheel], input))
			{
				m_byEvents[(int)EventType::MouseWheel].push_back(input);
			}
			else if ((events & Input::MouseWheelFlag) != Input::MouseWheelFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::MouseWheel], input);
			}
			//Gamepad button
			if ((events & Input::GamepadButtonFlag) == Input::GamepadButtonFlag && !Container::Contains(m_byEvents[(int)EventType::GamepadBtn], input))
			{
				m_byEvents[(int)EventType::GamepadBtn].push_back(input);
			}
			else if ((events & Input::GamepadButtonFlag) != Input::GamepadButtonFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::GamepadBtn], input);
			}
			//Gamepad stick
			if ((events & Input::GamepadStickFlag) == Input::GamepadStickFlag && !Container::Contains(m_byEvents[(int)EventType::GamepadStick], input))
			{
				m_byEvents[(int)EventType::GamepadStick].push_back(input);
			}
			else if ((events & Input::GamepadStickFlag) != Input::GamepadStickFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::GamepadStick], input);
			}
			//Gamepad trigger
			if ((events & Input::GamepadTriggerFlag) == Input::GamepadTriggerFlag && !Container::Contains(m_byEvents[(int)EventType::GamepadTrigger], input))
			{
				m_byEvents[(int)EventType::GamepadTrigger].push_back(input);
			}
			else if ((events & Input::GamepadTriggerFlag) != Input::GamepadTriggerFlag)
			{
				Container::Remove(m_byEvents[(int)EventType::GamepadTrigger], input);
			}
		}
		m_modified.clear();
		m_mustUpdate = false;
	}


	bool InputMap::CheckHaveInputAndDisplayWarning(std::string name) const
	{
		if (HaveInput(name))
		{
			LOG_WARN("Trying to add Input with already existing name \"" + name + "\" already exist in the InputMap \"" + GetName()
				+ "\". No input has been added.");
			return true;
		}
		return false;
	}

	void InputMap::AddInput(sptr<Input> input)
	{
		m_modified.insert(input);
		m_byNames.insert(std::make_pair(input->GetName(), input));
		input->m_inputMap = this;
		m_mustUpdate = true;
	}
}