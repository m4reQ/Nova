#pragma once
#include <Nova/graphics/opengl/FramebufferAttachment.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

FramebufferAttachment::FramebufferAttachment(const FramebufferAttachmentSpec &spec) noexcept
    : spec_(spec)
{
    NV_PROFILE_FUNC;

    if (spec.UseRenderbuffer())
    {
        id_ = GL::CreateRenderbuffer();
        GL::NamedRenderbufferStorage(id_, spec.Format, spec.Width, spec.Height);
    }
    else
    {
        id_ = GL::CreateTexture(TextureTarget::Texture2D);
        GL::TextureStorage2D(id_, 1, spec.Format, spec.Width, spec.Height);
        GL::TextureParameter(id_, TextureParameterName::TextureWrapR, (GLint)TextureWrapMode::ClampToEdge);
        GL::TextureParameter(id_, TextureParameterName::TextureWrapS, (GLint)TextureWrapMode::ClampToEdge);
        GL::TextureParameter(id_, TextureParameterName::TextureWrapT, (GLint)TextureWrapMode::ClampToEdge);
        GL::TextureParameter(id_, spec.MinFilter);
        GL::TextureParameter(id_, spec.MagFilter);
    }
}

void FramebufferAttachment::Attach(GLuint framebufferID, Attachment attachment, GLint level) const noexcept
{
    if (spec_.UseRenderbuffer())
        GL::NamedFramebufferRenderbuffer(framebufferID, attachment, id_);
    else
        GL::NamedFramebufferTexture(framebufferID, attachment, id_, level);
}

FramebufferAttachment::~FramebufferAttachment() noexcept
{
    if (id_)
    {
        if (spec_.UseRenderbuffer())
            GL::DeleteRenderbuffer(id_);
        else
            GL::DeleteTexture(id_);
    }
}