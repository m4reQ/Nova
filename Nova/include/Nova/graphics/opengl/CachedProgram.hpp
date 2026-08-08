#pragma once
#include <string>
#include <chrono>
#include <glad/gl.h>
#include <xxhash.h>
#include <nlohmann/json.hpp>

namespace Nova
{
    struct CachedProgram
    {
        std::string Name;
        std::chrono::system_clock::time_point CreatedAt;
        XXH64_hash_t Hash;
        GLenum BinaryType;

        static CachedProgram FromJSON(const nlohmann::json &json);

        nlohmann::json ToJSON() const;
    };
}