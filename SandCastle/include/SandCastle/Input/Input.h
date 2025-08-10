#pragma once

#include <SDL3/SDL.h>
#include "SandCastle/Entt.h"

#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Signal.h"
#include "SandCastle/Core/Vec.h"

namespace SandCastle
{
	class InputMap;


	/// @brief Type of Input, each type may hold a different state or send different type of signal
	enum class InputType : int
	{
		Button = 0,
		Directional,
		Textual
	};

	/// @brief Parent class for data sent by input when they are triggered.
	class InputSignal
	{
	public:
		virtual bool GetBool() const 
		{
			LOG_WARN("Input Signal GetBool not implemented, false returned.");
			return false;
		}
		virtual Vec2f GetVec2f() const 
		{ 
			LOG_WARN("Input Signal GetVec2f not implemented, default value returned.");
			return Vec2f();
		}
		virtual char* GetText() const 
		{ 
			LOG_WARN("Input Signal GetText not implemented, nullptr returned.");
			return nullptr; 
		}
		virtual float GetFloat() const
		{
			LOG_WARN("Input Signal GetFloat not implemented, 0.0f returned.");
			return 0.0f;
		}
	};

	/// @brief The name of an InputType
	std::string InputTypeName(InputType type);

	/// @brief Interface for an input, send signal when it's binding is triggered
	class Input : public std::enable_shared_from_this<Input>
	{
	public:
		/// @brief What events does an input listen to
		enum InputEventFlag : int
		{
			KeyButtonFlag = 1,
			KeyTextFlag = 2,
			MouseButtonFlag = 4,
			MouseWheelFlag = 8,
			MouseMovementFlag = 16,
			GamepadButtonFlag = 32,
			GamepadStickFlag = 64,
			GamepadTriggerFlag = 128
		};
	public:
		virtual ~Input() {};
		virtual std::string GetName() const = 0;
		virtual InputType GetType() const = 0;
		virtual void RemoveAllBindings() = 0;

		Signal<InputSignal*> signal;

		//To do, add enable mouse and keyboard/controller

	protected:
		Input() {};
		friend class InputMap;

		virtual bool KeyPressed(const SDL_Event& e) { return false; }
		virtual bool KeyReleased(const SDL_Event& e) { return false; }
		virtual bool MouseButtonPressed(const SDL_Event& e) { return false; }
		virtual bool MouseButtonReleased(const SDL_Event& e) { return false; }
		virtual bool MouseWheelMoved(const SDL_Event& e) { return false; }
		virtual bool MouseMoved(const SDL_Event& e) { return false; }
		virtual bool GamepadButtonPressed(const SDL_Event& e) { return false; }
		virtual bool GamepadButtonReleased(const SDL_Event& e) { return false; }
		virtual bool GamepadStickMoved(const SDL_Event& e) { return false; }
		virtual bool GamepadTriggerMoved(const SDL_Event& e) { return false; }
		virtual bool TextEntered(const SDL_Event& e) { return false; }

		virtual void UpdateEventListened() = 0;
		void OnEventListenedUpdated();
		int m_eventsListened; //flags

	private:

		friend class InputMap;
		InputMap* m_inputMap = nullptr;
	};
}