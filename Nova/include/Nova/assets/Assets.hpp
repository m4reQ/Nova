#pragma once
#include <Nova/assets/Asset.hpp>
#include <Nova/assets/AssetLoader.hpp>
#include <memory>
#include <span>
#include <type_traits>
#include <future>

namespace Nova
{
    template <typename T>
    concept IsValidAssetType = requires {
        std::is_base_of_v<Asset, T>;
        { T::GetStaticAssetType() } -> std::same_as<AssetType>;
    };

    namespace Assets
    {
        std::shared_ptr<Asset> InsertAsset(std::shared_ptr<Asset> &&asset);

        std::shared_ptr<Asset> GetAsset(const UUIDv4::UUID &uuid);

        std::shared_ptr<Asset> GetAsset(const std::string_view name);

        std::shared_ptr<Asset> LoadAsset(std::shared_ptr<Asset> &&asset);

        std::shared_ptr<Asset> LoadAssetAsync(std::shared_ptr<Asset> &&asset);

        template <IsValidAssetType T>
        std::shared_ptr<T> LoadAssetFromFile(
            const std::filesystem::path &filepath,
            std::optional<std::string_view> name = std::nullopt,
            AssetFlags flags = AssetFlags::None)
        {
            return std::static_pointer_cast<T>(
                LoadAsset(
                    std::make_shared<T>(
                        AssetSource{
                            .Type = AssetSourceType::File,
                            .Filepath = filepath,
                        },
                        T::GetStaticAssetType(),
                        name,
                        flags)));
        }

        template <IsValidAssetType T>
        std::shared_ptr<T> LoadAssetFromFileAsync(
            const std::filesystem::path &filepath,
            std::optional<std::string_view> name = std::nullopt,
            AssetFlags flags = AssetFlags::None)
        {
            return std::static_pointer_cast<T>(
                LoadAssetAsync(
                    std::make_shared<T>(
                        AssetSource{
                            .Type = AssetSourceType::File,
                            .Filepath = filepath,
                        },
                        T::GetStaticAssetType(),
                        name,
                        flags)));
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