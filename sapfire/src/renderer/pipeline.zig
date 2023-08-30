const std = @import("std");
const zgpu = @import("zgpu");
const core = @import("../core.zig");
const log = core.log;
const renderer = @import("renderer.zig");
const types = @import("renderer_types.zig");
const mat = @import("material.zig");

// zig fmt: off
pub const wgsl_common =
\\  struct Uniforms {
\\      aspect_ratio: f32,
\\      mip_level: f32,
\\      model: mat4x4<f32>,
\\  }
\\  struct Globals {
\\      view_proj: mat4x4<f32>,
\\  }
\\  struct Lights {
\\      position: vec3<f32>,
\\      color: vec3<f32>,
\\  }
\\  @group(0) @binding(0) var<uniform> globalUniforms: Globals;
\\  @group(1) @binding(0) var<uniform> uniforms: Uniforms;
;

pub const wgsl_vs = wgsl_common ++
\\  struct VertexOut {
\\      @builtin(position) position_clip: vec4<f32>,
\\      @location(0) uv: vec2<f32>,
\\  }
\\  @vertex fn main(
\\      @location(0) position: vec3<f32>,
\\      @location(1) normal: vec3<f32>,
\\      @location(2) tangent: vec4<f32>,
\\      @location(3) uv: vec2<f32>,
\\  ) -> VertexOut {
\\      //let p = vec2(position.x / uniforms.aspect_ratio, position.y);
\\      var output: VertexOut;
\\      //output.position_clip = vec4(p, 0.0, 1.0);
\\      output.position_clip = vec4(position, 1.0) * uniforms.model * globalUniforms.view_proj;
\\      output.uv = uv;
\\      return output;
\\  }
;

pub const wgsl_fs = wgsl_common ++
\\  struct PhongData {
\\       ambient: f32,
\\       diffuse: f32,
\\       reflection: f32,
\\  }
\\  @group(1) @binding(1) var image: texture_2d<f32>;
\\  @group(1) @binding(2) var image_sampler: sampler;
\\  @group(1) @binding(3) var<uniform> phong_data: PhongData;
\\  @fragment fn main(
\\      @location(0) uv: vec2<f32>,
\\  ) -> @location(0) vec4<f32> {
\\      return textureSampleLevel(image, image_sampler, uv, uniforms.mip_level);
\\  }
;

pub const vertexWriteGBuffers = wgsl_common ++
\\  struct VertexOut {
\\      @builtin(position) position_clip: vec4<f32>,
\\      @location(0) normal: vec3<f32>,
\\      @location(1) tangent: vec4<f32>,
\\      @location(2) uv: vec2<f32>,
\\  }
\\  @vertex fn main(
\\      @location(0) position: vec3<f32>,
\\      @location(1) normal: vec3<f32>,
\\      @location(2) tangent: vec4<f32>,
\\      @location(3) uv: vec2<f32>,
\\  ) -> VertexOut {
\\      //let p = vec2(position.x / uniforms.aspect_ratio, position.y);
\\      var output: VertexOut;
\\      //output.position_clip = vec4(p, 0.0, 1.0);
\\      output.position_clip = vec4(position, 1.0) * uniforms.model * globalUniforms.view_proj;
\\      output.uv = uv;
\\      output.tangent = tangent;
\\      output.normal = normal; // TODO: fix this
\\      return output;
\\  }
;

pub const fragmentWriteGBuffers = 
\\  struct PhongData {
\\       ambient: f32,
\\       diffuse: f32,
\\       reflection: f32,
\\  }
\\  struct GBufferOutput {
\\       @location(0) normal: vec4<f32>,
\\       @location(1) albedo: vec4<f32>,
\\  }
\\  @group(1) @binding(1) var image: texture_2d<f32>;
\\  @group(1) @binding(2) var image_sampler: sampler;
\\  @group(1) @binding(3) var<uniform> phong_data: PhongData;
\\  @fragment fn main(
\\      @location(0) fragNormal: vec3<f32>,
\\      @location(1) fragTangent: vec4<f32>,
\\      @location(2) fragUv: vec2<f32>,
\\  ) -> GBufferOutput {
\\      let uv = floor(30.0 * fragUv);
\\      let c = 0.2 + 0.5 * ((uv.x + uv.y) - 2.0 * floor((uv.x + uv.y) / 2.0));
\\      var output : GBufferOutput;
\\      output.normal = vec4<f32>(fragNormal, 1.0);
\\      output.albedo = vec4<f32>(c, c, c, 1.0);
\\      return output;
\\  }
;
pub const vertexTextureQuad = wgsl_common ++
\\  @vertex fn main(
\\      @builtin(vertex_index) VertexIndex: u32,
\\  ) -> @builtin(position) vec4<f32> {
\\      const pos = array(vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),
\\      vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));
\\      return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
\\  }
;

