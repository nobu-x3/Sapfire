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
    usingnamespace @import("../scene.zig");
    usingnamespace @import("material.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("../core.zig");
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
        const gctx = try zgpu.GraphicsContext.create(arena.allocator(), window, .{});
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
        fb_height = fb_height_passed;
        const renderer_state = try allocator.create(RendererState);
        renderer_state.* = .{
            .gctx = gctx,
            .depth_texture = depth_texture,
            .arena = arena,
        };
        return renderer_state;
    }

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
        renderer = renderer_state;
        fb_width = fb_width_passed;
        fb_height = fb_height_passed;
        const gctx = renderer_state.gctx;
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
                const vb_info = gctx.lookupResourceInfo(scene.vertex_buffer.handle) orelse break :pass;
                const ib_info = gctx.lookupResourceInfo(scene.index_buffer.handle) orelse break :pass;
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
                const glob = gctx.uniformsAllocate(sf.GlobalUniforms, 1);
                glob.slice[0] = .{
                    .view_projection = zm.transpose(cam_world_to_clip),
                };
                pass.setBindGroup(0, global_uniform_bind_group, &.{glob.offset});
                var query_desc = ecs.query_desc_t{};
                query_desc.filter.terms[0] = .{ .id = ecs.id(sf.Material) };
                query_desc.filter.terms[1] = .{ .id = ecs.id(sf.components.Transform) };
                query_desc.filter.terms[2] = .{ .id = ecs.id(sf.components.Mesh) };
                query_desc.order_by_component = ecs.id(sf.Material);
                var q = try ecs.query_init(scene.world.id, &query_desc);
                var it = ecs.query_iter(scene.world.id, q);
                while (ecs.query_next(&it)) {
                    const entities = it.entities();
                    var i: usize = 0;
                    while (i < it.count()) : (i += 1) {
                        if (!ecs.is_valid(it.world, entities[i]) or !ecs.is_alive(it.world, entities[i])) continue;
                        if (ecs.field(&it, sf.Material, 1)) |materials| {
                            const mat = materials[i];
                            const pipe = scene.pipeline_system.material_pipeline_map.get(mat.guid).?;
                            var current_pipeline: sf.Pipeline = undefined;
                            var current_material: sf.Material = undefined;
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
                            if (ecs.field(&it, sf.components.Transform, 2)) |transforms| {
                                const transform = transforms[i];
                                if (ecs.field(&it, sf.components.Mesh, 3)) |meshes| {
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
                                        const phong_data = gctx.uniformsAllocate(sf.PhongData, 1);
                                        phong_data.slice[0] = .{
                                            .ambient = current_material.phong_data.ambient,
                                            .diffuse = current_material.phong_data.diffuse,
                                            .reflection = current_material.phong_data.reflection,
                                        };
                                        pass.setBindGroup(1, bind_group, &.{ mem.offset, phong_data.offset });
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

pub const Renderer = struct {
    arena: std.heap.ArenaAllocator,
    gctx: *zgpu.GraphicsContext,
    depth_texture: sf.Texture,
    mesh_manager: sf.MeshManager,
    vertices: std.ArrayList(sf.Vertex),
    indices: std.ArrayList(u32),
    meshes: std.ArrayList(sf.components.Mesh),
    mip_level: i32 = 0,
    camera: sf.Camera = .{},
    mouse: struct {
        cursor_pos: [2]f64 = .{ 0, 0 },
    } = .{},
    global_uniform_bind_group: zgpu.BindGroupHandle,
    local_uniform_bind_group: zgpu.BindGroupHandle,
    g_buffer_textures_bind_group: zgpu.BindGroupHandle,
    lights_bind_group: zgpu.BindGroupHandle,
    vertex_buffer: sf.Buffer,
    index_buffer: sf.Buffer,
    global_uniform_buffer: sf.Buffer,
    lights_uniform_buffer: sf.Buffer,
    local_uniform_buffer: sf.Buffer,
    texture_gbuffer_2d_f16: sf.Texture,
    texture_albedo: sf.Texture,
    texture_diffuse: sf.Texture,
    g_buffer_pipeline: zgpu.RenderPipelineHandle,
    light_pipeline: zgpu.RenderPipelineHandle,

    pub var renderer: ?*Renderer = null;
    pub var fb_width: u32 = 800;
    pub var fb_height: u32 = 600;

    pub fn create_with_gctx(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, fb_width_passed: u32, fb_height_passed: u32, out_renderer: *Renderer, vb: *sf.Buffer, ib: *sf.Buffer) !void {
        out_renderer.camera = .{};
        out_renderer.arena = std.heap.ArenaAllocator.init(allocator);
        out_renderer.vertex_buffer = vb.*;
        out_renderer.index_buffer = ib.*;
        out_renderer.depth_texture = sf.Texture.create_with_wgpu_format(gctx, .{ .render_attachment = true, .texture_binding = true }, .{ .width = gctx.swapchain_descriptor.width, .height = gctx.swapchain_descriptor.height, .depth_or_array_layers = 1 }, .depth24_plus);
        fb_height = fb_height_passed;
        fb_width = fb_width_passed;
        out_renderer.texture_gbuffer_2d_f16 = sf.Texture.create_with_wgpu_format(gctx, .{ .render_attachment = true, .texture_binding = true }, .{ .width = gctx.swapchain_descriptor.width, .height = gctx.swapchain_descriptor.height, .depth_or_array_layers = 1 }, .rgba16_float);
        out_renderer.texture_diffuse = sf.Texture.create_with_wgpu_format(gctx, .{ .render_attachment = true, .texture_binding = true }, .{ .width = gctx.swapchain_descriptor.width, .height = gctx.swapchain_descriptor.height, .depth_or_array_layers = 1 }, .rgba16_float);
        out_renderer.texture_albedo = sf.Texture.create_with_wgpu_format(gctx, .{ .render_attachment = true, .texture_binding = true }, .{ .width = gctx.swapchain_descriptor.width, .height = gctx.swapchain_descriptor.height, .depth_or_array_layers = 1 }, .bgra8_unorm);
        out_renderer.global_uniform_buffer = sf.Buffer.create(gctx, .{ .uniform = true, .copy_dst = true }, @sizeOf(sf.GlobalUniforms));
        out_renderer.lights_uniform_buffer = sf.Buffer.create(gctx, .{ .uniform = true, .copy_dst = true }, @sizeOf(sf.LightingUniform));
        out_renderer.local_uniform_buffer = sf.Buffer.create(gctx, .{ .uniform = true, .copy_dst = true }, @sizeOf(sf.Uniforms));
        out_renderer.mip_level = 0;
        // bgls
        const global_uniform_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, false, 0),
        });
        defer gctx.releaseResource(global_uniform_bgl);
        out_renderer.global_uniform_bind_group = gctx.createBindGroup(global_uniform_bgl, &.{
            .{
                .binding = 0,
                .buffer_handle = out_renderer.global_uniform_buffer.handle,
                .offset = 0,
                .size = @sizeOf(sf.GlobalUniforms),
            },
        });
        const local_uniform_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, false, 0),
        });
        defer gctx.releaseResource(local_uniform_bgl);
        out_renderer.local_uniform_bind_group = gctx.createBindGroup(local_uniform_bgl, &.{
            .{
                .binding = 0,
                .buffer_handle = out_renderer.local_uniform_buffer.handle,
                .offset = 0,
                .size = @sizeOf(sf.Uniforms),
            },
        });
        var gbuffer_textures_bgl = gctx.createBindGroupLayout(&.{
            zgpu.textureEntry(0, .{ .fragment = true }, .unfilterable_float, .tvdim_2d, false),
            zgpu.textureEntry(1, .{ .fragment = true }, .unfilterable_float, .tvdim_2d, false),
            zgpu.textureEntry(2, .{ .fragment = true }, .depth, .tvdim_2d, false),
            zgpu.textureEntry(3, .{ .fragment = true }, .unfilterable_float, .tvdim_2d, false),
        });
        defer gctx.releaseResource(gbuffer_textures_bgl);

        out_renderer.g_buffer_textures_bind_group = gctx.createBindGroup(gbuffer_textures_bgl, &.{
            .{
                .binding = 0,
                .texture_view_handle = out_renderer.texture_gbuffer_2d_f16.view,
            },
            .{
                .binding = 1,
                .texture_view_handle = out_renderer.texture_albedo.view,
            },
            .{
                .binding = 2,
                .texture_view_handle = out_renderer.depth_texture.view,
            },
            .{
                .binding = 3,
                .texture_view_handle = out_renderer.texture_diffuse.view,
            },
        });
        var lights_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .fragment = true }, .uniform, false, @as(u64, @sizeOf(sf.LightingUniform))),
        });
        defer gctx.releaseResource(lights_bgl);
        var material_bgl = gctx.createBindGroupLayout(&.{
            zgpu.textureEntry(0, .{ .fragment = true }, .float, .tvdim_2d, false),
            zgpu.samplerEntry(1, .{ .fragment = true }, .filtering),
            zgpu.bufferEntry(2, .{ .fragment = true }, .uniform, false, @sizeOf(sf.PhongData)),
        });
        defer gctx.releaseResource(material_bgl);
        out_renderer.lights_bind_group = gctx.createBindGroup(lights_bgl, &.{.{
            .binding = 0,
            .buffer_handle = out_renderer.lights_uniform_buffer.handle,
            .offset = 0,
            .size = @sizeOf(sf.LightingUniform),
        }});
        // pipelines
        var write_gbuffers_pipeline_layout = gctx.createPipelineLayout(&.{ global_uniform_bgl, local_uniform_bgl, material_bgl });
        defer gctx.releaseResource(write_gbuffers_pipeline_layout);
        var light_pipeline_layout = gctx.createPipelineLayout(&.{ gbuffer_textures_bgl, lights_bgl, global_uniform_bgl });
        defer gctx.releaseResource(light_pipeline_layout);
        const vs_gbuffer = zgpu.createWgslShaderModule(gctx.device, sf.vertexWriteGBuffers, "vs_gbuffer");
        defer vs_gbuffer.release();
        const fs_gbuffer = zgpu.createWgslShaderModule(gctx.device, sf.fragmentWriteGBuffers, "fs_gbuffer");
        defer fs_gbuffer.release();
        const g_color_targets = [_]zgpu.wgpu.ColorTargetState{
            .{
                .format = .rgba16_float,
            },
            .{
                .format = zgpu.GraphicsContext.swapchain_format,
            },
            .{
                .format = .rgba16_float,
            },
        };
        const g_vertex_attributes = [_]zgpu.wgpu.VertexAttribute{
            .{ .format = .float32x3, .offset = 0, .shader_location = 0 },
            .{ .format = .float32x3, .offset = @offsetOf(sf.Vertex, "normal"), .shader_location = 1 },
            .{ .format = .float32x4, .offset = @offsetOf(sf.Vertex, "tangent"), .shader_location = 2 },
            .{ .format = .float32x2, .offset = @offsetOf(sf.Vertex, "uv"), .shader_location = 3 },
        };
        const g_vertex_buffers = [_]zgpu.wgpu.VertexBufferLayout{.{
            .array_stride = @sizeOf(sf.Vertex),
            .attribute_count = g_vertex_attributes.len,
            .attributes = &g_vertex_attributes,
        }};
        // Create g buffer render pipeline.
        const g_pipeline_descriptor = zgpu.wgpu.RenderPipelineDescriptor{
            .vertex = .{
                .module = vs_gbuffer,
                .entry_point = "main",
                .buffer_count = g_vertex_buffers.len,
                .buffers = &g_vertex_buffers,
            },
            .primitive = .{
                .front_face = .cw,
                .cull_mode = .back,
                .topology = .triangle_list,
            },
            .depth_stencil = &.{
                .format = .depth24_plus,
                .depth_write_enabled = true,
                .depth_compare = .less,
            },
            .fragment = &.{
                .module = fs_gbuffer,
                .entry_point = "main",
                .target_count = g_color_targets.len,
                .targets = &g_color_targets,
            },
        };
        out_renderer.g_buffer_pipeline = gctx.createRenderPipeline(write_gbuffers_pipeline_layout, g_pipeline_descriptor);
        const light_color_targets = [_]zgpu.wgpu.ColorTargetState{
            .{
                .format = zgpu.GraphicsContext.swapchain_format,
                // .blend = &.{ .color = zgpu.wgpu.BlendComponent{}, .alpha = zgpu.wgpu.BlendComponent{} },
            },
        };
        const vs_light = zgpu.createWgslShaderModule(gctx.device, sf.vertexTextureQuad, "vs_light");
        defer vs_light.release();
        const fs_light = zgpu.createWgslShaderModule(gctx.device, sf.fragmentDeferredRendering, "fs_light");
        defer fs_light.release();
        // Create light render pipeline.
        const light_pipeline_descriptor = zgpu.wgpu.RenderPipelineDescriptor{
            .vertex = .{
                .module = vs_light,
                .entry_point = "main",
            },
            .primitive = .{
                .front_face = .ccw,
                .cull_mode = .back,
                .topology = .triangle_list,
            },
            .fragment = &.{
                .module = fs_light,
                .entry_point = "main",
                .target_count = light_color_targets.len,
                .targets = &light_color_targets,
            },
        };
        out_renderer.light_pipeline = gctx.createRenderPipeline(light_pipeline_layout, light_pipeline_descriptor);
        out_renderer.gctx = gctx;
        out_renderer.depth_texture = out_renderer.depth_texture;
    }

    pub fn destroy(renderer_state: *Renderer, allocator: std.mem.Allocator) void {
        renderer_state.arena.deinit();
        allocator.destroy(renderer_state);
    }

    pub fn draw_deferred_to_texture(renderer_state: *Renderer, color_view_passed: *zgpu.wgpu.TextureView, fb_width_passed: u32, fb_height_passed: u32, scene: *sf.Scene) !void {
        renderer = renderer_state;
        fb_width = fb_width_passed;
        fb_height = fb_height_passed;
        const gctx = renderer_state.gctx;
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
        const view_proj = zm.transpose(cam_world_to_clip);
        const inv_view_proj = zm.inverse(view_proj);
        const back_buffer_view = gctx.swapchain.getCurrentTextureView();
        defer back_buffer_view.release();
        const commands = commands: {
            const encoder = gctx.device.createCommandEncoder(null);
            defer encoder.release();
            // GBuffer pass.
            pass_gbuffer: {
                const vb_info = gctx.lookupResourceInfo(scene.vertex_buffer.handle) orelse break :pass_gbuffer;
                const ib_info = gctx.lookupResourceInfo(scene.index_buffer.handle) orelse break :pass_gbuffer;
                const global_uniform_buffer = gctx.lookupResource(renderer_state.global_uniform_buffer.handle) orelse break :pass_gbuffer;
                const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass_gbuffer;
                const gcolor_view = gctx.lookupResource(renderer_state.texture_gbuffer_2d_f16.view) orelse break :pass_gbuffer;
                const gdiffuse_view = gctx.lookupResource(renderer_state.texture_diffuse.view) orelse break :pass_gbuffer;
                const galbedo_view = gctx.lookupResource(renderer_state.texture_albedo.view) orelse break :pass_gbuffer;
                const global_uniform_bind_group = gctx.lookupResource(renderer_state.global_uniform_bind_group) orelse break :pass_gbuffer;
                const pipeline = gctx.lookupResource(renderer_state.g_buffer_pipeline) orelse break :pass_gbuffer;
                const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{
                    .{
                        .view = gcolor_view,
                        .load_op = .clear,
                        .store_op = .store,
                        .clear_value = .{ .r = 0.0, .g = 0.0, .b = 1.0, .a = 1.0 },
                    },
                    .{
                        .view = galbedo_view,
                        .load_op = .clear,
                        .store_op = .store,
                        .clear_value = .{ .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
                    },
                    .{
                        .view = gdiffuse_view,
                        .load_op = .clear,
                        .store_op = .store,
                        .clear_value = .{ .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
                    },
                };
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
                const gbuffer_pass = encoder.beginRenderPass(render_pass_info);
                defer {
                    gbuffer_pass.end();
                    gbuffer_pass.release();
                }
                gbuffer_pass.setPipeline(pipeline);
                gbuffer_pass.setVertexBuffer(0, vb_info.gpuobj.?, 0, vb_info.size);
                gbuffer_pass.setIndexBuffer(ib_info.gpuobj.?, .uint32, 0, ib_info.size);
                gbuffer_pass.setBindGroup(0, global_uniform_bind_group, null);
                gctx.queue.writeBuffer(global_uniform_buffer, 0, sf.GlobalUniforms, &.{
                    .{
                        .view_projection = view_proj,
                        .inv_view_projection = inv_view_proj,
                    },
                });
                var query_desc = ecs.query_desc_t{};
                query_desc.filter.terms[0] = .{ .id = ecs.id(sf.Material) };
                query_desc.filter.terms[1] = .{ .id = ecs.id(sf.components.Transform) };
                query_desc.filter.terms[2] = .{ .id = ecs.id(sf.components.Mesh) };
                query_desc.order_by_component = ecs.id(sf.Material);
                var q = try ecs.query_init(scene.world.id, &query_desc);
                var it = ecs.query_iter(scene.world.id, q);
                while (ecs.query_next(&it)) {
                    const entities = it.entities();
                    var i: usize = 0;
                    while (i < it.count()) : (i += 1) {
                        if (!ecs.is_valid(it.world, entities[i]) or !ecs.is_alive(it.world, entities[i])) continue;
                        if (ecs.field(&it, sf.components.Transform, 2)) |transforms| {
                            const transform = transforms[i];
                            if (ecs.field(&it, sf.components.Mesh, 3)) |meshes| {
                                const mesh_comp = meshes[i];
                                const object_to_world = transform.world;
                                const local_uniform_buffer = gctx.lookupResource(renderer_state.local_uniform_buffer.handle) orelse break :pass_gbuffer;
                                const local_uniform_bg = gctx.lookupResource(renderer_state.local_uniform_bind_group) orelse break :pass_gbuffer;
                                gbuffer_pass.setBindGroup(1, local_uniform_bg, null);
                                gctx.queue.writeBuffer(local_uniform_buffer, 0, sf.Uniforms, &.{
                                    .{
                                        .aspect_ratio = @as(f32, @floatFromInt(fb_width_passed)) / @as(f32, @floatFromInt(fb_height_passed)),
                                        .mip_level = @as(f32, @floatFromInt(renderer_state.mip_level)),
                                        .model = zm.transpose(object_to_world),
                                    },
                                });
                                if (ecs.field(&it, sf.Material, 1)) |materials| {
                                    const mat = materials[i];
                                    const mat_bg = gctx.lookupResource(mat.bind_group) orelse break :pass_gbuffer;
                                    gbuffer_pass.setBindGroup(2, mat_bg, null);
                                }
                                gbuffer_pass.drawIndexed(mesh_comp.num_indices, 1, mesh_comp.index_offset, mesh_comp.vertex_offset, 0);
                            }
                        }
                    }
                }
            }
            // light pass.
            light_pass: {
                const color_attachments = [_]zgpu.wgpu.RenderPassColorAttachment{.{
                    .view = color_view_passed.*,
                    .load_op = .clear,
                    .store_op = .store,
                }};
                const render_pass_info = zgpu.wgpu.RenderPassDescriptor{
                    .color_attachment_count = color_attachments.len,
                    .color_attachments = &color_attachments,
                };
                const light_pass = encoder.beginRenderPass(render_pass_info);
                defer {
                    light_pass.end();
                    light_pass.release();
                }
                const g_buffer_bg = gctx.lookupResource(renderer_state.g_buffer_textures_bind_group) orelse break :light_pass;
                const global_uniform_bg = gctx.lookupResource(renderer_state.global_uniform_bind_group) orelse break :light_pass;
                const global_uniform_buffer = gctx.lookupResource(renderer_state.global_uniform_buffer.handle) orelse break :light_pass;
                const light_pipe = gctx.lookupResource(renderer_state.light_pipeline) orelse break :light_pass;
                const light_uniform = gctx.lookupResource(renderer_state.lights_uniform_buffer.handle) orelse break :light_pass;
                const light_bg = gctx.lookupResource(renderer_state.lights_bind_group) orelse break :light_pass;
                light_pass.setPipeline(light_pipe);
                light_pass.setBindGroup(0, g_buffer_bg, null);
                light_pass.setBindGroup(1, light_bg, null);
                gctx.queue.writeBuffer(light_uniform, 0, sf.LightingUniform, &.{
                    .{
                        .position = .{ -1.0, 1.0, 1.0 },
                        .color = .{ 1.0, 1.0, 1.0 },
                    },
                });
                light_pass.setBindGroup(2, global_uniform_bg, null);
                gctx.queue.writeBuffer(global_uniform_buffer, 0, sf.GlobalUniforms, &.{
                    .{
                        .view_projection = view_proj,
                        .inv_view_projection = inv_view_proj,
                    },
                });
                light_pass.draw(6, 1, 0, 0);
            }
            break :commands encoder.finish(null);
        };
        defer commands.release();
        gctx.submit(&.{commands});
    }
};
