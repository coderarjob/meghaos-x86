pub const debug = @import("debug.zig");
pub const process = @import("process.zig");
pub const MosApplicationPanic = @import("std").debug.FullPanic(crash);

fn crash(_: []const u8, _: ?usize) noreturn {
    process.abort(process.abort_exit_code);
}
