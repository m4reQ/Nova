#version 450 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inMaterialIndex;
layout(location = 4) in uvec2 inBindlessHandle;
layout(location = 5) in mat4 inTransform;
layout(location = 9) in mat3 inNormalTransform;

out flat uint vsMaterialIndex;
out flat uvec2 vsBindlessTextureHandle;
out vec3 vsPosition;
out vec3 vsNormal;
out vec2 vsTexCoord;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
    float zNear;
    float zFar;
};

void main()
{
	vsMaterialIndex = inMaterialIndex;
	vsTexCoord = inTexCoord;
	vsBindlessTextureHandle = inBindlessHandle;

	vec4 worldPos = inTransform * vec4(inPosition, 1.0);
	vsPosition = worldPos.xyz;

	vsNormal = normalize(inNormalTransform * inNormal);

	gl_Position = cameraProjection * cameraView * worldPos;
}