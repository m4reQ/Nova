#version 450 core

const vec3 cPositions[36] = {
    vec3(-1.0f, 1.0f, -1.0f),
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(1.0f, -1.0f, -1.0f),
    vec3(1.0f, -1.0f, -1.0f),
    vec3(1.0f, 1.0f, -1.0f),
    vec3(-1.0f, 1.0f, -1.0f),

    vec3(-1.0f, -1.0f, 1.0f),
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f, 1.0f, -1.0f),
    vec3(-1.0f, 1.0f, -1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(-1.0f, -1.0f, 1.0f),

    vec3(1.0f, -1.0f, -1.0f),
    vec3(1.0f, -1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, -1.0f),
    vec3(1.0f, -1.0f, -1.0f),

    vec3(-1.0f, -1.0f, 1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, -1.0f, 1.0f),
    vec3(-1.0f, -1.0f, 1.0f),

    vec3(-1.0f, 1.0f, -1.0f),
    vec3(1.0f, 1.0f, -1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(-1.0f, 1.0f, -1.0f),

    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f, 1.0f),
    vec3(1.0f, -1.0f, -1.0f),
    vec3(1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f, 1.0f),
    vec3(1.0f, -1.0f, 1.0f),
};

out vec3 vsTexCoords;

layout(std140) uniform uCameraData
{
	mat4 cameraView;
	mat4 cameraProjection;
	vec3 cameraPosition;
    float zNear;
    float zFar;
};

void main() {
    vsTexCoords = cPositions[gl_VertexID];
  
    vec4 pos = cameraProjection * mat4(mat3(cameraView)) * vec4(cPositions[gl_VertexID], 1.0);
    gl_Position = pos.xyww;
}