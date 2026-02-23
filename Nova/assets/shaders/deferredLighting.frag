#version 450 core

struct Light
{
    vec4 color;
    vec3 position;
};

layout(location=3) out vec4 outColor;

in vec2 vsTexCoord;

layout(binding=0) uniform sampler2D uGBufferAlbedoSpecular;
layout(binding=1) uniform sampler2D uGBufferPosition;
layout(binding=2) uniform sampler2D uGBufferNormal;
uniform uint uLightsCount;
uniform float uAmbient;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
};

layout(std430) buffer sLightData
{
	Light lightData[];
};

void main()
{
    vec3 fragPos = texture(uGBufferPosition, vsTexCoord).xyz;
    vec3 normal = texture(uGBufferNormal, vsTexCoord).xyz;
    vec4 albedoSpecular = texture(uGBufferAlbedoSpecular, vsTexCoord);

    vec3 lighting = albedoSpecular.rgb * uAmbient;

    vec3 viewDir = normalize(cameraPosition - fragPos);
    for (uint i = 0; i < uLightsCount; i++)
    {
        Light light = lightData[i];
        
        vec3 lightDir = normalize(light.position - fragPos);
        vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedoSpecular.a * light.color.rgb * light.color.a;

        lighting += diffuse;
    }

    outColor = vec4(lighting, 1.0);
}