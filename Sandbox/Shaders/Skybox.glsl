#type vertex
#version 440 core

out vec3 textureDir;

layout(location=0) in vec3 inPosition;

layout(std140, row_major, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 proj;
} matrices;

uniform mat4 uViewMatrix;
void main()
{
    vec4 position = vec4(inPosition.xy, 1.0, 1.0);
    textureDir = (position * inverse(matrices.view * matrices.proj)).xyz;
	gl_Position = position;
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
