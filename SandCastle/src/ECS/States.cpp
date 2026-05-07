#include "pch.h"
#include "SandCastle/ECS/States.h"

namespace SandCastle
{
    std::map<int32_t, sptr<StateMachineOpaque>> States::_map;

    void States::Update()
    {
        for (auto& kv : _map)
            kv.second->Update();
    }

    void States::LateUpdate()
    {
        for (auto& kv : _map)
            kv.second->LateUpdate();
    }

    void States::FixedUpdate()
    {
        for (auto& kv : _map)
            kv.second->FixedUpdate();
    }
}
