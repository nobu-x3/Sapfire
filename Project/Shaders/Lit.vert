#version 330

out vec2 fragTexCoord;
// in world space
out vec3 fragNormal;
out vec3 fragWorldPos;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inTexCoord;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
    // to world space
	pos = pos * uWorldTransform;
	fragWorldPos = pos.xyz;
    // to clip space
	gl_Position = pos * uViewProj;
    // don't need w in normal, so w = 0
	fragNormal = (vec4(inNormal, 0.0) * uWorldTransform).xyz;
    fragTexCoord = inTexCoord;
}
