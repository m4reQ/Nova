#pragma once
#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <filesystem>
#include <string_view>
#include <string>
#include <functional>
#include <unordered_map>

namespace Nova
{
    // TODO Checksum for generated binary
    /// @brief Specifies a cache entry for a single shader program
    struct CacheEntry
    {
        std::vector<ProgramDependency> Dependencies;
        std::string ProgramName;
        GLenum BinaryFormat;
    };

    using CacheEntryMap = std::unordered_map<std::string, CacheEntry, StringHash, std::equal_to<>>;

    struct CacheData
    {
        CacheEntryMap Entries;
        std::string VendorName;
        std::string GLSLVersionName;
        std::string RendererName;
        int Version;
    };

    using CachedProgramMap = std::unordered_map<std::string, ShaderProgram, StringHash, std::equal_to<>>;

    class ShaderCache
    {
    public:
        /// @brief Creates shader cache with the current working directory as a cache storage.
        ShaderCache();

        /// @brief Creates shader cache with the specified directory as a cache storage.
        /// @param directory A path to the directory containing cached data
        ShaderCache(const std::filesystem::path &directory);

        ShaderCache(const ShaderCache &) = delete;

        ShaderCache(ShaderCache &&) noexcept = default;

        ~ShaderCache() noexcept;

        void Clear(bool clearDirectory) noexcept;

        bool IsProgramCached(const std::string_view name) noexcept;

        ShaderProgram LoadCachedProgram(const std::string_view name);

        ShaderProgram LoadCachedProgram(const std::string_view name, std::function<ShaderProgram(void)> fallback);

        void CacheProgram(const ShaderProgram &program, const std::string_view name);

        std::filesystem::path GetCachedProgramFilepath(const std::string_view name);

        constexpr const std::filesystem::path &GetDirectory() const noexcept { return directory_; }

    private:
        CacheData cacheData_;
        std::filesystem::path directory_;
        bool isEnabled_ = true;
    };
}
