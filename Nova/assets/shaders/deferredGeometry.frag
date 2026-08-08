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

out vec4 outColor;
out vec3 outPosition;
out vec3 outNormal;
out vec3 outShininess;

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