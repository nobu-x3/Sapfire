#type vertex
#version 440 core

out vec2 fragTexCoord;
// in world space
out vec3 fragNormal;
out vec3 fragWorldPos;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTangent;
layout(location=3) in vec3 inBinormal;
layout(location=4) in vec2 inTexCoord;

layout(std140, row_major, binding = 0) uniform Matrices
{
    mat4 View;
    mat4 Proj;
} matrices;

uniform mat4 uWorldTransform;

void main()
{
	vec4 pos = vec4(inPosition, 1.0);
    // to world space
	pos = pos * uWorldTransform;
	fragWorldPos = pos.xyz;
    // to clip space
	gl_Position = pos * matrices.Projection * matrices.View;
	fragNormal = inNormal;
    fragTexCoord = inTexCoord;
}
#type fragment
#version 440 core

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
