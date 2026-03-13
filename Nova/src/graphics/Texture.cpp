#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/debug/Profile.hpp>
#include <stdexcept>

using namespace Nova;

static void CreateTextureStorage(TextureTarget target, GLuint texture, const TextureSpec &spec) noexcept
{
    NV_PROFILE_FUNC;

    switch (target)
    {
    case TextureTarget::Texture2D:
    case TextureTarget::Texture1DArray:
        GL::TextureStorage2D(
            texture,
            spec.Mipmaps,
            spec.Format,
            spec.Size.x,
            spec.Size.y);
        break;
    }
}

Texture::Texture(TextureTarget target, const TextureSpec &spec)
    : target_(target), spec_(spec)
{
    NV_PROFILE_FUNC;

    id_ = GL::CreateTexture(target);
    GL::TextureParameter(id_, spec.MinFilter);
    GL::TextureParameter(id_, spec.MagFilter);
    GL::TextureParameter(id_, TextureParameterName::TextureWrapR, (GLint)spec.Wrapping.R);
    GL::TextureParameter(id_, TextureParameterName::TextureWrapS, (GLint)spec.Wrapping.S);
    GL::TextureParameter(id_, TextureParameterName::TextureWrapT, (GLint)spec.Wrapping.T);

    constexpr GLfloat borderColor[4]{1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, borderColor);

    CreateTextureStorage(target, id_, spec);

    GLint isImmutable = GL_FALSE;
    glGetTextureParameteriv(id_, GL_TEXTURE_IMMUTABLE_FORMAT, &isImmutable);
    if (!isImmutable)
        throw std::runtime_error("Failed to create texture.");

    if (spec.AllowBindless)
    {
        bindlessHandle_ = glGetTextureHandleARB(id_);
        if (!bindlessHandle_)
            throw std::runtime_error("Failed to create bindless texture.");
    }
}

Texture::~Texture() noexcept
{
    glDeleteTextures(1, &id_);
}

void Texture::Bind(GLuint unit) const noexcept
{
    NV_PROFILE_FUNC;
    glBindTextureUnit(unit, id_);
}

void Texture::MakeResident() const noexcept
{
    NV_PROFILE_FUNC;
    glMakeTextureHandleResidentARB(bindlessHandle_);
}

void Texture::MakeNonResident() const noexcept
{
    NV_PROFILE_FUNC;
    glMakeTextureHandleNonResidentARB(bindlessHandle_);
}

void Texture::Upload(const TextureUploadInfo &info, const void *data) const noexcept
{
    NV_PROFILE_FUNC;

    GL::TextureSubImage2D(
        id_,
        info.Mipmap,
        info.Offset.x,
        info.Offset.y,
        info.Size.x,
        info.Size.y,
        info.PixelFormat,
        info.PixelType,
        data);

    {
        NV_PROFILE_SCOPE("::GenerateTextureMipmap");
        GL::GenerateTextureMipmap(id_);
    }
}

void Texture::UploadFromPBO() const noexcept
{
}