const std = @import("std");
const cm = @cImport({
    @cInclude("cm.h");
});
const osif = @cImport({
    @cInclude("osif.h");
});

pub const ProcessEvent = osif.OSIF_ProcessEvent;
pub const event_handler = *const fn (*const ProcessEvent) callconv(.c) void;
pub const abort_exit_code = cm.CM_ABORT_EXIT_CODE;

pub const ProcessEvents = enum(c_uint) {
    NONE = osif.OSIF_PROCESS_EVENT_NONE,
    PROCCESS_YIELD_REQ = osif.OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ,
    PROCCESS_CHILD_KILLED = osif.OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED,

    fn toC(self: @This()) c_uint {
        return @intFromEnum(self);
    }

    fn fromC(e: c_uint) @This() {
        return @enumFromInt(e);
    }
};

pub inline fn create_thread(start: *const fn () callconv(.c) void, isKernelMode: bool) c_int {
    return cm.cm_thread_create(start, isKernelMode);
}

pub inline fn create_process(filename: [*:0]const u8, isKernelMode: bool) c_int {
    return cm.cm_process_create(filename, isKernelMode);
}

pub inline fn yield() void {
    cm.cm_process_yield();
}

pub inline fn handle_events() bool {
    return cm.cm_process_handle_events();
}

pub inline fn exit(code: u16) noreturn {
    cm.cm_process_kill(code);
}

pub inline fn abort(code: u16) noreturn {
    cm.cm_process_abort(code);
}

pub inline fn register_event_handler(e: ProcessEvents, h: event_handler) bool {
    return cm.cm_process_register_event_handler(e.toC(), @ptrCast(h));
}
