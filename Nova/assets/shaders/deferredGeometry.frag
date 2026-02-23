#version 450 core

in flat uint vsMaterialIndex;
in vec3 vsPosition;
in vec3 vsNormal;
	// in vec2 vsTexCoord;

layout(location=0) out vec4 outColor;
layout(location=1) out vec3 outPosition;
layout(location=2) out vec3 outNormal;
// layout(location=3) out vec2 outTexCoord;

struct Material
{
	vec4 color;
	// float specular;
};

layout(std430) buffer sMaterialData
{
	Material materialData[];
};

void main()
{
	Material material = materialData[vsMaterialIndex];

	outColor = vec4(material.color.rgb, 1.0);
	outPosition = vsPosition;
	outNormal = vsNormal;
	// outTexCoord = vsTexCoord;
}