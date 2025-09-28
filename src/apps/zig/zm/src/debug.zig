const std = @import("std");
const config = @import("config");
const cm = @cImport({
    @cInclude("syscall.h");
    @cInclude("debug.h");
});

pub inline fn console_writeline(msg: [*:0]const u8) void {
    cm.cm_putstr(@constCast(msg));
}

pub inline fn isGraphicsMode() bool {
    return config.graphics_enabled == .ON;
}

pub inline fn isDebugMode() bool {
    return config.build_mode == .DEBUG;
}

pub inline fn isPortE9Enabled() bool {
    return config.port_e9_enabled == .ON;
}
