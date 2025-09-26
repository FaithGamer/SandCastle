#pragma once
#include <string>

#include "SandCastle/Input/InputMap.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Input/Gamepad.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Input/Keyboard.h"

namespace SandCastle
{
	class Systems;
	struct InputMapContainer
	{
		sptr<InputMap> Add(std::string name);
		sptr<InputMap> Get(std::string name);
		void Remove(std::string name);

		std::vector<sptr<InputMap>> inputs;
		std::vector<std::string> names;
	};

	class Inputs : public Singleton<Inputs>
	{
	public:
		enum PeripheralFlag : int
		{
			Mouse = 1,
			Keyboard = 2,
			Gamepad = 4
		};
	public:
		Inputs();
		
		/// @brief Every subsequent keyboard/mouse/controller event will try to bind to this input
		/// Call EndRebind()  when you're done.
		/// Warning: at the moment you can rebind only Button Input
		/// @param peripheral set flags of type InputSystem::PeripheralFlag
		/// @param version pass -1 to add a new version
		static void StartRebind(sptr<Input> input, int peripherals, int version = 0);

		//To do, need a rework of the directional input to be able to select which direction we wanna change button 
		///void SetRebindDirection(Vec2f direction);
		static void EndRebind();

		/// @brief Add a button that will be ignored during the rebinding, this will reset after a call to EndRebind
		static void AddForbiddenBinding(Key::Scancode key);
		/// @brief Add a button that will be ignored during the rebinding, this will reset after a call to EndRebind
		static void AddForbiddenBinding(Gamepad::Button button);
		/// @brief Add a button that will be ignored during the rebinding, this will reset after a call to EndRebind
		static void AddForbiddenBinding(Gamepad::Trigger trigger);
		/// @brief Add a button that will be ignored during the rebinding, this will reset after a call to EndRebind
		static void AddForbiddenBinding(Mouse::Button mouse);
		static bool IsRebinding();
		static sptr<Input> GetRebindingInput();
		/// @brief Create an input map with a default name "InputMap_0, 1, 2..."
		/// @return shared pointer to the created input map
		static sptr<InputMap> CreateInputMap();
		/// @brief Create an input map with a name
		/// @return shared pointer to the created input map
		static sptr<InputMap> CreateInputMap(std::string name);
		/// @brief Delete an input map based on the name
		/// @param name Name on the input map
		static void DestroyInputMap(std::string name);

		/// @brief Get an input from one of the input maps;
		static sptr<Input> Get(String mapName, String inputName);
		static std::vector<sptr<InputMap>>& GetInputMaps();
		static sptr<InputMap> GetInputMap(std::string name);
		static std::vector<std::string> GetInputMapNameList();
		/// @brief Used to check what peripheral has been used last, mouse&keyboard or controller
		bool controllerUsedLast = false;  // to do, private + accessor
	private:
		friend Systems;
		bool Rebind(SDL_Event& e);
		void InitGamepad(SDL_JoystickID id);
		bool OnEvent(SDL_Event& event);
	private:
		friend Engine;
		InputMapContainer m_inputMaps;

		//Rebinding
		sptr<Input> m_rebind;
		int m_rebindPeripherals = 0;
		int m_rebindVersion = -1;
		std::vector<Key::Scancode> m_forbiddenKeys;
		std::vector<Gamepad::Button> m_forbiddenButtons;
		std::vector<Gamepad::Trigger> m_forbiddenTriggers;
		std::vector<Mouse::Button> m_forbiddenMouses;
		
	};
}
