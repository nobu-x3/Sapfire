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
	// Surface normal
	vec3 N = normalize(fragNormal);
	// Vector from surface to light
	vec3 L = normalize(-uDirLight.mDirection);
	// Vector from surface to camera
	vec3 V = normalize(uCameraPos - fragWorldPos);
	// Reflection of -L about N
	vec3 R = normalize(reflect(-L, N));

	// Compute phong reflection
	vec3 Phong = uAmbientLight;
	float NdotL = dot(N, L);
	if (NdotL > 0)
	{
		vec3 Diffuse = uDirLight.mDiffuseColor * NdotL;
		vec3 Specular = uDirLight.mSpecColor * pow(max(0.0, dot(R, V)), uSpecPower);
		Phong += Diffuse + Specular;
	}

	// Final color is texture color times phong light (alpha = 1)
    outColor = texture(uTexture, fragTexCoord) * vec4(Phong, 1.0f);
    /* vec3 n = normalize(fragNormal);
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
     outColor = texture(uTexture, fragTexCoord) * vec4(phong, 1.0);*/
}
