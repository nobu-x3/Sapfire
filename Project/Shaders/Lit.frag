#version 330

out vec4 outColor;
// in world space
in vec3 fragNormal;
in vec3 fragWorldPos;
in vec2 fragTexCoord;

struct DirectionalLight
{
    vec3 mDirection;
    vec3 mDiffuseColor;
    vec3 mSpecColor;
};
uniform vec3 uCameraPos;
uniform vec3 uAmbientLight;
uniform float uSpecPower;
uniform DirectionalLight uDirLight;
uniform sampler2D uTexture;

void main()
{
    vec3 n = normalize(fragNormal);
     vec3 l = normalize(-uDirLight.mDirection);
     vec3 v = normalize(uCameraPos - fragWorldPos);
     vec3 r = normalize(reflect(-l, n));
     vec3 phong = uAmbientLight;
     float NdotL = dot(n, l);
     if(NdotL > 0)
     {
         vec3 diffuse = uDirLight.mDiffuseColor * NdotL;
         // max here to prevent spec component from being negative which removes the light from the scene
         vec3 specular = uDirLight.mSpecColor * pow(max(0.0, dot(r, v)), uSpecPower);
         phong += diffuse + specular;
     }
     outColor = texture(uTexture, fragTexCoord) * vec4(clamp(phong, 0.0, 1.0), 1.0);
}
