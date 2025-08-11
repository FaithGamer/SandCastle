#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/Signal.h"

namespace SandCastle
{
	template <typename T>
	template <typename Obj>
	void Signal<T>::Listen(void (Obj::* method)(T), Obj* const listener, SignalPriority priority)
	{
		m_listeners.insert(makesptr<MethodCallback<Obj>>(listener, method, priority));
	}

	template <typename T>
	void Signal<T>::Listen(void (*callback)(T), SignalPriority priority)
	{
		m_listeners.insert(makesptr<FunctionCallback>(callback, priority));
	}

	template <typename T>
	template <typename Obj>
	void Signal<T>::StopListen(Obj* const listener)
	{
		for (auto callback_it = m_listeners.begin(); callback_it != m_listeners.end();)
		{
			auto& callback = (*callback_it);
			if (callback->Type() != TypeId::GetId<Obj>())
			{
				callback_it++;
			}
			else
			{
				auto methodCb = static_pointer_cast<MethodCallback<Obj>>(callback);
				if (methodCb->delegate.GetObj() == listener)
				{
					m_listeners.erase(callback_it);
					break;
				}
				else
				{
					callback_it++;
				}
			}
		}
	}

	template <typename T>
	void Signal<T>::StopListen(void (*function)(T))
	{
		for (auto callback_it = m_listeners.begin(); callback_it != m_listeners.end();)
		{
			auto& callback = (*callback_it);
			if (callback->Type() != -1) //not a function callback
			{
				callback_it++;
			}
			auto functionCb = static_pointer_cast<FunctionCallback>(callback);
			if (functionCb->delegate.GetFunction() == function)
			{
				m_listeners.erase(callback_it);
				break;
			}
			else
			{
				callback_it++;
			}
		}

	}

	template<typename T>
	void Signal<T>::Send(T& signal)
	{
		for (const sptr<OpaqueCallback>& listener : m_listeners)
		{
			listener->Call(signal);
		}
	}
	template<typename T>
	void Signal<T>::Send(T&& signal)
	{
		for (const sptr<OpaqueCallback>& listener : m_listeners)
		{
			listener->Call(std::forward<T>(signal));
		}
	}
}