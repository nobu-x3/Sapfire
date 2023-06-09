const std = @import("std");
const zgpu = @import("zgpu");

pub const TextureFormat = struct { components_count: u32, components_width: u32, is_hdr: bool };

pub const Texture = struct {
    handle: zgpu.TextureHandle,
    view: zgpu.TextureViewHandle,
};

pub fn texture_create(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.TextureUsage, size: zgpu.wgpu.Extent3D, format: TextureFormat) Texture {
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

pub fn texture_load_data(gctx: *zgpu.GraphicsContext, texture: *Texture, width: u32, height: u32, row_width: u32, data: []u8) void {
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
