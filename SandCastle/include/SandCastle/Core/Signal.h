#pragma once

#include <unordered_set>
#include <unordered_map>
#include <set>
#include <any>
#include <functional>

#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Delegate.h"
#include "SandCastle/Core/TypeId.h"

namespace SandCastle
{
	enum class SignalPriority : int
	{
		high = 0,
		medium,
		low
	};

	/// @brief Wrapper around a Delegate of return void and argument T
	/// The Signal can be listened by method or function.
	///	Use Send to trigger all the bound callbacks.
	/// Ensure no double listener.
	template<typename T>
	class Signal
	{
	public:

		template <typename Obj>
		void Listen(Obj* const listener, void (Obj::* method)(T), SignalPriority priority = SignalPriority::medium);
		void Listen(void (*callback)(T), SignalPriority priority = SignalPriority::medium);

		template <typename Obj>
		void StopListen(Obj* const listener);
		void StopListen(void (*callback)(T));
		//to do add removeListener for object specific function

		void Send(T& signalData);
		void Send(T&& signalData);
	private:
		struct OpaqueCallback
		{
			OpaqueCallback(SignalPriority prio, const void* Listener = nullptr) : priority(prio), listener(Listener)
			{

			}
			virtual void Call(const T& data) = 0;
			virtual void Call(const T&& data) = 0;
			virtual int32_t Type() const = 0;
			bool Equals(const sptr<OpaqueCallback>& other) const
			{
				return Type() == other->Type() && listener == other->listener;
			}
			SignalPriority priority;
			const void* listener = nullptr;
		};

		template<typename Obj>
		struct MethodCallback : public OpaqueCallback
		{
			MethodCallback(Obj* obj, void(Obj::* method)(T), SignalPriority Priority)
				: OpaqueCallback(Priority, obj),
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
			int32_t Type() const override
			{
				return std::abs(TypeId::GetId<Obj>());
			}
			Delegate<void, Obj, T> delegate;

		};

		struct FunctionCallback : public OpaqueCallback
		{
			FunctionCallback(void(*function)(T), SignalPriority Priority)
				: OpaqueCallback(Priority),
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
			int32_t Type() const override
			{
				return -1;
			}
			Delegate<void, FunctionDelegate, T> delegate;
		};
	private:
		template<typename T>
		struct CompareCallback
		{
			bool operator()(const sptr<OpaqueCallback>& l, const sptr<OpaqueCallback>& r) const
			{
				if (l->Equals(r))
					return false;
				return l->priority < r->priority;
			}
		};
		std::set<sptr<OpaqueCallback>> m_listeners;
	};
}
#include "Signal.tpp"