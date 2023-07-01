const std = @import("std");
const time = std.time;

pub const Time = struct {
    delta_time: f32,
    current_ts: i64,
    prev_ts: i64,
    pub fn init() void {
        instance.current_ts = time.microTimestamp();
        instance.delta_time = 0.0;
        instance.prev_ts = time.microTimestamp();
    }

    pub fn update() void {
        instance.current_ts = time.microTimestamp();
        instance.delta_time = @intToFloat(f32, instance.current_ts - instance.prev_ts) / @as(f32, time.us_per_s);
        instance.prev_ts = instance.current_ts;
    }

    pub fn delta_time() f32 {
        return instance.delta_time;
    }
};

var instance: Time = undefined;
