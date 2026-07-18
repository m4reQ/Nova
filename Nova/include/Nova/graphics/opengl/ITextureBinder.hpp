#pragma once
#include <Nova/graphics/opengl/ITexture.hpp>

namespace Nova
{
    class ITextureBinder
    {
    public:
        virtual ~ITextureBinder() noexcept = default;

        virtual bool IsBound(const ITexture &texture) const noexcept = 0;

        virtual GLuint64 Bind(const ITexture &texture, bool isPersistent = false) = 0;

        virtual GLuint64 GetBinding(const ITexture &texture) const noexcept = 0;

        virtual void Unbind(const ITexture &texture, bool deleteHandle = false) noexcept = 0;

        virtual void Unbind(GLuint64 handle, bool deleteHandle = false) noexcept = 0;

        virtual void UnbindAll(bool unbindPersistent = false) noexcept = 0;

        virtual void Update() noexcept = 0;
    };
}