pub const fragmentDeferredRendering = 
\\@group(0) @binding(0) var gBufferNormal: texture_2d<f32>;
\\@group(0) @binding(1) var gBufferAlbedo: texture_2d<f32>;
\\@group(0) @binding(2) var gBufferDepth: texture_depth_2d;
\\struct LightData {
\\  position : vec3<f32>,
\\  color : vec3<f32>,
\\}
\\struct Camera {
\\  viewProjectionMatrix : mat4x4<f32>,
\\  invViewProjectionMatrix : mat4x4<f32>,
\\}
\\@group(1) @binding(0) var<uniform> light: LightData;
\\@group(2) @binding(0) var<uniform> camera: Camera;
\\fn world_from_screen_coord(coord : vec2<f32>, depth_sample: f32) -> vec3<f32> {
\\  // reconstruct world-space position from the screen coordinate.
\\  let posClip = vec4(coord.x * 2.0 - 1.0, (1.0 - coord.y) * 2.0 - 1.0, depth_sample, 1.0);
\\  let posWorldW = camera.invViewProjectionMatrix * posClip;
\\  let posWorld = posWorldW.xyz / posWorldW.www;
\\  return posWorld;
\\}
\\@fragment
\\fn main(
\\  @builtin(position) coord : vec4<f32>
\\) -> @location(0) vec4<f32> {
\\  var result : vec3<f32>;
\\  let depth = textureLoad(
\\  gBufferDepth,
\\  vec2<i32>(floor(coord.xy)),
\\  0
\\  );
\\  // Don't light the sky.
\\  if (depth >= 1.0) {
\\      discard;
\\  }
\\  let bufferSize = textureDimensions(gBufferDepth);
\\  let coordUV = coord.xy / vec2<f32>(bufferSize);
\\  let position = world_from_screen_coord(coordUV, depth);
\\  let normal = textureLoad(
\\      gBufferNormal,
\\      vec2<i32>(floor(coord.xy)),
\\      0
\\  ).xyz;
\\  let albedo = textureLoad(
\\      gBufferAlbedo,
\\      vec2<i32>(floor(coord.xy)),
\\      0
\\  ).rgb;
\\  // some manual ambient
\\  result += vec3(0.2);
\\  return vec4(result, 1.0);
\\}
// zig fmt: on
;

pub const PipelineSystem = struct {
    pipelines: std.ArrayList(Pipeline),
    material_pipeline_map: std.AutoHashMap([64]u8, Pipeline),
    arena: std.heap.ArenaAllocator,

    pub fn init(allocator: std.mem.Allocator) !PipelineSystem {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var material_pipeline_map = std.AutoHashMap([64]u8, Pipeline).init(allocator);
        try material_pipeline_map.ensureTotalCapacity(256);
        var pipelines = std.ArrayList(Pipeline).init(allocator);
        return PipelineSystem{
            .material_pipeline_map = material_pipeline_map,
            .arena = arena,
            .pipelines = pipelines,
        };
    }

    pub fn deinit(system: *PipelineSystem) void {
        system.arena.deinit();
    }

    pub fn add_pipeline(system: *PipelineSystem, gctx: *zgpu.GraphicsContext, layout: []const zgpu.BindGroupLayoutHandle, async_shader_compilation: bool, out_pipeline: *zgpu.RenderPipelineHandle) !void {
        Pipeline.create(system.arena.allocator(), gctx, layout, async_shader_compilation, out_pipeline);
        try system.pipelines.append(.{ .handle = out_pipeline.* });
    }

    pub fn add_material(system: *PipelineSystem, pipeline: *Pipeline, guid: [64]u8) !void {
        if (system.material_pipeline_map.contains(guid)) return;
        try system.material_pipeline_map.put(guid, pipeline.*);
    }
};

pub const Pipeline = struct {
    handle: zgpu.RenderPipelineHandle = .{},

    // TODO: make shader modules configurable
    pub fn create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, bind_group_layout: []const zgpu.BindGroupLayoutHandle, async_shader_compilation: bool, out_pipeline_handle: *zgpu.RenderPipelineHandle) void {
        const pipeline_layout = gctx.createPipelineLayout(bind_group_layout);
        defer gctx.releaseResource(pipeline_layout);
        const vs_module = zgpu.createWgslShaderModule(gctx.device, wgsl_vs, "vs");
        defer vs_module.release();
        const fs_module = zgpu.createWgslShaderModule(gctx.device, wgsl_fs, "fs");
        defer fs_module.release();
        const color_targets = [_]zgpu.wgpu.ColorTargetState{.{
            .format = zgpu.GraphicsContext.swapchain_format,
        }};
        const vertex_attributes = [_]zgpu.wgpu.VertexAttribute{
            .{ .format = .float32x3, .offset = 0, .shader_location = 0 },
            .{ .format = .float32x3, .offset = @offsetOf(types.Vertex, "normal"), .shader_location = 1 },
            .{ .format = .float32x4, .offset = @offsetOf(types.Vertex, "tangent"), .shader_location = 2 },
            .{ .format = .float32x2, .offset = @offsetOf(types.Vertex, "uv"), .shader_location = 3 },
        };
        const vertex_buffers = [_]zgpu.wgpu.VertexBufferLayout{.{
            .array_stride = @sizeOf(types.Vertex),
            .attribute_count = vertex_attributes.len,
            .attributes = &vertex_attributes,
        }};
        // Create a render pipeline.
        const pipeline_descriptor = zgpu.wgpu.RenderPipelineDescriptor{
            .vertex = .{
                .module = vs_module,
                .entry_point = "main",
                .buffer_count = vertex_buffers.len,
                .buffers = &vertex_buffers,
            },
            .primitive = .{
                .front_face = .cw,
                .cull_mode = .back,
                .topology = .triangle_list,
            },
            .depth_stencil = &.{
                .format = .depth32_float,
                .depth_write_enabled = true,
                .depth_compare = .less,
            },
            .fragment = &.{
                .module = fs_module,
                .entry_point = "main",
                .target_count = color_targets.len,
                .targets = &color_targets,
            },
        };
        if (async_shader_compilation) {
            gctx.createRenderPipelineAsync(allocator, pipeline_layout, pipeline_descriptor, out_pipeline_handle);
        } else {
            out_pipeline_handle.* = gctx.createRenderPipeline(pipeline_layout, pipeline_descriptor);
        }
    }
};
