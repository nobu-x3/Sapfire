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
    desc.run = run;
    return desc;
}

pub fn query() ecs.query_desc_t {
    var query_desc: ecs.query_desc_t = .{};
    return query_desc;
}

pub fn run(it: *ecs.iter_t) callconv(.C) void {
    _ = it;
    const renderer_state = renderer.renderer.?;
    const color_view_passed = renderer.color_view.?;
    const fb_width = renderer.fb_width;
    const fb_height = renderer.fb_height;
    const gctx = renderer_state.gctx;
    const encoder = renderer.encoder.?;
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
                .position = .{ -0.5, 0.0, 4.0, 1.0 },
                .color = .{ 1.0, 0.0, 0.0, 1.0 },
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
    const commands = encoder.finish(null);
    defer commands.release();
    defer encoder.release();
    renderer.encoder = null;
    gctx.submit(&.{commands});
}
