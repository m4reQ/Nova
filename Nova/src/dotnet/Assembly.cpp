#include <Nova/dotnet/Assembly.hpp>
#include <Nova/dotnet/Host.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

static std::unordered_map<std::string_view, DotnetType, StringHash, std::equal_to<>> RetrieveAssemblyTypes(int32_t assemblyID)
{
    NV_PROFILE_FUNC;

    std::unordered_map<std::string_view, DotnetType, StringHash, std::equal_to<>> types;
    
    auto typesInfo = Dotnet::GetAssemblyTypes_(assemblyID);
    for (auto& typeInfo : typesInfo.GetView())
    {
        DotnetType type(std::move(typeInfo));
        types.emplace(type.GetName(), std::move(type));
    }
    
    return types;
}

DotnetAssembly::DotnetAssembly(const std::filesystem::path& filepath)
    : info_(Dotnet::LoadAssemblyFromFilepath_(filepath)),
      types_(RetrieveAssemblyTypes(info_.GetID())) { }

DotnetAssembly::DotnetAssembly(const std::span<uint8_t> data)
    : info_(Dotnet::LoadAssemblyFromMemory_(data)),
      types_(RetrieveAssemblyTypes(info_.GetID())) { }

const DotnetType& DotnetAssembly::GetType(const std::string_view name)
{
    return types_.find(name)->second;
}