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
    mat4 viewTr = mat4(mat3(matrices.view));
    textureDir = inPosition;
	vec4 pos = vec4(inPosition, 1.0) * matrices.proj * mat4(1.0);
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
