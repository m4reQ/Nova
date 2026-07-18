#include <Nova/platform/windows/Icon.hpp>
#include <Nova/platform/windows/Bitmap.hpp>
#include <stdexcept>

static HICON LoadIconFromFile(const std::filesystem::path &filepath, HINSTANCE instance)
{
    HANDLE image{};
    if constexpr (std::is_same_v<std::filesystem::path::value_type, char>)
        image = LoadImageA(
            instance,
            reinterpret_cast<const char *>(filepath.c_str()),
            IMAGE_ICON,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);
    else
        image = LoadImageW(
            instance,
            reinterpret_cast<const wchar_t *>(filepath.c_str()),
            IMAGE_ICON,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);

    if (image == nullptr)
        throw std::runtime_error("Failed to load icon from file.");

    return static_cast<HICON>(image);
}

static HICON LoadIconFromData(int width, int height, std::span<const std::byte> data)
{
    auto bitmap = Nova::Bitmap(
        width,
        height,
        1,
        32,
        data);
    auto mask = Nova::Bitmap(
        width,
        height,
        1,
        1,
        {});

    ICONINFO iconInfo{
        .fIcon = TRUE,
        .hbmMask = mask,
        .hbmColor = bitmap,
    };

    auto icon = CreateIconIndirect(&iconInfo);
    if (icon == nullptr)
        throw std::runtime_error("Failed to load icon from data.");

    return icon;
}

Nova::Icon::Icon(HICON icon) noexcept
    : icon_(
          icon,
          [](auto x)
          {
              if (x != nullptr)
                  DestroyIcon(x);
          })
{
}

Nova::Icon::Icon(ICONINFO &info)
    : Icon(CreateIconIndirect(&info))
{
    if (icon_ == nullptr)
        throw std::runtime_error("Failed to create icon.");
}

Nova::Icon::Icon(const std::filesystem::path &filepath, HINSTANCE instance)
    : Icon(LoadIconFromFile(filepath, instance)) {}

Nova::Icon::Icon(int width, int height, std::span<const std::byte> data)
    : Icon(LoadIconFromData(width, height, data)) {}