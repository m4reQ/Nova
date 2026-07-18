#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/debug/Profile.hpp>
#include <memory>
#include <span>
#include <type_traits>
#include <future>

namespace Nova
{
    template <typename T>
    concept IsLoaderType = requires(std::shared_ptr<Asset> asset, void *loadingData) {
        { T::RequiresPreLoad() } -> std::same_as<bool>;
        { T::RequiresPostLoad() } -> std::same_as<bool>;
        { T::CreateLoadingData() } -> std::convertible_to<void *>;
        { T::FreeLoadingData(loadingData) } -> std::same_as<void>;
        { T::PreLoad(asset, loadingData) } -> std::same_as<void>;
        { T::Load(asset, loadingData) } -> std::same_as<void>;
        { T::PostLoad(asset, loadingData) } -> std::same_as<void>;
    };

    template <typename T>
    concept IsAssetType = std::is_base_of_v<Asset, T>;

    struct LoadingTask
    {
        std::shared_ptr<Asset> TheAsset;
        std::future<void> Future;
        std::function<void(void *)> LoadingDataFreeFunc;
        std::optional<std::function<void(std::shared_ptr<Asset>, void *loadingData)>> PostLoadFunc;
        void *LoadingData;
    };

    namespace Assets
    {
        std::shared_ptr<Asset> InsertAsset_(std::shared_ptr<Asset> &&asset);

        void InsertLoadingTask_(LoadingTask &&task);

        std::shared_ptr<Asset> GetAsset(const UUIDv4::UUID &uuid);

        std::shared_ptr<Asset> GetAsset(const std::string_view name);

        template <IsAssetType TAsset, IsLoaderType TLoader, typename... TAssetArgs>
        std::shared_ptr<TAsset> Load(const AssetSource &source, const std::string_view name, TAssetArgs &&...assetArgs)
        {
            NV_PROFILE_FUNC;

            auto asset = std::make_shared<TAsset>(source, name, std::forward<TAssetArgs>(assetArgs)...);
            auto loadingData = TLoader::CreateLoadingData();

            if (TLoader::RequiresPreLoad())
                TLoader::PreLoad(asset, loadingData);

            TLoader::Load(asset, loadingData);

            if (TLoader::RequiresPostLoad())
                TLoader::PostLoad(asset, loadingData);

            TLoader::FreeLoadingData(loadingData);

            return std::static_pointer_cast<TAsset>(InsertAsset_(asset));
        }

        template <IsAssetType TAsset, IsLoaderType TLoader, typename... TAssetArgs>
        std::shared_ptr<TAsset> LoadAsync(const AssetSource &source, const std::string_view name, TAssetArgs &&...assetArgs)
        {
            NV_PROFILE_FUNC;

            auto asset = std::make_shared<TAsset>(source, name, std::forward<TAssetArgs>(assetArgs)...);
            auto loadingData = TLoader::CreateLoadingData();

            if (TLoader::RequiresPreLoad())
                TLoader::PreLoad(asset, loadingData);

            LoadingTask task;
            task.TheAsset = asset;
            task.LoadingData = loadingData;
            task.LoadingDataFreeFunc = TLoader::FreeLoadingData;
            task.PostLoadFunc = TLoader::RequiresPostLoad() ? std::make_optional(TLoader::PostLoad) : std::nullopt;
            task.Future = std::async(
                std::launch::async,
                [](auto asset, auto loadingData)
                {
                    TLoader::Load(asset, loadingData);
                },
                asset,
                task.LoadingData);

            InsertLoadingTask_(std::move(task));

            return std::static_pointer_cast<TAsset>(InsertAsset_(asset));
        }

        // TODO Batch assets loading
        void UnloadAsset(const UUIDv4::UUID &uuid);
        void UnloadAsset(const std::string_view name);
        void UnloadAsset(std::shared_ptr<Asset> &asset);
        void UnloadAllAssets();
        void ReloadAsset(const UUIDv4::UUID &uuid);
        void ReloadAsset(const std::string_view name);
        void ReloadAsset(std::shared_ptr<Asset> &asset);
        const std::unordered_map<UUIDv4::UUID, std::shared_ptr<Asset>> &GetAssets();

        /// @brief Internal API. Don't use directly!
        void ProcessLoadingTasks_();
    }
}