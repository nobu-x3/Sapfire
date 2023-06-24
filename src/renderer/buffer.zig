const zgpu = @import("zgpu");

pub fn buffer_create(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.BufferUsage, size: u64) zgpu.BufferHandle {
    return gctx.createBuffer(.{ .usage = usage, .size = size });
}

pub fn buffer_create_and_load(gctx: *zgpu.GraphicsContext, usage: zgpu.wgpu.BufferUsage, comptime T: type, data: []const T) zgpu.BufferHandle {
    const buffer = gctx.createBuffer(.{ .usage = usage, .size = @sizeOf(T) * data.len });
    gctx.queue.writeBuffer(gctx.lookupResource(buffer).?, 0, T, data);
    return buffer;
}
