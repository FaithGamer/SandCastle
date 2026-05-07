#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "SandCastle/Core/std_macros.h"

namespace SandCastle
{
    /// @brief Type-erased base for StateMachine<T>, used by States to drive
    /// every machine without knowing its enum type at the call site.
    class StateMachineOpaque
    {
    public:
        virtual ~StateMachineOpaque() = default;
        virtual void Update() = 0;
        virtual void LateUpdate() = 0;
        virtual void FixedUpdate() = 0;
    };

    /// @brief Finite state machine driven by an enum (or any equality-comparable hashable T).
    /// Register member-function callbacks per state with PushEnter/PushExit/
    /// PushUpdate/PushLateUpdate/PushFixedUpdate. SetState fires Exit on the
    /// outgoing state and Enter on the new one. Drive Update/LateUpdate/
    /// FixedUpdate through the States system, which owns each machine.
    template <class T>
    class StateMachine : public StateMachineOpaque
    {
    public:
        StateMachine() = default;
        StateMachine(T initialState) : _state(initialState) {}
        ~StateMachine() override = default;

        /// @brief Current state.
        inline T GetState() const { return _state; }

        /// @brief Switch state. Fires Exit on the old state, then Enter on the new one.
        /// No-op if the requested state is already current.
        inline void SetState(T state)
        {
            if (state == _state)
                return;

            Callback(_state, _exit);
            _state = state;
            Callback(state, _enter);
        }

        /// @brief Dispatch every Update callback registered for the current state.
        void Update() override     { Callback(_state, _update); }
        /// @brief Dispatch every LateUpdate callback registered for the current state.
        void LateUpdate() override { Callback(_state, _lateUpdate); }
        /// @brief Dispatch every FixedUpdate callback registered for the current state.
        void FixedUpdate() override{ Callback(_state, _fixed); }

        /// @brief Register `caller->method()` to run every Update while in `state`.
        template <class C>
        void PushUpdate(T state, void (C::* method)(), C* caller)
        {
            _update[state].emplace_back(std::bind(method, caller));
        }
        /// @brief Register `caller->method()` to run every LateUpdate while in `state`.
        template <class C>
        void PushLateUpdate(T state, void (C::* method)(), C* caller)
        {
            _lateUpdate[state].emplace_back(std::bind(method, caller));
        }
        /// @brief Register `caller->method()` to run every FixedUpdate while in `state`.
        template <class C>
        void PushFixedUpdate(T state, void (C::* method)(), C* caller)
        {
            _fixed[state].emplace_back(std::bind(method, caller));
        }
        /// @brief Register `caller->method()` to run when entering `state`.
        template <class C>
        void PushEnter(T state, void (C::* method)(), C* caller)
        {
            _enter[state].emplace_back(std::bind(method, caller));
        }
        /// @brief Register `caller->method()` to run when leaving `state`.
        template <class C>
        void PushExit(T state, void (C::* method)(), C* caller)
        {
            _exit[state].emplace_back(std::bind(method, caller));
        }

    private:
        using CallbackMap = std::unordered_map<T, std::vector<std::function<void()>>>;

        void Callback(T state, CallbackMap& cb)
        {
            auto it = cb.find(state);
            if (it == cb.end())
                return;
            for (auto& fn : it->second)
                fn();
        }

        CallbackMap _update;
        CallbackMap _lateUpdate;
        CallbackMap _fixed;
        CallbackMap _enter;
        CallbackMap _exit;
        T _state = (T)0;
    };
}
