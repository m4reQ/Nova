#pragma once
#include <Nova/graphics/RendererBase.hpp>
#include <Nova/graphics/opengl/Framebuffer.hpp>

namespace Nova
{
    class PBRRenderer : public RendererBase
    {
    public:
        void Initialize(int frameWidth, int frameHeight) override;

        void Shutdown() noexcept override;

        void SetCamera(
            const glm::mat4 &view,
            const glm::mat4 &projection,
            const glm::vec3 &position) noexcept override;

    private:
        Framebuffer framebuffer_;
    };
}