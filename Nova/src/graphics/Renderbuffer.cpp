#include <Nova/graphics/opengl/Renderbuffer.hpp>

using namespace Nova;

Renderbuffer::Renderbuffer(GLsizei width, GLsizei height, InternalFormat format)
    : format_(format),
      width_(width),
      height_(height)
{
    glCreateRenderbuffers(1, &id_);
    glNamedRenderbufferStorage(id_, (GLenum)format, width, height);
}

Renderbuffer::~Renderbuffer() noexcept
{
    glDeleteRenderbuffers(1, &id_);
}

void Renderbuffer::Bind() const noexcept
{
    glBindRenderbuffer(GL_RENDERBUFFER, id_);
}

void Renderbuffer::Resize(GLsizei width, GLsizei height) noexcept
{
    glNamedRenderbufferStorage(id_, (GLenum)format_, width, height);
    width_ = width;
    height_ = height;
}

void Renderbuffer::Resize(GLsizei width, GLsizei height, InternalFormat format) noexcept
{
    glNamedRenderbufferStorage(id_, (GLenum)format, width, height);
    width_ = width;
    height_ = height;
    format_ = format;
}