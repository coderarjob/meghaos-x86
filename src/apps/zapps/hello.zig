const std = @import("std");
const cm = @cImport({
    @cInclude("cm.h");
});
pub const panic = std.debug.FullPanic(crash);

const text = "Test application";

fn crash(_: []const u8, _: ?usize) noreturn {
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
