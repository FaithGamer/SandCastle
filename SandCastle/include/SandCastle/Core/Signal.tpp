#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	template <typename T>
	template <typename Obj>
	void SignalSender<T>::AddListener(void (Obj::* callback)(T), Obj* const listener, SignalPriority priority)
	{
		m_listeners.insert(makesptr<MethodCallback<T, Obj>>(listener, callback, priority));
	}

	template <typename T>
	void SignalSender<T>::AddListener(void (*callback)(T), SignalPriority priority)
	{
		m_listeners.insert(makesptr<FunctionCallback<T>>(callback, priority));
	}

	template <typename T>
	void SignalSender<T>::RemoveListener(void* const listener)
	{
		/*for (auto callback = m_listeners.begin(); callback != m_listeners.end();)
		{
			if (callback->listener == listener)
			{
				m_listeners.erase(callback++);
			}
			else
			{
				callback++;
			}
		}*/
	}

	template <typename T>
	void SignalSender<T>::RemoveListener(void (*function)(T))
	{
		/*for (auto callback = m_listeners.begin(); callback != m_listeners.end();)
		{
			if (callback->delegate.IsSameFunction(function))
			{
				m_listeners.erase(callback++);
			}
			else
			{
				callback++;
			}
		}*/

	}

	template<typename T>
	void SignalSender<T>::SendSignal(T& signal)
	{
		for (const sptr<OpaqueCallback<T>>& listener : m_listeners)
		{
			listener->Call(signal);
		}
	}
	template<typename T>
	void SignalSender<T>::SendSignal(T&& signal)
	{
		for (const sptr<OpaqueCallback<T>>& listener : m_listeners)
		{
			listener->Call(std::forward<T>(signal));
		}
	}
}