#include <Nova/assets/Image.hpp>
#include <Nova/core/File.hpp>
#include <Nova/graphics/DDS.hpp>
#include <Nova/graphics/MagicNumber.hpp>
#include <Nova/debug/Profile.hpp>
#include <stb/stb_image.h>

using namespace Nova;

static void LoadImageFromFileSTBImage(File &file, ImageLoadingData &loadingData)
{
    NV_PROFILE_FUNC;

    int channels;
    loadingData.Data = stbi_load_from_file(
        file.GetHandle(),
        &loadingData.Width,
        &loadingData.Height,
        &channels,
        0);

    if (!loadingData.Data)
        throw std::runtime_error("Failed to load stb image.");

    loadingData.Format = channels == 4
                             ? InternalFormat::RGBA8
                             : InternalFormat::RGB8;
}

static void LoadImageFromFileDDS(File &file, ImageLoadingData &loadingData)
{
}

static void LoadImageFromFile(const std::filesystem::path &filepath, ImageLoadingData &loadingData)
{
    NV_PROFILE_FUNC;

    File file(filepath, "rbS");

    MagicNumber magicNumber;
    file.ReadChecked(&magicNumber);
    file.Seek(0, FileSeekOrigin::Begin);

    if (magicNumber.IsDDS())
        LoadImageFromFileDDS(file, loadingData);
    else
        LoadImageFromFileSTBImage(file, loadingData);
}

void Image::Load(void *loadingData)
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ImageLoadingData *>(loadingData);

    if (source_.Type == AssetSourceType::File)
        LoadImageFromFile(source_.Filepath, *loadingData_);
}

void Image::PostLoad(void *loadingData)
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ImageLoadingData *>(loadingData);

    Texture texture(
        TextureTarget::Texture2D,
        TextureSpec{
            .Size = {loadingData_->Width, loadingData_->Height, 0},
            .Format = loadingData_->Format,
            .Mipmaps = 4,
            .AllowBindless = true,
        });
    texture.Upload(
        TextureUploadInfo{
            .Size = {loadingData_->Width, loadingData_->Height, 1},
            .Offset = {0, 0, 0},
            .Mipmap = 0,
            .PixelFormat = loadingData_->Format == InternalFormat::RGB8
                               ? PixelFormat::RGB
                               : PixelFormat::RGBA,
            .PixelType = PixelType::UnsignedByte,
        },
        loadingData_->Data);

    stbi_image_free(loadingData_->Data);

    {
        std::lock_guard<std::mutex> lock(loadingMutex_);
        SetTexture(std::move(texture));
    }
}