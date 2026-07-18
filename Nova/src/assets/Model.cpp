#include <Nova/assets/Model.hpp>

using namespace Nova;

Model::Model(const AssetSource &source, std::optional<std::string_view> name) noexcept
    : Asset::Asset(source, name) {}