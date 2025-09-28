pub const debug = @import("debug.zig");
pub const process = @import("process.zig");
pub const syscall = @import("syscalls.zig");
pub const window = @import("window.zig");
pub const graphics = @import("graphics.zig");

pub const MosApplicationPanic = @import("std").debug.FullPanic(crash);

fn crash(msg: []const u8, addr: ?usize) noreturn {
    debug.log(
        .ERROR,
        @src(),
        "crashed!! {s}. At location 0x{x}",
        .{ msg, addr orelse @returnAddress() },
    );
    process.abort(process.abort_exit_code);
}
