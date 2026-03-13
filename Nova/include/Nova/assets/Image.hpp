#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/graphics/opengl/Texture.hpp>

namespace Nova
{
    struct ImageLoadingData
    {
        GLsizei Width, Height;
        InternalFormat Format;
        void *Data;
    };

    class Image : public Asset
    {
    public:
        constexpr static AssetType GetStaticAssetType() { return AssetType::Image; }

        using Asset::Asset;

        void Load(void *loadingData) override;

        void PostLoad(void *loadingData) override;

        constexpr bool RequiresPostLoad() const noexcept override { return true; }

        constexpr void *CreateLoadingData() const override { return new ImageLoadingData(); }

        constexpr void FreeLoadingData(void *loadingData) const override { delete static_cast<ImageLoadingData *>(loadingData); }

        constexpr Texture &&RemoveTexture() noexcept { return std::move(texture_); }

        constexpr void SetTexture(Texture &&texture) noexcept { texture_ = std::move(texture); }

        constexpr const Texture &GetTexture() const noexcept { return texture_; }

        constexpr Texture &GetTexture() noexcept { return texture_; }

    private:
        Texture texture_;
    };
}