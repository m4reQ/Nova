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
uniform float uShininess;
uniform uint uPointLightsCount;
uniform uint uDirLightsCount;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
};

layout(std430, binding = 1) readonly buffer sPointLightsBuffer
{
	PointLight pointLights[];
};

layout(std430, binding = 2) readonly buffer sDirLightsBuffer
{
    DirLight dirLights[];
};

void main()
{
    vec3 fragPos = texture(uGBufferPosition, vsTexCoord).xyz;
    vec3 normal = texture(uGBufferNormal, vsTexCoord).xyz;
    vec4 albedoSpecular = texture(uGBufferAlbedoSpecular, vsTexCoord);

    vec3 lighting = albedoSpecular.rgb * uAmbient;

    vec3 viewDir = normalize(cameraPosition - fragPos);

    for (uint i = 0; i < uDirLightsCount; i++)
    {
        DirLight light = dirLights[i];

        vec3 l = normalize(-light.direction);
        vec3 v = viewDir;
        vec3 h = normalize(l + v);

        // diffuse
        float nDotL = max(dot(normal, l), 0.0);
        vec3 diffuse = nDotL * albedoSpecular.rgb * light.color.rgb * light.color.a;

        // specular
        vec3 specular = pow(max(dot(normal, h), 0.0), uShininess) *
            albedoSpecular.a *
            light.color.rgb *
            light.color.a;

        lighting += diffuse;
    }

    for (uint i = 0; i < uPointLightsCount; i++)
    {
        PointLight light = pointLights[i];

        vec3 lightVec = light.position - fragPos;
        float dist = length(lightVec);

        if (dist < light.radius)
        {
            vec3 l = normalize(lightVec);
            vec3 v = viewDir;
            vec3 h = normalize(l + v);

            // Smooth radius attenuation
            float x = dist / light.radius;
            float attenuation = max(1.0 - x * x, 0.0);
            attenuation *= attenuation;

            // diffuse
            float nDotL = max(dot(normal, l), 0.0);
            vec3 diffuse = nDotL * albedoSpecular.rgb * light.color.rgb * light.color.a;

            // specular
            vec3 specular = 
                pow(max(dot(normal, h), 0.0), uShininess) *
                albedoSpecular.a *
                light.color.rgb *
                light.color.a;
            
            lighting += (diffuse + specular) * attenuation;
        }
    }

    outColor = vec4(lighting, 1.0);
}