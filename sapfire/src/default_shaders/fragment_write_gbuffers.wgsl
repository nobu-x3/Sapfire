struct GBufferOutput {
   @location(0) normal: vec4<f32>,
   @location(1) albedo: vec4<f32>,
   @location(2) phong_data: vec4<f32>,
}
@group(2) @binding(0) var image: texture_2d<f32>;
@group(2) @binding(1) var image_sampler: sampler;
@group(2) @binding(2) var<uniform> phong_data: PhongData;
@fragment fn main(
  @location(0) fragNormal: vec3<f32>,
  @location(1) fragTangent: vec4<f32>,
  @location(2) fragUv: vec2<f32>,
) -> GBufferOutput {
  var output : GBufferOutput;
  output.normal = vec4<f32>(fragNormal, 1.0);
  output.albedo = textureSampleLevel(image, image_sampler, fragUv, uniforms.mip_level);
  output.phong_data = vec4<f32>(phong_data.ambient,phong_data.diffuse,phong_data.specular, 1.0);
  return output;
}
