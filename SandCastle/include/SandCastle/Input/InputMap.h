#pragma once

#include "SandCastle/Input/Input.h"
#include "SandCastle/Core/Serialization.h"

namespace SandCastle
{
	class Inputs;
	class ButtonInput;
	class DirectionalInput;
	struct InputMapContainer;

	enum class EventType : int
	{
		Key = 0,
		Text,
		MouseBtn,
		MouseMove,
		MouseWheel,
		ControllerBtn,
		ControllerStick,
		ControllerTrigger,
		EventTypeCount
	};

	//To do: enable serialization/deserialization
	//To do: take into consideration JoyId for multiple controller connected
	class InputMap : Serializable
	{
	public:
		InputMap(std::string name);
		~InputMap();

		//Serializable

		/// @brief Load from a serialized.
		/// It will override bindings of already existing inputs, add new ones, but won't remove the inputs already in place.
		/// Only works for ButtonInput atm
		/// @param config 
		void Deserialize(Serialized& config) override;
		/// @brief Only works for ButtonInput atm
		Serialized Serialize() override;

		/// @brief Create a ButtonInput 
		/// @param name input name
		/// @return shared_ptr to the created input
		sptr<ButtonInput> CreateButtonInput(std::string name);
		sptr<DirectionalInput> CreateDirectionalInput(std::string name);
		/// @brief Set wether or not this InputMap is used by the InputSystem
		/// @param active 
		void SetActive(bool active);
		/// @brief Set true to capture event even if ImGui captured it. Default: false.
		/// @param passThrough 
		void SetPassThroughImGui(bool passThrough);
		/// @brief Delete an input
		/// @param name Name of the Input to delete
		void DestroyInput(std::string name);

		bool OnEvent(const SDL_Event& e);
		void OnInputEventModified(sptr<Input> input);

		/// @brief Should this input map be used or not.
		bool IsActive() const;
		/// @brief Get an input according to it's name
		sptr<Input> GetInput(std::string name);
		/// @brief Get the whole input map
		std::unordered_map<std::string, sptr<Input>> GetInputs();
		/// @brief Get the name of this InputMap
		std::string GetName() const;
		bool HaveInput(std::string name) const;

	private:
		void UpdateInputsEvents();
		bool CheckHaveInputAndDisplayWarning(std::string name) const;
		void AddInput(sptr<Input> input);

		std::vector<std::vector<sptr<Input>>> m_byEvents;
		std::unordered_map<std::string, sptr<Input>> m_byNames;
		std::set<sptr<Input>> m_modified;
		std::vector<sptr<Input>> m_toDelete;

		bool m_mustUpdate;
		bool m_isActive;
		bool m_passThroughImGui;

		std::string m_name;
	};
}