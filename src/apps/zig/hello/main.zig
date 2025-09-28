const std = @import("std");
const zm = @import("zm");

pub const panic = zm.MosApplicationPanic;

const ProcessEvent = zm.process.ProcessEvent;

var child_count: u32 = 0;

export fn proc_main() void {
    _ = zm.process.create_thread(&thread0, false);
    _ = zm.process.create_thread(&thread2, false);
    child_count = 2;

    _ = zm.process.register_event_handler(.PROCCESS_CHILD_KILLED, &child_exit_event_handler);

    wait_for_all_child_exit();
    zm.process.exit(3);
}

fn wait_for_all_child_exit() void {
    while (child_count > 0) {
        _ = zm.process.handle_events();
    }
}

fn child_exit_event_handler(_: *const ProcessEvent) callconv(.c) void {
    if (child_count > 0) {
        zm.debug.console_writeline("A child process exitted!!\n");
        child_count -= 1;
    }
}

fn thread0() callconv(.c) void {
    zm.debug.console_writeline("[thread0 - 0]: Hello from Zig!!\n");
    zm.process.yield();
    zm.debug.console_writeline("[thread0 - 1]: Hello from Zig!!\n");
    zm.process.yield();
    zm.debug.console_writeline("[thread0 - 2]: thread0 exiting!!\n");
    zm.process.exit(1);
}

fn thread2() callconv(.c) void {
    zm.debug.console_writeline("[thread0 - 0]: Hello from Zig!!\n");
    zm.process.yield();
    zm.debug.console_writeline("[thread0 - 1]: Hello from Zig!!\n");
    zm.process.yield();
    zm.debug.console_writeline("[thread0 - 2]: thread0 exiting!!\n");
    zm.process.exit(1);
}
