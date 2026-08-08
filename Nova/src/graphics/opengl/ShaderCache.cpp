#include <Nova/graphics/opengl/ShaderCache.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Utility.hpp>
#include <stdexcept>
#include <fstream>

using namespace Nova;

static std::filesystem::path _GetCachedProgramFilepath(const std::filesystem::path &cacheDir, const CachedProgram &cachedProgram)
{
    return cacheDir / std::format("{}.bin", cachedProgram.Name);
}

static std::filesystem::path GetCacheInfoFilepath(const std::filesystem::path &directory)
{
    return directory / "CacheInfo.json";
}

static std::unordered_map<std::string, CachedProgram, StringHash, std::equal_to<>> ReadCacheFilepath(const std::filesystem::path &cacheDir)
{
    NV_PROFILE_FUNC;

    std::unordered_map<std::string, CachedProgram, StringHash, std::equal_to<>> cachedPrograms{};

    const auto cacheInfoFilepath = GetCacheInfoFilepath(cacheDir);
    if (std::filesystem::exists(cacheInfoFilepath))
    {
        std::ifstream cacheInfoFile(cacheInfoFilepath);
        if (!cacheInfoFile.is_open())
            return cachedPrograms;

        nlohmann::json cacheInfoJson;
        cacheInfoFile >> cacheInfoJson;

        cachedPrograms.reserve(cacheInfoJson.size());
        for (const auto &cacheEntryData : cacheInfoJson)
        {
            const auto cacheEntry = CachedProgram::FromJSON(cacheEntryData);
            cachedPrograms.emplace(cacheEntry.Name, std::move(cacheEntry));
        }
    }

    return cachedPrograms;
}

static void DumpCacheRegistry(
    const std::filesystem::path &registryFilepath,
    const std::unordered_map<std::string, CachedProgram, StringHash, std::equal_to<>> &cachedPrograms)
{
    NV_PROFILE_FUNC;

    std::ofstream output(registryFilepath);
    if (!output.is_open())
        throw std::runtime_error("Failed to save shader cache registry.");

    nlohmann::json json = nlohmann::json::array();
    for (const auto &[_, entry] : cachedPrograms)
        json.push_back(entry.ToJSON());

    output << json;
}

ShaderCache::ShaderCache()
    : ShaderCache(std::filesystem::current_path()) {}

ShaderCache::ShaderCache(const std::filesystem::path &directory)
    : directory_(directory)
{
    ReadCacheFilepath(directory_);
}

ShaderCache::~ShaderCache() noexcept
{
    DumpCacheRegistry(GetCacheInfoFilepath(directory_), m_CachedPrograms);
}

void ShaderCache::Clear(bool removeData) noexcept
{
    NV_PROFILE_FUNC;

    if (removeData)
    {
        for (const auto &[_, cachedProgram] : m_CachedPrograms)
        {
            const auto dataFilepath = _GetCachedProgramFilepath(directory_, cachedProgram);
            std::filesystem::remove(dataFilepath);
        }

        std::filesystem::remove(GetCacheInfoFilepath(directory_));
    }

    m_CachedPrograms.clear();
}

bool ShaderCache::IsProgramCached(const std::string_view name) noexcept
{
    return m_CachedPrograms.find(name) != m_CachedPrograms.end();
}

std::filesystem::path ShaderCache::GetCachedProgramFilepath(const std::string_view name)
{
    const auto cachedProgram = m_CachedPrograms.find(name);
    if (cachedProgram == m_CachedPrograms.end())
        throw std::runtime_error("Couldn't find cached shader program with given name.");

    return _GetCachedProgramFilepath(directory_, cachedProgram->second);
}

ShaderProgram ShaderCache::LoadCachedProgram(const std::string_view name)
{
    NV_PROFILE_FUNC;

    const auto cachedProgram = m_CachedPrograms.find(name);
    if (cachedProgram == m_CachedPrograms.end())
        throw std::runtime_error("Couldn't find cached shader program with given name.");

    return ShaderProgram::FromBinary(
        cachedProgram->second.BinaryType,
        directory_ / std::format("{}.bin", cachedProgram->second.Name));
}

ShaderProgram ShaderCache::LoadCachedProgram(const std::string_view name, std::function<ShaderProgram(void)> fallback)
{
    NV_PROFILE_FUNC;

    const auto cachedProgram = m_CachedPrograms.find(name);
    if (cachedProgram == m_CachedPrograms.end())
    {
        auto program = fallback();
        CacheProgram(program, name);

        return program;
    }

    return ShaderProgram::FromBinary(
        cachedProgram->second.BinaryType,
        directory_ / std::format("{}.bin", cachedProgram->second.Name));
}

void ShaderCache::CacheProgram(ShaderProgram &program, const std::string_view name)
{
    NV_PROFILE_FUNC;

    const auto [binary, binaryType] = program.GetBinary();

    std::ofstream binaryFile(directory_ / std::format("{}.bin", name), std::ios::binary);
    if (!binaryFile.is_open())
        throw std::runtime_error("Failed to open shader binary file for writing.");

    {
        NV_PROFILE_SCOPE("WriteProgramBinary");
        binaryFile.write((char *)binary.data(), binary.size_bytes());
    }

    const auto nameStr = std::string(name);
    CachedProgram cachedProgram{
        .Name = nameStr,
        .CreatedAt = std::chrono::system_clock::now(),
        .Hash = XXH64(binary.data(), binary.size_bytes(), 2137),
        .BinaryType = binaryType,
    };

    m_CachedPrograms.insert_or_assign(nameStr, cachedProgram);
}

void ShaderCache::SetDirectory(const std::filesystem::path &directory)
{
    NV_PROFILE_FUNC;

    if (directory == directory_)
        return;

    if (!std::filesystem::exists(directory))
    {
        NV_PROFILE_SCOPE("CreateCacheDirectory");

        if (!std::filesystem::create_directories(directory))
            throw std::runtime_error("Failed to create shader cache directory.");
    }

    if (!std::filesystem::is_directory(directory))
        throw std::runtime_error("Shader cache path is not a directory.");

    directory_ = directory;
    m_CachedPrograms = ReadCacheFilepath(directory);
}
