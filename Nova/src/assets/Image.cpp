#include <Nova/assets/Image.hpp>

using namespace Nova;

Image::Image(const AssetSource &source,
             std::optional<std::string_view> name,
             std::optional<ImageSettings> settings) noexcept
    : Asset::Asset(source, name),
      settings_(settings.value_or(ImageSettings{})) {}