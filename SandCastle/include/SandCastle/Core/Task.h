#pragma once

#include "SandCastle/Core/Delegate.h"

namespace SandCastle
{
	/// @brief Type-erased unit of work. Any Task<...> can be queued as an OpaqueTask.
	class OpaqueTask
	{
	public:
		virtual ~OpaqueTask() {};
		/// @brief Run the wrapped delegate once.
		virtual void Perform() = 0;
	};

	/// @brief Concrete task that wraps a Delegate so it can be enqueued on a WorkerThread.
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