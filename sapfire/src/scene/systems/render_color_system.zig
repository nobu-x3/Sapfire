const std = @import("std");
const zm = @import("zmath");
const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const components = @import("../components.zig");
const Material = @import("../../renderer/material.zig").Material;
const renderer = @import("../../renderer/renderer.zig").RendererState;
const Pipeline = @import("../../renderer/pipeline.zig").Pipeline;
const sf = struct {
    usingnamespace @import("../../renderer/renderer_types.zig");
};

pub fn system() ecs.system_desc_t {
    var desc: ecs.system_desc_t = .{};
    desc.query.filter.terms[0] = .{ .id = ecs.id(Material) };
    desc.query.filter.terms[1] = .{ .id = ecs.id(components.Transform) };
    desc.query.filter.terms[2] = .{ .id = ecs.id(components.Mesh) };
    desc.query.order_by_component = ecs.id(Material);
    desc.run = run;
    return desc;
}

pub fn query() ecs.query_desc_t {
    var query_desc: ecs.query_desc_t = .{};
    query.filter.terms[0] = .{ .id = ecs.id(Material) };
    query.filter.terms[1] = .{ .id = ecs.id(components.Transform) };
    query.filter.terms[2] = .{ .id = ecs.id(components.Mesh) };
    query.order_by_component = ecs.id(Material);
    return query_desc;
}

pub fn run(it: *ecs.iter_t) callconv(.C) void {
    const renderer_state = renderer.renderer.?;
    const color_view = renderer.color_view.?;
    const fb_width = renderer.fb_width;
    const fb_height = renderer.fb_height;
    const gctx = renderer_state.gctx;
    const scene = @import("../../scene/scene.zig").Scene.scene.?;
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
            const vb_info = gctx.lookupResourceInfo(scene.vertex_buffer) orelse break :pass;
            const ib_info = gctx.lookupResourceInfo(scene.index_buffer) orelse break :pass;
            const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass;
            const global_uniform_bind_group = gctx.lookupResource(scene.global_uniform_bind_group) orelse break :pass;
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
            const glob = gctx.uniformsAllocate(sf.GlobalUniforms, 1);
            glob.slice[0] = .{
                .view_projection = zm.transpose(cam_world_to_clip),
            };
            pass.setBindGroup(0, global_uniform_bind_group, &.{glob.offset});
            while (ecs.query_next(it)) {
                const entities = it.entities();
                var i: usize = 0;
                while (i < it.count()) : (i += 1) {
                    if (!ecs.is_valid(it.world, entities[i]) or !ecs.is_alive(it.world, entities[i])) continue;
                    var current_pipeline: Pipeline = undefined;
                    var current_material: Material = undefined;
                    if (ecs.field(it, Material, 1)) |materials| {
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
                        if (ecs.field(it, components.Transform, 2)) |transforms| {
                            const transform = transforms[i];
                            if (ecs.field(it, components.Mesh, 3)) |meshes| {
                                const mesh_comp = meshes[i];
                                const object_to_world = transform.world;
                                const mem = gctx.uniformsAllocate(sf.Uniforms, 1);
                                mem.slice[0] = .{
                                    .aspect_ratio = @as(f32, @floatFromInt(fb_width)) / @as(f32, @floatFromInt(fb_height)),
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
