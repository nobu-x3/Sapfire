const std = @import("std");
const zgpu = @import("zgpu");
const stbi = @import("zstbi");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core.zig");
};
const log = sf.log;

const INVALID_ID = 4294967295;

pub const TextureFormat = struct {
    components_count: u32,
    components_width: u32,
    bytes_per_row: u32,
    is_hdr: bool,
};

pub const Texture = struct {
    // guid: [64]u8,
    handle: zgpu.TextureHandle,
    view: zgpu.TextureViewHandle,

    pub fn create(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage, size: zgpu.wgpu.Extent3D, format: TextureFormat) Texture {
        const handle = gctx.createTexture(.{
            .usage = usage,
            .size = size,
            .format = zgpu.imageInfoToTextureFormat(
                format.components_count,
                format.components_width,
                format.is_hdr,
            ),
            .mip_level_count = std.math.log2_int(u32, @max(size.width, size.height)) + 1,
        });
        const texture_view = gctx.createTextureView(handle, .{});
        return Texture{ .handle = handle, .view = texture_view };
    }

    pub fn create_with_wgpu_format(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage, size: zgpu.wgpu.Extent3D, format: zgpu.wgpu.TextureFormat) Texture {
        const handle = gctx.createTexture(.{
            .usage = usage,
            .size = size,
            .format = format,
            .mip_level_count = 1,
        });
        const texture_view = gctx.createTextureView(handle, .{});
        return Texture{ .handle = handle, .view = texture_view };
    }

    pub fn generate_default(gctx: *zgpu.GraphicsContext, texture_manager: *TextureManager) !void {
        // Generate default texture
        @setEvalBranchQuota(256 * 256 * 4);
        const dimensions: u32 = 256;
        const channels: u32 = 4;
        const byte_count: u32 = dimensions * dimensions * channels;
        var pixels = [_]u8{255} ** byte_count;
        inline for (0..dimensions) |row| {
            inline for (0..dimensions) |col| {
                const index = ((row * dimensions) + col) * channels;
                if (row % 2 != 0) {
                    if (col % 2 != 0) {
                        pixels[index + 1] = 0;
                    } else {
                        pixels[index] = 0;
                        pixels[index + 1] = 0;
                        pixels[index + 2] = 0;
                    }
                } else {
                    if (col % 2 == 0) {
                        pixels[index + 1] = 0;
                    } else {
                        pixels[index] = 0;
                        pixels[index + 1] = 0;
                        pixels[index + 2] = 0;
                    }
                }
            }
        }
        // loadFromMemory segfaults, so don't use this.
        // var image = stbi.Image.loadFromMemory(&pixels, channels) catch |e| {
        //     std.log.err("Failed to load default texture.", .{});
        //     return e;
        // };
        texture_manager.default_texture = Texture.create(gctx, .{ .texture_binding = true, .copy_dst = true }, .{
            .width = 256,
            .height = 256,
            .depth_or_array_layers = 1,
        }, .{
            .components_count = 4,
            .components_width = 1,
            .is_hdr = false,
            .bytes_per_row = dimensions,
        });
        Texture.load_data(&texture_manager.default_texture.?, gctx, dimensions, dimensions, dimensions * channels, pixels[0..]);
    }

    pub fn create_depth(gctx: *zgpu.GraphicsContext, width: u32, height: u32) Texture {
        const texture = gctx.createTexture(.{
            .usage = .{ .render_attachment = true },
            .dimension = .tdim_2d,
            .size = .{
                .width = width,
                .height = height,
                .depth_or_array_layers = 1,
            },
            .format = .depth32_float,
            .mip_level_count = 1,
            .sample_count = 1,
        });
        const view = gctx.createTextureView(texture, .{});
        return Texture{ .handle = texture, .view = view };
    }

    fn load_data(texture: *Texture, gctx: *zgpu.GraphicsContext, width: u32, height: u32, row_width: u32, data: []const u8) void {
        var res = gctx.lookupResource(texture.handle).?;
        gctx.queue.writeTexture(
            .{ .texture = res },
            .{
                .bytes_per_row = row_width,
                .rows_per_image = height,
            },
            .{ .width = width, .height = height },
            u8,
            data,
        );
    }
};

