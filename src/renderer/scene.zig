const std = @import("std");
const json = std.json;
const zgpu = @import("zgpu");
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("../renderer/mesh.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("material.zig");
    usingnamespace @import("buffer.zig");
    usingnamespace @import("texture.zig");
    usingnamespace @import("pipeline.zig");
};
const log = @import("../core/logger.zig");

pub const SceneManager = struct {
    arena_allocator: std.heap.ArenaAllocator,
    asset_map: std.AutoHashMap([64]u8, SceneAsset),

    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !SceneManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse texture config file. Given path:{s}", .{config_path});
            return e;
        };
        const Config = struct {
            database: [][:0]const u8,
        };
        const config = try json.parseFromSlice(Config, arena.allocator(), config_data, .{});
        defer json.parseFree(Config, parse_arena.allocator(), config);
        var asset_map = std.AutoHashMap([64]u8, SceneAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(u32, config.database.len));
        for (config.database) |path| {
            const scene_asset = try SceneAsset.create(arena.allocator(), parse_arena.allocator(), path);
            try asset_map.putNoClobber(scene_asset.guid, scene_asset);
        }
        return SceneManager{
            .arena_allocator = arena,
            .asset_map = asset_map,
        };
    }

    pub fn deinit(self: *SceneManager) void {
        self.arena_allocator.deinit();
    }
};

pub const SimpleScene = struct {
    guid: [64]u8,
    meshes: std.ArrayList(sf.Mesh),
    vertices: std.ArrayList(sf.Vertex),
    indices: std.ArrayList(u32),
    arena: std.heap.ArenaAllocator,
    pipeline_system: sf.PipelineSystem,
    mesh_manager: sf.MeshManager,
    texture_manager: sf.TextureManager,
    material_manager: sf.MaterialManager,
    global_uniform_bind_group: zgpu.BindGroupHandle,
    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,

    pub fn create(allocator: std.mem.Allocator, config_path: [:0]const u8, gctx: *zgpu.GraphicsContext) !SimpleScene {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        const scene_asset = try SceneAsset.create(arena.allocator(), parse_arena.allocator(), config_path);
        // manager inits can be jobified
        var texman = try sf.TextureManager.init_from_slice(arena.allocator(), scene_asset.texture_paths.items);
        var matman = try sf.MaterialManager.init_from_slice(arena.allocator(), scene_asset.material_paths.items);
        var meshman = try sf.MeshManager.init_from_slice(arena.allocator(), scene_asset.geometry_paths.items);
        // Texture loading
        const global_uniform_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
        });
        defer gctx.releaseResource(global_uniform_bgl);
        for (scene_asset.texture_paths.items) |path| {
            try sf.TextureManager.add_texture(&texman, path, gctx, .{ .texture_binding = true, .copy_dst = true });
        }
        // Material loading
        const global_uniform_bg = gctx.createBindGroup(global_uniform_bgl, &.{
            .{
                .binding = 0,
                .buffer_handle = gctx.uniforms.buffer,
                .offset = 0,
                .size = @sizeOf(sf.GlobalUniforms),
            },
        });
        const local_bgl = gctx.createBindGroupLayout(
            &.{
                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
            },
        );
        defer gctx.releaseResource(local_bgl);
        var pipeline_system = try sf.PipelineSystem.init(allocator);
        var pipeline = try sf.PipelineSystem.add_pipeline(&pipeline_system, gctx, &.{ global_uniform_bgl, local_bgl }, false);
        // TODO: a module that parses material files (json or smth) and outputs bind group layouts to pass to pipeline system
        for (scene_asset.material_paths.items) |path| {
            const material_asset = matman.material_asset_map.get(sf.AssetManager.generate_guid(path)).?;
            // TODO: look into making multiple textures per material
            try sf.MaterialManager.add_material(&matman, path, gctx, &texman, &.{
                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
            }, @sizeOf(sf.Uniforms), material_asset.texture_guid.?);
            var mat0 = matman.materials.getPtr(sf.AssetManager.generate_guid(path)).?;
            try sf.PipelineSystem.add_material(&pipeline_system, pipeline, mat0);
        }

        // Mesh loading
        var meshes = std.ArrayList(sf.Mesh).init(arena.allocator());
        try meshes.ensureTotalCapacity(128);
        var vertices = std.ArrayList(sf.Vertex).init(arena.allocator());
        defer vertices.deinit();
        try vertices.ensureTotalCapacity(256);
        var indices = std.ArrayList(u32).init(arena.allocator());
        defer indices.deinit();
        try indices.ensureTotalCapacity(256);
        for (scene_asset.geometry_paths.items) |path| {
            try sf.MeshAsset.load_mesh(path, &meshman, &matman, &meshes, &vertices, &indices);
        }
        var vertex_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .vertex = true }, sf.Vertex, vertices.items);
        // Create an index buffer.
        const index_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .index = true }, u32, indices.items);

        return SimpleScene{
            .guid = scene_asset.guid,
            .meshes = meshes,
            .vertices = vertices,
            .indices = indices,
            .arena = arena,
            .pipeline_system = pipeline_system,
            .mesh_manager = meshman,
            .material_manager = matman,
            .texture_manager = texman,
            .vertex_buffer = vertex_buffer,
            .index_buffer = index_buffer,
            .global_uniform_bind_group = global_uniform_bg,
        };
    }

    pub fn destroy(scene: *SimpleScene) void {
        scene.pipeline_system.deinit();
        scene.arena.deinit();
    }
};

const SceneAsset = struct {
    guid: [64]u8,
    texture_paths: std.ArrayList([:0]const u8),
    material_paths: std.ArrayList([:0]const u8),
    geometry_paths: std.ArrayList([:0]const u8),

    fn create(database_allocator: std.mem.Allocator, parse_allocator: std.mem.Allocator, path: [:0]const u8) !SceneAsset {
        const scene_guid = sf.AssetManager.generate_guid(path);
        const config_data = std.fs.cwd().readFileAlloc(parse_allocator, path, 512 * 16) catch |e| {
            log.err("Failed to parse scene config file. Given path:{s}", .{path});
            return e;
        };
        const MeshParser = struct {
            geometry_path: [:0]const u8,
            material_path: [:0]const u8,
            texture_path: [:0]const u8,
        };
        const Config = struct {
            meshes: []const MeshParser,
        };
        const config = try json.parseFromSlice(Config, database_allocator, config_data, .{});
        var texture_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        var geometry_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        var material_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        for (config.meshes) |mesh| {
            try texture_paths.append(mesh.texture_path);
            try geometry_paths.append(mesh.geometry_path);
            try material_paths.append(mesh.material_path);
        }
        return SceneAsset{
            .guid = scene_guid,
            .texture_paths = texture_paths,
            .material_paths = material_paths,
            .geometry_paths = geometry_paths,
        };
    }

    fn destroy(self: *SceneAsset, database_allocator: std.mem.Allocator) void {
        database_allocator.free(self.texture_paths);
        database_allocator.free(self.material_paths);
        database_allocator.free(self.geometry_paths);
    }
};
