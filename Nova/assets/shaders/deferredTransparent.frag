#version 450 core

struct Material
{
	vec4 color;
	float specularIntensity;
};

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

in flat uint vsMaterialIndex;
in vec3 vsPosition;
in vec3 vsNormal;
// in vec2 vsTexCoord;

layout(location=3) out vec4 outColor;

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

layout(std430, binding = 1) readonly buffer sMaterialData
{
	Material materialData[];
};

layout(std430, binding = 2) readonly buffer sPointLightsBuffer
{
	PointLight pointLights[];
};

layout(std430, binding = 3) readonly buffer sDirLightsBuffer
{
    DirLight dirLights[];
};

void main()
{
	Material material = materialData[vsMaterialIndex];
    vec3 baseColor = material.color.rgb;
    float baseAlpha = material.color.a;

    vec3 normal = normalize(vsNormal);
    
    vec3 viewDir = normalize(cameraPosition - vsPosition);
    vec3 lighting = baseColor * uAmbient;

    for (uint i = 0; i < uDirLightsCount; i++)
    {
        DirLight light = dirLights[i];

        vec3 l = normalize(-light.direction);
        vec3 v = viewDir;
        vec3 h = normalize(l + v);

        // diffuse
        float nDotL = max(dot(normal, l), 0.0);
        vec3 diffuse = nDotL * baseColor * light.color.rgb * light.color.a;

        // specular
        vec3 specular = pow(max(dot(normal, h), 0.0), uShininess) *
            material.specularIntensity * 
            light.color.rgb *
            light.color.a;

        lighting += diffuse + specular;
    }

    for (uint i = 0; i < uPointLightsCount; i++)
    {
        PointLight light = pointLights[i];

        vec3 lightVec = light.position - vsPosition;
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
            vec3 diffuse = nDotL * baseColor * light.color.rgb * light.color.a;

            // specular
            vec3 specular = 
                pow(max(dot(normal, h), 0.0), uShininess) *
                material.specularIntensity *
                light.color.rgb *
                light.color.a;
            
            lighting += (diffuse + specular) * attenuation;
        }
    }

	outColor = vec4(lighting, material.color.a);
}