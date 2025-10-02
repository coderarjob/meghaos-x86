const syscall = @import("syscalls.zig");

const osif = @cImport({
    @cInclude("osif.h");
});
const cm = @cImport({
    @cInclude("syscall.h");
});

pub const WindowFrameBufferInfo = osif.OSIF_WindowFrameBufferInfo;
pub const Handle = c_int;

pub inline fn create(title: [*:0]const u8) !Handle {
    const handle = cm.cm_window_create(title);
    return if (handle != syscall.KERNEL_FAILURE) handle else syscall.KernelError;
}

pub inline fn destroy(handle: Handle) void {
    _ = cm.cm_window_destory(handle);
}

pub inline fn flush() void {
    cm.cm_window_flush_graphics();
}

pub inline fn frameBuffer(handle: Handle) !WindowFrameBufferInfo {
    var bi: WindowFrameBufferInfo= undefined;
    return if (cm.cm_window_getFB(handle, @ptrCast(&bi))) bi else syscall.KernelError;
}
