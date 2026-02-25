#version 450 core

struct PointLight
{
    vec4 color;
    vec3 position;
    float radius;
};

struct DirLight
{
    vec4 color;
    vec3 direction;
};

layout(location=3) out vec4 outColor;

in vec2 vsTexCoord;

layout(binding=0) uniform sampler2D uGBufferAlbedoSpecular;
layout(binding=1) uniform sampler2D uGBufferPosition;
layout(binding=2) uniform sampler2D uGBufferNormal;
uniform float uAmbient;
uniform uint uPointLightsCount;
uniform uint uDirLightsCount;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
};

layout(std430) readonly buffer sPointLightsBuffer
{
	PointLight pointLights[];
};

layout(std430) readonly buffer sDirLightsBuffer
{
    DirLight dirLights[];
};

void main()
{
    vec3 fragPos = texture(uGBufferPosition, vsTexCoord).xyz;
    vec3 normal = texture(uGBufferNormal, vsTexCoord).xyz;
    vec4 albedoSpecular = texture(uGBufferAlbedoSpecular, vsTexCoord);

    vec3 lighting = albedoSpecular.rgb * uAmbient;

    for (uint i = 0; i < uDirLightsCount; i++)
    {
        DirLight light = dirLights[i];

        vec3 lightDir = normalize(-light.direction);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse =
            diff *
            albedoSpecular.rgb *
            light.color.rgb *
            light.color.a;

        lighting += diffuse;
    }

    vec3 viewDir = normalize(cameraPosition - fragPos);
    for (uint i = 0; i < uPointLightsCount; i++)
    {
        PointLight light = pointLights[i];

        float distance = length(light.position - fragPos);
        if (distance < light.radius)
        {
            vec3 lightDir = normalize(light.position - fragPos);
            vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedoSpecular.a * light.color.rgb * light.color.a;

            lighting += diffuse;
        }
    }

    outColor = vec4(lighting, 1.0);
}