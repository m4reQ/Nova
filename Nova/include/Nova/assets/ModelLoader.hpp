#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/assets/ModelVertex.hpp>

namespace Nova
{
    struct ModelLoadingData
    {
        std::vector<ModelVertex> ModelVertices;
        std::vector<uint32_t> ModelIndices;
    };

    class OBJModelLoader
    {
    public:
        constexpr static bool RequiresPreLoad() noexcept { return false; }

        constexpr static bool RequiresPostLoad() noexcept { return true; }

        static ModelLoadingData *CreateLoadingData() noexcept { return new ModelLoadingData(); }

        static void FreeLoadingData(void *loadingData) noexcept { delete (ModelLoadingData *)loadingData; }

        static void PreLoad(std::shared_ptr<Asset>, void *) noexcept {}

        static void Load(std::shared_ptr<Asset> asset, void *loadingData) noexcept;

        static void PostLoad(std::shared_ptr<Asset> asset, void *loadingData) noexcept;
    };
}