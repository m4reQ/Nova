#pragma once
#include <Nova/assets/Asset.hpp>
#include <type_traits>
#include <filesystem>

namespace Nova
{
    template <typename TAsset>
        requires std::is_base_of_v<Asset, TAsset>
    class AssetLoader
    {
    public:
        constexpr AssetLoader(std::shared_ptr<TAsset> &asset) noexcept
            : asset_(asset) {}

        // happens on main thread, before loading, cannot be interrupted by Unload
        void PreLoad();

        // happens either on main or auxiliary thread, can be interrupted by Unload in case of async load
        // path is taken by value to avoid synchronization between threads in case of async load
        void LoadFromFile(std::filesystem::path filepath);

        // happens on main thread, after loading, can be interrupted by Unload in case of async load
        void PostLoad();

        // happens on main thread
        void Unload();

    private:
        std::shared_ptr<TAsset> asset_;
    };
}