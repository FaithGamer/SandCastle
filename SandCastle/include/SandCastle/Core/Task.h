#pragma once

#include "SandCastle/Core/Delegate.h"

namespace SandCastle
{
	class OpaqueTask
	{
	public:
		virtual ~OpaqueTask() {};
		virtual void Perform() = 0;
	};

	template<typename Ret, typename... Args>
	class Task : public OpaqueTask
	{
	public:

		Task(Delegate<Ret, Args...>& delegate)
			: m_delegate(delegate)
		{

		}
		Task(Delegate<Ret, Args...>&& delegate)
			: m_delegate(std::forward<Delegate<Ret, Args...>>(delegate))
		{

		}

		void Perform() override
		{
			m_delegate.Call();
		}
	private:
		Delegate<Ret, Args...> m_delegate;
	};
}