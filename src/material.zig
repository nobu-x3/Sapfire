pub const tex = @import("texture.zig");
pub const Mesh = @import("types.zig").Mesh;
pub const pip = @import("pipeline.zig");
pub const zgpu = @import("zgpu");
pub const std = @import("std");

pub const MaterialSystem = struct {
    map: std.AutoArrayHashMap(Material, std.ArrayList(Mesh)),
    names: std.StringHashMap(Material),
    allocator: std.mem.Allocator,
};

const DEFAULT_MESH_LIST_CAPACITY = 8;

pub fn material_system_init(allocator: std.mem.Allocator, unique_materials: usize) !MaterialSystem {
    var arena = std.heap.ArenaAllocator.init(allocator);
    var map = std.AutoArrayHashMap(Material, std.ArrayList(Mesh)).initCapacity(arena.allocator());
    try map.ensureTotalCapacity(unique_materials);
    var names = std.StringHashMap(Material).init(arena.allocator());
    try names.ensureTotalCapacity(unique_materials);
    return MaterialSystem{
        .map = map,
        .names = names,
        .allocator = allocator,
    };
}

pub fn material_system_deinit(system: *MaterialSystem) void {
    system.map.deinit();
    system.names.deinit();
    system.allocator.deinit();
}

pub fn material_system_add_material(system: *MaterialSystem, name: [:0]const u8, gctx: *zgpu.GraphicsContext, global_uniform_bgl: zgpu.BindGroupLayoutHandle, texture_system: *tex.TextureSystem, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) !void {
    if (!system.names.contains(name)) {
        var material = material_create(system.allocator, name, gctx, global_uniform_bgl, texture_system, layout, uniform_size, texture_name);
        try system.names.putNoClobber(name, material);
        var meshes = std.ArrayList(Mesh).initCapacity(system.allocator, DEFAULT_MESH_LIST_CAPACITY);
        try system.map.putNoClobber(material, meshes);
    }
}

pub fn material_system_add_material_to_mesh(system: *MaterialSystem, material: *Material, mesh: Mesh) !void {
    var list = system.map.get(material.*);
    list.?.append(mesh);
}

pub fn material_system_add_material_to_mesh_by_name(system: *MaterialSystem, name: [:0]const u8, mesh: Mesh) !void {
    const material = system.names.getPtr(name).?;
    material_system_add_material_to_mesh(system, material, mesh);
}

pub const Material = struct {
    name: [:0]const u8,
    pipeline: zgpu.RenderPipelineHandle = .{},
    bind_group: zgpu.BindGroupHandle,
    depth_texture: tex.Texture,
    sampler: zgpu.SamplerHandle, // in case we need it later
};

fn material_create(allocator: std.mem.Allocator, name: [:0]const u8, gctx: *zgpu.GraphicsContext, global_uniform_bgl: zgpu.BindGroupLayoutHandle, texture_system: *tex.TextureSystem, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) Material {
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
    pip.pipeline_create(allocator, gctx, &.{ global_uniform_bgl, local_bgl }, false, &material.pipeline);
    return material;
}
