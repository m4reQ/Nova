#pragma once
#include <cstdint>
#include <cstring>
#include <glm/vec4.hpp>

namespace Nova
{
    struct Material
	{
		glm::vec4 Color;
	};

	inline bool operator==(const Material& a, const Material& b) noexcept
	{
		return a.Color == b.Color;
	}
}