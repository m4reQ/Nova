#pragma once
#include <Nova/core/Event.hpp>
#include <entt/entt.hpp>

class ScriptController
{
public:
    virtual ~ScriptController() noexcept = default;

    virtual void OnAttach(entt::registry& scene, entt::entity parentEntity) { };
    virtual void OnEvent(const Nova::Event& event) { };
    virtual void OnUpdate(double frametime) { };
    virtual void OnRender() { };
};