#type vertex
#version 440 core

out vec3 textureDir;

layout(location=0) in vec3 inPosition;

layout(std140, row_major, binding = 0) uniform Matrices
{
    mat4 VP;
} matrices;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
void main()
{
    textureDir = inPosition;
	vec4 pos = vec4(inPosition, 1.0) * uProjectionMatrix * uViewMatrix;
	gl_Position = pos.xyww;
}

#type fragment
#version 440 core

out vec4 outColor;

in vec3 textureDir;

uniform samplerCube cubemap;

void main()
{
    outColor = texture(cubemap, textureDir);
}
