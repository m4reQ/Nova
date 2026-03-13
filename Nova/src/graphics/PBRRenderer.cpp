#include <Nova/graphics/PBRRenderer.hpp>

using namespace Nova;

void PBRRenderer::Initialize(int frameWidth, int frameHeight)
{
    framebuffer_ = Framebuffer({
        FramebufferAttachmentSpec{
            .Width = frameWidth,
            .Height = frameHeight,
            .Format = InternalFormat::RGBA8,
            .Flags = AttachmentFlags::DrawDest,
        },
        FramebufferAttachmentSpec{
            .Width = frameWidth,
            .Height = frameHeight,
            .Format = InternalFormat::RGBA8,
            .Flags = AttachmentFlags::DrawDest,
        },
        FramebufferAttachmentSpec{
            .Width = frameWidth,
            .Height = frameHeight,
            .Format = InternalFormat::Depth24Stencil8,
            .Flags = AttachmentFlags::UseRenderbuffer,
        },
    });
}