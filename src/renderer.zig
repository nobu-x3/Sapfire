const std = @import("std");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const stbi = @import("zstbi");
const zm = @import("zmath");
const mesh = @import("zmesh");
const sf = struct {
    usingnamespace @import("texture.zig");
    usingnamespace @import("resources.zig");
    usingnamespace @import("buffer.zig");
    usingnamespace @import("pipeline.zig");
};

pub const Vertex = extern struct {
    position: [3]f32,
    uv: [2]f32,
};

pub const Mesh = struct {
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
};

pub const Uniforms = extern struct {
    aspect_ratio: f32,
    mip_level: f32,
    model: zm.Mat,
};

pub const GlobalUniforms = extern struct {
    view_projection: zm.Mat,
};

pub const Camera = struct {
    position: [3]f32 = .{ 0.0, 0.0, -3.0 },
    forward: [3]f32 = .{ 0.0, 0.0, 0.0 },
    pitch: f32 = 0.0,
    // yaw: f32 = std.math.pi + 0.25 * std.math.pi,
    yaw: f32 = 0.0,
};

const RendererState = struct {
    gctx: *zgpu.GraphicsContext,
    pipeline: zgpu.RenderPipelineHandle = .{},
    bind_group: zgpu.BindGroupHandle,
    global_uniform_bind_group: zgpu.BindGroupHandle,
    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,
    texture: sf.Texture,
    depth_texture: sf.Texture,
    sampler: zgpu.SamplerHandle,
    mip_level: i32 = 0,
    meshes: std.ArrayList(Mesh),
    camera: Camera = .{},
    mouse: struct {
        cursor_pos: [2]f64 = .{ 0, 0 },
    } = .{},
};

pub fn renderer_create(allocator: std.mem.Allocator, window: *glfw.Window) !*RendererState {
    const gctx = try zgpu.GraphicsContext.create(allocator, window);
    var arena_state = std.heap.ArenaAllocator.init(allocator);
    defer arena_state.deinit();
    const arena = arena_state.allocator();
    const global_uniform_bgl = gctx.createBindGroupLayout(&.{
        zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
    });
    defer gctx.releaseResource(global_uniform_bgl);
    const local_bgl = gctx.createBindGroupLayout(&.{
        zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
        zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
        zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
    });
    defer gctx.releaseResource(local_bgl);
    var meshes = std.ArrayList(Mesh).init(allocator);
    try meshes.ensureTotalCapacity(128);
    var vertices = std.ArrayList(Vertex).init(arena);
    defer vertices.deinit();
    try vertices.ensureTotalCapacity(256);
    var indices = std.ArrayList(u32).init(arena);
    defer indices.deinit();
    try indices.ensureTotalCapacity(256);
    mesh.init(arena);
    defer mesh.deinit();
    try sf.resources_load_mesh(arena, "assets/models/cube.gltf", &meshes, &vertices, &indices);
    var vertex_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .vertex = true }, Vertex, vertices.items);
    // Create an index buffer.
    const index_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .index = true }, u32, indices.items);
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
    sf.texture_load_data(gctx, &texture, image.width, image.height, image.bytes_per_row, image.data);
    // Depth texture
    const depth_texture = sf.texture_depth_create(gctx);
    // Create a sampler.
    const sampler = gctx.createSampler(.{
        .address_mode_u = .repeat,
        .address_mode_v = .repeat,
        .address_mode_w = .repeat,
    });
    const global_uniform_bg = gctx.createBindGroup(global_uniform_bgl, &.{
        .{
            .binding = 0,
            .buffer_handle = gctx.uniforms.buffer,
            .offset = 0,
            .size = @sizeOf(GlobalUniforms),
        },
    });
    const local_bg = gctx.createBindGroup(local_bgl, &.{
        .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = @sizeOf(Uniforms) },
        .{ .binding = 1, .texture_view_handle = default_texture.view },
        .{ .binding = 2, .sampler_handle = sampler },
    });
    const renderer_state = try allocator.create(RendererState);
    renderer_state.* = .{
        .gctx = gctx,
        .bind_group = local_bg,
        .vertex_buffer = vertex_buffer,
        .index_buffer = index_buffer,
        .texture = default_texture,
        .depth_texture = depth_texture,
        .sampler = sampler,
        .global_uniform_bind_group = global_uniform_bg,
        .meshes = meshes,
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
    sf.pipeline_create(allocator, gctx, &.{ global_uniform_bgl, local_bgl }, &renderer_state.pipeline);
    return renderer_state;
}

pub fn destroy(allocator: std.mem.Allocator, renderer_state: *RendererState) void {
    renderer_state.gctx.destroy(allocator);
    renderer_state.meshes.deinit();
    allocator.destroy(renderer_state);
}

