#pragma once
#include <cstdint>
#include <cstring>
#include <glm/vec4.hpp>

namespace Nova
{
    struct Material
	{
		glm::vec4 Color;
		float SpecularIntensity;
		float _Padding[3];
	};

	inline bool operator==(const Material& a, const Material& b) noexcept
	{
		return a.Color == b.Color && a.SpecularIntensity == b.SpecularIntensity;
	}
}