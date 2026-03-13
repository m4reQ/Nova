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
    h = normalize(lightDir + viewDir);
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