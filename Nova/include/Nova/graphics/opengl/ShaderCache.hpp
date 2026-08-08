#pragma once
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/CachedProgram.hpp>
#include <filesystem>
#include <string_view>
#include <string>
#include <functional>
#include <chrono>
#include <concepts>
#include <unordered_map>
#include <type_traits>
#include <optional>
#include <nlohmann/json.hpp>
#include <xxhash.h>

namespace Nova
{
    class ShaderCache
    {
    public:
        ShaderCache();

        ShaderCache(const std::filesystem::path &directory);

        ~ShaderCache() noexcept;

        void Clear(bool removeData) noexcept;

        bool IsProgramCached(const std::string_view name) noexcept;

        ShaderProgram LoadCachedProgram(const std::string_view name);

        ShaderProgram LoadCachedProgram(const std::string_view name, std::function<ShaderProgram(void)> fallback);

        void CacheProgram(ShaderProgram &program, const std::string_view name);

        void SetDirectory(const std::filesystem::path &directory);

        std::filesystem::path GetCachedProgramFilepath(const std::string_view name);

        constexpr const std::filesystem::path &GetDirectory() const noexcept { return directory_; }

    private:
        std::unordered_map<std::string, CachedProgram, StringHash, std::equal_to<>> m_CachedPrograms;
        std::filesystem::path directory_;
        bool isEnabled_ = true;
    };
}
