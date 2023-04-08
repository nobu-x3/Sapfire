const std = @import("std");
const sdl = @cImport({
    @cInclude("SDL.h");
});
const vk = @import("vulkan");
const stdout = std.io.getStdOut().writer();
pub fn main() !void {
    _ = sdl.SDL_Init(sdl.SDL_INIT_VIDEO);
    defer sdl.SDL_Quit();
    var window = sdl.SDL_CreateWindow("hello gamedev", sdl.SDL_WINDOWPOS_CENTERED, sdl.SDL_WINDOWPOS_CENTERED, 640, 400, 0);
    if (window == null) {
        try stdout.print("Failed to create window", .{});
        return;
    }
    defer sdl.SDL_DestroyWindow(window);
    var renderer = sdl.SDL_CreateRenderer(window, 0, sdl.SDL_RENDERER_PRESENTVSYNC);
    defer sdl.SDL_DestroyRenderer(renderer);
    // VK INSTANCE
    const app_info = vk.ApplicationInfo{
        .p_application_name = "Sapfire",
        .application_version = vk.makeApiVersion(0, 0, 0, 0),
        .p_engine_name = "Sapfire",
        .engine_version = vk.makeApiVersion(0, 0, 0, 0),
        .api_version = vk.API_VERSION_1_2,
    };
    _ = app_info;
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
