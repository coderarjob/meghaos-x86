const std = @import("std");
const MosBuild = @import("MosBuild.zig");

pub fn build(b: *std.Build) !void {
    const mos_build = try MosBuild.init(b);
    const hello_install = mos_build.addExecutable("hello", "./hello.zig");
    b.getInstallStep().dependOn(hello_install);
}
