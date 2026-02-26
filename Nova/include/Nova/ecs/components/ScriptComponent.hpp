#pragma once
#include <Nova/ecs/components/ScriptController.hpp> // TODO Move script controller somewhere else
#include <memory>
#include <type_traits>

struct CPPScriptComponent
{
    std::unique_ptr<ScriptController> ControllerInstance;

    template <typename T, typename... Args>
    requires std::is_base_of_v<ScriptController, T>
    static CPPScriptComponent Create(Args&&... args)
    {
        return CPPScriptComponent { 
            .ControllerInstance = std::make_unique<T>(std::forward<Args>(args)...),
        };
    }
};