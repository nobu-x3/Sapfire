const std = @import("std");
const glfw = @import("zglfw");
const wgpu = @import("zgpu");

const Renderer = struct {
    context: *wgpu.GraphicsContext,
};

pub fn init(allocator: std.mem.Allocator, window: *glfw.Window) !*wgpu.GraphicsContext {
    return try wgpu.GraphicsContext.create(allocator, window);
}
pub fn main() !void {
    glfw.init() catch {
        std.log.err("Failed to initialize glfw", .{});
        return;
    };
    defer glfw.terminate();
    const window: *glfw.Window = glfw.Window.create(800, 600, "test", null) catch {
        std.log.err("Failed to initialize the window", .{});
        return;
    };
    defer window.destroy();
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    const context = try init(allocator, window);
    defer context.destroy(allocator);

    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        const back_buffer_view = context.swapchain.getCurrentTextureView();
        const commands = commands: {
            const encoder = context.device.createCommandEncoder(null);
            defer encoder.release();

            {
                const color_attachments = [_]wgpu.wgpu.RenderPassColorAttachment{.{
                    .view = back_buffer_view,
                    .load_op = .clear,
                    .store_op = .store,
                }};
                const render_pass_info = wgpu.wgpu.RenderPassDescriptor{
                    .color_attachment_count = color_attachments.len,
                    .color_attachments = &color_attachments,
                    .depth_stencil_attachment = null,
                };
                const pass = encoder.beginRenderPass(render_pass_info);
                defer {
                    pass.end();
                    pass.release();
                }
            }
            // {
            //     const color_attachments = [_]wgpu.wgpu.RenderPassColorAttachment{.{
            //         .view = back_buffer_view,
            //         .load_op = .load,
            //         .store_op = .store,
            //     }};
            //     const render_pass_info = wgpu.wgpu.RenderPassDescriptor{
            //         .color_attachment_count = color_attachments.len,
            //         .color_attachments = &color_attachments,
            //     };
            //     const pass = encoder.beginRenderPass(render_pass_info);
            //     defer {
            //         pass.end();
            //         pass.release();
            //     }

            //     // zgui.backend.draw(pass);
            // }

            break :commands encoder.finish(null);
        };
        defer commands.release();

        context.submit(&.{commands});
    }
}
