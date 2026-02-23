#version 450 core

// per-vertex data
layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec2 inTextureCoords;
// per-instance data
layout(location = 2) in uint inMaterialIndex;
layout(location = 3) in mat4 inTransform;

// out vec2 vsTextureCoords;
out flat uint vsMaterialIndex;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
};

void main()
{
	gl_Position = cameraProjection * cameraView * inTransform * vec4(inPosition, 1.0);
	// vsTextureCoords = inTextureCoords;
	vsMaterialIndex = inMaterialIndex;
}