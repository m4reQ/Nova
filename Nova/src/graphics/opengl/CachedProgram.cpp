#include <Nova/graphics/opengl/CachedProgram.hpp>

Nova::CachedProgram Nova::CachedProgram::FromJSON(const nlohmann::json &json)
{
    return CachedProgram{
        .Name = json["name"],
        .CreatedAt = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(
                (unsigned long)json["created_at"])),
        .Hash = std::stoull(std::string(json["hash"])),
        .BinaryType = json["type"],
    };
}

nlohmann::json Nova::CachedProgram::ToJSON() const
{
    return nlohmann::json{
        {"name", Name},
        {"type", BinaryType},
        {"created_at", std::chrono::duration_cast<std::chrono::milliseconds>(CreatedAt.time_since_epoch()).count()},
        {"hash", std::to_string(Hash)},
    };
}