@group(1) @binding(1) var image: texture_2d<f32>;
@group(1) @binding(2) var image_sampler: sampler;
@group(1) @binding(3) var<uniform> phong_data: PhongData;
@fragment fn main(
  @location(0) uv: vec2<f32>,
) -> @location(0) vec4<f32> {
  return textureSampleLevel(image, image_sampler, uv, uniforms.mip_level);
}
