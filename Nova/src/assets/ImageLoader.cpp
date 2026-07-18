#include <Nova/assets/ImageLoader.hpp>
#include <Nova/assets/Image.hpp>
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/Utility.hpp>
#include <stb/stb_image.h>
#include <glm/glm.hpp>

using namespace Nova;

static void LoadImageFromFile(ImageLoadingData *loadingData, const std::filesystem::path &filepath)
{
    NV_PROFILE_FUNC;

    int width = 0;
    int height = 0;
    int channelsCount = 0;
    auto pixels = stbi_load(
        filepath.string().c_str(),
        &width,
        &height,
        &channelsCount,
        0);

    if (pixels == nullptr)
        throw std::runtime_error("Failed to load image from file.");

    loadingData->Data.emplace_back(pixels);
    loadingData->UploadInfos.emplace_back(
        std::make_pair(
            TextureUploadInfo{
                .Size = {width, height, 1},
                .Offset = {0, 0, 0},
                .Mipmap = 0,
                .PixelFormat = channelsCount == 4 ? PixelFormat::RGBA : PixelFormat::RGB,
                .PixelType = PixelType::UnsignedByte,
            },
            0));

    loadingData->TextureFormat = channelsCount == 4 ? InternalFormat::RGBA8 : InternalFormat::RGB8;
    loadingData->TextureTarget = TextureTarget::Texture2D;
    loadingData->Mipmaps = 1;
}

static void LoadImageFromFileList(ImageLoadingData *loadingData, const std::vector<std::filesystem::path> &filepaths)
{
    NV_PROFILE_FUNC;

    if (filepaths.size() != 6)
        throw std::runtime_error("Invalid amount of files for cube texture.");

    for (int i = 0; i < filepaths.size(); i++)
    {
        int channelsCount = 0;
        int width = 0;
        int height = 0;
        auto filepath = filepaths[i].string();
        auto pixels = stbi_load(
            filepath.c_str(),
            &width,
            &height,
            &channelsCount,
            0);

        if (pixels == nullptr)
        {
            NV_LOG_ERROR("Failed to load image from file \"{}\": {}", filepath, stbi_failure_reason());
            throw std::runtime_error("Failed to load image from file.");
        }

        loadingData->Data.emplace_back(pixels);
        loadingData->UploadInfos.emplace_back(
            std::make_pair(
                TextureUploadInfo{
                    .Size = {width, height, 1},
                    .Offset = {0, 0, i},
                    .Mipmap = 0,
                    .PixelFormat = channelsCount == 4 ? PixelFormat::RGBA : PixelFormat::RGB,
                    .PixelType = PixelType::UnsignedByte,
                },
                i));
    }

    // TODO Better way of determining the format?
    loadingData->TextureFormat = loadingData->UploadInfos[0].first.PixelFormat == PixelFormat::RGBA
                                     ? InternalFormat::RGBA8
                                     : InternalFormat::RGB8;
    loadingData->TextureTarget = TextureTarget::TextureCubeMap;
    loadingData->Mipmaps = 1;
    loadingData->GenerateMipmap = false;
    // TODO Check if all faces have the same format etc
}

static void LoadImageFromMemory(ImageLoadingData *loadingData, std::span<const uint8_t> data)
{
    // TODO Implement
}

void ImageLoader::Load(std::shared_ptr<Asset> asset, void *loadingData) noexcept
{
    NV_PROFILE_FUNC;

    auto _loadingData = static_cast<ImageLoadingData *>(loadingData);

    std::visit(
        Overloaded{
            [=](const std::filesystem::path &filepath)
            {
                LoadImageFromFile(_loadingData, filepath);
            },
            [=](const std::vector<std::filesystem::path> &filepaths)
            {
                LoadImageFromFileList(_loadingData, filepaths);
            },
            [=](std::span<const uint8_t> data)
            {
                LoadImageFromMemory(_loadingData, data);
            }},
        asset->GetSource());
}

void ImageLoader::PostLoad(std::shared_ptr<Asset> asset, void *loadingData)
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ImageLoadingData *>(loadingData);
    auto asset_ = std::static_pointer_cast<Image>(asset);

    if (loadingData_->UploadInfos.size() < 1)
        throw std::runtime_error("No texture data loaded.");

    asset_->texture_ = std::make_shared<Texture>(
        loadingData_->TextureTarget,
        TextureSpec{
            .Size = loadingData_->UploadInfos[0].first.Size,
            .Wrapping = asset_->settings_.Wrapping,
            .Format = loadingData_->TextureFormat,
            .Mipmaps = loadingData_->Mipmaps,
            .MinFilter = asset_->settings_.MinFilter,
            .MagFilter = asset_->settings_.MagFilter,
        });

    for (const auto &[uploadInfo, dataIndex] : loadingData_->UploadInfos)
    {
        asset_->texture_->Upload(
            uploadInfo,
            loadingData_->Data[dataIndex],
            loadingData_->GenerateMipmap);
    }
}

ImageLoadingData *ImageLoader::CreateLoadingData() noexcept
{
    return new ImageLoadingData();
}

void ImageLoader::FreeLoadingData(void *loadingData) noexcept
{
    auto loadingData_ = static_cast<ImageLoadingData *>(loadingData);
    for (auto pixels : loadingData_->Data)
        stbi_image_free(pixels);

    delete loadingData_;
}

void ImageLoader::PreLoad(std::shared_ptr<Asset> asset, void *loadingData) noexcept
{
    auto asset_ = std::static_pointer_cast<Image>(asset);
    asset_->texture_ = Renderer::GetWhiteTexture_();
}