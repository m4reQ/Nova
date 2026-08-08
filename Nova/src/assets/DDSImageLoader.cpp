#include <Nova/assets/DDSImageLoader.hpp>
#include <Nova/assets/Image.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/graphics/DDS.hpp>
#include <Nova/core/File.hpp>
#include <Nova/core/Utility.hpp>
#include <Nova/debug/Profile.hpp>
#include <algorithm>

using namespace Nova;

static void CheckMagicNumber(File &file)
{
    uint32_t magicNumber = 0;
    file.ReadChecked(&magicNumber);

    if (magicNumber != DDSMagicNumber)
        throw std::runtime_error("Invalid magic number for a DDS file.");
}

static std::pair<InternalFormat, uint32_t> DetermineInternalFormatAndBlockSize(const DDSPixelFormat &pixelFormat)
{
    switch (pixelFormat.FourCC)
    {
    case DDSFourCC::DXT1:
    case DDSFourCC::DXT2:
        return pixelFormat.HasAlpha()
                   ? std::make_pair(InternalFormat::CompressedRGBAS3TCDXT1, 8)
                   : std::make_pair(InternalFormat::CompressedRGBS3TCDXT1, 8);
    case DDSFourCC::DXT3:
    case DDSFourCC::DXT4:
        return std::make_pair(InternalFormat::CompressedRGBAS3TCDXT3, 16);
    case DDSFourCC::DXT5:
        return std::make_pair(InternalFormat::CompressedRGBAS3TCDXT5, 16);
    }

    // TODO Handle DX10 with their formats

    throw std::runtime_error("Cannot determine internal format from provided fourCC.");
}

static void LoadFromCompressedFile(File &file, const DDSHeader &header, DDSImageLoadingData &loadingData)
{
    NV_PROFILE_FUNC;

    if (header.PixelFormat.IsDX10())
        throw std::runtime_error("Loader doesn't support DXT10 format.");

    const auto [internalFormat, blockSize] = DetermineInternalFormatAndBlockSize(header.PixelFormat);

    loadingData.GenerateMipmap = false;
    loadingData.Mipmaps = header.GetMipmapCount();
    loadingData.TextureFormat = internalFormat;
    loadingData.TextureTarget = header.HasDepth() ? TextureTarget::Texture3D : TextureTarget::Texture2D;
    loadingData.Data = file.ReadToEndBinary();

    size_t dataOffset = 0;
    for (GLint mipmap = 0; mipmap < static_cast<GLint>(header.GetMipmapCount()); mipmap++)
    {
        const auto wBlocks = (std::max)(1u, (header.Width + 3) / 4);
        const auto hBlocks = (std::max)(1u, (header.Height + 3) / 4);
        const auto depth = (std::max)(1u, header.Depth >> mipmap);
        const auto byteSize = wBlocks * hBlocks * depth * blockSize;

        loadingData.UploadInfos.emplace_back(
            TextureUploadInfo{
                .Size = glm::ivec3(header.Width, header.Height, header.GetDepth()),
                .Offset = glm::ivec3(0, 0, 0),
                .DataOffset = dataOffset,
                .Mipmap = mipmap,
                .PixelFormat = static_cast<PixelFormat>(internalFormat),
                .PixelType = PixelType::UnsignedByte,
            });

        // FIX overflow potential
        dataOffset += byteSize;
    }
}

static void LoadFromUncompressedFile(File &, const DDSHeader &, DDSImageLoadingData &)
{
    // const auto internalFormat = DetermineInternalFormat(header.PixelFormat);
}

static void LoadFromFilepath(const std::filesystem::path &filepath, DDSImageLoadingData &loadingData)
{
    NV_PROFILE_FUNC;

    File file(filepath, "rb");

    CheckMagicNumber(file);

    DDSHeader ddsHeader{};
    file.ReadChecked(&ddsHeader);

    // sanity checks
    if (ddsHeader.Size != sizeof(ddsHeader))
        throw std::runtime_error("DDS header size mismatch.");

    if (!ddsHeader.IsValid())
        throw std::runtime_error("DDS header missing width, height or format information.");

    if (ddsHeader.PixelFormat.IsCompressed())
        LoadFromCompressedFile(file, ddsHeader, loadingData);
    else
        LoadFromUncompressedFile(file, ddsHeader, loadingData);
}

DDSImageLoadingData *DDSImageLoader::CreateLoadingData() noexcept
{
    return new DDSImageLoadingData();
}

void DDSImageLoader::FreeLoadingData(void *loadingData) noexcept
{
    delete static_cast<DDSImageLoadingData *>(loadingData);
}

void DDSImageLoader::PreLoad(std::shared_ptr<Asset> asset, void *) noexcept
{
    NV_PROFILE_FUNC;

    auto image = std::static_pointer_cast<Image>(asset);
    // image->texture_ = Renderer::GetWhiteTexture_();
}

void DDSImageLoader::Load(std::shared_ptr<Asset> asset, void *loadingData) noexcept
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<DDSImageLoadingData *>(loadingData);
    std::visit(
        Overloaded{
            [=](const std::filesystem::path &filepath)
            {
                LoadFromFilepath(filepath, *loadingData_);
            },
            [](const std::vector<std::filesystem::path> &)
            {
                // TODO Implement DDS loading from file list
                throw std::runtime_error("Not implemented.");
            },
            [](const std::span<const uint8_t> &)
            {
                // TODO Implement DDS loading from memory
                throw std::runtime_error("Not implemented.");
            },
        },
        asset->GetSource());
}

void DDSImageLoader::PostLoad(std::shared_ptr<Asset>, void *) noexcept
{
    NV_PROFILE_FUNC;
}
