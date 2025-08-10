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

	template<typename Obj = FunctionDelegate, typename... Args>
	class Task : public OpaqueTask
	{
	public:

		Task(Delegate<void, Obj, Args...>& delegate)
			: m_delegate(delegate)
		{

		}
		Task(Delegate<void, Obj, Args...>&& delegate)
			: m_delegate(std::forward<Delegate<void, Obj, Args...>>(delegate))
		{

		}

		void Perform() override
		{
			m_delegate.Call();
		}
	private:
		Delegate<void, Obj, Args...> m_delegate;
	};
}