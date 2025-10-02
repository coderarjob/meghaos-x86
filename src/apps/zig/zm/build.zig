const std = @import("std");
const mos = @import("mos_build.zig");

pub fn build(b: *std.Build) !void {
    _ = mos.addModule(b, "zm", .{
        .optimize = b.standardOptimizeOption(.{}),
        .options = try mos.addCMakeExportOptions(b),
        .root_src_file = "src/root.zig",
        .target = b.resolveTargetQuery(mos.i686_target_query),
    });
}
