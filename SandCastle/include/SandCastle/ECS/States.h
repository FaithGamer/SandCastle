#pragma once

#include <map>

#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/TypeId.h"
#include "SandCastle/Core/std_macros.h"
#include "SandCastle/ECS/StateMachine.h"
#include "SandCastle/ECS/System.h"

namespace SandCastle
{
    /// @brief Engine-wide registry of StateMachine<T> instances, one per enum type.
    /// Push<T>() to register a state machine for enum type T, Set<T>(value) to
    /// transition it, Get<T>() to read its current state, Fetch<T>() to grab the
    /// machine itself (e.g. to register callbacks). The States system, when
    /// pushed into Systems, drives Update/LateUpdate/FixedUpdate on every
    /// registered machine.
    class States : public System
    {
    public:
        void Update() override;
        void LateUpdate() override;
        void FixedUpdate() override;

        std::string DebugName() override { return "States"; }

        /// @brief Register a state machine for enum type T. Idempotent in spirit
        /// but will replace an existing entry, dropping previously registered callbacks.
        template <class T>
        static void Push()
        {
            auto id = TypeId::GetId<T>();
            _map.insert(std::make_pair(id, makesptr<StateMachine<T>>()));
        }

        /// @brief Read the current state of T's machine. Logs and returns T() if not pushed.
        template <class T>
        static T Get()
        {
            auto id = TypeId::GetId<T>();
            auto it = _map.find(id);
            if (it == _map.end())
            {
                LOG_ERROR("State machine id {0}, hasn't been pushed.", id);
                return T();
            }
            return std::static_pointer_cast<StateMachine<T>>(it->second)->GetState();
        }

        /// @brief Grab the StateMachine<T> directly, e.g. to register callbacks. nullptr if not pushed.
        template <class T>
        static sptr<StateMachine<T>> Fetch()
        {
            auto id = TypeId::GetId<T>();
            auto it = _map.find(id);
            if (it == _map.end())
            {
                LOG_ERROR("State machine id {0}, hasn't been pushed.", id);
                return nullptr;
            }
            return std::static_pointer_cast<StateMachine<T>>(it->second);
        }

        /// @brief Transition T's machine to `state`. Fires Exit/Enter callbacks. Logs if not pushed.
        template <class T>
        static void Set(T state)
        {
            auto id = TypeId::GetId<T>();
            auto it = _map.find(id);
            if (it == _map.end())
            {
                LOG_ERROR("State machine id {0}, hasn't been pushed.", id);
                return;
            }
            std::static_pointer_cast<StateMachine<T>>(it->second)->SetState(state);
        }

    private:
        static std::map<int32_t, sptr<StateMachineOpaque>> _map;
    };
}
