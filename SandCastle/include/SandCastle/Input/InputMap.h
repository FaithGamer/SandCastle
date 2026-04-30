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
		GamepadBtn,
		GamepadStick,
		GamepadTrigger,
		EventTypeCount
	};

	//To do: enable serialization/deserialization
	//To do: take into consideration JoyId for multiple controller connected
	/// @brief A named collection of Inputs sharing a lifetime and an active flag.
	/// Typical usage: one map per gameplay context (gameplay, menu, dialog) so
	/// you can SetActive(false) on one map while another is foreground.
	/// Created and owned by the Inputs singleton via Inputs::CreateInputMap.
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
		/// @brief Create a DirectionalInput in this map. Use SetBindings/BindWASD/BindStick to configure it.
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

		/// @brief Internal: dispatch an SDL event to inputs in this map. Returns true if any input consumed it.
		bool OnEvent(const SDL_Event& e);
		/// @brief Internal: mark an input as needing event-listener bookkeeping refresh.
		void OnInputEventModified(sptr<Input> input);

		/// @brief Should this input map be used or not.
		bool IsActive() const;
		/// @brief Get an input according to it's name
		sptr<Input> GetInput(std::string name);
		/// @brief Get the whole input map
		std::unordered_map<std::string, sptr<Input>> GetInputs();
		/// @brief Get the name of this InputMap
		std::string GetName() const;
		/// @brief True if an input with this name exists in the map.
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