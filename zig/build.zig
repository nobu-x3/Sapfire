const std = @import("std");
const vkgen = @import("libs/vulkan-zig/generator/index.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const exe = b.addExecutable(.{
        .name = "sapfire",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    const targetOS = (std.zig.system.NativeTargetInfo.detect(exe.target) catch unreachable).target;
    // Create a step that generates vk.zig (stored in zig-cache) from the provided vulkan registry.
    // const gen = vkgen.VkGenerateStep.create(b, "libs/Vulkan/vk.xml");
    const sdl_path_origin = "libs/SDL2/";
    _ = sdl_path_origin;
    switch (targetOS.os.tag) {
        .macos => {
            // const sdl_path = sdl_path_origin ++ "macos/";
            // exe.addIncludePath(sdl_path ++ "Headers/");
            // b.installBinFile(sdl_path ++ "SDL2", "SDL2");
            // exe.linkLibC();
        },
        else => {},
    }

    // Add the generated file as package to the final executable
    // exe.addModule("vulkan", gen.getModule());
    exe.linkSystemLibrary("sdl2");
    exe.linkSystemLibrary("vulkan");
    exe.install();

    const run_cmd = exe.run();

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Creates a step for unit testing.
    const exe_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
