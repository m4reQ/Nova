#pragma once
#include <Nova/graphics/opengl/Texture.hpp>
#include <Nova/graphics/opengl/ID.hpp>
#include <Nova/core/Utility.hpp>
#include <imgui.h>
#include <utility>
#include <filesystem>
#include <array>

struct Icon
{
    Nova::GLID TextureID;
    ImVec2 UV1, UV2;
    bool IsValid = false;
};

enum class IconID : size_t
{
    ObjectResize,
    ObjectMove,
    ObjectRotate,
    EntityShow,
    EntityHide,
    EntityRemove,
    EntityAdd,
    EditText,
    AlphaCheckerboard,
    Warning,

    EnumMax_,
};

struct IconDescription
{
    std::filesystem::path Filepath;
    IconID ID;
};

class IconManager
{
public:
    IconManager() = default;

    IconManager(uint32_t maxAtlasWidth, uint32_t maxAtlasHeight);

    IconManager(const IconManager &) = delete;

    IconManager(IconManager &&) noexcept = default;

    ~IconManager() noexcept = default;

    constexpr const Nova::Texture &GetAtlasTexture() const noexcept { return texture_; }

    constexpr Nova::Texture &GetAtlasTexture() noexcept { return texture_; }

    constexpr const Nova::Texture &GetEmptyTexture() const noexcept { return emptyTexture_; }

    constexpr Nova::Texture &GetEmptyTexture() noexcept { return emptyTexture_; }

    Icon GetIcon(IconID id) const;

    constexpr std::span<const Icon> GetIcons() const noexcept { return icons_; }

    void LoadIcons(std::initializer_list<IconDescription> icons) { LoadIcons(std::span(icons)); }

    void LoadIcons(std::span<const IconDescription> icons);

    IconManager &operator=(const IconManager &) = delete;

    IconManager &operator=(IconManager &&) noexcept = default;

private:
    std::array<Icon, static_cast<std::underlying_type_t<IconID>>(IconID::EnumMax_)> icons_;
    Nova::Texture texture_;
    Nova::Texture emptyTexture_;
};