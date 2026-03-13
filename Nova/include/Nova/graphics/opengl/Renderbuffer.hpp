#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/graphics/opengl/FramebufferAttachment.hpp>
#include <glm/vec2.hpp>
#include <utility>

namespace Nova
{
    class Renderbuffer : public IFramebufferAttachment
    {
    public:
        Renderbuffer() = default;

        Renderbuffer(GLsizei width, GLsizei height, InternalFormat format);

        constexpr Renderbuffer(Renderbuffer &&other) noexcept
            : format_(other.format_),
              width_(other.width_),
              height_(other.height_),
              id_(std::exchange(other.id_, 0)) {}

        ~Renderbuffer() noexcept;

        void Bind() const noexcept;

        void Resize(GLsizei width, GLsizei height) noexcept;

        void Resize(GLsizei width, GLsizei height, InternalFormat format) noexcept;

        constexpr GLsizei GetWidth() const noexcept override { return width_; }

        constexpr GLsizei GetHeight() const noexcept override { return width_; }

        constexpr glm::ivec2 GetSize() const noexcept { return {width_, height_}; }

        constexpr InternalFormat GetFormat() const noexcept override { return format_; }

        constexpr GLuint GetID() const noexcept override { return id_; }

        constexpr Renderbuffer &operator=(Renderbuffer &&other) noexcept
        {
            format_ = other.format_;
            width_ = other.width_;
            height_ = other.height_;
            id_ = std::exchange(other.id_, 0);

            return *this;
        }

    private:
        InternalFormat format_;
        GLsizei width_, height_;
        GLuint id_;
    };
}