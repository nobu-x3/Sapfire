@group(0) @binding(0) var gBufferNormal: texture_2d<f32>;
@group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;
@group(0) @binding(2) var gBufferDepth: texture_depth_2d;
@group(0) @binding(3) var gBufferPhong: texture_2d<f32>;
struct LightData {
  position : vec3<f32>,
  color : vec3<f32>,
}
struct Camera {
  viewProjectionMatrix : mat4x4<f32>,
  invViewProjectionMatrix : mat4x4<f32>,
}
@group(1) @binding(0) var<uniform> light: LightData;
@group(2) @binding(0) var<uniform> camera: Camera;
fn world_from_screen_coord(coord : vec2<f32>, depth_sample: f32) -> vec3<f32> {
  // reconstruct world-space position from the screen coordinate.
  let posClip = vec4(coord.x * 2.0 - 1.0, (1.0 - coord.y) * 2.0 - 1.0, depth_sample, 1.0);
  let posWorldW = camera.invViewProjectionMatrix * posClip;
  let posWorld = posWorldW.xyz / posWorldW.www;
  return posWorld;
}
@fragment
fn main(
  @builtin(position) coord : vec4<f32>
) -> @location(0) vec4<f32> {
  var result : vec3<f32>;
  let depth = textureLoad(
  gBufferDepth,
  vec2<i32>(floor(coord.xy)),
  0
  );
  // Don't light the sky.
  if (depth >= 1.0) {
      discard;
  }
  let bufferSize = textureDimensions(gBufferDepth);
  let coordUV = coord.xy / vec2<f32>(bufferSize);
  let position = world_from_screen_coord(coordUV, depth);
  let normal = textureLoad(
      gBufferNormal,
      vec2<i32>(floor(coord.xy)),
      0
  ).xyz;
  let albedo = textureLoad(
      gBufferAlbedo,
      vec2<i32>(floor(coord.xy)),
      0
  ).rgb;
  let phong_data = textureLoad(
      gBufferPhong,
      vec2<i32>(floor(coord.xy)),
      0
  ).rgb;
  // diffuse
  let light_dir = normalize(light.position - position);
  let diffuse_strength = phong_data.y * max(dot(normal, light_dir), 0.0);
  let diffuse_color = light.color * diffuse_strength;
  // specular
  let camera_position = camera.viewProjectionMatrix[3];
  let view_dir = normalize(camera_position.xyz - position);
  let reflect_dir = reflect(-light_dir, normal);
  let spec_strength = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
  let spec_color = spec_strength * light.color;
  result = albedo * (phong_data.x + diffuse_color + spec_color) * light.color ;
  return vec4(result, 1.0);
}

