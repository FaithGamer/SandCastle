#pragma once
#include <string>

#include "SandCastle/Input/InputMap.h"
#include "SandCastle/Internal/Singleton.h"
#include "SandCastle/Input/Gamepad.h"
#include "SandCastle/Input/Mouse.h"
#include "SandCastle/Input/Keyboard.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	class Systems;
	/// @brief Internal owner of all InputMaps registered with the engine.
	struct InputMapContainer
	{
		sptr<InputMap> Add(std::string name);
		sptr<InputMap> Get(std::string name);
		void Remove(std::string name);

		std::vector<sptr<InputMap>> inputs;
		std::vector<std::string> names;
	};

	/// @brief Engine-wide input registry and SDL event router.
	/// Holds every InputMap, drives the rebinding workflow, and tracks whether
	/// the player last interacted using mouse/keyboard or gamepad. Access via
	/// Inputs::Instance() or its many static helpers.
	class Inputs : public Singleton<Inputs>
	{
	public:
		/// @brief Bitmask of peripherals to listen to during a rebinding session.
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
		/// @brief True if a rebinding session is currently in progress.
		static bool IsRebinding();
		/// @brief Get the input being rebound (or nullptr).
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

		/// @brief Create the default UI gamepad-navigation bindings on the given map.
		/// Adds six ButtonInputs ("ui_left", "ui_right", "ui_up", "ui_down",
		/// "ui_select", "ui_cancel") bound to the D-pad + South/East and to
		/// arrow keys + Enter/Escape, then wires them to Ui::NavigateLeft / ... /
		/// Ui::Select / Ui::Cancel. Select and Cancel fire on both press and
		/// release; the four directions fire on press only.
		/// Logs an error and does nothing if the map doesn't exist.
		static void CreateDefaultGamepadNav(const String& mapName);

		/// @brief Get an input from one of the input maps;
		static sptr<Input> Get(String mapName, String inputName);
		/// @brief Direct vector access to every input map (advanced).
		static std::vector<sptr<InputMap>>& GetInputMaps();
		/// @brief Find a map by name; nullptr if unknown.
		static sptr<InputMap> GetInputMap(std::string name);
		/// @brief Snapshot of every map name currently registered.
		static std::vector<std::string> GetInputMapNameList();

		/*---Input mode (gamepad vs mouse/keyboard)---*/

		/// @brief True when the player is in gamepad mode. The engine defaults
		/// to gamepad mode at launch if a controller is plugged in or if running
		/// on a Steam Deck; otherwise it defaults to mouse/keyboard mode and
		/// flips automatically based on real input events (see AutoToggleGamepadMode).
		/// In gamepad mode the OS cursor is hidden (unless disabled via
		/// SetHideCursorOnGamepadMode) and Ui draws the gamepad-nav selector
		/// around the navigated UiElem.
		static bool IsGamepadMode();
		/// @brief Force gamepad mode on or off. Sends gamepadModeSignal and
		/// hides/shows the cursor. Note that if AutoToggleGamepadMode is left
		/// at its default (true), the next real input event may flip the mode
		/// back — call AutoToggleGamepadMode(false) first to lock it.
		static void SetGamepadMode(bool gamepad);
		/// @brief When true (default), gamepad mode flips automatically as the
		/// player switches between controller and mouse/keyboard. Pass false for
		/// games that never want gamepad navigation even if a controller is
		/// connected, or that drive the mode entirely through SetGamepadMode.
		static void AutoToggleGamepadMode(bool autoToggle);
		/// @brief Current AutoToggleGamepadMode setting.
		static bool IsAutoToggleGamepadMode();
		/// @brief When true (default), entering gamepad mode hides the OS cursor
		/// and leaving it shows the cursor again. Pass false for games that
		/// want to keep the cursor visible regardless of the input mode (e.g.
		/// games using a custom OS cursor where stray gamepad noise would
		/// otherwise blink it off).
		static void SetHideCursorOnGamepadMode(bool hide);
		/// @brief Current SetHideCursorOnGamepadMode setting.
		static bool IsHideCursorOnGamepadMode();
		/// @brief Broadcast on every gamepad-mode flip. Payload is the new
		/// mode (true = gamepad, false = mouse/keyboard).
		Signal<bool> gamepadModeSignal;
	private:
		friend Systems;
		bool Rebind(SDL_Event& e);
		void InitGamepad(SDL_JoystickID id);
		bool OnEvent(SDL_Event& event);
		void AutoDetectGamepadModeFromEvent(const SDL_Event& event);
		void SetGamepadModeInternal(bool gamepad);
		void ApplyGamepadMode(bool gamepad);
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

		//Input mode
		bool m_gamepadMode = false;
		bool m_autoToggleGamepadMode = true;
		bool m_hideCursorOnGamepadMode = true;
	};
}
