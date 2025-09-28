const std = @import("std");
const mos = @import("mos_build.zig");

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{});
    const target = b.resolveTargetQuery(mos.i686_target_query);
    const options = try mos.addDefaultOptions(b);
    const exe = mos.addExecutable(b, "zello", .{
        .optimize = optimize,
        .root_src_file = "./main.zig",
        .options = options,
        .target = target,
    });

    const zm = b.dependency("zm", .{}).module("zm");
    mos.addDefaultDependencies(b, zm, options);
    exe.compilation_step.root_module.addImport("zm",zm);
    b.getInstallStep().dependOn(exe.root_step);
}
