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
    float zNear;
    float zFar;
};

layout(std430, binding = 1) readonly buffer sPointLightsBuffer
{
	PointLight pointLights[];
};

layout(std430, binding = 2) readonly buffer sDirLightsBuffer
{
    DirLight dirLights[];
};

// TODO Add support for #include directives

/// Calculates diffuse color using Blinn-Phong reflection model.
/// `normal` vector must be normalized before being passed to this function.
/// `lightDir` must be normalized before being passed to this function.
vec3 calculateDiffuseBlinnPhong(
    vec3 normal,
    vec3 albedoColor,
    vec3 lightColor,
    vec3 lightDir,
    float lightIntensity)
{
    float nDotL = max(dot(normal, lightDir), 0.0);
    return nDotL * albedoColor * lightColor * lightIntensity;
}

/// Calculates specular reflection color using Blinn-Phong reflection model.
/// `normal` vector must be normalized before being passed to this function.
/// `lightDir` must be normalized before being passed to this function.
vec3 calculateSpecularBlinnPhong(
    vec3 normal,
    vec3 viewDir,
    vec3 lightColor,
    vec3 lightDir,
    float lightIntensity,
    float specularIntensity,
    float shininess)
{
    vec3 h = normalize(lightDir + viewDir);
    return pow(max(dot(normal, h), 0.0), shininess)
        * specularIntensity
        * lightColor
        * lightIntensity;
}

/// Calculates light attenuation based on distance from the light source.
float calculateSmoothRadiusAttenuation(float lightDistance, float lightRadius)
{
    float x = lightDistance / lightRadius;
    float attenuation = max(1.0 - x * x, 0.0);
    return attenuation * attenuation;
}

/// Calculates light attenuation based on position of light and shaded fragment.
float calculateSmoothRadiusAttenuationPos(vec3 lightPos, vec3 fragPos, float lightRadius)
{
    return calculateSmoothRadiusAttenuation(length(lightPos - fragPos), lightRadius);
}

void main()
{
    vec3 fragPos = texture(uGBufferPosition, vsTexCoord).xyz;
    vec3 normal = normalize(texture(uGBufferNormal, vsTexCoord).xyz);
    vec4 albedoSpecular = texture(uGBufferAlbedoSpecular, vsTexCoord);

    vec3 lighting = albedoSpecular.rgb * uAmbient;

    vec3 viewDir = normalize(cameraPosition - fragPos);

    for (uint i = 0; i < uDirLightsCount; i++)
    {
        DirLight light = dirLights[i];
        
        vec3 lightDir = normalize(-light.direction);
        
        vec3 diffuse = calculateDiffuseBlinnPhong(
            normal,
            albedoSpecular.rgb,
            light.color.rgb,
            lightDir,
            light.color.a);
        
        vec3 specular = calculateSpecularBlinnPhong(
            normal,
            viewDir,
            light.color.rgb,
            lightDir,
            light.color.a,
            albedoSpecular.a,
            uShininess);

        lighting += diffuse + specular;
    }

    for (uint i = 0; i < uPointLightsCount; i++)
    {
        PointLight light = pointLights[i];

        vec3 lightVec = light.position - fragPos;
        float dist = length(lightVec);

        if (dist < light.radius)
        {
            vec3 lightDir = normalize(lightVec);

            float attenuation = calculateSmoothRadiusAttenuation(dist, light.radius);

            vec3 diffuse = calculateDiffuseBlinnPhong(
                normal,
                albedoSpecular.rgb,
                light.color.rgb,
                lightDir,
                light.color.a);

            vec3 specular = calculateSpecularBlinnPhong(
                normal,
                viewDir,
                light.color.rgb,
                lightDir,
                light.color.a,
                albedoSpecular.a,
                uShininess);

            lighting += (diffuse + specular) * attenuation;
        }
    }

    outColor = vec4(lighting, 1.0);
}