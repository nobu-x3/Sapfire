const std = @import("std");
const zgpu = @import("zgpu");
const ecs = @import("zflecs");
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
    usingnamespace @import("../scene.zig");
};

pub const RendererState = struct {
    arena: std.heap.ArenaAllocator,
    gctx: *zgpu.GraphicsContext,
    depth_texture: sf.Texture,
    mip_level: i32 = 0,
    camera: sf.Camera = .{},
    mouse: struct {
        cursor_pos: [2]f64 = .{ 0, 0 },
    } = .{},

    pub var renderer: ?*RendererState = null;
    pub var color_view: ?*zgpu.wgpu.TextureView = null;
    pub var fb_width: u32 = 800;
    pub var fb_height: u32 = 600;

    // TODO: runtime dependent assets should be multithreaded here. Probably pass a pointer to scene asset here.
    pub fn create(allocator: std.mem.Allocator, window: *glfw.Window) !*RendererState {
        var arena = std.heap.ArenaAllocator.init(allocator);
        const gctx = try zgpu.GraphicsContext.create(arena.allocator(), window);
        const depth_texture = sf.Texture.create_depth(gctx, gctx.swapchain_descriptor.width, gctx.swapchain_descriptor.height);
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
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
        renderer = renderer_state;
        return renderer_state;
    }

    pub fn create_with_gctx(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, fb_width_passed: u32, fb_height_passed: u32) !*RendererState {
        var arena = std.heap.ArenaAllocator.init(allocator);
        const depth_texture = sf.Texture.create_depth(gctx, fb_width_passed, fb_height_passed);
        fb_width = fb_width_passed;
        fb_height = fb_height_passed;
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
            .gctx = gctx,
            .depth_texture = depth_texture,
            .arena = arena,
        };
        return renderer_state;
    }

    // pub fn load_scene(state: *RendererState, scene_config: []const u8) void {}

    pub fn destroy(renderer_state: *RendererState, allocator: std.mem.Allocator) void {
        renderer_state.arena.deinit();
        allocator.destroy(renderer_state);
    }

    pub fn update(renderer_state: *RendererState, window: *glfw.Window) void {
        const cursor_pos = window.getCursorPos();
        const delta_x: f32 = @floatCast(cursor_pos[0] - renderer_state.mouse.cursor_pos[0]);
        const delta_y: f32 = @floatCast(cursor_pos[1] - renderer_state.mouse.cursor_pos[1]);
        renderer_state.mouse.cursor_pos = cursor_pos;
        if (window.getMouseButton(.left) == .press) {
            // renderer_state.current_simple_scene.meshes.items[0].transform.rotate(delta_y * 0.0025, .{ 1.0, 0.0, 0.0 });
            // renderer_state.current_simple_scene.meshes.items[0].transform.rotate(delta_x * 0.0025, .{ 0.0, 1.0, 0.0 });
        } else if (window.getMouseButton(.right) == .press) {
            renderer_state.camera.pitch += 0.0025 * delta_y;
            renderer_state.camera.yaw += 0.0025 * delta_x;
            renderer_state.camera.pitch = @min(renderer_state.camera.pitch, 0.48 * std.math.pi);
            renderer_state.camera.pitch = @max(renderer_state.camera.pitch, -0.48 * std.math.pi);
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
    }

    pub fn draw(renderer_state: *RendererState) !void {
        const gctx = renderer_state.gctx;
        fb_width = gctx.swapchain_descriptor.width;
        fb_height = gctx.swapchain_descriptor.height;
        const cam_world_to_view = zm.lookAtLh(
            zm.loadArr3(renderer_state.camera.position),
            zm.loadArr3(renderer_state.camera.forward),
            zm.f32x4(0.0, 1.0, 0.0, 0.0),
        );
        const cam_view_to_clip = zm.perspectiveFovLh(
            0.25 * std.math.pi,
            @as(f32, @floatFromInt(fb_width)) / @as(f32, @floatFromInt(fb_height)),
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
                // const vb_info = gctx.lookupResourceInfo(renderer_state.current_simple_scene.vertex_buffer) orelse break :pass;
                // const ib_info = gctx.lookupResourceInfo(renderer_state.current_simple_scene.index_buffer) orelse break :pass;
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
                // const global_uniform_bind_group = gctx.lookupResource(renderer_state.current_simple_scene.global_uniform_bind_group) orelse break :pass;
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
                // pass.setVertexBuffer(0, vb_info.gpuobj.?, 0, vb_info.size);
                // pass.setIndexBuffer(ib_info.gpuobj.?, .uint32, 0, ib_info.size);
                // const object_to_clip = zm.mul(object_to_world, cam_world_to_clip);

                const glob = gctx.uniformsAllocate(sf.GlobalUniforms, 1);
                glob.slice[0] = .{
                    .view_projection = zm.transpose(cam_world_to_clip),
                };
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

    pub fn draw_to_texture(renderer_state: *RendererState, color_view_passed: *zgpu.wgpu.TextureView, fb_width_passed: u32, fb_height_passed: u32, scene: *sf.Scene) !void {
        const gctx = renderer_state.gctx;
        color_view = color_view_passed;
        fb_width = fb_width_passed;
        fb_height = fb_height_passed;
        const cam_world_to_view = zm.lookAtLh(
            zm.loadArr3(renderer_state.camera.position),
            zm.loadArr3(renderer_state.camera.forward),
            zm.f32x4(0.0, 1.0, 0.0, 0.0),
        );
        const cam_view_to_clip = zm.perspectiveFovLh(
            0.25 * std.math.pi,
            @as(f32, @floatFromInt(fb_width_passed)) / @as(f32, @floatFromInt(fb_height_passed)),
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
                const vb_info = gctx.lookupResourceInfo(scene.vertex_buffer) orelse break :pass;
                const ib_info = gctx.lookupResourceInfo(scene.index_buffer) orelse break :pass;
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
                const global_uniform_bind_group = gctx.lookupResource(scene.global_uniform_bind_group) orelse break :pass;
                const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{.{
                    .view = color_view_passed.*,
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
                pass.setBindGroup(0, global_uniform_bind_group, &.{glob.offset});
                var query_desc = ecs.query_desc_t{};
                query_desc.filter.terms[0] = .{ .id = ecs.id(sf.Components.Transform) };
                query_desc.filter.terms[1] = .{ .id = ecs.id(sf.Material) };
                query_desc.filter.terms[2] = .{ .id = ecs.id(sf.Components.Mesh) };
                var q = try ecs.query_init(scene.world.id, &query_desc);
                var it = ecs.query_iter(scene.world.id, q);
                while (ecs.query_next(&it)) {
                    var i: usize = 0;
                    while (i < it.count()) : (i += 1) {
                        var current_pipeline: sf.Pipeline = undefined;
                        var current_material: sf.Material = undefined;
                        if (ecs.field(&it, sf.Material, 2)) |materials| {
                            const mat = materials[i];
                            const pipe = scene.pipeline_system.material_pipeline_map.get(mat.guid).?;
                            if (pipe.handle.id != current_pipeline.handle.id) {
                                current_pipeline = pipe;
                                const pipeline = gctx.lookupResource(current_pipeline.handle) orelse break :pass;
                                pass.setPipeline(pipeline);
                            }
                            var should_bind_group = false;

                            if (!std.mem.eql(u8, mat.guid[0..], current_material.guid[0..])) {
                                current_material = mat;
                                should_bind_group = true;
                            }
                            if (ecs.field(&it, sf.Components.Transform, 1)) |transforms| {
                                const transform = transforms[i];
                                if (ecs.field(&it, sf.Components.Mesh, 3)) |meshes| {
                                    const mesh_comp = meshes[i];
                                    const object_to_world = transform.world;
                                    const mem = gctx.uniformsAllocate(sf.Uniforms, 1);
                                    mem.slice[0] = .{
                                        .aspect_ratio = @as(f32, @floatFromInt(fb_width_passed)) / @as(f32, @floatFromInt(fb_height_passed)),
                                        .mip_level = @floatFromInt(renderer_state.mip_level),
                                        .model = zm.transpose(object_to_world),
                                    };
                                    if (should_bind_group) {
                                        const bind_group = gctx.lookupResource(current_material.bind_group) orelse break :pass;
                                        pass.setBindGroup(1, bind_group, &.{mem.offset});
                                    }
                                    pass.drawIndexed(mesh_comp.num_indices, 1, mesh_comp.index_offset, mesh_comp.vertex_offset, 0);
                                }
                            }
                        }
                    }
                }
            }
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
