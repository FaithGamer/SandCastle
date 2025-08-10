#pragma once

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <any>
#include <functional>

#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Delegate.h"

namespace SandCastle
{
	enum class SignalPriority : int
	{
		high = 0,
		medium,
		low
	};

	template<typename T>
	struct SignalCallback
	{
		SignalCallback(Delegate<void, T>& Delegate, void* const Listener, SignalPriority Priority)
			: delegate(Delegate), listener(Listener), priority(Priority)
		{

		}
		void* const listener;
		Delegate<void, T> delegate;
		SignalPriority priority;
	};

	template<typename T>
	struct OpaqueCallback
	{
		virtual void Call(const T& data) = 0;
		virtual void Call(const T&& data) = 0;
		//virtual int Type() = 0;
		//virtual bool Equals(sptr<OpaqueCallback>& other) = 0;
	};

	template<typename T, typename Obj>
	struct MethodCallback : public OpaqueCallback<T>
	{
		MethodCallback(Obj* obj, void(Obj::* method)(T), SignalPriority Priority)
			: priority(Priority),
			delegate(method, obj)
		{

		}
		void Call(const T& data) override
		{
			delegate.Call(data);
		}
		void Call(const T&& data) override
		{
			delegate.Call(data);
		}
		/*int Type() override
		{
			return 0;
		}
		bool Equals(sptr<OpaqueCallback>& other) override
		{
			if (Type() != other->Type())
				return false;
			if
		}*/

		Delegate<void, Obj, T> delegate;
		SignalPriority priority;
	};

	template<typename T>
	struct FunctionCallback : public OpaqueCallback<T>
	{
		FunctionCallback(void(*function)(T), SignalPriority Priority)
			: priority(Priority),
			delegate(function)
		{

		}
		void Call(const T& data) override
		{
			delegate.Call(data);
		}
		void Call(const T&& data) override
		{
			delegate.Call(data);
		}
		Delegate<void, FunctionDelegate, T> delegate;
		SignalPriority priority;
	};

	/// @brief Send signals to a collection of listeners using Delegate.
	/// This is an implementation of the observer pattern
	template<typename T>
	class SignalSender
	{
	public:

		template <typename Obj>
		void AddListener(void (Obj::* callback)(T), Obj* const listener, SignalPriority priority = SignalPriority::medium);
		void AddListener(void (*callback)(T), SignalPriority priority = SignalPriority::medium);


		void RemoveListener(void (*callback)(T));
		void RemoveListener(void* const listener);
		//to do add removeListener for object specific function

		void SendSignal(T& signalData);
		void SendSignal(T&& signalData);
	private:
		/*template<typename T>
		struct CompareCallback
		{
			bool operator()(const sptr<OpaqueCallback<T>>& l, const sptr<OpaqueCallback<T>>& r) const
			{
				return l.priority < r.priority;
			}
		};*/
		std::multiset<sptr<OpaqueCallback<T>>> m_listeners;

	};
}
#include "Signal.tpp"