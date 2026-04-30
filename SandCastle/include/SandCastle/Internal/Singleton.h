#pragma once
#include "SandCastle/Core/std_macros.h"


namespace SandCastle
{
	class Engine;

	/// @brief CRTP singleton base used by engine-wide systems (Audio, Renderer2D,
	/// Window, Inputs, Ui, Physics, Assets, Log, ...). Lazily constructs T on
	/// first Instance() call; Engine::Kill() tears them down at shutdown.
	template <typename T>
	class Singleton
	{
	public:
		T& operator= (const T&) = delete;
		/// @brief Lazily construct (if needed) and return the shared instance.
		static sptr<T> Instance()
		{
			if (m_instance == nullptr)
			{
				m_instance.reset(new T);
			}
			return m_instance;
		}
	protected:
		friend Engine;

		/// @brief Engine-only: destroy the singleton (called during shutdown).
		static void Kill()
		{
			m_instance.reset();
		}

		Singleton() {}
		virtual ~Singleton() {}

	private:
		inline static sptr<T> m_instance = nullptr;

	};
}