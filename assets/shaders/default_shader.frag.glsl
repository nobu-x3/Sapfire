//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform  SceneData{
	vec4 ambientColor;
} sceneData;


void main()
{
	outFragColor = vec4(inColor + sceneData.ambientColor.xyz,1.0f);
}
