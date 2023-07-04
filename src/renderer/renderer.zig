const std = @import("std");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const stbi = @import("zstbi");
const zm = @import("zmath");
const mesh = @import("zmesh");
const zgui = @import("zgui");
const sf = struct {
    usingnamespace @import("texture.zig");
    usingnamespace @import("mesh.zig");
    usingnamespace @import("buffer.zig");
    usingnamespace @import("pipeline.zig");
    usingnamespace @import("material.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("../core/time.zig");
};

pub const RendererState = struct {
    gctx: *zgpu.GraphicsContext,
    global_uniform_bind_group: zgpu.BindGroupHandle,
    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,
    pipeline_system: sf.PipelineSystem,
    depth_texture: sf.Texture,
    mip_level: i32 = 0,
    meshes: std.ArrayList(sf.Mesh),
    camera: sf.Camera = .{},
    mouse: struct {
        cursor_pos: [2]f64 = .{ 0, 0 },
    } = .{},

    pub fn create(allocator: std.mem.Allocator, window: *glfw.Window) !*RendererState {
        const gctx = try zgpu.GraphicsContext.create(allocator, window);
        var arena_state = std.heap.ArenaAllocator.init(allocator);
        defer arena_state.deinit();
        const arena = arena_state.allocator();
        const global_uniform_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
        });
        defer gctx.releaseResource(global_uniform_bgl);
        const transform = sf.Transform.init();
        _ = transform;
        var meshes = std.ArrayList(sf.Mesh).init(allocator);
        try meshes.ensureTotalCapacity(128);
        var vertices = std.ArrayList(sf.Vertex).init(arena);
        defer vertices.deinit();
        try vertices.ensureTotalCapacity(256);
        var indices = std.ArrayList(u32).init(arena);
        defer indices.deinit();
        try indices.ensureTotalCapacity(256);
        try sf.MeshAsset.load_mesh("assets/models/cube.gltf", &meshes, &vertices, &indices);
        try sf.MeshAsset.load_mesh("assets/models/SciFiHelmet/SciFiHelmet.gltf", &meshes, &vertices, &indices);
        var vertex_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .vertex = true }, sf.Vertex, vertices.items);
        // Create an index buffer.
        const index_buffer: zgpu.BufferHandle = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .index = true }, u32, indices.items);
        var pipeline_system = try sf.PipelineSystem.init(allocator);
        var texture_system = sf.AssetManager.texture_manager();
        try sf.TextureManager.add_texture(texture_system, "assets/textures/" ++ "cobblestone.png", gctx, .{ .texture_binding = true, .copy_dst = true });
        try sf.TextureManager.add_texture(texture_system, "assets/textures/" ++ "genart_0025_5.png", gctx, .{ .texture_binding = true, .copy_dst = true });
        const depth_texture = sf.Texture.create_depth(gctx);
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
        var pipeline = try sf.PipelineSystem.add_pipeline(&pipeline_system, gctx, &.{ global_uniform_bgl, local_bgl }, false);
        // TODO: a module that parses material files (json or smth) and outputs bind group layouts to pass to pipeline system
        var material_manager = sf.AssetManager.material_manager();
        try sf.MaterialManager.add_material(material_manager, "material", gctx, texture_system, &.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
            zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
            zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
        }, @sizeOf(sf.Uniforms), "assets/textures/" ++ "cobblestone.png");
        try sf.MaterialManager.add_material(material_manager, "material1", gctx, texture_system, &.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
            zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
            zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
        }, @sizeOf(sf.Uniforms), "assets/textures/" ++ "genart_0025_5.png");
        var mat0 = material_manager.materials.getPtr(sf.AssetManager.generate_guid("material")).?;
        var mat1 = material_manager.materials.getPtr(sf.AssetManager.generate_guid("material1")).?;
        try sf.PipelineSystem.add_material(&pipeline_system, pipeline, mat0);
        try sf.PipelineSystem.add_material(&pipeline_system, pipeline, mat1);
        try sf.MaterialManager.add_material_to_mesh_by_name(material_manager, "material", &meshes.items[0]);
        try sf.MaterialManager.add_material_to_mesh_by_name(material_manager, "material1", &meshes.items[1]);
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
            .gctx = gctx,
            .vertex_buffer = vertex_buffer,
            .index_buffer = index_buffer,
            .pipeline_system = pipeline_system,
            .global_uniform_bind_group = global_uniform_bg,
            .meshes = meshes,
            .depth_texture = depth_texture,
        };
        // Generate mipmaps on the GPU.
        // const commands = commands: {
        //     const encoder = gctx.device.createCommandEncoder(null);
        //     defer encoder.release();
        //     gctx.generateMipmaps(arena, encoder, renderer_state.texture.handle);
        //     break :commands encoder.finish(null);
        // };
        // defer commands.release();
        // gctx.submit(&.{commands});
        // (Async) Create a render pipeline.
        return renderer_state;
    }

    pub fn destroy(allocator: std.mem.Allocator, renderer_state: *RendererState) void {
        renderer_state.meshes.deinit();
        sf.PipelineSystem.deinit(&renderer_state.pipeline_system);
        renderer_state.gctx.destroy(allocator);
        allocator.destroy(renderer_state);
    }

    pub fn update(renderer_state: *RendererState, window: *glfw.Window) void {
        const cursor_pos = window.getCursorPos();
        const delta_x = @floatCast(f32, cursor_pos[0] - renderer_state.mouse.cursor_pos[0]);
        const delta_y = @floatCast(f32, cursor_pos[1] - renderer_state.mouse.cursor_pos[1]);
        renderer_state.mouse.cursor_pos = cursor_pos;
        if (window.getMouseButton(.left) == .press) {
            renderer_state.meshes.items[0].transform.rotate(delta_y * 0.0025, .{ 1.0, 0.0, 0.0 });
            renderer_state.meshes.items[0].transform.rotate(delta_x * 0.0025, .{ 0.0, 1.0, 0.0 });
        } else if (window.getMouseButton(.right) == .press) {
            renderer_state.camera.pitch += 0.0025 * delta_y;
            renderer_state.camera.yaw += 0.0025 * delta_x;
            renderer_state.camera.pitch = std.math.min(renderer_state.camera.pitch, 0.48 * std.math.pi);
            renderer_state.camera.pitch = std.math.max(renderer_state.camera.pitch, -0.48 * std.math.pi);
            renderer_state.camera.yaw = zm.modAngle(renderer_state.camera.yaw);
        }
        const speed = zm.f32x4s(2.0);
        // const delta_time = zm.f32x4s(renderer_state.gctx.stats.delta_time);
        const delta_time = zm.f32x4s(sf.Time.delta_time());
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
        for (renderer_state.meshes.items, 0..) |_, index| {
            // renderer_state.meshes.items[index].transform.update();
            sf.Transform.update(&renderer_state.meshes.items[index].transform);
        }
    }

    pub fn draw(renderer_state: *RendererState) void {
        const gctx = renderer_state.gctx;
        const fb_width = gctx.swapchain_descriptor.width;
        const fb_height = gctx.swapchain_descriptor.height;
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
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
                const global_uniform_bind_group = gctx.lookupResource(renderer_state.global_uniform_bind_group) orelse break :pass;
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
                // const object_to_clip = zm.mul(object_to_world, cam_world_to_clip);
                const glob = gctx.uniformsAllocate(sf.GlobalUniforms, 1);
                glob.slice[0] = .{
                    .view_projection = zm.transpose(cam_world_to_clip),
                };
                for (renderer_state.pipeline_system.pipelines.items) |pipe| {
                    const pipeline = gctx.lookupResource(pipe.handle) orelse break :pass;
                    pass.setPipeline(pipeline);
                    for (pipe.materials.items) |material| {
                        const bind_group = gctx.lookupResource(material.bind_group) orelse break :pass;
                        const meshes = sf.AssetManager.material_manager().map.getPtr(material.*).?;
                        for (meshes.items) |item| {
                            const object_to_world = item.transform.matrix;
                            const mem = gctx.uniformsAllocate(sf.Uniforms, 1);
                            mem.slice[0] = .{
                                .aspect_ratio = @intToFloat(f32, fb_width) / @intToFloat(f32, fb_height),
                                .mip_level = @intToFloat(f32, renderer_state.mip_level),
                                .model = zm.transpose(object_to_world),
                            };
                            pass.setBindGroup(0, global_uniform_bind_group, &.{glob.offset});
                            pass.setBindGroup(1, bind_group, &.{mem.offset});
                            pass.drawIndexed(item.num_indices, 1, item.index_offset, item.vertex_offset, 0);
                        }
                    }
                }
            }
            { // gui pass
                const gui_pass = zgpu.beginRenderPassSimple(encoder, .load, back_buffer_view, null, null, null);
                defer zgpu.endReleasePass(gui_pass);
                zgui.backend.newFrame(fb_width, fb_height);

                zgui.bulletText(
                    "Average :  {d:.3} ms/frame ({d:.1} fps)",
                    .{ renderer_state.gctx.stats.average_cpu_time, renderer_state.gctx.stats.fps },
                );
                zgui.backend.draw(gui_pass);
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
            renderer_state.depth_texture = sf.Texture.create_depth(gctx);
        }
    }
};
