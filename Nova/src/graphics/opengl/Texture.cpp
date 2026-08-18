#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/debug/Profile.hpp>
#include <stdexcept>

static void CreateTextureStorage(Nova::TextureTarget target, GLuint texture, const Nova::TextureSpec &spec) noexcept
{
    NV_PROFILE_FUNC;

    switch (target)
    {
    case Nova::TextureTarget::Texture2D:
    case Nova::TextureTarget::Texture1DArray:
    case Nova::TextureTarget::TextureCubeMap:
        Nova::GL::TextureStorage2D(
            texture,
            spec.Mipmaps,
            spec.Format,
            spec.Size.x,
            spec.Size.y);
        break;
    }
}

Nova::Texture::Texture(TextureTarget target, const TextureSpec &spec)
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
}

Nova::Texture::~Texture() noexcept
{
    if (id_)
        GL::DeleteTexture(id_);
}

void Nova::Texture::Bind(GLuint unit) const noexcept
{
    NV_PROFILE_FUNC;
    glBindTextureUnit(unit, id_);
}

void Nova::Texture::Upload(const TextureUploadInfo &info, const void *data, bool generateMipmap) const noexcept
{
    NV_PROFILE_FUNC;

    // TODO Better check for 1D/2D/3D targets
    if (target_ == TextureTarget::TextureCubeMap)
        glTextureSubImage3D(
            id_,
            info.Mipmap,
            info.Offset.x,
            info.Offset.y,
            info.Offset.z,
            info.Size.x,
            info.Size.y,
            info.Size.z,
            (GLenum)info.PixelFormat,
            (GLenum)info.PixelType,
            (uint8_t *)data + info.DataOffset);
    else
        GL::TextureSubImage2D(
            id_,
            info.Mipmap,
            info.Offset.x,
            info.Offset.y,
            info.Size.x,
            info.Size.y,
            info.PixelFormat,
            info.PixelType,
            (uint8_t *)data + info.DataOffset);

    if (generateMipmap)
    {
        NV_PROFILE_SCOPE("::GenerateTextureMipmap");
        GL::GenerateTextureMipmap(id_);
    }
}

void Nova::Texture::UploadFromPBO() const noexcept
{
}

void Nova::Texture::SetDebugName(const std::string_view name)
{
    GL::ObjectLabel(ObjectIdentifier::Texture, id_, name);
}