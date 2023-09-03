struct Uniforms {
  aspect_ratio: f32,
  mip_level: f32,
  model: mat4x4<f32>,
}
struct Globals {
  view_proj: mat4x4<f32>,
  inv_view_proj: mat4x4<f32>,
}
struct Lights {
  position: vec3<f32>,
  color: vec3<f32>,
}
struct PhongData {
   ambient: f32,
   diffuse: f32,
   reflection: f32,
}
@group(0) @binding(0) var<uniform> globalUniforms: Globals;
@group(1) @binding(0) var<uniform> uniforms: Uniforms;
