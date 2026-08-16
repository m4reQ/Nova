#include <Nova/graphics/opengl/ShaderCache.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Utility.hpp>
#include <Nova/core/File.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <stdexcept>
#include <fstream>
#include <format>
#include <nlohmann/json.hpp>

using namespace Nova;

constexpr auto cCacheInfoFilename = "ShaderCacheInfo.json";
constexpr auto cCacheVersion = 1;

static std::filesystem::path GetCacheInfoFilepath(const std::filesystem::path &directory)
{
    return directory / cCacheInfoFilename;
}

static std::filesystem::file_time_type TimestampFromJSON(const nlohmann::json &json)
{
    return std::filesystem::file_time_type(
        std::filesystem::file_time_type::duration(
            json.get<std::filesystem::file_time_type::rep>()));
}

static std::variant<SourceDependency, BinaryStageDependency, BinaryProgramDependency> ProgramDependencyDataFromJSON(const nlohmann::json &json)
{
    switch (json["type"].get<DependencyType>())
    {
    case DependencyType::Source:
        return SourceDependency{
            .ShaderType = json["shader_type"].get<ShaderType>(),
        };
    case DependencyType::BinaryStage:
        return BinaryStageDependency{
            .ShaderType = json["shader_type"].get<ShaderType>(),
            .BinaryFormat = json["format"].get<GLenum>(),
        };
    case DependencyType::BinaryProgram:
        return BinaryProgramDependency{
            .BinaryFormat = json["format"].get<GLenum>(),
        };
    }

    std::unreachable();
}

static ProgramDependency ProgramDependencyFromJSON(const nlohmann::json &json)
{
    return ProgramDependency{
        .Timestamp = TimestampFromJSON(json["timestamp"]),
        .Filepath = json["filepath"].get<std::filesystem::path>(),
        .FileSize = json["size"].get<uintmax_t>(),
        .Data = ProgramDependencyDataFromJSON(json),
    };
}

static std::vector<ProgramDependency> LoadDependenciesFromJSON(const nlohmann::json &json)
{
    std::vector<ProgramDependency> dependencies;
    dependencies.reserve(json.size());

    for (const auto &dependencyData : json)
        dependencies.emplace_back(ProgramDependencyFromJSON(dependencyData));

    return dependencies;
}

static CacheEntryMap LoadCacheEntriesFromJSON(const nlohmann::json &json)
{
    CacheEntryMap entries;
    entries.reserve(json.size());

    for (const auto &entryData : json)
        entries.insert(
            std::make_pair(
                entryData["name"].get<std::string>(),
                CacheEntry{
                    .Dependencies = LoadDependenciesFromJSON(entryData["dependencies"]),
                    .ProgramName = entryData["name"].get<std::string>(),
                    .BinaryFormat = entryData["format"].get<GLenum>(),
                }));

    return entries;
}

static Nova::CacheData LoadCacheDataFromFile(const std::filesystem::path &file)
{
    NV_PROFILE_FUNC;

    std::ifstream fileStream(file);
    if (!fileStream.is_open())
        return Nova::CacheData{
            .Entries = {},
            .VendorName = std::string(GL::GetString(StringName::Vendor)),
            .GLSLVersionName = std::string(GL::GetString(StringName::ShadingLanguageVersion)),
            .RendererName = std::string(GL::GetString(StringName::Renderer)),
            .Version = cCacheVersion,
        };

    const auto json = nlohmann::json::parse(fileStream);
    return Nova::CacheData{
        .Entries = LoadCacheEntriesFromJSON(json["entries"]),
        .VendorName = json["vendor"].get<std::string>(),
        .GLSLVersionName = json["glsl"].get<std::string>(),
        .RendererName = json["renderer"].get<std::string>(),
        .Version = json["version"].get<int>(),
    };
}

static void DependencyToJSON(nlohmann::json &json, const SourceDependency &dependency) noexcept
{
    json["shader_type"] = dependency.ShaderType;
    json["type"] = DependencyType::Source;
}

static void DependencyToJSON(nlohmann::json &json, const BinaryStageDependency &dependency) noexcept
{
    json["shader_type"] = dependency.ShaderType;
    json["format"] = dependency.BinaryFormat;
    json["type"] = DependencyType::BinaryStage;
}

static void DependencyToJSON(nlohmann::json &json, const BinaryProgramDependency &dependency) noexcept
{
    json["format"] = dependency.BinaryFormat;
    json["type"] = DependencyType::BinaryProgram;
}

static nlohmann::json ProgramDependencyToJSON(const ProgramDependency &dependency) noexcept
{
    auto json = nlohmann::json::object();
    json["timestamp"] = dependency.Timestamp.time_since_epoch().count();
    json["filepath"] = dependency.Filepath;
    json["size"] = dependency.FileSize;

    std::visit(
        [&](const auto &x)
        {
            return DependencyToJSON(json, x);
        },
        dependency.Data);

    return json;
}

static nlohmann::json ProgramDependenciesToJSON(std::span<const ProgramDependency> dependencies) noexcept
{
    auto json = nlohmann::json::array();
    for (const auto &dependency : dependencies)
        json.emplace_back(ProgramDependencyToJSON(dependency));

    return json;
}

