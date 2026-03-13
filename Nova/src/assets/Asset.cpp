#include <Nova/assets/Asset.hpp>

using namespace Nova;

Asset::Asset(
    const AssetSource &source,
    AssetType type,
    std::optional<std::string_view> name,
    AssetFlags flags)
    : source_(source),
      name_(name.value_or("")),
      type_(type),
      flags_(flags),
      uuid_(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID()) {}