const std = @import("std");
const cm = @cImport({
    @cInclude("cm.h");
});

pub fn exit(code: u16) void {
    cm.cm_process_kill(code);
}
