const std = @import("std");
const sdl = @cImport({
    @cInclude("SDL.h");
    @cInclude("SDL_vulkan.h");
});
const vk = @import("vulkan.zig");
const stdout = std.io.getStdOut().writer();
pub fn main() !void {
    _ = sdl.SDL_Init(sdl.SDL_INIT_VIDEO | sdl.SDL_INIT_EVENTS);
    defer sdl.SDL_Quit();
    var window: ?*sdl.SDL_Window = sdl.SDL_CreateWindow("hello gamedev", sdl.SDL_WINDOWPOS_CENTERED, sdl.SDL_WINDOWPOS_CENTERED, 640, 400, sdl.SDL_WINDOW_VULKAN);
    if (window == null) {
        try stdout.print("Failed to create window", .{});
        return;
    }
    defer sdl.SDL_DestroyWindow(window);
    var renderer = sdl.SDL_CreateRenderer(window, 0, sdl.SDL_RENDERER_PRESENTVSYNC);
    defer sdl.SDL_DestroyRenderer(renderer);
    // VK INSTANCE
    var instance: vk.Instance = undefined;
    const app_info = vk.ApplicationInfo{
        .p_application_name = "Sapfire",
        .application_version = vk.makeApiVersion(0, 0, 0, 0),
        .p_engine_name = "Sapfire",
        .engine_version = vk.makeApiVersion(0, 0, 0, 0),
        .api_version = vk.API_VERSION_1_2,
    };
    var create_info = vk.InstanceCreateInfo{ .s_type = vk.StructureType.instance_create_info, .p_application_info = &app_info };
    var extension_count: u32 = 0;
    var extension_names: [*c][*c]const u8 = undefined;
    // if (sdl.SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names) == sdl.SDL_FALSE) {
    //     try stdout.print("Could not get SDL Vulkan extensions", .{});
    //     return;
    // }
    _ = sdl.SDL_Vulkan_GetInstanceExtensions(window, &extension_count, @ptrCast([*c][*:0]const u8, extension_names));
    try stdout.print("{}", .{extension_count});
    create_info.enabled_extension_count = extension_count;
    create_info.pp_enabled_extension_names = extension_names;
    const result: vk.Result = vk.PfnCreateInstance(&create_info, null, &instance);
    if (result != vk.Result.success) {
        try stdout.print("Error instantiating vulkan", .{});
        return;
    }
    mainloop: while (true) {
        var sdl_event: sdl.SDL_Event = undefined;
        while (sdl.SDL_PollEvent(&sdl_event) != 0) {
            switch (sdl_event.type) {
                sdl.SDL_QUIT => break :mainloop,
                sdl.SDL_KEYDOWN => {
                    if (sdl_event.key.keysym.sym == sdl.SDLK_ESCAPE)
                        break :mainloop;
                },
                else => {},
            }
        }
    }
}
