#type vertex
#version 450

out vec3 textureDir;

layout(location=0) in vec3 inPosition;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
	gl_Position = pos * uViewMatrix * uProjectionMatrix;
    textureDir = inPosition;
}

#type fragment
#version 450

out vec4 outColor;

in vec3 textureDir;

uniform samplerCube cubemap;

void main()
{
    outColor = texture(cubemap, textureDir);
}