static nlohmann::json CacheEntryToJSON(const std::string_view name, const CacheEntry &entry) noexcept
{
    auto json = nlohmann::json::object();
    json["name"] = name;
    json["dependencies"] = ProgramDependenciesToJSON(entry.Dependencies);
    json["format"] = entry.BinaryFormat;

    return json;
}

static nlohmann::json CacheEntriesToJSON(const CacheEntryMap &cacheEntries) noexcept
{
    auto json = nlohmann::json::array();

    for (const auto &[name, entry] : cacheEntries)
        json.emplace_back(CacheEntryToJSON(name, entry));

    return json;
}

static nlohmann::json CacheDataToJSON(const Nova::CacheData &data) noexcept
{
    auto json = nlohmann::json::object();
    json["entries"] = CacheEntriesToJSON(data.Entries);
    json["vendor"] = data.VendorName;
    json["glsl"] = data.GLSLVersionName;
    json["renderer"] = data.RendererName;
    json["version"] = data.Version;

    return json;
}

static void DumpCacheData(const std::filesystem::path &directory, const Nova::CacheData &cacheData) noexcept
{
    NV_PROFILE_FUNC;

    std::ofstream output(directory / cCacheInfoFilename);
    if (!output.is_open())
        // NOTE Called from noexcept destructor so just ignore error
        return;

    output << CacheDataToJSON(cacheData);
}

ShaderCache::ShaderCache()
    : ShaderCache(std::filesystem::current_path()) {}

ShaderCache::ShaderCache(const std::filesystem::path &directory)
    : directory_(directory),
      cacheData_(LoadCacheDataFromFile(GetCacheInfoFilepath(directory)))
{
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);
}

ShaderCache::~ShaderCache() noexcept
{
    DumpCacheData(directory_, cacheData_);
}

void ShaderCache::Clear(bool clearDirectory) noexcept
{
    NV_PROFILE_FUNC;

    if (clearDirectory)
    {
        for (const auto &dir : std::filesystem::directory_iterator(directory_))
            std::filesystem::remove_all(dir);
    }

    cacheData_.Entries.clear();
}

bool ShaderCache::IsProgramCached(const std::string_view name) noexcept
{
    return cacheData_.Entries.find(name) != cacheData_.Entries.end();
}

std::filesystem::path ShaderCache::GetCachedProgramFilepath(const std::string_view name)
{
    return directory_ / std::format("{}.bin", name);
}

ShaderProgram ShaderCache::LoadCachedProgram(const std::string_view name)
{
    NV_PROFILE_FUNC;

    const auto cacheEntry = cacheData_.Entries.find(name);
    if (cacheEntry == cacheData_.Entries.end())
        throw std::runtime_error("Couldn't find cache entry with given name.");

    // TODO Check if context is the same as the one that created cached program
    for (const auto &dependency : cacheEntry->second.Dependencies)
    {
        const auto lastWriteTime = std::filesystem::last_write_time(dependency.Filepath);
        if (lastWriteTime > dependency.Timestamp)
            throw std::runtime_error("One of the shader program dependencies is outdated.");
    }

    const auto binaryFilepath = GetCachedProgramFilepath(name);
    return ShaderProgram(binaryFilepath, cacheEntry->second.BinaryFormat);
}

ShaderProgram ShaderCache::LoadCachedProgram(const std::string_view name, std::function<ShaderProgram(void)> fallback)
{
    NV_PROFILE_FUNC;

    const auto binaryFilepath = GetCachedProgramFilepath(name);

    const auto cacheEntry = cacheData_.Entries.find(name);
    if (cacheEntry == cacheData_.Entries.end())
    {
        auto program = fallback();

        const auto [binary, binaryFormat] = program.GetBinary();
        File::WriteBinary(binaryFilepath, binary);

        cacheData_.Entries.insert(
            std::make_pair(
                name,
                CacheEntry{
                    .Dependencies = program.GetDependencies(),
                    .ProgramName = std::string(name),
                    .BinaryFormat = binaryFormat}));

        // TODO Is move here necessarry?
        return std::move(program);
    }

    // TODO Check if context is the same as the one that created cached program
    for (const auto &dependency : cacheEntry->second.Dependencies)
    {
        const auto lastWriteTime = std::filesystem::last_write_time(dependency.Filepath);
        if (lastWriteTime > dependency.Timestamp)
        {
            // One of the dependencies changed -> recompile program and update cache
            auto program = fallback();

            const auto [binary, binaryFormat] = program.GetBinary();
            File::WriteBinary(binaryFilepath, binary);

            cacheEntry->second.Dependencies = program.GetDependencies();

            // TODO Same with move here
            return std::move(program);
        }
    }

    return ShaderProgram(binaryFilepath, cacheEntry->second.BinaryFormat);
}

void ShaderCache::CacheProgram(const ShaderProgram &program, const std::string_view name)
{
    NV_PROFILE_FUNC;

    const auto [binary, binaryFormat] = program.GetBinary();
    const auto binaryFilepath = directory_ / name;

    File::WriteBinary(binaryFilepath, binary);

    cacheData_.Entries.insert_or_assign(
        std::string(name),
        CacheEntry{
            .Dependencies = program.GetDependencies(),
            .ProgramName = std::string(name),
            .BinaryFormat = binaryFormat});
}
