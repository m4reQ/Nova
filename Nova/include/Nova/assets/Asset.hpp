#pragma once
#include <Nova/core/Utility.hpp>
#include <atomic>
#include <string_view>
#include <optional>
#include <filesystem>
#include <mutex>
#include <uuid_v4.h>

namespace Nova
{
    enum class AssetSourceType
    {
        File,
        Memory,
    };

    struct AssetSource
    {
        AssetSourceType Type;
        std::filesystem::path Filepath;

        static AssetSource Memory() noexcept { return AssetSource{.Type = AssetSourceType::Memory}; }
    };

    enum class AssetStatus
    {
        Loaded,
        Loading,
        LoadingFailed,
        LoadingFailedFinalization,
        UnloadReady,
        UnloadRequested,
    };

    enum class AssetType
    {
        Image,
        Model,
        Font,
    };

    enum class AssetFlags
    {
        None = 0,
        Internal = 1,
    };

    class Asset
    {
    public:
        Asset() = default;

        Asset(
            const AssetSource &source,
            AssetType type,
            std::optional<std::string_view> name = std::nullopt,
            AssetFlags flags = AssetFlags::None);

        virtual ~Asset() noexcept {};

        virtual void PreLoad(void *loadingData) {}

        virtual void Load(void *loadingData) {}

        virtual void PostLoad(void *loadingData) {}

        virtual void Unload() {}

        virtual void *CreateLoadingData() const = 0;

        virtual void FreeLoadingData(void *loadingData) const = 0;

        virtual bool RequiresPostLoad() const noexcept { return false; }

        virtual bool RequiresPreLoad() const noexcept { return false; }

        constexpr AssetType GetType() const noexcept { return type_; }

        constexpr const AssetSource &GetSource() const noexcept { return source_; }

        constexpr void SetName(const std::string_view name) noexcept { name_ = name; }

        constexpr std::optional<const std::string_view> GetName() const noexcept { return name_; }

        constexpr const UUIDv4::UUID &GetUUID() const noexcept { return uuid_; }

        constexpr bool IsInternal() const noexcept { return IsFlagSet(flags_, AssetFlags::Internal); }

        bool CheckStatus(AssetStatus status) const noexcept { return status_.load() == status; }

        void SetStatus(AssetStatus status) noexcept { status_.store(status); }

        constexpr std::atomic<AssetStatus> &GetStatus() noexcept { return status_; }

        constexpr const std::atomic<AssetStatus> &GetStatus() const noexcept { return status_; }

    protected:
        std::mutex loadingMutex_;
        UUIDv4::UUID uuid_;
        AssetType type_;
        AssetFlags flags_;
        std::optional<std::string> name_;
        AssetSource source_ = AssetSource::Memory();
        std::atomic<AssetStatus> status_ = AssetStatus::Loading;
    };

    NV_DEFINE_BITWISE_OPERATORS(AssetFlags);
}