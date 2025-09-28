const std = @import("std");
const cm = @cImport({
    @cInclude("syscall.h");
    @cInclude("debug.h");
});

pub inline fn console_writeline(msg: [*:0]const u8) void {
    cm.cm_putstr(@constCast(msg));
}
