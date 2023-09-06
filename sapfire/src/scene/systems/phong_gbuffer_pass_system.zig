const std = @import("std");
const zm = @import("zmath");
const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const components = @import("../components.zig");
const sf = struct {
    usingnamespace @import("../../rendering.zig");
};

const renderer = sf.Renderer;
const Material = sf.Material;
const Pipeline = sf.Pipeline;

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
    const view_proj = zm.transpose(cam_world_to_clip);
    const inv_view_proj = zm.inverse(view_proj);
    renderer.encoder = gctx.device.createCommandEncoder(null);
    // GBuffer pass.
    pass_gbuffer: {
        const vb_info = gctx.lookupResourceInfo(scene.vertex_buffer.handle) orelse break :pass_gbuffer;
        const ib_info = gctx.lookupResourceInfo(scene.index_buffer.handle) orelse break :pass_gbuffer;
        const global_uniform_buffer = gctx.lookupResource(renderer_state.global_uniform_buffer.handle) orelse break :pass_gbuffer;
        const depth_view = gctx.lookupResource(renderer_state.depth_texture.view) orelse break :pass_gbuffer;
        const gcolor_view = gctx.lookupResource(renderer_state.texture_gbuffer_2d_f16.view) orelse break :pass_gbuffer;
        const gphong_view = gctx.lookupResource(renderer_state.texture_phong_data.view) orelse break :pass_gbuffer;
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
                .view = gphong_view,
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
        const gbuffer_pass = renderer.encoder.?.beginRenderPass(render_pass_info);
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
        while (ecs.query_next(it)) {
            const entities = it.entities();
            var i: usize = 0;
            while (i < it.count()) : (i += 1) {
                if (!ecs.is_valid(it.world, entities[i]) or !ecs.is_alive(it.world, entities[i])) continue;
                if (ecs.field(it, components.Transform, 2)) |transforms| {
                    const transform = transforms[i];
                    if (ecs.field(it, components.Mesh, 3)) |meshes| {
                        const mesh_comp = meshes[i];
                        const object_to_world = transform.world;
                        const model_matrix = zm.transpose(object_to_world);
                        const invert_model: zm.Mat = zm.inverse(model_matrix);
                        const local_uniform_buffer = gctx.lookupResource(renderer_state.local_uniform_buffer.handle) orelse break :pass_gbuffer;
                        const local_uniform_bg = gctx.lookupResource(renderer_state.local_uniform_bind_group) orelse break :pass_gbuffer;
                        gbuffer_pass.setBindGroup(1, local_uniform_bg, null);
                        gctx.queue.writeBuffer(local_uniform_buffer, 0, sf.Uniforms, &.{
                            .{
                                .aspect_ratio = @as(f32, @floatFromInt(fb_width)) / @as(f32, @floatFromInt(fb_height)),
                                .mip_level = @as(f32, @floatFromInt(renderer_state.mip_level)),
                                .model = model_matrix,
                                .normal_model = invert_model,
                            },
                        });
                        if (ecs.field(it, sf.Material, 1)) |materials| {
                            const mat = materials[i];
                            const mat_bg = gctx.lookupResource(mat.bind_group) orelse break :pass_gbuffer;
                            const mat_buffer = gctx.lookupResource(mat.buffer.handle) orelse break :pass_gbuffer;
                            gbuffer_pass.setBindGroup(2, mat_bg, null);
                            gctx.queue.writeBuffer(mat_buffer, 0, sf.PhongData, &.{mat.phong_data});
                        }
                        gbuffer_pass.drawIndexed(mesh_comp.num_indices, 1, mesh_comp.index_offset, mesh_comp.vertex_offset, 0);
                    }
                }
            }
        }
    }
}
