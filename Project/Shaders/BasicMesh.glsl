#type vertex
#version 330

out vec2 fragTexCoord;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec3 inBinormal;
layout(location=4) in vec2 inTexCoord;

uniform mat4 uWorldTransform;
uniform mat4 uViewProj;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
	gl_Position = pos * uWorldTransform * uViewProj;
    fragTexCoord = inTexCoord;
}

#type fragment
#version 330

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D uTexture;

void main()
{
    outColor = texture(uTexture, fragTexCoord);
}
