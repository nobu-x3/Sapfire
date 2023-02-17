#type vertex
#version 460 core

out vec2 fragTexCoord;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec3 inBinormal;
layout(location=4) in vec2 inTexCoord;

layout(std140, row_major, binding = 5) uniform Matrices
{
	mat4 VP;
} matrices;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
	gl_Position = pos * uWorldTransform * matrices.VP;
    fragTexCoord = inTexCoord;
}

#type fragment
#version 460 core

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D uTexture;

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    outColor = texture(uTexture, fragTexCoord);
    //float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    //outColor = vec4(vec3(depth), 1.0);
}
