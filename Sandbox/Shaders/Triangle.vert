#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 uViewProjection;

out vec3 v_Position;
out vec4 v_Color;

void main()
{
	v_Position = a_Position;
	gl_Position = uViewProjection * vec4(a_Position, 1.0);
}
