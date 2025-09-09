const std = @import("std");
const builtin = std.builtin;
const cm = @cImport({
    @cInclude("cm.h");
});

const text = "Test application";

pub fn panic(_: []const u8, _: ?*builtin.StackTrace, _: ?usize) noreturn {
    cm.cm_process_abort(cm.CM_ABORT_EXIT_CODE);
    while(true) {}
}

fn get_length() usize {
    return 500;
}

export fn proc_main() void {
    const data = text;
    const a = data[get_length()];
    //@panic("Hello");
    cm.cm_process_kill(a);
}
