const std = @import("std");
// const zgpu = @import("libs/zgpu/build.zig");
// const zpool = @import("libs/zpool/build.zig");
// const zglfw = @import("libs/zglfw/build.zig");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});
    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "sapfire",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const targetOS = (std.zig.system.NativeTargetInfo.detect(exe.target) catch unreachable).target;

    // const zglfw_pkg = zglfw.package(b, target, optimize, .{});
    // const zpool_pkg = zpool.package(b, target, optimize, .{});
    // const zgpu_pkg = zgpu.package(b, target, optimize, .{
    //     .deps = .{ .zpool = zpool_pkg.zpool, .zglfw = zglfw_pkg.zglfw },
    // });
    // zgpu_pkg.link(exe);
    // zglfw_pkg.link(exe);

    const sdl_path_origin = "libs/SDL2/";
    switch (targetOS.os.tag) {
        .macos => {
            const sdl_path = sdl_path_origin ++ "macos/";
            exe.addIncludePath(sdl_path ++ "Headers/");
            b.installBinFile(sdl_path ++ "SDL2", "SDL2");
            exe.linkSystemLibrary("sdl2");
            exe.linkLibC();
        },
        else => {},
    }
    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    exe.install();

    // This *creates* a RunStep in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = exe.run();

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    run_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Creates a step for unit testing.
    const exe_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
