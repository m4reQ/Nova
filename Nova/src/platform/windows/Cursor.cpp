#include <Nova/platform/windows/Cursor.hpp>

static HCURSOR LoadCursorFromFile(const std::filesystem::path &filepath, HINSTANCE instance)
{
    HANDLE image{};
    if constexpr (std::is_same_v<std::filesystem::path::value_type, char>)
        image = LoadImageA(
            instance,
            reinterpret_cast<const char *>(filepath.c_str()),
            IMAGE_CURSOR,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);
    else
        image = LoadImageW(
            instance,
            reinterpret_cast<const wchar_t *>(filepath.c_str()),
            IMAGE_CURSOR,
            0,
            0,
            LR_LOADFROMFILE | LR_DEFAULTSIZE);

    if (!image)
        throw std::runtime_error("Failed to load cursor from file.");

    return static_cast<HCURSOR>(image);
}

Nova::Cursor::Cursor(HCURSOR handle) noexcept
    : handle_(
          handle,
          [](auto x)
          {
              if (x != nullptr)
                  DestroyCursor(x);
          }) {}

Nova::Cursor::Cursor(const std::filesystem::path &filepath, HINSTANCE instance)
    : Cursor(LoadCursorFromFile(filepath, instance)) {}
