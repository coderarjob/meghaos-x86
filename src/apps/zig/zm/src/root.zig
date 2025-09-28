pub const debug = @import("debug.zig");
pub const process = @import("process.zig");
pub const syscall = @import("syscalls.zig");
pub const window = @import("window.zig");
pub const graphics = @import("graphics.zig");

pub const MosApplicationPanic = @import("std").debug.FullPanic(crash);

fn crash(_: []const u8, _: ?usize) noreturn {
    process.abort(process.abort_exit_code);
}
