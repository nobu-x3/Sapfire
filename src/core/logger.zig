const std = @import("std");
const builtin = @import("builtin");
const log = std.log;

const LoggingSystem = struct {
    handle: std.fs.File,
    debug_logging: bool = true,
};

var instance: LoggingSystem = LoggingSystem{
    .handle = undefined,
};

pub const debug_enabled: bool = builtin.mode == .Debug;

pub fn init() !void {
    var buf: [128]u8 = undefined;
    const path = try std.fmt.bufPrint(&buf, "logs/log_{}", .{std.time.timestamp()});
    instance.handle = try std.fs.cwd().createFile(path, .{ .read = true });
    instance.debug_logging = debug_enabled;
}

pub fn deinit() void {
    instance.handle.close();
}

pub fn debug(comptime format: []const u8, args: anytype) void {
    if (!instance.debug_logging) return;
    var buf: [256]u8 = undefined;
    const str = std.fmt.bufPrint(&buf, "[DEBUG]:\t" ++ format ++ "\n", args) catch {
        var wider_buf: [512]u8 = undefined;
        const str = std.fmt.bufPrint(&wider_buf, "[DEBUG]:\t" ++ format ++ "\n", args) catch {
            std.log.err("Message too long", .{});
            return;
        };
        _ = instance.handle.write(str) catch |e| {
            std.log.err("Failed to write to log file. Error: {}", .{e});
        };
        log.debug(format, args);
        return;
    };
    _ = instance.handle.write(str) catch |e| {
        std.log.err("Failed to write to log file. Error: {}", .{e});
    };
    log.debug(format, args);
}

pub fn err(comptime format: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const str = std.fmt.bufPrint(&buf, "[ERROR]:\t" ++ format ++ "\n", args) catch {
        var wider_buf: [512]u8 = undefined;
        const str = std.fmt.bufPrint(&wider_buf, "[ERROR]:\t" ++ format ++ "\n", args) catch {
            std.log.err("Message too long", .{});
            return;
        };
        _ = instance.handle.write(str) catch |e| {
            std.log.err("Failed to write to log file. Error: {}", .{e});
        };
        log.err(format, args);
        return;
    };
    _ = instance.handle.write(str) catch |e| {
        std.log.err("Failed to write to log file. Error: {}", .{e});
    };
    log.err(format, args);
}

pub fn warn(comptime format: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const str = std.fmt.bufPrint(&buf, "[WARN]:\t" ++ format ++ "\n", args) catch {
        var wider_buf: [512]u8 = undefined;
        const str = std.fmt.bufPrint(&wider_buf, "[WARN]:\t" ++ format ++ "\n", args) catch {
            std.log.err("Message too long", .{});
            return;
        };
        _ = instance.handle.write(str) catch |e| {
            std.log.err("Failed to write to log file. Error: {}", .{e});
        };
        log.warn(format, args);
        return;
    };
    _ = instance.handle.write(str) catch |e| {
        std.log.err("Failed to write to log file. Error: {}", .{e});
    };
    log.warn(format, args);
}

pub fn info(comptime format: []const u8, args: anytype) void {
    var buf: [256]u8 = undefined;
    const str = std.fmt.bufPrint(&buf, "[INFO]:\t" ++ format ++ "\n", args) catch {
        var wider_buf: [512]u8 = undefined;
        const str = std.fmt.bufPrint(&wider_buf, "[INFO]:\t" ++ format ++ "\n", args) catch {
            std.log.err("Message too long", .{});
            return;
        };
        _ = instance.handle.write(str) catch |e| {
            std.log.err("Failed to write to log file. Error: {}", .{e});
        };
        log.info(format, args);
        return;
    };
    _ = instance.handle.write(str) catch |e| {
        std.log.err("Failed to write to log file. Error: {}", .{e});
    };
    log.info(format, args);
}
