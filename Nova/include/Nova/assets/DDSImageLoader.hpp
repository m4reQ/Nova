#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/assets/ImageLoadingData.hpp>

namespace Nova
{
    class DDSImageLoader
    {
    public:
        constexpr static bool RequiresPreLoad() noexcept { return true; }

        constexpr static bool RequiresPostLoad() noexcept { return true; }

        static DDSImageLoadingData *CreateLoadingData() noexcept;

        static void FreeLoadingData(void *loadingData) noexcept;

        static void PreLoad(std::shared_ptr<Asset> asset, void *loadingData) noexcept;

        static void Load(std::shared_ptr<Asset> asset, void *loadingData) noexcept;

        static void PostLoad(std::shared_ptr<Asset> asset, void *loadingData) noexcept;
    };
}