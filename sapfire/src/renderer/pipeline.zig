const std = @import("std");
const zgpu = @import("zgpu");
const core = @import("../core.zig");
const log = core.log;
const renderer = @import("renderer.zig");
const types = @import("renderer_types.zig");
const mat = @import("material.zig");

pub const wgsl_common = @embedFile("../default_shaders/wgsl_common.wgsl");
pub const wgsl_vs = wgsl_common ++ @embedFile("../default_shaders/wgsl_vs.wgsl");
pub const wgsl_fs = wgsl_common ++ @embedFile("../default_shaders/wgsl_fs.wgsl");
pub const vertexWriteGBuffers = wgsl_common ++ @embedFile("../default_shaders/vertex_write_gbuffers.wgsl");
pub const fragmentWriteGBuffers = wgsl_common ++ @embedFile("../default_shaders/fragment_write_gbuffers.wgsl");
pub const vertexTextureQuad = @embedFile("../default_shaders/vertex_texture_quad.wgsl");
pub const fragmentDeferredRendering = @embedFile("../default_shaders/fragment_deferred_rendering.wgsl");

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
