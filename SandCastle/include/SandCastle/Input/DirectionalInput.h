#pragma once
#include "SandCastle/Input/Input.h"
#include "SandCastle/Input/ButtonInput.h"
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	/// @brief What state the input is currently in.
	/// This is also the data sent by the broadcasted signal when the input is triggered.
	/// the final state is the sum of all the pressed buttons
	/// and sticks currentDirection vectors. The resulting vector's axis (x, y) are then clamped from 0 ot 1.
	class DirectionalInputState : public InputSignal
	{
	public:
		Vec2f GetVec2f() const override { return direction; }
		Vec2f direction = Vec2f(0, 0);
	};

	//Using either a set of buttons or a stick to represent a Vec2f state
	class DirectionalInput : public Input
	{

	public:
		DirectionalInput(std::string name);

		/// @brief Set all the versions of the various keys, controller buttons, controller triggers, mouse buttons
		/// and their corresponding directions, as well as the controller analog sticks, that are bound to this input.
		/// @param bindings 
		virtual void SetBindings(const DirectionalBindings& bindings);

		virtual std::string GetName() const override;
		virtual InputType GetType() const override;
		virtual void RemoveAllBindings() override;

		/// @brief Bind binding to a stick
		/// @param stick The controller stick
		void BindStick(Gamepad::Stick stick);
		/// @brief Bind binding to buttons, for example W, A, S, D
		/// @param buttons Each value of the vector contains a button bound with a direction
		/// When multiple buttons are being pressed at the same time, the final state is the sum of all the pressed buttons
	    /// and sticks direction vectors, where each axis (x, y) is clamped from 0 ot 1.
		void BindButtons(std::vector<DirectionalButton> buttons);
		/// @brief Mouse wheel will trigger Y axis 1 or -1
		/// @param mouseWheel set true to listen to mouse wheel, set to false by default.
		void SetMouseWheel(bool mouseWheel);
		/// @brief Set a specific binding's stick
		/// @param version Binding version
		void SetStick(Gamepad::Stick stick, int version);
		/// @brief Set a specific binding's buttons
		/// @param version Is equivalent to the order of insertion (starting at 0)
		void SetButtons(std::vector<DirectionalButton> buttons, int version);
		/// @brief Remove a binding
		/// @param version Binding version
		void RemoveBinding(int version);
		/// @brief Set stick deadzone
		/// @param deadzone range from 0.0 to 1.0, where 1.0 = 100% deadzone
		void SetStickDeadZone(float deadzone);
		/// @brief Set trigger deadzone
		/// @param deadzone range from 0.0 to 1.0, where 1.0 = 100% deadzone
		void SetTriggerDeadZone(float deadzone);

		/// @brief Check if the stick is used in any of the bindings
		/// @return true if yes
		bool HaveBinding(Gamepad::Stick stick);
		/// @brief Check if any of the directional button in the vector matches any of the directional button in another binding
		/// @return true if yes
		bool HaveBinding(std::vector<DirectionalButton> buttons);
		/// @brief Check if the button is used in any of the bindings
		/// @return true if yes
		bool HaveBinding(Button button);
		/// @brief Get the count of bindings
		int GetBindingsCount() const;
		/// @brief Get the bindings
		DirectionalBindings& GetBindings();

	protected:
		virtual bool KeyPressed(const SDL_Event& e) override;
		virtual bool KeyReleased(const SDL_Event& e) override;
		virtual bool MouseButtonPressed(const SDL_Event& e) override;
		virtual bool MouseButtonReleased(const SDL_Event& e) override;
		virtual bool MouseWheelMoved(const SDL_Event& e) override;
		virtual bool GamepadButtonPressed(const SDL_Event& e) override;
		virtual bool GamepadButtonReleased(const SDL_Event& e) override;
		virtual bool GamepadTriggerMoved(const SDL_Event& e) override;
		virtual bool GamepadStickMoved(const SDL_Event& e) override;

		virtual void UpdateEventListened() override;

	private:
		friend InputMap;
		void AxisMove(float value, bool x, Direction& direction);

		void ComputeState();

	private:
		std::string m_name;
		DirectionalBindings m_bindings;
		DirectionalInputState m_state;
		bool m_mouseWheel;
		float m_triggerDeadzone;
		float m_stickDeadzone;

	};
}