const std = @import("std");
const zgpu = @import("libs/zig-gamedev/libs/zgpu/build.zig");
const zpool = @import("libs/zig-gamedev/libs/zpool/build.zig");
const zglfw = @import("libs/zig-gamedev/libs/zglfw/build.zig");
const zstbi = @import("libs/zig-gamedev/libs/zstbi/build.zig");
const zmath = @import("libs/zig-gamedev/libs/zmath/build.zig");
const zmesh = @import("libs/zig-gamedev/libs/zmesh/build.zig");
const zgui = @import("libs/zig-gamedev/libs/zgui/build.zig");
const zflecs = @import("libs/zig-gamedev/libs/zflecs/build.zig");
// TODO: implement our own job system based on fibers instead of threads
const zjobs = @import("libs/zig-gamedev/libs/zjobs/build.zig");
const nfd = @import("libs/nfd-zig/build.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});

    const nfd_path = "libs/nfd-zig/";
    const nfd_module = b.createModule(.{
        .source_file = .{ .path = nfd_path ++ "src/lib.zig" },
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
    const zgui_pkg = zgui.package(b, target, optimize, .{ .options = .{ .backend = .glfw_wgpu } });
    const zflecs_pkg = zflecs.package(b, target, optimize, .{});
    const sapfire_module = b.createModule(.{
        .source_file = .{ .path = "sapfire/src/sapfire.zig" },
    });
    sapfire_module.dependencies.put("zgpu", zgpu_pkg.zgpu) catch {};
    sapfire_module.dependencies.put("zglfw", zglfw_pkg.zglfw) catch {};
    sapfire_module.dependencies.put("zpool", zpool_pkg.zpool) catch {};
    sapfire_module.dependencies.put("zstbi", zstbi_pkg.zstbi) catch {};
    sapfire_module.dependencies.put("zmath", zmath_pkg.zmath) catch {};
    sapfire_module.dependencies.put("zjobs", zjobs_pkg.zjobs) catch {};
    sapfire_module.dependencies.put("zgui", zgui_pkg.zgui) catch {};
    sapfire_module.dependencies.put("zmesh", zmesh_pkg.zmesh) catch {};
    sapfire_module.dependencies.put("zflecs", zflecs_pkg.zflecs) catch {};
    sapfire_module.dependencies.put("nfd", nfd_module) catch {};

    const game_exe = b.addExecutable(.{
        .name = "Sandbox",
        .root_source_file = .{ .path = "sandbox/src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    zgpu_pkg.link(game_exe);
    zglfw_pkg.link(game_exe);
    zpool_pkg.link(game_exe);
    zstbi_pkg.link(game_exe);
    zmath_pkg.link(game_exe);
    zmesh_pkg.link(game_exe);
    zjobs_pkg.link(game_exe);
    zgui_pkg.link(game_exe);
    zflecs_pkg.link(game_exe);
    game_exe.addModule("sapfire", sapfire_module);

    const game_artifact = b.addInstallArtifact(game_exe, .{});
    b.getInstallStep().dependOn(&game_artifact.step);

    const editor_exe = b.addExecutable(.{
        .name = "Sapling",
        .root_source_file = .{ .path = "sapling/src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    zgpu_pkg.link(editor_exe);
    zglfw_pkg.link(editor_exe);
    zpool_pkg.link(editor_exe);
    zstbi_pkg.link(editor_exe);
    zmath_pkg.link(editor_exe);
    zmesh_pkg.link(editor_exe);
    zjobs_pkg.link(editor_exe);
    zgui_pkg.link(editor_exe);
    zflecs_pkg.link(editor_exe);
    editor_exe.addModule("sapfire", sapfire_module);

    const nfd_lib = b.addStaticLibrary(.{
        .name = "nfd",
        .root_source_file = .{ .path = nfd_path ++ "src/lib.zig" },
        .target = target,
        .optimize = optimize,
        .main_pkg_path = .{ .path = "." },
    });
    nfd_lib.addModule("nfd", nfd_module);

    const cflags = [_][]const u8{"-Wall"};
    nfd_lib.addIncludePath(.{ .path = nfd_path ++ "nativefiledialog/src/include" });
    nfd_lib.addCSourceFile(.{ .file = .{ .path = nfd_path ++ "nativefiledialog/src/nfd_common.c" }, .flags = &cflags });
    if (nfd_lib.target.isDarwin()) {
        nfd_lib.addCSourceFile(.{ .file = .{ .path = nfd_path ++ "nativefiledialog/src/nfd_cocoa.m" }, .flags = &cflags });
    } else if (nfd_lib.target.isWindows()) {
        nfd_lib.addCSourceFile(.{ .file = .{ .path = nfd_path ++ "nativefiledialog/src/nfd_win.cpp" }, .flags = &cflags });
    } else {
        nfd_lib.addCSourceFile(.{ .file = .{ .path = nfd_path ++ "nativefiledialog/src/nfd_gtk.c" }, .flags = &cflags });
    }

    nfd_lib.linkLibC();
    if (nfd_lib.target.isDarwin()) {
        nfd_lib.linkFramework("AppKit");
    } else if (nfd_lib.target.isWindows()) {
        nfd_lib.linkSystemLibrary("shell32");
        nfd_lib.linkSystemLibrary("ole32");
        nfd_lib.linkSystemLibrary("uuid"); // needed by MinGW
    } else {
        nfd_lib.linkSystemLibrary("atk-1.0");
        nfd_lib.linkSystemLibrary("gdk-3");
        nfd_lib.linkSystemLibrary("gtk-3");
        nfd_lib.linkSystemLibrary("glib-2.0");
        nfd_lib.linkSystemLibrary("gobject-2.0");
    }
    nfd_lib.installHeadersDirectory(nfd_path ++ "nativefiledialog/src/include", ".");
    b.installArtifact(nfd_lib);
    editor_exe.addIncludePath(.{ .path = "nativefiledialog/src/include" });
    editor_exe.addModule("nfd", nfd_module);
    editor_exe.linkLibrary(nfd_lib);
    const nfd_art = b.addInstallArtifact(nfd_lib, .{});
    b.getInstallStep().dependOn(&nfd_art.step);

    const editor_artifact = b.addInstallArtifact(editor_exe, .{});
    b.getInstallStep().dependOn(&editor_artifact.step);

    const run_cmd_game = b.addRunArtifact(game_exe);

    run_cmd_game.step.dependOn(&editor_artifact.step);

    if (b.args) |args| {
        run_cmd_game.addArgs(args);
    }
    const run_game_step = b.step("run-game", "Run the sandbox");
    run_game_step.dependOn(&run_cmd_game.step);

    const run_cmd_editor = b.addRunArtifact(editor_exe);

    run_cmd_editor.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd_editor.addArgs(args);
    }

    const run_editor_step = b.step("run-editor", "Run the editor");
    run_editor_step.dependOn(&run_cmd_editor.step);
}
