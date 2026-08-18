#version 450 core
#extension GL_ARB_bindless_texture : require

struct Material
{
	vec4 color;
	float specularIntensity;
	float shininess;
	uvec2 albedoTextureHandle;
};

in flat uint vsMaterialIndex;
in vec3 vsPosition;
in vec3 vsNormal;
in vec2 vsTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outShininess;

layout(std430, binding = 1) buffer sMaterialData
{
	Material materialData[];
};

void main()
{
	Material material = materialData[vsMaterialIndex];

	outColor = texture(sampler2D(material.albedoTextureHandle), vsTexCoord) * vec4(material.color.rgb, material.specularIntensity);
	outPosition = vsPosition;
	outNormal = normalize(vsNormal);
	outShininess = vec3(material.shininess, 0.0, 0.0);
}