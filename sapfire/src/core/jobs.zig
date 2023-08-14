const std = @import("std");
const zjobs = @import("zjobs");
pub const Jobs = zjobs.JobQueue(.{});
pub const JobId = zjobs.JobId;

pub const JobsManager = struct {
    jobs: Jobs,

    var instance: JobsManager = undefined;

    pub fn init() void {
        instance.jobs = Jobs.init();
    }

    pub fn deinit() void {
        instance.jobs.deinit();
    }

    pub fn jobs() *Jobs {
        return &instance.jobs;
    }
};
