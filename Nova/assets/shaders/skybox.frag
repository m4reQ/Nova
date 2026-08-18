#version 450 core

in vec3 vsTexCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube uSkybox;

void main()
{
    outColor = texture(uSkybox, vsTexCoords);
}