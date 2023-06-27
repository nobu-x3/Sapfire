const std = @import("std");
const zgpu = @import("zgpu");
const stbi = @import("zstbi");
const log = @import("../core/logger.zig");
const json = std.json;
const asset_manager = @import("../core/asset_manager.zig");

const INVALID_ID = 4294967295;

pub const TextureFormat = struct {
    components_count: u32,
    components_width: u32,
    is_hdr: bool,
};

pub const Texture = struct {
    // guid: [64]u8,
    handle: zgpu.TextureHandle,
    view: zgpu.TextureViewHandle,
};

pub const TextureAsset = struct {
    guid: [64]u8,
    data: []const u8,
    parse_success: bool,
};

pub const TextureManager = struct {
    map: std.StringHashMap(Texture),
    arena: std.heap.ArenaAllocator,
    default_texture: ?Texture = null,
    texture_assets_map: std.StringHashMap(TextureAsset),
};

// Config loads assets to texture_assets_map, renderer will load it from
// texture_assets_map using guid and then make image & view
pub fn texture_system_init(allocator: std.mem.Allocator, config_path: []const u8) !TextureManager {
    var arena = std.heap.ArenaAllocator.init(allocator);
    var arena_alloc = arena.allocator();
    var map = std.StringHashMap(Texture).init(arena_alloc);
    try map.ensureTotalCapacity(256);
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
    var out_list = std.StringHashMap(TextureAsset).init(allocator);
    try out_list.ensureTotalCapacity(@intCast(u32, config.database.len));
    try parse_pngs(allocator, config.database, &out_list);
    return TextureManager{
        .arena = arena,
        .map = map,
        .texture_assets_map = out_list,
    };
}

// TODO: temp function
fn parse_pngs(allocator: std.mem.Allocator, paths: [][:0]const u8, out_map: *std.StringHashMap(TextureAsset)) !void {
    var arena = std.heap.ArenaAllocator.init(allocator);
    defer arena.deinit();
    stbi.init(arena.allocator());
    defer stbi.deinit();
    for (paths) |path| {
        const hash = asset_manager.generate_guid(path);
        var image = stbi.Image.loadFromFile(path, 4) catch {
            log.err("Error loading texture from path {s}.", .{path});
            const asset: TextureAsset = .{
                .guid = hash,
                .data = &.{0},
                .parse_success = false,
            };
            try out_map.putNoClobber(&hash, asset);
            continue;
        };
        defer image.deinit();
        const asset: TextureAsset = .{
            .guid = hash,
            .data = image.data,
            .parse_success = false,
        };
        try out_map.putNoClobber(&hash, asset);
    }
}

pub fn texture_system_deinit(system: *TextureManager) void {
    // system.map.deinit();
    system.arena.deinit();
}

pub fn texture_system_add_texture(system: *TextureManager, pathname: [:0]const u8, gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage) !void {
    var texture: Texture = undefined;
    var arena_alloc = system.arena.allocator();
    stbi.init(arena_alloc);
    defer stbi.deinit();
    var image = stbi.Image.loadFromFile(pathname, 4) catch {
        log.err("Error loading texture from path {s}, using default texture.", .{pathname});
        if (system.default_texture == null) {
            system.default_texture = try generate_default_texture(gctx);
        }
        texture = system.default_texture.?;
        return;
    };
    defer image.deinit();
    texture = texture_create(gctx, usage, .{
        .width = image.width,
        .height = image.height,
        .depth_or_array_layers = 1,
    }, .{
        .components_count = image.num_components,
        .components_width = image.bytes_per_component,
        .is_hdr = image.is_hdr,
    });
    texture_load_data(gctx, &texture, image.width, image.height, image.bytes_per_row, image.data);
    try system.map.put(pathname, texture);
}

pub fn texture_system_get_texture(system: *TextureManager, name: [:0]const u8) Texture {
    if (system.map.contains(name)) {
        return system.map.get(name) orelse unreachable;
    }
    return system.default_texture.?;
}

fn texture_create(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage, size: zgpu.wgpu.Extent3D, format: TextureFormat) Texture {
    const texture = gctx.createTexture(.{
        .usage = usage,
        .size = size,
        .format = zgpu.imageInfoToTextureFormat(
            format.components_count,
            format.components_width,
            format.is_hdr,
        ),
        .mip_level_count = std.math.log2_int(u32, std.math.max(size.width, size.height)) + 1,
    });
    const texture_view = gctx.createTextureView(texture, .{});
    return Texture{ .handle = texture, .view = texture_view };
}

pub fn texture_depth_create(gctx: *zgpu.GraphicsContext) Texture {
    const texture = gctx.createTexture(.{
        .usage = .{ .render_attachment = true },
        .dimension = .tdim_2d,
        .size = .{
            .width = gctx.swapchain_descriptor.width,
            .height = gctx.swapchain_descriptor.height,
            .depth_or_array_layers = 1,
        },
        .format = .depth32_float,
        .mip_level_count = 1,
        .sample_count = 1,
    });
    const view = gctx.createTextureView(texture, .{});
    return Texture{ .handle = texture, .view = view };
}

fn texture_load_data(gctx: *zgpu.GraphicsContext, texture: *Texture, width: u32, height: u32, row_width: u32, data: []u8) void {
    gctx.queue.writeTexture(
        .{ .texture = gctx.lookupResource(texture.handle).? },
        .{
            .bytes_per_row = row_width,
            .rows_per_image = height,
        },
        .{ .width = width, .height = height },
        u8,
        data,
    );
}

fn generate_default_texture(gctx: *zgpu.GraphicsContext) !Texture {
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
    var new_texture = texture_create(gctx, .{ .texture_binding = true, .copy_dst = true }, .{
        .width = 256,
        .height = 256,
        .depth_or_array_layers = 1,
    }, .{ .components_count = 4, .components_width = 1, .is_hdr = false });
    texture_load_data(gctx, &new_texture, dimensions, dimensions, dimensions * channels, pixels[0..]);
    return new_texture;
}