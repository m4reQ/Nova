#pragma once
#include <glad/gl.h>
#include <utility>

namespace Nova
{
    /// @brief A simple wrapper for GLuint that provides automatic reset to zero when moved.
    /// Enables usage of default move constructors/assignments for most OpenGL classes.
    class GLID
    {
    public:
        GLID() noexcept = default;

        GLID(const GLID &) noexcept = default;

        constexpr GLID(GLID &&other) noexcept
            : id_(std::exchange(other.id_, 0ul)) {}

        constexpr GLID(GLuint id) noexcept
            : id_(id) {}

        constexpr operator GLuint() const noexcept { return id_; }

        constexpr GLID &operator=(const GLID &) noexcept = default;

        constexpr GLID &operator=(GLID &&other) noexcept
        {
            id_ = std::exchange(other.id_, 0ul);
            return *this;
        }

    private:
        GLuint id_ = 0;
    };
}