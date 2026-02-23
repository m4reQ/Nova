#version 450 core

// in vec2 vsTextureCoords;
in flat uint vsMaterialIndex;

// uniform sampler2D uTexture;

layout(location=0) out vec4 outColor;

struct Material
{
	vec4 color;
};

layout(std430) buffer sMaterialData
{
	Material materialData[];
};

void main()
{
	Material material = materialData[vsMaterialIndex];
	// outColor = material.color * texture(uTexture, vsTextureCoords);
	outColor = material.color;
}