pub fn update(renderer_state: *RendererState, window: *glfw.Window) void {
    const cursor_pos = window.getCursorPos();
    const delta_x = @floatCast(f32, cursor_pos[0] - renderer_state.mouse.cursor_pos[0]);
    const delta_y = @floatCast(f32, cursor_pos[1] - renderer_state.mouse.cursor_pos[1]);
    renderer_state.mouse.cursor_pos = cursor_pos;
    if (window.getMouseButton(.left) == .press) {} else if (window.getMouseButton(.right) == .press) {
        renderer_state.camera.pitch += 0.0025 * delta_y;
        renderer_state.camera.yaw += 0.0025 * delta_x;
        renderer_state.camera.pitch = std.math.min(renderer_state.camera.pitch, 0.48 * std.math.pi);
        renderer_state.camera.pitch = std.math.max(renderer_state.camera.pitch, -0.48 * std.math.pi);
        renderer_state.camera.yaw = zm.modAngle(renderer_state.camera.yaw);
    }
    const speed = zm.f32x4s(2.0);
    const delta_time = zm.f32x4s(renderer_state.gctx.stats.delta_time);
    const transform = zm.mul(zm.rotationX(renderer_state.camera.pitch), zm.rotationY(renderer_state.camera.yaw));
    var forward = zm.normalize3(zm.mul(zm.f32x4(0.0, 0.0, 1.0, 0.0), transform));
    zm.storeArr3(&renderer_state.camera.forward, forward);
    const right = speed * delta_time * zm.normalize3(zm.cross3(zm.f32x4(0.0, 1.0, 0.0, 0.0), forward));
    forward = speed * delta_time * forward;
    var cam_pos = zm.loadArr3(renderer_state.camera.position);
    if (window.getKey(.up) == .press) {
        cam_pos += forward;
    } else if (window.getKey(.down) == .press) {
        cam_pos -= forward;
    }
    if (window.getKey(.right) == .press) {
        cam_pos += right;
    } else if (window.getKey(.left) == .press) {
        cam_pos -= right;
    }
    zm.storeArr3(&renderer_state.camera.position, cam_pos);
}

pub fn draw(renderer_state: *RendererState) void {
    const gctx = renderer_state.gctx;
    const fb_width = gctx.swapchain_descriptor.width;
    const fb_height = gctx.swapchain_descriptor.height;
    const t = @floatCast(f32, gctx.stats.time);
    const cam_world_to_view = zm.lookAtLh(
        zm.loadArr3(renderer_state.camera.position),
        zm.loadArr3(renderer_state.camera.forward),
        zm.f32x4(0.0, 1.0, 0.0, 0.0),
    );
    const cam_view_to_clip = zm.perspectiveFovLh(
        0.25 * std.math.pi,
        @intToFloat(f32, fb_width) / @intToFloat(f32, fb_height),
        0.01,
        200.0,
    );
    const cam_world_to_clip = zm.mul(cam_world_to_view, cam_view_to_clip);
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
            const global_uniform_bind_group = gctx.lookupResource(renderer_state.global_uniform_bind_group) orelse break :pass;
            const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
            const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{.{
                .view = back_buffer_view,
                .load_op = .clear,
                .store_op = .store,
            }};
            const depth_attachment = zgpu.wgpu.RenderPassDepthStencilAttachment{
                .view = depth_view,
                .depth_load_op = .clear,
                .depth_store_op = .store,
                .depth_clear_value = 1.0,
            };
            const render_pass_info = zgpu.wgpu.RenderPassDescriptor{
                .color_attachment_count = color_attachments.len,
                .color_attachments = &color_attachments,
                .depth_stencil_attachment = &depth_attachment,
            };
            const pass = encoder.beginRenderPass(render_pass_info);
            defer {
                pass.end();
                pass.release();
            }
            pass.setVertexBuffer(0, vb_info.gpuobj.?, 0, vb_info.size);
            pass.setIndexBuffer(ib_info.gpuobj.?, .uint32, 0, ib_info.size);
            pass.setPipeline(pipeline);
            const object_to_world = zm.mul(zm.rotationY(t), zm.translation(-1.0, 0.0, 0.0));
            // const object_to_clip = zm.mul(object_to_world, cam_world_to_clip);
            const glob = gctx.uniformsAllocate(GlobalUniforms, 1);
            glob.slice[0] = .{
                .view_projection = zm.transpose(cam_world_to_clip),
            };
            const mem = gctx.uniformsAllocate(Uniforms, 1);
            mem.slice[0] = .{
                .aspect_ratio = @intToFloat(f32, fb_width) / @intToFloat(f32, fb_height),
                .mip_level = @intToFloat(f32, renderer_state.mip_level),
                .model = zm.transpose(object_to_world),
            };
            pass.setBindGroup(0, global_uniform_bind_group, &.{glob.offset});
            pass.setBindGroup(1, bind_group, &.{mem.offset});
            pass.drawIndexed(6, 1, 0, 0, 0);
        }
        break :commands encoder.finish(null);
    };
    defer commands.release();
    gctx.submit(&.{commands});
    if (gctx.present() == .swap_chain_resized) {
        // Release old depth texture.
        gctx.releaseResource(renderer_state.depth_texture.view);
        gctx.destroyResource(renderer_state.depth_texture.handle);
        // Create a new depth texture to match the new window size.
        renderer_state.depth_texture = sf.texture_depth_create(gctx);
    }
}
