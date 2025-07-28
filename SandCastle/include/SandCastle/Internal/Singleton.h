#pragma once
#include "SandCastle/Core/std_macros.h"


namespace SandCastle
{
	class Engine;

	template <typename T>
	class Singleton
	{
	public:
		T& operator= (const T&) = delete;
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