pub const TextureAsset = struct {
    guid: [64]u8,
    path: [:0]const u8,
    data: []u8,
    parse_success: bool,
    width: u32,
    height: u32,
    format: TextureFormat,
};

const Config = struct {
    database: [][:0]const u8,
};

pub const TextureManager = struct {
    map: std.AutoHashMap([64]u8, Texture),
    arena: std.heap.ArenaAllocator,
    default_texture: ?Texture = null,
    texture_assets_map: std.AutoHashMap([64]u8, TextureAsset),
    parse_arena: std.heap.ArenaAllocator,
    path_database: std.ArrayList([:0]const u8),

    const INIT_TEXTURE_MAP_SIZE = 16;

    pub fn init_empty(allocator: std.mem.Allocator) !TextureManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var map = std.AutoHashMap([64]u8, Texture).init(arena_alloc);
        try map.ensureTotalCapacity(256);
        var asset_map = std.AutoHashMap([64]u8, TextureAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(INIT_TEXTURE_MAP_SIZE);
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        return TextureManager{
            .arena = arena,
            .map = map,
            .texture_assets_map = asset_map,
            .parse_arena = parse_arena,
            .path_database = path_database,
        };
    }

    // Config loads assets to texture_assets_map, renderer will load it from
    // texture_assets_map using guid and then make image & view
    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !TextureManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var map = std.AutoHashMap([64]u8, Texture).init(arena_alloc);
        try map.ensureTotalCapacity(256);
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse texture config file. Given path:{s}", .{config_path});
            return e;
        };
        const config = try json.parseFromSliceLeaky(Config, arena.allocator(), config_data, .{});
        for (config.database) |path| {
            try path_database.append(path);
        }
        var asset_map = std.AutoHashMap([64]u8, TextureAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(config.database.len));
        try parse_pngs(arena_alloc, config.database, &asset_map);
        return TextureManager{
            .arena = arena,
            .map = map,
            .texture_assets_map = asset_map,
            .parse_arena = parse_arena,
            .path_database = path_database,
        };
    }

    pub fn init_from_slice(allocator: std.mem.Allocator, paths: [][:0]const u8) !TextureManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var map = std.AutoHashMap([64]u8, Texture).init(arena_alloc);
        try map.ensureTotalCapacity(256);
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        for (paths) |path| {
            try path_database.append(path);
        }
        var asset_map = std.AutoHashMap([64]u8, TextureAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(paths.len));
        try parse_pngs(arena_alloc, paths, &asset_map);
        return TextureManager{
            .arena = arena,
            .map = map,
            .texture_assets_map = asset_map,
            .parse_arena = parse_arena,
            .path_database = path_database,
        };
    }

    pub fn deinit(system: *TextureManager) void {
        // system.map.deinit();
        system.path_database.deinit();
        system.parse_arena.deinit();
        system.arena.deinit();
    }

    pub fn add_texture(system: *TextureManager, pathname: [:0]const u8, gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage) !void {
        var texture: Texture = undefined;
        const guid = sf.AssetManager.generate_guid(pathname);
        if (system.default_texture == null) {
            try Texture.generate_default(gctx, system);
        }
        if (!system.texture_assets_map.contains(guid)) {
            log.err("Texture at path {s} is not present in the asset database. Using default texture.", .{pathname});
            texture = system.default_texture.?;
            return;
        }
        var image = system.texture_assets_map.getPtr(guid).?;
        if (!image.parse_success) {
            log.err("Texture at path {s} encountered an error when adding to the asset database and cannot be used until imported properly. Using default texture.", .{pathname});
            texture = system.default_texture.?;
            return;
        }
        texture = Texture.create(gctx, usage, .{
            .width = image.width,
            .height = image.height,
            .depth_or_array_layers = 1,
        }, image.format);
        Texture.load_data(&texture, gctx, image.width, image.height, image.format.bytes_per_row, image.data);
        // TODO: rework this to use guids
        try system.map.put(guid, texture);
    }

    pub fn get_texture_by_name(system: *TextureManager, name: [:0]const u8) Texture {
        const guid = sf.AssetManager.generate_guid(name);
        return system.map.get(guid) orelse system.default_texture.?;
    }

    pub fn get_texture(system: *TextureManager, guid: [64]u8) Texture {
        return system.map.get(guid) orelse system.default_texture.?;
    }

    // TODO: pass an allocator instead of caching parse_arena
    pub fn import_texture_asset(self: *TextureManager, path: [:0]const u8) !void {
        stbi.init(self.parse_arena.allocator());
        defer stbi.deinit();
        var split = std.mem.splitAny(u8, path, "/");
        var name = split.first();
        while (split.next() != null) {
            if (split.peek() != null and std.mem.eql(u8, split.peek().?, "assets")) {
                name = split.buffer[split.index.?..];
                break;
            }
        }
        const texture_json_path = try std.mem.concatWithSentinel(self.parse_arena.allocator(), u8, &.{name}, 0);
        const guid = sf.AssetManager.generate_guid(texture_json_path);
        if (self.texture_assets_map.contains(guid)) return;
        var path_cpy = try self.parse_arena.allocator().allocSentinel(u8, texture_json_path.len, 0);
        @memcpy(path_cpy, texture_json_path);
        var image = stbi.Image.loadFromFile(path, 4) catch {
            log.err("Error loading texture from path {s}.", .{path});
            const asset: TextureAsset = .{
                .guid = guid,
                .path = path_cpy,
                .data = undefined,
                .parse_success = false,
                .width = 0,
                .height = 0,
                .format = .{
                    .components_count = 0,
                    .components_width = 0,
                    .bytes_per_row = 0,
                    .is_hdr = false,
                },
            };
            try self.texture_assets_map.put(guid, asset);
            return;
        };
        defer image.deinit();
        // WARNING: if not using arena allocator do not forget to clean up
        var data: []u8 = try self.parse_arena.allocator().alloc(u8, image.data.len);
        @memcpy(data, image.data);
        const asset: TextureAsset = .{
            .guid = guid,
            .path = texture_json_path,
            .data = data,
            .parse_success = true,
            .width = image.width,
            .height = image.height,
            .format = .{
                .components_count = image.num_components,
                .components_width = image.bytes_per_component,
                .bytes_per_row = image.bytes_per_row,
                .is_hdr = image.is_hdr,
            },
        };
        try self.texture_assets_map.put(guid, asset);
        try self.path_database.append(texture_json_path);
        { // serialize mesh_config.json
            var file = try std.fs.cwd().createFile("project/texture_config.json", .{});
            defer file.close();
            var writer = file.writer();
            try json.stringify(Config{ .database = self.path_database.items }, .{}, writer);
        }
        log.info("Adding texture at path {s} with guid \n{d} to database.", .{ texture_json_path, guid });
    }
};

