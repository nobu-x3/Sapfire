const std = @import("std");
const time = std.time;

pub const Time = struct {
    delta_time: f32,
    time_scale: f32 = 1.0,
    current_ts: i64,
    prev_ts: i64,

    var instance: Time = undefined;

    pub fn init() void {
        instance.current_ts = time.microTimestamp();
        instance.delta_time = 0.0;
        instance.time_scale = 1.0;
        instance.prev_ts = time.microTimestamp();
    }

    pub fn update() void {
        instance.current_ts = time.microTimestamp();
        instance.delta_time = @as(f32, @floatFromInt(instance.current_ts - instance.prev_ts)) / @as(f32, time.us_per_s) * instance.time_scale;
        instance.prev_ts = instance.current_ts;
    }

    pub fn delta_time() f32 {
        return instance.delta_time;
    }

    pub fn time_scale() *f32 {
        return &instance.time_scale;
    }
};
