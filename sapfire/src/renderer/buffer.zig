const zgpu = @import("zgpu");
const std = @import("std");

pub const Buffer = struct {
    handle: zgpu.BufferHandle,

    pub fn create(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.BufferUsage, size: u64) Buffer {
        return Buffer{
            .handle = gctx.createBuffer(.{ .usage = usage, .size = size }),
        };
    }

    pub fn create_and_load(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.BufferUsage, comptime T: type, data: []const T) Buffer {
        const handle = gctx.createBuffer(.{ .usage = usage, .size = @sizeOf(T) * data.len });
        gctx.queue.writeBuffer(gctx.lookupResource(handle).?, 0, T, data);
        return Buffer{
            .handle = handle,
        };
    }
};