// TODO: temp function
fn parse_pngs(allocator: std.mem.Allocator, paths: [][:0]const u8, out_map: *std.AutoHashMap([64]u8, TextureAsset)) !void {
    var arena = std.heap.ArenaAllocator.init(allocator);
    defer arena.deinit();
    stbi.init(arena.allocator());
    defer stbi.deinit();
    for (paths) |path| {
        const guid = sf.AssetManager.generate_guid(path);
        if (out_map.contains(guid)) continue;
        var path_cpy = try allocator.allocSentinel(u8, path.len, 0);
        @memcpy(path_cpy, path);
        var image = stbi.Image.loadFromFile(path, 4) catch {
            log.err("Error loading texture from path {s}.", .{path});
            const asset: TextureAsset = .{
                .guid = guid,
                .path = path,
                .data = undefined,
                .parse_success = false,
                .width = 0,
                .height = 0,
                .format = .{
                    .components_count = 0,
                    .components_width = 0,
                    .bytes_per_row = 0,
                    .is_hdr = false,
                },
            };
            try out_map.putNoClobber(guid, asset);
            continue;
        };
        defer image.deinit();

        // WARNING: if not using arena allocator do not forget to clean up
        var data: []u8 = try allocator.alloc(u8, image.data.len); // idk but this was damn hard to find
        @memcpy(data, image.data);
        const asset: TextureAsset = .{
            .guid = guid,
            .path = path,
            .data = data,
            .parse_success = true,
            .width = image.width,
            .height = image.height,
            .format = .{
                .components_count = image.num_components,
                .components_width = image.bytes_per_component,
                .bytes_per_row = image.bytes_per_row,
                .is_hdr = image.is_hdr,
            },
        };
        try out_map.putNoClobber(guid, asset);
        log.info("Adding texture at path {s} with guid \n{d} to database.", .{ path, guid });
    }
}
