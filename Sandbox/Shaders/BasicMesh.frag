#version 330

out vec4 outColor;

in vec2 fragTexCoord;

uniform sampler2D uTexture;

void main()
{
    outColor = texture(uTexture, fragTexCoord);
}
