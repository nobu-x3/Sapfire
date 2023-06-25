pub const tex = @import("texture.zig");
pub const types = @import("renderer_types.zig");
pub const pip = @import("pipeline.zig");
pub const zgpu = @import("zgpu");
pub const std = @import("std");

pub const MaterialSystem = struct {
    map: std.AutoArrayHashMap(Material, std.ArrayList(types.Mesh)),
    names: std.StringHashMap(Material),
    arena: std.heap.ArenaAllocator,
};

const DEFAULT_MESH_LIST_CAPACITY = 8;

pub fn material_system_init(allocator: std.mem.Allocator, unique_materials: u32) !MaterialSystem {
    var arena = std.heap.ArenaAllocator.init(allocator);
    var alloc = arena.allocator();
    var map = std.AutoArrayHashMap(Material, std.ArrayList(types.Mesh)).init(alloc);
    try map.ensureTotalCapacity(unique_materials);
    var names = std.StringHashMap(Material).init(alloc);
    try names.ensureTotalCapacity(unique_materials);
    return MaterialSystem{
        .map = map,
        .names = names,
        .arena = arena,
    };
}

pub fn material_system_deinit(system: *MaterialSystem) void {
    // system.names.deinit();
    // var iter = system.map.iterator();
    // while (iter.next()) |entry| {
    //     entry.value_ptr.deinit();
    // }
    // system.map.deinit();
    system.arena.deinit();
}

pub fn material_system_add_material(system: *MaterialSystem, name: [:0]const u8, gctx: *zgpu.GraphicsContext, global_uniform_bgl: zgpu.BindGroupLayoutHandle, texture_system: *tex.TextureSystem, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) !void {
    if (!system.names.contains(name)) {
        var arena = system.arena.allocator();
        var material = material_create(arena, gctx, global_uniform_bgl, texture_system, layout, uniform_size, texture_name);
        try system.names.putNoClobber(name, material);
        var meshes = try std.ArrayList(types.Mesh).initCapacity(arena, DEFAULT_MESH_LIST_CAPACITY);
        try system.map.putNoClobber(material, meshes);
    }
}

pub fn material_system_add_material_to_mesh(system: *MaterialSystem, material: *Material, mesh: types.Mesh) !void {
    var list = system.map.getPtr(material.*);
    try list.?.append(mesh);
}

pub fn material_system_add_material_to_mesh_by_name(system: *MaterialSystem, name: [:0]const u8, mesh: types.Mesh) !void {
    const material = system.names.getPtr(name).?;
    try material_system_add_material_to_mesh(system, material, mesh);
}

pub const Material = struct {
    bind_group: zgpu.BindGroupHandle,
    sampler: zgpu.SamplerHandle, // in case we need it later
};

// TODO: make bind group configurable
pub fn material_create(gctx: *zgpu.GraphicsContext, texture_system: *tex.TextureSystem, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) Material {
    // bind group
    const local_bgl = gctx.createBindGroupLayout(layout);
    defer gctx.releaseResource(local_bgl);
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
        .bind_group = local_bg,
        .sampler = sampler,
    };
    return material;
}
