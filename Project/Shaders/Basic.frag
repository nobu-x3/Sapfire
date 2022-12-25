#version 330

out vec4 outColor;

void main()
{
    outColor = texture(uTexture, fragTexCoord);
}
