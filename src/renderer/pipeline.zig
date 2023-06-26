const std = @import("std");
const zgpu = @import("zgpu");
const log = @import("../core/logger.zig");
const renderer = @import("renderer.zig");
const types = @import("renderer_types.zig");
const mat = @import("material.zig");

// zig fmt: off
const wgsl_common =
\\  struct Uniforms {
\\      aspect_ratio: f32,
\\      mip_level: f32,
\\      model: mat4x4<f32>,
\\  }
\\  struct Globals {
\\      view_proj: mat4x4<f32>,
\\  }
\\  @group(0) @binding(0) var<uniform> globalUniforms: Globals;
\\  @group(1) @binding(0) var<uniform> uniforms: Uniforms;
;
const wgsl_vs = wgsl_common ++
\\  struct VertexOut {
\\      @builtin(position) position_clip: vec4<f32>,
\\      @location(0) uv: vec2<f32>,
\\  }
\\  @vertex fn main(
\\      @location(0) position: vec3<f32>,
\\      @location(1) uv: vec2<f32>,
\\  ) -> VertexOut {
\\      //let p = vec2(position.x / uniforms.aspect_ratio, position.y);
\\      var output: VertexOut;
\\      //output.position_clip = vec4(p, 0.0, 1.0);
\\      output.position_clip = vec4(position, 1.0) * uniforms.model * globalUniforms.view_proj;
\\      output.uv = uv;
\\      return output;
\\  }
;
const wgsl_fs = wgsl_common ++
\\  @group(1) @binding(1) var image: texture_2d<f32>;
\\  @group(1) @binding(2) var image_sampler: sampler;
\\  @fragment fn main(
\\      @location(0) uv: vec2<f32>,
\\  ) -> @location(0) vec4<f32> {
\\      return textureSampleLevel(image, image_sampler, uv, uniforms.mip_level);
\\  }
// zig fmt: on
;

pub const PipelineSystem = struct {
    pipelines: std.ArrayList(Pipeline),
    arena: std.heap.ArenaAllocator,
};

pub fn pipeline_system_init(allocator: std.mem.Allocator) !PipelineSystem {
    var arena = std.heap.ArenaAllocator.init(allocator);
    var pipelines = try std.ArrayList(Pipeline).initCapacity(arena.allocator(), 8);
    return PipelineSystem{
        .arena = arena,
        .pipelines = pipelines,
    };
}

pub fn pipeline_system_deinit(system: *PipelineSystem) void {
    system.arena.deinit();
}

pub fn pipeline_system_add_pipeline(system: *PipelineSystem, gctx: *zgpu.GraphicsContext, layout: []const zgpu.BindGroupLayoutHandle, async_shader_compilation: bool) !*const Pipeline {
    // var bgls: [64]zgpu.BindGroupLayoutHandle = undefined;
    // for (0..layout.len) |index| {
    //     bgls[index] = gctx.createBindGroupLayout(layout[index]);
    // }
    // defer {
    //     for (0..layout.len) |index| {
    //         gctx.releaseResource(bgls[index]);
    //     }
    // }
    var pipeline: Pipeline = undefined;
    // pipeline_create(system.arena, gctx, bgls[0..layout.len], async_shader_compilation, &pipeline.handle);
    pipeline_create(system.arena.allocator(), gctx, layout, async_shader_compilation, &pipeline.handle);
    pipeline.materials = std.ArrayList(*const mat.Material).init(system.arena.allocator());
    try system.pipelines.append(pipeline);
    return &system.pipelines.items[system.pipelines.items.len - 1];
}

pub fn pipeline_system_add_material(system: *PipelineSystem, pipeline: *const Pipeline, material: *const mat.Material) !void {
    var found: bool = false;
    var pipeline_index: usize = 0;
    for (system.pipelines.items, 0..) |pipe, index| {
        if (pipe.handle.id == pipeline.handle.id) {
            // try pipe.materials.append(material);
            pipeline_index = index;
            found = true;
            break;
        }
    }
    if (found) {
        try system.pipelines.items[pipeline_index].materials.append(material);
        return;
    }
    log.err("Non-existent pipeline passed to pipeline_system_add_material()", .{});
}

pub const Pipeline = struct {
    handle: zgpu.RenderPipelineHandle,
    materials: std.ArrayList(*const mat.Material),
};

// TODO: make shader modules configurable
pub fn pipeline_create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, bind_group_layout: []const zgpu.BindGroupLayoutHandle, async_shader_compilation: bool, out_pipeline_handle: *zgpu.RenderPipelineHandle) void {
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
        .{ .format = .float32x2, .offset = @offsetOf(types.Vertex, "uv"), .shader_location = 1 },
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
