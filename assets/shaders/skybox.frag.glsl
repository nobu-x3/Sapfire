#version 450


//shader input
layout (location = 0) in vec3 texCoord;
//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 2, binding = 0) uniform samplerCube samplerCubeMap;

void main()
{
    outFragColor = texture(samplerCubeMap, texCoord);
//    outFragColor = vec4(texCoord, 1.0)
}
