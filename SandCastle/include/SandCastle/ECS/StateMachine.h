#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
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
        /// Scopes losing coverage fire their exit-of-scope callback after Exit; scopes
        /// gaining coverage fire their enter-of-scope callback before Enter.
        /// No-op if the requested state is already current.
        inline void SetState(T state)
        {
            if (state == _state)
                return;

            Callback(_state, _exit);
            for (Scope& scope : _scopes)
                if (scope.onExitScope && scope.Covers(_state) && !scope.Covers(state))
                    scope.onExitScope();
            T previous = _state;
            _state = state;
            for (Scope& scope : _scopes)
                if (scope.onEnterScope && scope.Covers(state) && !scope.Covers(previous))
                    scope.onEnterScope();
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

        /// @brief Register a scope: a set of states paired with enter/exit-of-scope
        /// callbacks. `onEnterScope` fires when transitioning from a state outside
        /// the set to one inside it, before that state's Enter callbacks;
        /// `onExitScope` fires when transitioning from inside the set to outside,
        /// after the old state's Exit callbacks. Transitions between two states of
        /// the set fire neither. If the current state is already inside the set,
        /// `onEnterScope` fires immediately.
        void PushScope(std::unordered_set<T> states, std::function<void()> onEnterScope, std::function<void()> onExitScope)
        {
            _scopes.emplace_back(Scope{ std::move(states), std::move(onEnterScope), std::move(onExitScope) });
            Scope& scope = _scopes.back();
            if (scope.onEnterScope && scope.Covers(_state))
                scope.onEnterScope();
        }

    private:
        using CallbackMap = std::unordered_map<T, std::vector<std::function<void()>>>;

        struct Scope
        {
            std::unordered_set<T> states;
            std::function<void()> onEnterScope;
            std::function<void()> onExitScope;

            bool Covers(T state) const { return states.count(state) != 0; }
        };

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
        std::vector<Scope> _scopes;
        T _state = (T)0;
    };
}
