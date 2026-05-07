#pragma once

#include "SandCastle/Core/Serialization.h"
#include "SandCastle/ECS/Entity.h"
#include "SandCastle/ECS/States.h"
#include "SandCastle/ECS/System.h"

namespace SandCastle
{
    /// @brief Empty payload type, use as the D parameter of GameSys when the
    /// system has no per-run data component.
    struct NoData {};
    /// @brief Empty payload type, use as the A parameter of GameSys when the
    /// system has no asset component.
    struct NoAsset {};

    /// @brief Convenience System base that pairs the system with two component
    /// payloads: a data entity (D, recreated per run) and an assets entity
    /// (A, created once at startup). Subclasses override OnStart/LoadAssets/
    /// LinkCallbacks to wire themselves up, and use PushEnter/PushExit/
    /// PushUpdate/PushLateUpdate/PushFixedUpdate to attach callbacks to a
    /// state machine registered with States. Inherits from Serializable so the
    /// system can be serialized as part of the save/load flow; default Serialize/
    /// Deserialize are no-ops.
    template <class D, class A>
    class GameSys : public System, public Serializable
    {
    public:
        ~GameSys() override;

        void Deserialize(Serialized& config) override {}
        Serialized Serialize() override { return Serialized(); }

    protected:
        /// @brief Final System::Start. Invokes LoadAssets, then LinkCallbacks, then OnStart.
        /// Subclasses should override the three hooks below rather than Start.
        void Start() override;
        /// @brief Subclass hook: post-callback startup logic.
        virtual void OnStart() {}
        /// @brief Subclass hook: load assets into _a (typically populates the assets component).
        virtual void LoadAssets() {}
        /// @brief Subclass hook: register callbacks on state machines, signals, UI events, ...
        virtual void LinkCallbacks() {}

        /// @brief Register a state Enter callback on the StateMachine<T> registered with States.
        template <class T, class C>
        void PushEnter(T state, void (C::* method)());
        /// @brief Register a state Exit callback on the StateMachine<T> registered with States.
        template <class T, class C>
        void PushExit(T state, void (C::* method)());
        /// @brief Register an Update callback on the StateMachine<T> registered with States.
        template <class T, class C>
        void PushUpdate(T state, void (C::* method)());
        /// @brief Register a LateUpdate callback on the StateMachine<T> registered with States.
        template <class T, class C>
        void PushLateUpdate(T state, void (C::* method)());
        /// @brief Register a FixedUpdate callback on the StateMachine<T> registered with States.
        template <class T, class C>
        void PushFixedUpdate(T state, void (C::* method)());

        /// @brief Destroy any existing data entity and create a fresh one carrying a default-constructed D.
        void CreateData();
        /// @brief Create the assets entity and attach a default-constructed A to it.
        void CreateAssets();
        /// @brief Destroy the data entity if any, leaving assets alone.
        void DeleteData();

    protected:
        Entity _de;
        Entity _da;
        D* _d = nullptr;
        A* _a = nullptr;
    };

    template <class D, class A>
    GameSys<D, A>::~GameSys()
    {
        DeleteData();
    }

    template <class D, class A>
    void GameSys<D, A>::Start()
    {
        LoadAssets();
        LinkCallbacks();
        OnStart();
    }

    template <class D, class A>
    void GameSys<D, A>::CreateData()
    {
        DeleteData();
        _de = Entity::Create();
        _d = _de.AddGet<D>();
    }

    template <class D, class A>
    void GameSys<D, A>::CreateAssets()
    {
        _da = Entity::Create();
        _a = _da.AddGet<A>();
    }

    template <class D, class A>
    void GameSys<D, A>::DeleteData()
    {
        if (_de.Valid())
            _de.Destroy();
        _d = nullptr;
    }

    template <class D, class A>
    template <class T, class C>
    void GameSys<D, A>::PushEnter(T state, void (C::* method)())
    {
        States::Fetch<T>()->PushEnter(state, method, static_cast<C*>(this));
    }

    template <class D, class A>
    template <class T, class C>
    void GameSys<D, A>::PushExit(T state, void (C::* method)())
    {
        States::Fetch<T>()->PushExit(state, method, static_cast<C*>(this));
    }

    template <class D, class A>
    template <class T, class C>
    void GameSys<D, A>::PushUpdate(T state, void (C::* method)())
    {
        States::Fetch<T>()->PushUpdate(state, method, static_cast<C*>(this));
    }

    template <class D, class A>
    template <class T, class C>
    void GameSys<D, A>::PushLateUpdate(T state, void (C::* method)())
    {
        States::Fetch<T>()->PushLateUpdate(state, method, static_cast<C*>(this));
    }

    template <class D, class A>
    template <class T, class C>
    void GameSys<D, A>::PushFixedUpdate(T state, void (C::* method)())
    {
        States::Fetch<T>()->PushFixedUpdate(state, method, static_cast<C*>(this));
    }
}
