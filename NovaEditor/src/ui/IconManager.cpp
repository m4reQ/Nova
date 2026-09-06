#include "IconManager.hpp"
#include <stb/stb_image.h>
#include <stb/stb_rect_pack.h>
#include <ranges>
#include <future>

struct STBIDeleter
{
    void operator()(std::byte *ptr)
    {
        stbi_image_free(ptr);
    }
};

struct IconLoadData
{
    std::unique_ptr<std::byte, STBIDeleter> Pixels;
    uint32_t Width, Height;
};

static IconLoadData LoadIconFile(const std::filesystem::path &filepath)
{
    int width, height;
    auto pixels = reinterpret_cast<std::byte *>(
        stbi_load(
            filepath.string().c_str(),
            &width,
            &height,
            nullptr,
            4));
    if (pixels == nullptr)
        throw std::runtime_error("Failed to load icon file.");

    return IconLoadData{
        .Pixels = std::unique_ptr<std::byte, STBIDeleter>(pixels, STBIDeleter{}),
        .Width = static_cast<uint32_t>(width),
        .Height = static_cast<uint32_t>(height),
    };
}

IconManager::IconManager(uint32_t maxAtlasWidth, uint32_t maxAtlasHeight)
    : texture_(
          Nova::TextureTarget::Texture2D,
          Nova::TextureSpec{
              .Size = glm::ivec3(maxAtlasWidth, maxAtlasHeight, 1),
              .Format = Nova::InternalFormat::RGBA8,
              .Mipmaps = 1,
              .MinFilter = Nova::TextureMinFilter::Nearest,
              .MagFilter = Nova::TextureMagFilter::Nearest,
          }),
      emptyTexture_(
          Nova::TextureTarget::Texture2D,
          Nova::TextureSpec{
              .Size = glm::ivec3(2, 2, 1),
              .Format = Nova::InternalFormat::RGB8,
              .Mipmaps = 1,
              .MinFilter = Nova::TextureMinFilter::Nearest,
              .MagFilter = Nova::TextureMagFilter::Nearest,
          })
{
    constexpr std::array<uint8_t, 2 * 2 * 3> invalidTexturePixels{
        207,
        12,
        194,
        0,
        0,
        0,
        0,
        0,
        0,
        207,
        12,
        194,
    };
    emptyTexture_.Upload(
        Nova::TextureUploadInfo{
            .Size = glm::ivec3(2, 2, 1),
            .Offset = glm::ivec3(0),
            .DataOffset = 0,
            .Mipmap = 0,
            .PixelFormat = Nova::PixelFormat::RGB,
            .PixelType = Nova::PixelType::UnsignedByte,
        },
        invalidTexturePixels.data(),
        true);
}

Icon IconManager::GetIcon(IconID id) const
{
    const auto &icon = icons_[static_cast<std::underlying_type_t<IconID>>(id)];
    return icon.IsValid
               ? icon
               : Icon{
                     .TextureID = emptyTexture_.GetID(),
                     .UV1 = ImVec2{0.0f, 0.0f},
                     .UV2 = ImVec2{1.0f, 1.0f}};
}

void IconManager::LoadIcons(std::span<const IconDescription> icons)
{
    constexpr auto cRectPackNodesCount = 64zu;

    std::vector<std::pair<IconID, std::future<IconLoadData>>> futures;
    futures.reserve(icons.size());

    for (const auto &description : icons)
        futures.emplace_back(
            std::make_pair(
                description.ID,
                std::async(
                    LoadIconFile,
                    description.Filepath)));

    std::vector<std::pair<IconID, IconLoadData>> results;
    results.reserve(icons.size());

    std::vector<stbrp_rect> rects;
    rects.reserve(icons.size());

    for (auto &[id, future] : futures)
    {
        auto loadData = std::move(future.get());
        rects.emplace_back(
            stbrp_rect{
                .id = static_cast<int>(id),
                .w = static_cast<int>(loadData.Width),
                .h = static_cast<int>(loadData.Height),
            });
        results.emplace_back(std::make_pair(id, std::move(loadData)));
    }

    stbrp_context rectPackContext;
    std::array<stbrp_node, cRectPackNodesCount> rectPackNodes;
    stbrp_init_target(
        &rectPackContext,
        texture_.GetWidth(),
        texture_.GetHeight(),
        rectPackNodes.data(),
        rectPackNodes.size());
    if (!stbrp_pack_rects(&rectPackContext, rects.data(), rects.size()))
        throw std::runtime_error("Failed to build icon atlas.");

    for (auto i = 0zu; i < rects.size(); i++)
    {
        const auto &[name, loadData] = results[i];
        const auto &rect = rects[i];

        texture_.Upload(
            Nova::TextureUploadInfo{
                .Size = glm::ivec3(rect.w, rect.h, 1),
                .Offset = glm::ivec3(rect.x, rect.y, 0),
                .DataOffset = 0,
                .Mipmap = 0,
                .PixelFormat = Nova::PixelFormat::RGBA,
                .PixelType = Nova::PixelType::UnsignedByte,
            },
            loadData.Pixels.get(),
            false);
        icons_[rect.id] = Icon{
            .TextureID = texture_.GetID(),
            .UV1 = ImVec2{
                (static_cast<float>(rect.x) + 0.5f) / static_cast<float>(texture_.GetWidth()),
                (static_cast<float>(rect.y) + 0.5f) / static_cast<float>(texture_.GetHeight()),
            },
            .UV2 = ImVec2{
                (static_cast<float>(rect.x + rect.w) - 0.5f) / static_cast<float>(texture_.GetWidth()),
                (static_cast<float>(rect.y + rect.h) - 0.5f) / static_cast<float>(texture_.GetHeight()),
            },
            .IsValid = true};
    }
}
