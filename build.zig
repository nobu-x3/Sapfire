const std = @import("std");
const zgpu = @import("libs/zig-gamedev/libs/zgpu/build.zig");
const zpool = @import("libs/zig-gamedev/libs/zpool/build.zig");
const zglfw = @import("libs/zig-gamedev/libs/zglfw/build.zig");
const zstbi = @import("libs/zig-gamedev/libs/zstbi/build.zig");
const zmath = @import("libs/zig-gamedev/libs/zmath/build.zig");
const zmesh = @import("libs/zig-gamedev/libs/zmesh/build.zig");
// TODO: implement our own job system based on fibers instead of threads
const zjobs = @import("libs/zig-gamedev/libs/zjobs/build.zig");
// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be sap_executed by an external
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

    const sapfire_lib = b.addSharedLibrary(.{
        .name = "Sapfire",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "sapfire/src/lib.zig" },
        .target = target,
        .optimize = optimize,
    });

    const zglfw_pkg = zglfw.package(b, target, optimize, .{});
    const zpool_pkg = zpool.package(b, target, optimize, .{});
    const zgpu_pkg = zgpu.package(b, target, optimize, .{
        .deps = .{ .zpool = zpool_pkg.zpool, .zglfw = zglfw_pkg.zglfw },
    });
    const zjobs_pkg = zjobs.package(b, target, optimize, .{});
    const zstbi_pkg = zstbi.package(b, target, optimize, .{});
    const zmath_pkg = zmath.package(b, target, optimize, .{});
    const zmesh_pkg = zmesh.package(b, target, optimize, .{});
    zgpu_pkg.link(sapfire_lib);
    zglfw_pkg.link(sapfire_lib);
    zpool_pkg.link(sapfire_lib);
    zstbi_pkg.link(sapfire_lib);
    zmath_pkg.link(sapfire_lib);
    zmesh_pkg.link(sapfire_lib);
    zjobs_pkg.link(sapfire_lib);

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(sapfire_lib);

    const sapfire_module = b.addModule("sapfire", .{ .source_file = .{ .path = "sapfire/src/lib.zig" } });

    const sapling_exe = b.addExecutable(.{
        .name = "Sapling",
        // In this case the main source file is merely a path, however, in more
        // complicated build scripts, this could be a generated file.
        .root_source_file = .{ .path = "sapling/src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    sapling_exe.addModule("sapfire", sapfire_module);
    sapling_exe.linkLibrary(sapfire_lib);

    // This declares intent for the sap_executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    b.installArtifact(sapling_exe);

    // This *creates* a Run step in the build graph, to be sap_executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = b.addRunArtifact(sapling_exe);

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
}
