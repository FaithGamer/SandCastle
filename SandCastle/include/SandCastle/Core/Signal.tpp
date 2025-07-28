#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	template <typename SignalType>
	template <typename ListenerType>
	void SignalSender<SignalType>::AddListener(void (ListenerType::* callback)(SignalType), ListenerType* const listener, SignalPriority priority)
	{
		Delegate<void, SignalType> delegate(callback, listener);
		m_listeners.insert(SignalCallback<SignalType>(delegate, listener, priority));
	}

	template <typename SignalType>
	void SignalSender<SignalType>::AddListener(void (*callback)(SignalType), SignalPriority priority)
	{
		Delegate delegate(callback);
		m_listeners.insert(SignalCallback<SignalType>(delegate, nullptr, priority));
	}

	template <typename SignalType>
	void SignalSender<SignalType>::RemoveListener(void* const listener)
	{
		for (auto callback = m_listeners.begin(); callback != m_listeners.end();)
		{
			if (callback->listener == listener)
			{
				m_listeners.erase(callback++);
			}
			else
			{
				callback++;
			}
		}
	}

	template <typename SignalType>
	void SignalSender<SignalType>::RemoveListener(void (*function)(SignalType))
	{
		for (auto callback = m_listeners.begin(); callback != m_listeners.end();)
		{
			if (callback->delegate.IsSameFunction(function))
			{
				m_listeners.erase(callback++);
			}
			else
			{
				callback++;
			}
		}

	}

	template<typename SignalType>
	void SignalSender<SignalType>::SendSignal(SignalType& signal)
	{
		for (SignalCallback<SignalType> listener : m_listeners)
		{
			listener.delegate.Call(std::forward<SignalType>(signal));
		}
	}
	template<typename SignalType>
	void SignalSender<SignalType>::SendSignal(SignalType&& signal)
	{
		for (SignalCallback<SignalType> listener : m_listeners)
		{
			listener.delegate.Call(std::forward<SignalType>(signal));
		}
	}
}