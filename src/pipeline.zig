const std = @import("std");
const zgpu = @import("zgpu");
const root = @import("root");

// zig fmt: off
const wgsl_common =
\\  struct Uniforms {
\\      aspect_ratio: f32,
\\      mip_level: f32,
\\  }
\\  @group(0) @binding(0) var<uniform> uniforms: Uniforms;
;
const wgsl_vs = wgsl_common ++
\\  struct VertexOut {
\\      @builtin(position) position_clip: vec4<f32>,
\\      @location(0) uv: vec2<f32>,
\\  }
\\  @stage(vertex) fn main(
\\      @location(0) position: vec2<f32>,
\\      @location(1) uv: vec2<f32>,
\\  ) -> VertexOut {
\\      let p = vec2(position.x / uniforms.aspect_ratio, position.y);
\\      var output: VertexOut;
\\      output.position_clip = vec4(p, 0.0, 1.0);
\\      output.uv = uv;
\\      return output;
\\  }
;
const wgsl_fs = wgsl_common ++
\\  @group(0) @binding(1) var image: texture_2d<f32>;
\\  @group(0) @binding(2) var image_sampler: sampler;
\\  @stage(fragment) fn main(
\\      @location(0) uv: vec2<f32>,
\\  ) -> @location(0) vec4<f32> {
\\      return textureSampleLevel(image, image_sampler, uv, uniforms.mip_level);
\\  }
// zig fmt: on
;

pub fn pipeline_create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, bind_group_layout: []const zgpu.BindGroupLayoutHandle, out_pipeline_handle: *zgpu.RenderPipelineHandle) void {
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
        .{ .format = .float32x2, .offset = 0, .shader_location = 0 },
        .{ .format = .float32x2, .offset = @offsetOf(root.Vertex, "uv"), .shader_location = 1 },
    };
    const vertex_buffers = [_]zgpu.wgpu.VertexBufferLayout{.{
        .array_stride = @sizeOf(root.Vertex),
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
        .fragment = &.{
            .module = fs_module,
            .entry_point = "main",
            .target_count = color_targets.len,
            .targets = &color_targets,
        },
    };
    gctx.createRenderPipelineAsync(allocator, pipeline_layout, pipeline_descriptor, out_pipeline_handle);
}
