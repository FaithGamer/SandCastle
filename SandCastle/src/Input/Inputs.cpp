#include "pch.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Input/Inputs.h"
#include "SandCastle/Core/Container.h"

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