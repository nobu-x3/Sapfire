#version 330

out vec4 outColor;

in vec2 fragTexCoord;

// uniform sampler2D uTexture;

void main()
{
    // outColor = texture(uTexture, fragTexCoord);
    outColor = vec4(fragTexCoord, 0.0, 1.0);
}
