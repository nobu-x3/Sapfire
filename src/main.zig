const std = @import("std");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const stbi = @import("zstbi");
const sf = struct {
    usingnamespace @import("texture.zig");
    usingnamespace @import("resources.zig");
    usingnamespace @import("buffer.zig");
    usingnamespace @import("pipeline.zig");
};

pub const Vertex = extern struct {
    position: [2]f32,
    uv: [2]f32,
};

pub const Uniforms = extern struct {
    aspect_ratio: f32,
    mip_level: f32,
};

const RendererState = struct {
    gctx: *zgpu.GraphicsContext,

    pipeline: zgpu.RenderPipelineHandle = .{},
    bind_group: zgpu.BindGroupHandle,

    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,

    texture: sf.Texture,
    sampler: zgpu.SamplerHandle,
    mip_level: i32 = 0,
};

fn renderer_init(allocator: std.mem.Allocator, window: *glfw.Window) !*RendererState {
    const gctx = try zgpu.GraphicsContext.create(allocator, window);
    var arena_state = std.heap.ArenaAllocator.init(allocator);
    defer arena_state.deinit();
    const arena = arena_state.allocator();
    const bind_group_layout = gctx.createBindGroupLayout(&.{
        zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
        zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
        zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
    });
    defer gctx.releaseResource(bind_group_layout);
    // Create a vertex buffer.
    const vertex_data = [_]Vertex{
        .{ .position = [2]f32{ -0.9, 0.9 }, .uv = [2]f32{ 0.0, 0.0 } },
        .{ .position = [2]f32{ 0.9, 0.9 }, .uv = [2]f32{ 1.0, 0.0 } },
        .{ .position = [2]f32{ 0.9, -0.9 }, .uv = [2]f32{ 1.0, 1.0 } },
        .{ .position = [2]f32{ -0.9, -0.9 }, .uv = [2]f32{ 0.0, 1.0 } },
    };
    var vertex_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .vertex = true }, Vertex, vertex_data[0..]);
    // Create an index buffer.
    const index_data = [_]u16{ 0, 1, 3, 1, 2, 3 };
    const index_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .index = true }, u16, index_data[0..]);

    stbi.init(arena);
    defer stbi.deinit();
    var image = try stbi.Image.loadFromFile("assets/textures/" ++ "genart_0025_5.png", 4);
    defer image.deinit();
    // Default texture
    var default_texture = try sf.resources_generate_default_texture(gctx);
    // Create a texture.
    var texture = sf.texture_create(gctx, .{ .texture_binding = true, .copy_dst = true }, .{
        .width = image.width,
        .height = image.height,
        .depth_or_array_layers = 1,
    }, .{ .components_count = image.num_components, .components_width = image.bytes_per_component, .is_hdr = image.is_hdr });
    std.log.info("{}", .{image.bytes_per_row});
    sf.texture_load_data(gctx, &texture, image.width, image.height, image.bytes_per_row, image.data);
    // Create a sampler.
    const sampler = gctx.createSampler(.{});
    const bind_group = gctx.createBindGroup(bind_group_layout, &.{
        .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = 256 },
        .{ .binding = 1, .texture_view_handle = default_texture.view },
        .{ .binding = 2, .sampler_handle = sampler },
    });
    const renderer_state = try allocator.create(RendererState);
    renderer_state.* = .{
        .gctx = gctx,
        .bind_group = bind_group,
        .vertex_buffer = vertex_buffer,
        .index_buffer = index_buffer,
        .texture = default_texture,
        .sampler = sampler,
    };
    // Generate mipmaps on the GPU.
    const commands = commands: {
        const encoder = gctx.device.createCommandEncoder(null);
        defer encoder.release();
        gctx.generateMipmaps(arena, encoder, renderer_state.texture.handle);
        break :commands encoder.finish(null);
    };
    defer commands.release();
    gctx.submit(&.{commands});
    // (Async) Create a render pipeline.
    sf.pipeline_create(allocator, gctx, &.{bind_group_layout}, &renderer_state.pipeline);
    return renderer_state;
}

fn destroy(allocator: std.mem.Allocator, renderer_state: *RendererState) void {
    renderer_state.gctx.destroy(allocator);
    allocator.destroy(renderer_state);
}

fn draw(renderer_state: *RendererState) void {
    const gctx = renderer_state.gctx;
    const fb_width = gctx.swapchain_descriptor.width;
    const fb_height = gctx.swapchain_descriptor.height;

    const back_buffer_view = gctx.swapchain.getCurrentTextureView();
    defer back_buffer_view.release();

    const commands = commands: {
        const encoder = gctx.device.createCommandEncoder(null);
        defer encoder.release();

        // Main pass.
        pass: {
            const vb_info = gctx.lookupResourceInfo(renderer_state.vertex_buffer) orelse break :pass;
            const ib_info = gctx.lookupResourceInfo(renderer_state.index_buffer) orelse break :pass;
            const pipeline = gctx.lookupResource(renderer_state.pipeline) orelse break :pass;
            const bind_group = gctx.lookupResource(renderer_state.bind_group) orelse break :pass;

            const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{.{
                .view = back_buffer_view,
                .load_op = .clear,
                .store_op = .store,
            }};
            const render_pass_info = zgpu.wgpu.RenderPassDescriptor{
                .color_attachment_count = color_attachments.len,
                .color_attachments = &color_attachments,
            };
            const pass = encoder.beginRenderPass(render_pass_info);
            defer {
                pass.end();
                pass.release();
            }

            pass.setVertexBuffer(0, vb_info.gpuobj.?, 0, vb_info.size);
            pass.setIndexBuffer(ib_info.gpuobj.?, .uint16, 0, ib_info.size);

            pass.setPipeline(pipeline);

            const mem = gctx.uniformsAllocate(Uniforms, 1);
            mem.slice[0] = .{
                .aspect_ratio = @intToFloat(f32, fb_width) / @intToFloat(f32, fb_height),
                .mip_level = @intToFloat(f32, renderer_state.mip_level),
            };
            pass.setBindGroup(0, bind_group, &.{mem.offset});
            pass.drawIndexed(6, 1, 0, 0, 0);
        }
        break :commands encoder.finish(null);
    };
    defer commands.release();

    gctx.submit(&.{commands});
    _ = gctx.present();
}

pub fn main() !void {
    glfw.init() catch {
        std.log.err("Failed to init glfw.", .{});
        return;
    };
    defer glfw.terminate();
    const window = glfw.Window.create(800, 600, "Sapfire", null) catch {
        std.log.err("Failed to create window.", .{});
        return;
    };
    defer window.destroy();
    window.setSizeLimits(400, 400, -1, -1);
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    const renderer_state = renderer_init(allocator, window) catch {
        std.log.err("Failed to initialize renderer.", .{});
        return;
    };
    defer destroy(allocator, renderer_state);
    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        glfw.pollEvents();
        draw(renderer_state);
    }
}

test "simple test" {
    var list = std.ArrayList(i32).init(std.testing.allocator);
    defer list.deinit(); // try commenting this out and see if zig detects the memory leak!
    try list.append(42);
    try std.testing.expectEqual(@as(i32, 42), list.pop());
}
