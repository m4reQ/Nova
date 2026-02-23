#pragma once
#include <Nova/dotnet/Memory.hpp>

namespace Nova
{
    class TypeInfo
    {
    public:
        TypeInfo() = default;

        constexpr TypeInfo(TypeInfo&& other) noexcept = default;

        constexpr const std::string_view GetName() const noexcept { return name_.Get(); }
        
        constexpr const std::string_view GetFullName() const noexcept { return fullName_.Get(); }
        
        constexpr const std::string_view GetAssemblyQualifiedName() const noexcept { return assemblyQualifiedName_.Get(); }

        constexpr int32_t GetID() const noexcept { return id_; }

        constexpr TypeInfo& operator=(TypeInfo&& other) noexcept = default;

    private:
        LocalMemory<char> name_;
        LocalMemory<char> fullName_;
        LocalMemory<char> assemblyQualifiedName_;
        int32_t id_;
    };
}