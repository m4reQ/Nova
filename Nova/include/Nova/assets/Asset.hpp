#pragma once
#include <Nova/core/Flag.hpp>
#include <uuid_v4.h>
#include <atomic>
#include <string_view>
#include <optional>
#include <filesystem>
#include <mutex>
#include <variant>
#include <span>

namespace Nova
{
    using AssetSource = std::variant<
        std::filesystem::path,
        std::vector<std::filesystem::path>,
        std::span<const uint8_t>>;

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

        Asset(const AssetSource &source, std::optional<std::string_view> name = std::nullopt) noexcept;

        virtual ~Asset() noexcept = default;

        virtual void Unload() {}

        virtual AssetType GetType() const noexcept = 0;

        constexpr const AssetSource &GetSource() const noexcept { return source_; }

        constexpr void SetName(const std::string_view name) noexcept { name_ = name; }

        constexpr std::optional<const std::string_view> GetName() const noexcept { return name_; }

        constexpr const UUIDv4::UUID &GetUUID() const noexcept { return uuid_; }

        constexpr bool IsInternal() const noexcept { return Flag::IsSet(flags_, AssetFlags::Internal); }

        bool CheckStatus(AssetStatus status) const noexcept { return status_.load() == status; }

        void SetStatus(AssetStatus status) noexcept { status_.store(status); }

        constexpr std::atomic<AssetStatus> &GetStatus() noexcept { return status_; }

        constexpr const std::atomic<AssetStatus> &GetStatus() const noexcept { return status_; }

    protected:
        std::mutex loadingMutex_;
        UUIDv4::UUID uuid_;
        AssetType type_;
        std::optional<std::string> name_;
        AssetSource source_;
        std::atomic<AssetStatus> status_ = AssetStatus::Loading;
        AssetFlags flags_;
    };

    NV_DEFINE_BITWISE_OPERATORS(AssetFlags);
}