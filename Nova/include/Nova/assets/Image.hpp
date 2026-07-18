#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/graphics/opengl/Texture.hpp>

namespace Nova
{
    struct ImageSettings
    {
        TextureWrapping Wrapping = {
            .R = TextureWrapMode::ClampToEdge,
            .S = TextureWrapMode::ClampToEdge,
            .T = TextureWrapMode::ClampToEdge,
        };
        TextureMinFilter MinFilter = TextureMinFilter::LinearMipmapLinear;
        TextureMagFilter MagFilter = TextureMagFilter::Linear;
    };

    class Image : public Asset
    {
    public:
        Image() = default;

        Image(const AssetSource &source,
              std::optional<std::string_view> name = std::nullopt,
              std::optional<ImageSettings> settings = std::nullopt) noexcept;

        constexpr static AssetType GetStaticAssetType() { return AssetType::Image; }

        const Texture &GetTexture() const noexcept { return *texture_; }

        constexpr AssetType GetType() const noexcept override { return AssetType::Image; }

        Texture &GetTexture() noexcept { return *texture_; }

        const std::shared_ptr<Texture> GetTextureRef() const noexcept { return texture_; }

        std::shared_ptr<Texture> GetTextureRef() noexcept { return texture_; }

    private:
        std::shared_ptr<Texture> texture_;
        ImageSettings settings_;

        friend class ImageLoader;
        friend class DDSImageLoader;
    };
}