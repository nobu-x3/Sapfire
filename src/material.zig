pub const tex = @import("texture.zig");
pub const pip = @import("pipeline.zig");
pub const zgpu = @import("zgpu");
pub const std = @import("std");

pub const MaterialSystem = struct {};

pub const Material = struct {
    name: [:0]const u8,
    pipeline: zgpu.RenderPipelineHandle = .{},
    bind_group: zgpu.BindGroupHandle,
    depth_texture: tex.Texture,
    sampler: zgpu.SamplerHandle, // in case we need it later
};

pub fn material_create(name: [:0]const u8, allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, global_uniform_bgl: zgpu.BindGroupLayoutHandle, texture_system: *tex.TextureSystem, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) Material {
    // bind group
    const local_bgl = gctx.createBindGroupLayout(layout);
    defer gctx.releaseResource(local_bgl);
    // Depth texture
    const depth_texture = tex.texture_depth_create(gctx);
    // Create a sampler.
    const sampler = gctx.createSampler(.{
        .address_mode_u = .repeat,
        .address_mode_v = .repeat,
        .address_mode_w = .repeat,
    });
    const local_bg = gctx.createBindGroup(local_bgl, &.{
        .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = uniform_size },
        .{ .binding = 1, .texture_view_handle = tex.texture_system_get_texture(texture_system, texture_name).view },
        .{ .binding = 2, .sampler_handle = sampler },
    });
    var material = Material{
        .name = name,
        .bind_group = local_bg,
        .depth_texture = depth_texture,
        .sampler = sampler,
    };
    pip.pipeline_create(allocator, gctx, &.{ global_uniform_bgl, local_bgl }, &material.pipeline);
    return material;
}
