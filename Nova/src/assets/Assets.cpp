#include <Nova/assets/Assets.hpp>
#include <Nova/core/Utility.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <fstream>
#include <future>

using namespace Nova;

enum class AssetLoadingState
{
    Loaded,
    Failed,
};

struct AssetLoadingTask
{
    std::future<void> LoadingFuture;
    std::shared_ptr<Asset> Asset;
    void *LoadingData;
    AssetLoadingState LoadingState;
};

std::unordered_map<UUIDv4::UUID, std::shared_ptr<Asset>> assetsMap_;
std::unordered_map<std::string, UUIDv4::UUID, StringHash, std::equal_to<>> assetsNameToUUIDMap_;
std::queue<std::shared_ptr<Asset>> assetsUnloadQueue_;
std::queue<std::shared_ptr<Asset>> assetsLoadFinalizationQueue_;
std::mutex assetsLoadFinalizationQueueMutex_;
std::vector<AssetLoadingTask> loadingTasks_;

std::shared_ptr<Asset> Assets::LoadAsset(std::shared_ptr<Asset> &&asset)
{
    NV_PROFILE_FUNC;

    auto loadingData = asset->CreateLoadingData();

    if (asset->RequiresPreLoad())
        asset->PreLoad(loadingData);

    asset->Load(loadingData);

    if (asset->RequiresPostLoad())
        asset->PostLoad(loadingData);

    asset->FreeLoadingData(loadingData);

    if (asset->GetName().has_value())
        assetsNameToUUIDMap_.emplace(asset->GetName().value(), asset->GetUUID());

    return assetsMap_.emplace(asset->GetUUID(), asset).first->second;
}

std::shared_ptr<Asset> Assets::LoadAssetAsync(std::shared_ptr<Asset> &&asset)
{
    NV_PROFILE_FUNC;

    auto loadingData = asset->CreateLoadingData();

    if (asset->RequiresPreLoad())
        asset->PreLoad(loadingData);

    if (asset->GetName().has_value())
        assetsNameToUUIDMap_.emplace(asset->GetName().value(), asset->GetUUID());

    auto insertedAsset = assetsMap_.emplace(asset->GetUUID(), asset).first->second;

    AssetLoadingTask task{};
    task.Asset = insertedAsset;
    task.LoadingData = loadingData;
    task.LoadingFuture = std::async(
        std::launch::async,
        [=]()
        {
            insertedAsset->Load(loadingData);
        });

    loadingTasks_.emplace_back(std::move(task));

    return insertedAsset;
}

std::shared_ptr<Asset> Assets::GetAsset(const UUIDv4::UUID &uuid)
{
    NV_PROFILE_FUNC;

    auto it = assetsMap_.find(uuid);
    return it == assetsMap_.end()
               ? nullptr
               : it->second;
}

std::shared_ptr<Asset> Assets::GetAsset(const std::string_view name)
{
    NV_PROFILE_FUNC;

    auto idIt = assetsNameToUUIDMap_.find(name);
    return idIt == assetsNameToUUIDMap_.end()
               ? nullptr
               : GetAsset(idIt->second);
}

void Assets::UnloadAsset(const UUIDv4::UUID &uuid)
{
    NV_PROFILE_FUNC;

    auto asset = GetAsset(uuid);
    UnloadAsset(asset);
}

void Assets::UnloadAsset(const std::string_view name)
{
    NV_PROFILE_FUNC;

    auto asset = GetAsset(name);
    UnloadAsset(asset);
}

void Assets::UnloadAsset(std::shared_ptr<Asset> &asset)
{
    NV_PROFILE_FUNC;

    asset->GetStatus().exchange(AssetStatus::UnloadRequested);

    assetsMap_.erase(asset->GetUUID());

    if (asset->GetName().has_value())
        assetsNameToUUIDMap_.erase(asset->GetName().value());

    assetsUnloadQueue_.emplace(asset);
}

void Assets::UnloadAllAssets()
{
}

const std::unordered_map<UUIDv4::UUID, std::shared_ptr<Asset>> &Assets::GetAssets()
{
    return assetsMap_;
}

void Assets::ProcessLoadingTasks_()
{
    NV_PROFILE_FUNC;

    auto tasksIt = loadingTasks_.begin();
    while (tasksIt != loadingTasks_.end())
    {
        auto &task = *tasksIt;

        const auto futureStatus = task.LoadingFuture.wait_for(std::chrono::seconds(0));
        const auto isFutureReady = futureStatus == std::future_status::ready;

        if (isFutureReady && task.Asset->RequiresPostLoad())
        {
            task.Asset->PostLoad(task.LoadingData);
            task.Asset->SetStatus(AssetStatus::Loaded);
            task.Asset->FreeLoadingData(task.LoadingData);

            tasksIt = loadingTasks_.erase(tasksIt);
        }
        else
        {
            tasksIt++;
        }
    }
}