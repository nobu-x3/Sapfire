#include "common.hlsl"
#include "bindless_rs.hlsl"
#include "lighting_util.hlsl"

ConstantBuffer<PerDrawConstants> renderResources : register(b0);

struct SceneData
{
	float4x4 world;
};

struct PassData
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

struct MaterialData
{
    float4  gAlbedo;
    float3 gFresnel;
    float gRoughness;
};

struct VSOut
{
    float4 posH    : SV_POSITION;
    float3 posW    : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW: TANGENT;
    float2 uv      : UV0;
};

[RootSignature(BindlessRootSignature)]
VSOut VS(uint vertex_id: SV_VertexID)
{
    StructuredBuffer<float3> pos_buffer = ResourceDescriptorHeap[renderResources.pos_buffer_index];
    StructuredBuffer<float3> normal_buffer = ResourceDescriptorHeap[renderResources.normal_buffer_index];
    StructuredBuffer<float3> tangent_buffer = ResourceDescriptorHeap[renderResources.tangent_buffer_index];
    StructuredBuffer<float2> uv_buffer = ResourceDescriptorHeap[renderResources.uv_buffer_index];
    ConstantBuffer<SceneData> scene_data = ResourceDescriptorHeap[renderResources.scene_data_buffer_index];
    ConstantBuffer<PassData> pass_data = ResourceDescriptorHeap[renderResources.pass_data_buffer_index];

    // Transform to world space.
    float4 posW = mul(scene_data.world, float4(pos_buffer[vertex_id], 1.0));
    float2 uv = uv_buffer[vertex_id];
    VSOut output = (VSOut)0.0f;
    output.posW = posW.xyz;
    output.normalW = mul((float3x3)scene_data.world, normal_buffer[vertex_id]);
    // Transform to homogeneous clip space.
    output.posH = mul(pass_data.gViewProj, posW);
    output.uv = uv;
    return output;
}

[RootSignature(BindlessRootSignature)]
float4 PS(VSOut input) : SV_TARGET
{
    Texture2D<float4> texture = ResourceDescriptorHeap[NonUniformResourceIndex(renderResources.texture_data_buffer_index)];
    ConstantBuffer<MaterialData> material_data = ResourceDescriptorHeap[renderResources.material_data_buffer_index];
    ConstantBuffer<PassData> pass_data = ResourceDescriptorHeap[renderResources.pass_data_buffer_index];

    // Interpolating normal can unnormalize it, so renormalize it.
    input.normalW = normalize(input.normalW);

    // Vector from point being lit to eye.
    float3 toEyeW = normalize(pass_data.gEyePosW - input.posW);

	// Indirect lighting.
    float4 ambient = pass_data.gAmbientLight * material_data.gAlbedo;
    const float shininess = 1.0f - material_data.gRoughness;
    Material mat = { material_data.gAlbedo, material_data.gFresnel, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(pass_data.gLights, mat, input.posW,
        input.normalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = material_data.gAlbedo.a;

    return litColor * texture.Sample(pointClampSampler, input.uv);
}
