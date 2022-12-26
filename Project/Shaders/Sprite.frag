#version 330

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D uTexture;
void main()
{
    outColor = texture(uTexture, fragTexCoord);
    //outColor = vec4(0.0, 0.0, 1.0, 1.0);
}