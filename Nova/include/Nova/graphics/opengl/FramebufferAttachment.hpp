#pragma once
#include <Nova/graphics/opengl/ITexture.hpp>

namespace Nova
{
    enum class AttachmentFlags
    {
        Default = 0,
        Resizable = 1,
        DrawDest = 2,
        UseRenderbuffer = 4,
    };

    struct FramebufferAttachmentSpec
    {
        GLsizei Width;
        GLsizei Height;
        InternalFormat Format;
        AttachmentFlags Flags = AttachmentFlags::Default;
        TextureMinFilter MinFilter = TextureMinFilter::Nearest;
        TextureMagFilter MagFilter = TextureMagFilter::Nearest;

        constexpr bool IsResizable() const noexcept { return Flag::IsSet(Flags, AttachmentFlags::Resizable); }

        constexpr bool IsDrawDest() const noexcept { return Flag::IsSet(Flags, AttachmentFlags::DrawDest); }

        constexpr bool UseRenderbuffer() const noexcept { return Flag::IsSet(Flags, AttachmentFlags::UseRenderbuffer); }

        constexpr glm::ivec2 GetSize() const noexcept { return {Width, Height}; }
    };

    class FramebufferAttachment : public ITexture
    {
    public:
        FramebufferAttachment() = default;

        FramebufferAttachment(const FramebufferAttachment &) = delete;

        FramebufferAttachment(const FramebufferAttachmentSpec &spec) noexcept;

        constexpr FramebufferAttachment(GLuint id, const FramebufferAttachmentSpec &spec) noexcept
            : id_(id),
              spec_(spec) {}

        constexpr FramebufferAttachment(FramebufferAttachment &&other) noexcept
            : id_(std::exchange(other.id_, 0)),
              spec_(std::move(other.spec_)) {}

        ~FramebufferAttachment() noexcept override;

        void Attach(GLuint framebufferID, Attachment attachment, GLint level = 0) const noexcept;

        constexpr void SetID(GLuint id) noexcept { id_ = id; }

        constexpr GLuint GetID() const noexcept { return id_; }

        constexpr GLsizei GetWidth() const noexcept { return spec_.Width; }

        constexpr GLsizei GetHeight() const noexcept { return spec_.Height; }

        constexpr GLsizei GetDepth() const noexcept { return 1; }

        constexpr glm::ivec3 GetSize() const noexcept { return glm::ivec3(spec_.Width, spec_.Height, 1); }

        constexpr GLsizei GetMipmaps() const noexcept { return 1; }

        constexpr InternalFormat GetFormat() const noexcept { return spec_.Format; }

        constexpr TextureTarget GetTarget() const noexcept { return TextureTarget::Texture2D; }

        constexpr const FramebufferAttachmentSpec &GetSpec() const noexcept { return spec_; }

        constexpr FramebufferAttachmentSpec &GetSpec() noexcept { return spec_; }

        FramebufferAttachment &operator=(const FramebufferAttachment &) = delete;

        constexpr FramebufferAttachment &operator=(FramebufferAttachment &&other) noexcept
        {
            id_ = std::exchange(other.id_, 0);
            spec_ = std::move(other.spec_);

            return *this;
        }

    private:
        GLuint id_ = 0;
        FramebufferAttachmentSpec spec_;
    };

    NV_DEFINE_BITWISE_OPERATORS(AttachmentFlags);
}