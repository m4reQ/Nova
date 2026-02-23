#include <Nova/graphics/opengl/Texture.hpp>

using namespace Nova;

Texture Texture::s_WhiteTexture;

void Texture::BindToUnit(GLuint unit) const noexcept
{
    glBindTextureUnit(unit, (GLuint)m_ID);
}

void Texture::Delete() noexcept
{
    glDeleteTextures(1, &m_ID);
    m_ID = 0;
}
