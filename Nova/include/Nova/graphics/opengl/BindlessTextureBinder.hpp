#pragma once
#include <Nova/graphics/opengl/ITextureBinder.hpp>
#include <list>
#include <unordered_map>

namespace Nova
{
    // TODO This could itself have a destructor calling glMakeTextureHandleNonResidentARB
    struct BindlessTextureBinding
    {
        GLuint64 Handle;
        GLuint TextureID;
        GLuint Age;
        bool IsPersistent;
    };

    class BindlessTextureBinder final : public ITextureBinder
    {
    public:
        BindlessTextureBinder() = default;

        BindlessTextureBinder(const BindlessTextureBinder &) = delete;

        BindlessTextureBinder(BindlessTextureBinder &&other) noexcept;

        BindlessTextureBinder(GLuint maxBindingAge) noexcept;

        ~BindlessTextureBinder() noexcept;

        bool IsBound(const ITexture &texture) const noexcept override;

        GLuint64 Bind(const ITexture &texture, bool isPersistent = false) override;

        GLuint64 GetBinding(const ITexture &texture) const noexcept override;

        void Unbind(const ITexture &texture, bool deleteHandle = false) noexcept override;

        void Unbind(GLuint64 handle, bool deleteHandle = false) noexcept override;

        void UnbindAll(bool unbindPersistent = false) noexcept override;

        void Update() noexcept override;

        BindlessTextureBinder &operator=(const BindlessTextureBinder &) = delete;

        BindlessTextureBinder &operator=(BindlessTextureBinder &&other) noexcept;

    private:
        std::list<BindlessTextureBinding> bindings_;
        std::unordered_map<GLuint, GLuint64> bindlessHandles_;
        GLuint maxBindingAge_;

        std::list<BindlessTextureBinding>::iterator GetBindingInternal(const ITexture &texture) noexcept;

        std::list<BindlessTextureBinding>::const_iterator GetBindingInternal(const ITexture &texture) const noexcept;
    };
}