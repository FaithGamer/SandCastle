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

	template<typename SignalType>
	struct SignalCallback
	{
		SignalCallback(Delegate<void, SignalType>& Delegate, void* const Listener, SignalPriority Priority)
			: delegate(Delegate), listener(Listener), priority(Priority)
		{

		}
		void* const listener;
		Delegate<void, SignalType> delegate;
		SignalPriority priority;
	};

	/// @brief Send signals to a collection of listeners using Delegate.
	/// This is an implementation of the observer pattern
	template<typename SignalType>
	class SignalSender
	{
	public:

		template <typename ListenerType>
		void AddListener(void (ListenerType::* callback)(SignalType), ListenerType* const listener, SignalPriority priority = SignalPriority::medium);
		void AddListener(void (*callback)(SignalType), SignalPriority priority = SignalPriority::medium);


		void RemoveListener(void (*callback)(SignalType));
		void RemoveListener(void* const listener);
		//to do add removeListener for object specific function

		void SendSignal(SignalType& signalData);
		void SendSignal(SignalType&& signalData);
	private:
		template<typename SignalType>
		struct CompareCallback
		{
			bool operator()(const SignalCallback<SignalType>& l, const SignalCallback<SignalType>& r) const
			{
				return l.priority < r.priority;
			}
		};
		std::multiset<SignalCallback<SignalType>, CompareCallback<SignalType>> m_listeners;

	};
}
#include "Signal.tpp"