const std = @import("std");
const zm = @import("zm");

export fn proc_main() void {
    zm.debug.console_writeline("\n\n");
    for (0..9) |_| {
        zm.debug.console_writeline("Hello from Zig!!\n");
    }
    zm.process.exit(1);
}
