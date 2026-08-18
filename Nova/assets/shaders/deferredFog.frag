#version 450 core
#extension GL_ARB_bindless_texture : require

in vec2 vsTexCoord;

layout(location = 0) out vec4 outColor;

layout(bindless_sampler) uniform sampler2D uSceneColor;
layout(bindless_sampler) uniform sampler2D uDepth;
uniform vec4 uFogColor; // rgb - color, a - density

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
    float zNear;
    float zFar;
};

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main()
{
    vec3 sceneColor = texture(uSceneColor, vsTexCoord).rgb;
    float depth = texture(uDepth, vsTexCoord).r;
    float linearDepth = linearizeDepth(depth);
    float fogIntensity = uFogColor.a;

    float fogFactor = clamp(
        exp(-fogIntensity * fogIntensity * linearDepth * linearDepth),
        0.0,
        1.0);

    outColor = vec4(mix(uFogColor.rgb, sceneColor, fogFactor).rgb, 1.0);
}