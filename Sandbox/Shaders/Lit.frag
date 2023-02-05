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

struct PointLight
{
	vec3 mPosition;
	vec3 mDiffuseColor;
	vec3 mSpecColor;
    float mIntensity;
	float mRadius;
};
uniform vec3 uCameraPos;
uniform vec3 uAmbientLight;
uniform float uSpecPower;
uniform DirectionalLight uDirLight;
uniform PointLight uPointLights[4];
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

     for(int i = 0; i < 4; i++)
     {
        n = normalize(fragNormal);
        l = normalize(uPointLights[i].mPosition - fragWorldPos);
        v = normalize(uCameraPos - fragWorldPos);
        r = normalize(reflect(-l, n));
        float dist = distance(uCameraPos, fragWorldPos);
        NdotL = dot(n, l);
        if(NdotL > 0)
        {
            vec3 diffuse = (uPointLights[i].mDiffuseColor*(uPointLights[i].mIntensity/dist)) * NdotL;
            vec3 specular = (uPointLights[i].mSpecColor*(uPointLights[i].mIntensity/dist)) * pow(max(0.0, dot(r, v)), uSpecPower);
            phong += diffuse + specular;
        }
     }

     outColor = texture(uTexture, fragTexCoord) * vec4(clamp(phong, 0.0, 1.0), 1.0);
}
