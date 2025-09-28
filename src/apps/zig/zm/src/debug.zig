const std = @import("std");
const config = @import("config");
const root = @import("root");

const cm = @cImport({
    @cInclude("syscall.h");
    @cInclude("debug.h");
});

pub const LogType = if (isDebugMode() and isPortE9Enabled())
    enum(c_uint) {
        INFO = cm.CM_DEBUG_LOG_TYPE_INFO,
        FUNC = cm.CM_DEBUG_LOG_TYPE_FUNC,
        ERROR = cm.CM_DEBUG_LOG_TYPE_ERROR,
        WARN = cm.CM_DEBUG_LOG_TYPE_WARN,

        fn to(self: @This()) c_uint {
            return @intFromEnum(self);
        }
    }
else
    enum { INFO, FUNC, ERROR, WARN }; // Dummy implementation for NDEBUG builds.

pub inline fn console_writeline(msg: [*:0]const u8) void {
    cm.cm_putstr(@constCast(msg));
}

pub inline fn isGraphicsMode() bool {
    return config.graphics_enabled == .ON;
}

pub inline fn isDebugMode() bool {
    return config.build_mode == .DEBUG;
}

pub inline fn isPortE9Enabled() bool {
    return config.port_e9_enabled == .ON;
}

pub fn log(
    log_type: LogType,
    src_loc: std.builtin.SourceLocation,
    comptime fmt: []const u8,
    args: anytype,
) void {
    if (isDebugMode() and isPortE9Enabled()) {
        var buffer: [100]u8 = undefined;
        const out = std.fmt.bufPrintZ(&buffer, fmt, args) catch "";
        cm.cm_debug_log_ndu(log_type.to(), src_loc.fn_name, src_loc.line, "%s", out.ptr);
    } else {}
}
