#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Nova
{
    class RendererBase
    {
    public:
        virtual ~RendererBase() noexcept = default;

        virtual void Initialize(int frameWidth, int frameHeight) = 0;

        virtual void Shutdown() = 0;

        virtual void SetCamera(
            const glm::mat4 &view,
            const glm::mat4 &projection,
            const glm::vec3 &position) = 0;

        virtual void BeginFrame() = 0;

        virtual void EndFrame() = 0;
    };
}