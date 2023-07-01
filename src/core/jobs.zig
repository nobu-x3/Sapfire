const std = @import("std");
const zjobs = @import("zjobs");
const Jobs = zjobs.JobQueue(.{});
pub const JobId = zjobs.JobId;

pub const JobsManager = struct {
    jobs: Jobs,

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

var instance: JobsManager = undefined;
