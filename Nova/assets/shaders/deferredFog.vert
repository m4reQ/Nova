#version 450 core

const vec4 cPositions[6] = {
    vec4(-1.0, -1.0, 0.0, 0.0),
    vec4(1.0, -1.0, 1.0, 0.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(-1.0, 1.0, 0.0, 1.0),
    vec4(-1.0, -1.0, 0.0, 0.0),
};

out vec2 vsTexCoord;

void main()
{
    gl_Position = vec4(cPositions[gl_VertexID].xy, 0.0, 1.0);
    vsTexCoord = cPositions[gl_VertexID].zw;
}