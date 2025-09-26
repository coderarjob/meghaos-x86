const std = @import("std");
const mos = @import("mos_build.zig");

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{});
    const exe = mos.addExecutable(b, "hello", .{
        .optimize = optimize,
        .root_src_file = "./hello.zig",
        .options = try mos.addDefaultOptions(b),
        .target = b.resolveTargetQuery(mos.i686_target_query),
    });
    b.getInstallStep().dependOn(exe);
}
