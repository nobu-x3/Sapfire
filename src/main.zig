const std = @import("std");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const stbi = @import("zstbi");
const Vertex = extern struct {
    position: [2]f32,
    uv: [2]f32,
};

// zig fmt: off
const wgsl_common =
\\  struct Uniforms {
\\      aspect_ratio: f32,
\\      mip_level: f32,
\\  }
\\  @group(0) @binding(0) var<uniform> uniforms: Uniforms;
;
const wgsl_vs = wgsl_common ++
\\  struct VertexOut {
\\      @builtin(position) position_clip: vec4<f32>,
\\      @location(0) uv: vec2<f32>,
\\  }
\\  @stage(vertex) fn main(
\\      @location(0) position: vec2<f32>,
\\      @location(1) uv: vec2<f32>,
\\  ) -> VertexOut {
\\      let p = vec2(position.x / uniforms.aspect_ratio, position.y);
\\      var output: VertexOut;
\\      output.position_clip = vec4(p, 0.0, 1.0);
\\      output.uv = uv;
\\      return output;
\\  }
;
const wgsl_fs = wgsl_common ++
\\  @group(0) @binding(1) var image: texture_2d<f32>;
\\  @group(0) @binding(2) var image_sampler: sampler;
\\  @stage(fragment) fn main(
\\      @location(0) uv: vec2<f32>,
\\  ) -> @location(0) vec4<f32> {
\\      return textureSampleLevel(image, image_sampler, uv, uniforms.mip_level);
\\  }
// zig fmt: on
;
const Uniforms = extern struct {
    aspect_ratio: f32,
    mip_level: f32,
};

const Texture = struct { handle: zgpu.TextureHandle, view: zgpu.TextureViewHandle };

const RendererState = struct {
    gctx: *zgpu.GraphicsContext,
    pipeline: zgpu.RenderPipelineHandle = .{},
    bind_group: zgpu.BindGroupHandle,
    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,
    texture: Texture,
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
    const vertex_buffer = gctx.createBuffer(.{
        .usage = .{ .copy_dst = true, .vertex = true },
        .size = vertex_data.len * @sizeOf(Vertex),
    });
    gctx.queue.writeBuffer(gctx.lookupResource(vertex_buffer).?, 0, Vertex, vertex_data[0..]);
    // Create an index buffer.
    const index_data = [_]u16{ 0, 1, 3, 1, 2, 3 };
    const index_buffer = gctx.createBuffer(.{
        .usage = .{ .copy_dst = true, .index = true },
        .size = index_data.len * @sizeOf(u16),
    });
    gctx.queue.writeBuffer(gctx.lookupResource(index_buffer).?, 0, u16, index_data[0..]);
    stbi.init(arena);
    defer stbi.deinit();
    var image = try stbi.Image.loadFromFile("assets/textures/" ++ "genart_0025_5.png", 4);
    defer image.deinit();
    // Create a texture.
    const texture_handle = gctx.createTexture(.{
        .usage = .{ .texture_binding = true, .copy_dst = true },
        .size = .{
            .width = image.width,
            .height = image.height,
            .depth_or_array_layers = 1,
        },
        .format = zgpu.imageInfoToTextureFormat(
            image.num_components,
            image.bytes_per_component,
            image.is_hdr,
        ),
        .mip_level_count = std.math.log2_int(u32, std.math.max(image.width, image.height)) + 1,
    });
    const texture_view = gctx.createTextureView(texture_handle, .{});
    gctx.queue.writeTexture(
        .{ .texture = gctx.lookupResource(texture_handle).? },
        .{
            .bytes_per_row = image.bytes_per_row,
            .rows_per_image = image.height,
        },
        .{ .width = image.width, .height = image.height },
        u8,
        image.data,
    );
    const texture = Texture{ .handle = texture_handle, .view = texture_view };
    // Create a sampler.
    const sampler = gctx.createSampler(.{});
    const bind_group = gctx.createBindGroup(bind_group_layout, &.{
        .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = 256 },
        .{ .binding = 1, .texture_view_handle = texture_view },
        .{ .binding = 2, .sampler_handle = sampler },
    });
    const renderer_state = try allocator.create(RendererState);
    renderer_state.* = .{
        .gctx = gctx,
        .bind_group = bind_group,
        .vertex_buffer = vertex_buffer,
        .index_buffer = index_buffer,
        .texture = texture,
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
    renderer_pipeline_create(allocator, gctx, &.{bind_group_layout}, &renderer_state.pipeline);
    return renderer_state;
}

fn renderer_pipeline_create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, bind_group_layout: []const zgpu.BindGroupLayoutHandle, out_pipeline_handle: *zgpu.RenderPipelineHandle) void {
    const pipeline_layout = gctx.createPipelineLayout(bind_group_layout);
    defer gctx.releaseResource(pipeline_layout);
    const vs_module = zgpu.createWgslShaderModule(gctx.device, wgsl_vs, "vs");
    defer vs_module.release();
    const fs_module = zgpu.createWgslShaderModule(gctx.device, wgsl_fs, "fs");
    defer fs_module.release();
    const color_targets = [_]zgpu.wgpu.ColorTargetState{.{
        .format = zgpu.GraphicsContext.swapchain_format,
    }};
    const vertex_attributes = [_]zgpu.wgpu.VertexAttribute{
        .{ .format = .float32x2, .offset = 0, .shader_location = 0 },
        .{ .format = .float32x2, .offset = @offsetOf(Vertex, "uv"), .shader_location = 1 },
    };
    const vertex_buffers = [_]zgpu.wgpu.VertexBufferLayout{.{
        .array_stride = @sizeOf(Vertex),
        .attribute_count = vertex_attributes.len,
        .attributes = &vertex_attributes,
    }};
    // Create a render pipeline.
    const pipeline_descriptor = zgpu.wgpu.RenderPipelineDescriptor{
        .vertex = .{
            .module = vs_module,
            .entry_point = "main",
            .buffer_count = vertex_buffers.len,
            .buffers = &vertex_buffers,
        },
        .primitive = .{
            .front_face = .cw,
            .cull_mode = .back,
            .topology = .triangle_list,
        },
        .fragment = &.{
            .module = fs_module,
            .entry_point = "main",
            .target_count = color_targets.len,
            .targets = &color_targets,
        },
    };
    gctx.createRenderPipelineAsync(allocator, pipeline_layout, pipeline_descriptor, out_pipeline_handle);
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
