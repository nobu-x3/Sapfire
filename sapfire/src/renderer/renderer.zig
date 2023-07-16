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
    usingnamespace @import("scene.zig");
    usingnamespace @import("material.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("../core/time.zig");
};

pub const RendererState = struct {
    arena: std.heap.ArenaAllocator,
    gctx: *zgpu.GraphicsContext,
    depth_texture: sf.Texture,
    current_scene: sf.SimpleScene,
    mip_level: i32 = 0,
    camera: sf.Camera = .{},
    mouse: struct {
        cursor_pos: [2]f64 = .{ 0, 0 },
    } = .{},

    // TODO: runtime dependent assets should be multithreaded here. Probably pass a pointer to scene asset here.
    pub fn create(allocator: std.mem.Allocator, window: *glfw.Window) !*RendererState {
        var arena = std.heap.ArenaAllocator.init(allocator);
        const gctx = try zgpu.GraphicsContext.create(arena.allocator(), window);
        const depth_texture = sf.Texture.create_depth(gctx, gctx.swapchain_descriptor.width, gctx.swapchain_descriptor.height);
        const scene = try sf.SimpleScene.create(arena.allocator(), "project/scenes/simple_scene.json", gctx);
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
            .current_scene = scene,
            .gctx = gctx,
            .depth_texture = depth_texture,
            .arena = arena,
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

    pub fn create_with_gctx(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, scene_path: [:0]const u8, fb_width: u32, fb_height: u32) !*RendererState {
        var arena = std.heap.ArenaAllocator.init(allocator);
        const depth_texture = sf.Texture.create_depth(gctx, fb_width, fb_height);
        const scene = try sf.SimpleScene.create(arena.allocator(), scene_path, gctx);
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
            .current_scene = scene,
            .gctx = gctx,
            .depth_texture = depth_texture,
            .arena = arena,
        };
        return renderer_state;
    }

    // pub fn load_scene(state: *RendererState, scene_config: []const u8) void {}

    pub fn destroy(renderer_state: *RendererState, allocator: std.mem.Allocator) void {
        // renderer_state.current_scene.destroy();
        // renderer_state.gctx.destroy(allocator);
        renderer_state.arena.deinit();
        allocator.destroy(renderer_state);
    }

    pub fn update(renderer_state: *RendererState, window: *glfw.Window) void {
        const cursor_pos = window.getCursorPos();
        const delta_x = @floatCast(f32, cursor_pos[0] - renderer_state.mouse.cursor_pos[0]);
        const delta_y = @floatCast(f32, cursor_pos[1] - renderer_state.mouse.cursor_pos[1]);
        renderer_state.mouse.cursor_pos = cursor_pos;
        if (window.getMouseButton(.left) == .press) {
            renderer_state.current_scene.meshes.items[0].transform.rotate(delta_y * 0.0025, .{ 1.0, 0.0, 0.0 });
            renderer_state.current_scene.meshes.items[0].transform.rotate(delta_x * 0.0025, .{ 0.0, 1.0, 0.0 });
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
        // for (renderer_state.current_scene.meshes.items, 0..) |_, index| {
        //     // renderer_state.meshes.items[index].transform.update();
        //     sf.Transform.update(&renderer_state.current_scene.meshes.items[index].transform);
        // }
    }

    pub fn draw(renderer_state: *RendererState) !void {
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
                const vb_info = gctx.lookupResourceInfo(renderer_state.current_scene.vertex_buffer) orelse break :pass;
                const ib_info = gctx.lookupResourceInfo(renderer_state.current_scene.index_buffer) orelse break :pass;
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
                const global_uniform_bind_group = gctx.lookupResource(renderer_state.current_scene.global_uniform_bind_group) orelse break :pass;
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
                for (renderer_state.current_scene.pipeline_system.pipelines.items) |pipe| {
                    const pipeline = gctx.lookupResource(pipe.handle) orelse break :pass;
                    pass.setPipeline(pipeline);
                    for (pipe.materials.items) |material| {
                        const bind_group = gctx.lookupResource(material.bind_group) orelse break :pass;
                        const meshes = renderer_state.current_scene.material_manager.map.getPtr(material.*).?;
                        for (meshes.items) |item| {
                            const object_to_world = item.transform.get_world_mat();
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
                if (zgui.begin("Stats", .{ .flags = .{ .always_auto_resize = true } })) {
                    zgui.bulletText(
                        "Average :  {d:.3} ms/frame ({d:.1} fps)",
                        .{ renderer_state.gctx.stats.average_cpu_time, renderer_state.gctx.stats.fps },
                    );
                }
                zgui.end();
                if (zgui.begin("Scene switching", .{ .flags = .{ .always_auto_resize = true } })) {
                    if (zgui.button("Scene 1", .{ .w = 100.0, .h = 100.0 })) {
                        renderer_state.current_scene = try sf.SimpleScene.create(renderer_state.arena.allocator(), "project/scenes/simple_scene.json", renderer_state.gctx);
                    } else if (zgui.button("Scene 2", .{ .w = 100.0, .h = 100.0 })) {
                        renderer_state.current_scene = try sf.SimpleScene.create(renderer_state.arena.allocator(), "project/scenes/simple_scene1.json", renderer_state.gctx);
                    }
                }
                zgui.end();
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
            renderer_state.depth_texture = sf.Texture.create_depth(gctx, gctx.swapchain_descriptor.width, gctx.swapchain_descriptor.height);
        }
    }

    pub fn draw_to_texture(renderer_state: *RendererState, color_view: *zgpu.wgpu.TextureView, fb_width: u32, fb_height: u32) !void {
        const gctx = renderer_state.gctx;
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
                const vb_info = gctx.lookupResourceInfo(renderer_state.current_scene.vertex_buffer) orelse break :pass;
                const ib_info = gctx.lookupResourceInfo(renderer_state.current_scene.index_buffer) orelse break :pass;
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
                const global_uniform_bind_group = gctx.lookupResource(renderer_state.current_scene.global_uniform_bind_group) orelse break :pass;
                const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{.{
                    .view = color_view.*,
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
                for (renderer_state.current_scene.pipeline_system.pipelines.items) |pipe| {
                    const pipeline = gctx.lookupResource(pipe.handle) orelse break :pass;
                    pass.setPipeline(pipeline);
                    for (pipe.materials.items) |material| {
                        const bind_group = gctx.lookupResource(material.bind_group) orelse break :pass;
                        const meshes = renderer_state.current_scene.material_manager.map.getPtr(material.*).?;
                        for (meshes.items) |item| {
                            const object_to_world = item.transform.get_world_mat();
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
            // { // gui pass
            //     const gui_pass = zgpu.beginRenderPassSimple(encoder, .load, back_buffer_view, null, null, null);
            //     defer zgpu.endReleasePass(gui_pass);
            //     zgui.backend.newFrame(fb_width, fb_height);
            //     if (zgui.begin("Stats", .{ .flags = .{ .always_auto_resize = true } })) {
            //         zgui.bulletText(
            //             "Average :  {d:.3} ms/frame ({d:.1} fps)",
            //             .{ renderer_state.gctx.stats.average_cpu_time, renderer_state.gctx.stats.fps },
            //         );
            //     }
            //     zgui.end();
            //     if (zgui.begin("Scene switching", .{ .flags = .{ .always_auto_resize = true } })) {
            //         if (zgui.button("Scene 1", .{ .w = 100.0, .h = 100.0 })) {
            //             renderer_state.current_scene = try sf.SimpleScene.create(renderer_state.arena.allocator(), "project/scenes/simple_scene.json", renderer_state.gctx);
            //         } else if (zgui.button("Scene 2", .{ .w = 100.0, .h = 100.0 })) {
            //             renderer_state.current_scene = try sf.SimpleScene.create(renderer_state.arena.allocator(), "project/scenes/simple_scene1.json", renderer_state.gctx);
            //         }
            //     }
            //     zgui.end();
            //     zgui.backend.draw(gui_pass);
            // }
            break :commands encoder.finish(null);
        };
        defer commands.release();
        gctx.submit(&.{commands});
        // if (gctx.present() == .swap_chain_resized) {
        //     // Release old depth texture.
        //     gctx.releaseResource(renderer_state.depth_texture.view);
        //     gctx.destroyResource(renderer_state.depth_texture.handle);
        //     // Create a new depth texture to match the new window size.
        //     renderer_state.depth_texture = sf.Texture.create_depth(gctx);
        // }
    }
};
