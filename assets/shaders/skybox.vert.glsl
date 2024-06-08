#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 texCoord;

layout(set = 0, binding = 0) uniform  CameraBuffer{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraData;

struct ObjectData{
	mat4 model;
};

//all object matrices
layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;


void main()
{
	texCoord = vPosition;
	// Convert cubemap coordinates into Vulkan coordinate space
	//texCoord.xy *= -1.0;
    mat4 view = mat4(mat3(cameraData.view));
	gl_Position = (cameraData.proj * view * vec4(vPosition.xyz, 1.0)).xyww;
}
