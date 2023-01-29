#version 330

out vec2 fragTexCoord;

layout(location=0) in vec3 inPosition;
// layout(location=1) in vec3 inNormal;
layout(location=1) in vec2 inTexCoord;

// uniform mat4 uWorldTransform;
uniform mat4 uViewProjection;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
	gl_Position = pos /** uWorldTransform*/ * uViewProjection;
    fragTexCoord = inTexCoord;
}
