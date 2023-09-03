struct VertexOut {
  @builtin(position) position_clip: vec4<f32>,
  @location(0) normal: vec3<f32>,
  @location(1) tangent: vec4<f32>,
  @location(2) uv: vec2<f32>,
}
@vertex fn main(
  @location(0) position: vec3<f32>,
  @location(1) normal: vec3<f32>,
  @location(2) tangent: vec4<f32>,
  @location(3) uv: vec2<f32>,
) -> VertexOut {
  var output: VertexOut;
  let p = vec2(position.x / uniforms.aspect_ratio, position.y);
  output.position_clip = vec4(p, 0.0, 1.0) * uniforms.model * globalUniforms.view_proj;
  output.uv = uv;
  output.tangent = tangent;
  output.normal = normal; // TODO: fix this
  return output;
}

