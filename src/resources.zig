const std = @import("std");
const zgpu = @import("zgpu");
const texture = @import("texture.zig");

pub fn resources_generate_default_texture(gctx: *zgpu.GraphicsContext) !texture.Texture {
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
    var new_texture = texture.texture_create(gctx, .{ .texture_binding = true, .copy_dst = true }, .{
        .width = 256,
        .height = 256,
        .depth_or_array_layers = 1,
    }, .{ .components_count = 4, .components_width = 1, .is_hdr = false });
    texture.texture_load_data(gctx, &new_texture, dimensions, dimensions, dimensions * channels, pixels[0..]);
    return new_texture;
}
