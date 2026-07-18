#include <Nova/assets/Asset.hpp>

using namespace Nova;

Asset::Asset(const AssetSource &source, std::optional<std::string_view> name) noexcept
    : source_(source),
      name_(name.value_or("")),
      uuid_(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID()) {}